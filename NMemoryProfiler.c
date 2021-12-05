
//////////////////////////////////////////////////////
// Created by Omar El Sayyed on 26th of November 2021.
//////////////////////////////////////////////////////

#include <NMemoryProfiler.h>
#include <NVector.h>
#include <NSystemUtils.h>
#include <NCString.h>

#define EXPANSION_RATIO 0.75f   // Expansions takes place when the:
                                //   (allocated blocks count) / (total space for allocations)
                                // ratio is larger than this value.

static boolean profilingEnabled=True;
static int64_t mallocCallsCount=0, freeCallsCount=0;
static int64_t allocatedBlocksCount=0;
static int64_t currentlyUsedMemory=0;
static int64_t maxUsedMemory=0;
static uint32_t lastAllocationIndex;

struct AllocationData {
    char* id;
    void* pointer;
    int32_t size;
};
static struct NVector allocationDatas;

struct BundledAllocationData {
    uint32_t allocationIndex;
};

void NMemoryProfiler_initialize() {
    profilingEnabled = False;
    NVector.initialize(&allocationDatas, 1, sizeof(struct AllocationData));
    profilingEnabled = True;
}

//#include <stdio.h>
//int32_t expand=0, sparse=0;
static uint32_t getUnusedAllocationDataIndex() {

    uint32_t allocationDatasSize = NVector.size(&allocationDatas);
    if ((NPROFILE_MEMORY==2) ||
        (allocationDatasSize < allocationDatas.capacity) ||                        // If the already allocated vector has space for more, or
        (allocatedBlocksCount / (float) allocationDatasSize >= EXPANSION_RATIO)) { // if we've reached our expansion ratio, just expand.
        //printf("Expand: %d, allocatedBlocksCount: %d, haveCapacity: %s\n", ++expand, allocatedBlocksCount, (allocationDatasSize < allocationDatas.capacity) ? "True" : "False");
        NVector.emplaceBack(&allocationDatas);
        return allocationDatasSize;
    }

    // Ratio not met, the vector is sparse enough. Look for empty records,
    //int32_t iterationsCount=0;
    do {
        struct AllocationData* allocationData = NVector.get(&allocationDatas, lastAllocationIndex);
        if (!allocationData->pointer) {
            //printf("Sparse: %d, iterations: %d\n", ++sparse, iterationsCount);
            return lastAllocationIndex;
        }
        //iterationsCount++;
        lastAllocationIndex++;
        if (lastAllocationIndex==allocationDatasSize) lastAllocationIndex = 0;
    } while (True);
}

void* NMemoryProfiler_malloc(int32_t size, const char* id) {

    // Attempt allocating memory,
    void* pointer = NSystemUtils.malloc(size + (profilingEnabled ? sizeof(struct BundledAllocationData) : 0));
    if (!pointer) {
        NLOGE("NMemoryProfiler", "malloc failed. Id: %s%s%s", NTCOLOR(HIGHLIGHT), id, NTCOLOR(STREAM_DEFAULT));
        return 0;
    }

    // Don't proceed if profiling is disabled,
    if (!profilingEnabled) return pointer;

    // Turn off profiling,
    profilingEnabled = False;

    // Add allocation data,
    uint32_t allocationIndex = getUnusedAllocationDataIndex();
    struct AllocationData* allocationData = NVector.get(&allocationDatas, allocationIndex);
    allocationData->id = NCString.clone(id);
    allocationData->pointer = pointer + sizeof(struct BundledAllocationData);
    allocationData->size = size;

    // Set the bundled data,
    struct BundledAllocationData* bundledData = pointer;
    bundledData->allocationIndex = allocationIndex;

    // Update status,
    allocatedBlocksCount++;
    mallocCallsCount++;
    currentlyUsedMemory += size;
    if (currentlyUsedMemory > maxUsedMemory) maxUsedMemory = currentlyUsedMemory;

    // Re-enable profiling,
    profilingEnabled = True;

    return allocationData->pointer;
}

