
//////////////////////////////////////////////////////
// Created by Omar El Sayyed on 26th of November 2021.
//////////////////////////////////////////////////////

#include <NMemoryProfiler.h>
#include <NVector.h>
#include <NSystemUtils.h>
#include <NCString.h>

#define COMPACTION_THRESHOLD 100
static void compactAllocationsVector();

static boolean profilingEnabled=True;
static int64_t mallocCallsCount=0, freeCallsCount=0;
static int64_t allocatedBlocksCount=0;
static int64_t currentlyUsedMemory=0;
static int64_t maxUsedMemory=0;

struct AllocationData {
    char* id;
    void* pointer;
    int32_t size;
};
static struct NVector allocationDatas;

void NMemoryProfiler_initialize() {
    NVector.initialize(&allocationDatas, 0, sizeof(struct AllocationData));
}

void* NMemoryProfiler_malloc(int32_t size, const char* id) {

    // TODO: allocate extra 4 bytes for the allocation index...

    // Attempt allocating memory,
    void* pointer = NSystemUtils.malloc(size);
    if (!pointer) {
        NLOGE("NMemoryProfiler", "malloc failed. Id: %s%s%s", NTCOLOR(HIGHLIGHT), id, NTCOLOR(STREAM_DEFAULT));
        return 0;
    }

    // Don't proceed if profiling is disabled,
    if (!profilingEnabled) return pointer;

    // Turn off profiling,
    profilingEnabled = False;

    // Add allocation data,
    struct AllocationData* allocationData = NVector.emplaceBack(&allocationDatas);
    allocationData->id = NCString.clone(id);
    allocationData->pointer = pointer;
    allocationData->size = size;

    allocatedBlocksCount++;
    mallocCallsCount++;
    currentlyUsedMemory += size;
    if (currentlyUsedMemory > maxUsedMemory) maxUsedMemory = currentlyUsedMemory;

    // Re-enable profiling,
    profilingEnabled = True;

    return pointer;
}

void NMemoryProfiler_free(void* address, const char* id) {

    // TODO: maintain an expanding circular array of indices...

    if (!profilingEnabled) {
        NSystemUtils.free(address);
        return;
    }

    // Find the allocation data,
    struct AllocationData* allocationData=0;
    for (int32_t index=NVector.size(&allocationDatas)-1; index>=0; index--) {
        allocationData = NVector.get(&allocationDatas, index);
        if (address==allocationData->pointer) break;
    }

    // If not found,
    if (!allocationData || (allocationData->pointer!=address)) {
        NLOGE("NMemoryProfiler", "free failed, attempted freeing an unallocated block. Id: %s%s%s", NTCOLOR(HIGHLIGHT), id, NTCOLOR(STREAM_DEFAULT));
        return ;
    }

    // Free the block,
    NSystemUtils.free(address);
    allocatedBlocksCount--;
    currentlyUsedMemory -= allocationData->size;

    freeCallsCount++;

    #if NPROFILE_MEMORY==1
        // Default mode, track memory leaks,
        allocationData->pointer = 0;
        NSystemUtils.free(allocationData->id);

        // Compact allocation data if needed,
        if (NVector.size(&allocationDatas)-allocatedBlocksCount >= COMPACTION_THRESHOLD) compactAllocationsVector();
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

static void compactAllocationsVector() {

    // TODO: eradicate the need for compaction...

    profilingEnabled = False;
    int32_t vectorSize = NVector.size(&allocationDatas);
    int32_t destinationIndex=0;
    for (int32_t sourceIndex=0; sourceIndex<vectorSize; sourceIndex++) {
        struct AllocationData* source = NVector.get(&allocationDatas, sourceIndex);
        if (source->pointer) {
            if (sourceIndex!=destinationIndex) {
                struct AllocationData* destination = NVector.get(&allocationDatas, destinationIndex);
                *destination = *source;
            }
            destinationIndex++;
        }
    }
    NVector.resize(&allocationDatas, allocatedBlocksCount);
    profilingEnabled = True;
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
