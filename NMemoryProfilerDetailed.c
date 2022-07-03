
/////////////////////////////////////////////////////////
// Created by Omar El Sayyed on the 1st of January 2022.
/////////////////////////////////////////////////////////

//#define NPROFILE_MEMORY 3

#if NPROFILE_MEMORY == 3

// This mode isn't meant for detecting leaks and such. It's for analyzing your app's memory
// usage performance.

#include <NMemoryProfiler.h>
#include <NVector.h>
#include <NSystemUtils.h>
#include <NCString.h>

static boolean profilingEnabled=True;
static int64_t currentlyUsedMemory=0;
static int64_t maxUsedMemory=0;

struct AllocationData {
    char* id;
    uint64_t totalSize, maxTotalSize; // Size of simultaneously allocated data with this id.
    uint32_t allocationsCount;
};
static struct NVector allocationDatas;

struct AllocationBundledData {
    struct AllocationData* allocationData;
    uint32_t size;
};

void NMemoryProfiler_initialize() {
    profilingEnabled = False;
    NVector.initialize(&allocationDatas, 1, sizeof(struct AllocationData*));
    profilingEnabled = True;
}

static struct AllocationData* getAllocationData(const char* id) {

    // TODO: use a binary search approach (based on id) later...
    struct AllocationData* allocationData;
    boolean found=False;
    for (int32_t i=NVector.size(&allocationDatas)-1; i>=0; i--) {
        allocationData = *((struct AllocationData**) NVector.get(&allocationDatas, i));
        if (NCString.equals(allocationData->id, id)) {
            found = True;
            break;
        }
    }

    if (!found) {
        // Create a new allocation data for this id,
        allocationData = NSystemUtils.malloc(sizeof(struct AllocationData));
        allocationData->id = NCString.clone(id);
        allocationData->allocationsCount = 0;
        allocationData->totalSize = 0;
        allocationData->maxTotalSize = 0;

        NVector.pushBack(&allocationDatas, &allocationData);
    }

    return allocationData;
}

void* NMemoryProfiler_malloc(int32_t size, const char* id) {

    // Attempt allocating memory,
    void* pointer = NSystemUtils.malloc(size + (profilingEnabled ? sizeof(struct AllocationBundledData) : 0));
    if (!pointer) {
        NLOGE("NMemoryProfilerDetailed", "malloc failed. Id: %s%s%s", NTCOLOR(HIGHLIGHT), id, NTCOLOR(STREAM_DEFAULT));
        return 0;
    }

    // Don't proceed if profiling is disabled,
    if (!profilingEnabled) return pointer;

    // Turn off profiling,
    profilingEnabled = False;

    // Add allocation data,
    struct AllocationData* allocationData = getAllocationData(id);
    allocationData->allocationsCount++;
    allocationData->totalSize += size;
    if (allocationData->totalSize > allocationData->maxTotalSize) allocationData->maxTotalSize = allocationData->totalSize;

    // Set the bundled data,
    struct AllocationBundledData* bundledData = pointer;
    bundledData->allocationData = allocationData;
    bundledData->size = size;

    // Update status,
    currentlyUsedMemory += size;
    if (currentlyUsedMemory > maxUsedMemory) maxUsedMemory = currentlyUsedMemory;

    // Re-enable profiling,
    profilingEnabled = True;

    return pointer + sizeof(struct AllocationBundledData);
}

void NMemoryProfiler_free(void* address, const char* id) {

    if (!profilingEnabled) {
        NSystemUtils.free(address);
        return;
    }

    // Get the allocation data,
    struct AllocationBundledData* bundledData = address - sizeof(struct AllocationBundledData);
    struct AllocationData* allocationData = bundledData->allocationData;

    // Try our best to detect any issues with this pointer (but we can never be sure in this mode),
    if ((!allocationData) ||
        (allocationData->allocationsCount<0) ||
        (allocationData->totalSize < bundledData->size) ||
        (allocationData->maxTotalSize < allocationData->totalSize)) {
        profilingEnabled = False;
        NLOGE("NMemoryProfilerDetailed", "free failed, attempted freeing a block with corrupted bundled data. Id: %s%s%s", NTCOLOR(HIGHLIGHT), id, NTCOLOR(STREAM_DEFAULT));
        NLOGE("sdf", "%d %d %d %d", allocationData->allocationsCount, allocationData->totalSize, allocationData->maxTotalSize, bundledData->size);
        profilingEnabled = True;
        return ;
    }

    // Free the block,
    currentlyUsedMemory -= bundledData->size;
    allocationData->totalSize -= bundledData->size;;
    allocationData->allocationsCount--;
    NSystemUtils.free(bundledData);
}

void NMemoryProfiler_logOnExitReport() {

    profilingEnabled = False;

    // Log and free,
    for (int32_t i=NVector.size(&allocationDatas)-1; i>=0; i--) {
        struct AllocationData* allocationData = *((struct AllocationData**) NVector.get(&allocationDatas, i));
        NLOGI("NMemoryProfilerDetailed", "  ID: %s%s%s, Max simultaneously used memory: %s%ld%s", NTCOLOR(HIGHLIGHT), allocationData->id, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), allocationData->maxTotalSize, NTCOLOR(STREAM_DEFAULT));
        if (allocationData->allocationsCount || allocationData->totalSize) {
            NLOGE("NMemoryProfilerDetailed", "  %sLEAK DETECTED!%s Unfreed instances count: %s%d%s, Unfreed memory size: %s%ld%s", NTCOLOR(ERROR_STRONG), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), allocationData->allocationsCount, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), allocationData->totalSize, NTCOLOR(STREAM_DEFAULT));
        }
        NSystemUtils.free(allocationData->id);
        NSystemUtils.free(allocationData);
    }
    NVector.destroy(&allocationDatas);

    if (currentlyUsedMemory) NLOGE("NMemoryProfilerDetailed", "Total unfreed memory: %s%ld%s", NTCOLOR(HIGHLIGHT), currentlyUsedMemory, NTCOLOR(STREAM_DEFAULT));
    NLOGI("NMemoryProfilerDetailed", "Maximum memory used at any instance: %s%ld%s", NTCOLOR(HIGHLIGHT), maxUsedMemory, NTCOLOR(STREAM_DEFAULT));
}

#endif