void NMemoryProfiler_free(void* address, const char* id) {

    if (!profilingEnabled) {
        NSystemUtils.free(address);
        return;
    }

    // Get the allocation data,
    struct BundledAllocationData* bundledData = address - sizeof(struct BundledAllocationData);
    struct AllocationData* allocationData = NVector.get(&allocationDatas, bundledData->allocationIndex);

    // If not found,
    if (!allocationData || (allocationData->pointer!=address)) {
        NLOGE("NMemoryProfiler", "free failed, attempted freeing an unallocated block. Id: %s%s%s", NTCOLOR(HIGHLIGHT), id, NTCOLOR(STREAM_DEFAULT));
        return ;
    }

    // Free the block,
    NSystemUtils.free(bundledData);
    allocatedBlocksCount--;
    currentlyUsedMemory -= allocationData->size;

    freeCallsCount++;

    #if NPROFILE_MEMORY==1
        // Default mode, track memory leaks,
        allocationData->pointer = 0;
        NSystemUtils.free(allocationData->id);
    #elif NPROFILE_MEMORY==2
        // Track all allocations mode. Nothing needs to be done here.
    #else
        // An unsupported value,
        #define STRINGIFY(x) #x
        #define TOSTRING(x) STRINGIFY(x)
        profilingEnabled = False;
        NLOGE("NMemoryProfiler", "%sNPROFILE_MEMORY%s supported values are %s0%s, %s1%s and %s2%s, Found %s" TOSTRING(NPROFILE_MEMORY) "%s\n", NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT));
    #endif
}

struct AllocationDataAggregation {
    char* id;
    int32_t count;
    int64_t totalSize;
};

void NMemoryProfiler_logOnExitReport() {

    // Create vector for the aggregated data,
    profilingEnabled = False;
    static struct NVector allocationDataAggregation;
    NVector.initialize(&allocationDataAggregation, 0, sizeof(struct AllocationDataAggregation));

    // Aggregate the results by id,
    for (int32_t i=NVector.size(&allocationDatas)-1; i>=0; i--) {

        // Process only unfreed allocations,
        struct AllocationData *allocation = NVector.get(&allocationDatas, i);
        if (!allocation->pointer) continue;

        // Insert into the correct bin,
        boolean found=False;
        for (int32_t j=NVector.size(&allocationDataAggregation)-1; j>=0; j--) {
            struct AllocationDataAggregation *aggregation = NVector.get(&allocationDataAggregation, j);
            if (NCString.equals(aggregation->id, allocation->id)) {
                aggregation->count++;
                aggregation->totalSize += allocation->size;
                found=True;
                break;
            }
        }

        // Bin not found, create a new one,
        if (!found) {
            struct AllocationDataAggregation *aggregation = NVector.emplaceBack(&allocationDataAggregation);
            aggregation->id = NCString.clone(allocation->id);
            aggregation->count = 1;
            aggregation->totalSize = allocation->size;
        }
    }

    // Log and free,
    if (mallocCallsCount != freeCallsCount) NLOGE("NMemoryProfiler", "mallocs != frees. mallocs: %s%ld%s, frees: %s%ld%s", NTCOLOR(HIGHLIGHT), mallocCallsCount, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), freeCallsCount, NTCOLOR(STREAM_DEFAULT));

    for (int32_t i=NVector.size(&allocationDataAggregation)-1; i>=0; i--) {
        struct AllocationDataAggregation *aggregation = NVector.get(&allocationDataAggregation, i);
        NLOGE("NMemoryProfiler", "  ID: %s%s%s, Count: %s%d%s, TotalSize: %s%ld%s", NTCOLOR(HIGHLIGHT), aggregation->id, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), aggregation->count, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), aggregation->totalSize, NTCOLOR(STREAM_DEFAULT));
        NSystemUtils.free(aggregation->id);
    }
    NVector.destroy(&allocationDataAggregation);

    NLOGE("NMemoryProfiler", "Total unfreed memory: %s%ld%s", NTCOLOR(HIGHLIGHT), currentlyUsedMemory, NTCOLOR(STREAM_DEFAULT));
    NLOGE("NMemoryProfiler", "Maximum memory used at any instance: %s%ld%s", NTCOLOR(HIGHLIGHT), maxUsedMemory, NTCOLOR(STREAM_DEFAULT));

    profilingEnabled = True;
}
