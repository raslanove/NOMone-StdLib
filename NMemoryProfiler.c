
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
struct AllocationData {
    char* id;
    void* pointer;
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

    // Turn off profiling,
    if (!profilingEnabled) return pointer;
    profilingEnabled = False;

    // Add allocation data,
    struct AllocationData* allocationData = NVector.emplaceBack(&allocationDatas);
    allocationData->pointer = pointer;

    // Id,
    allocationData->id = NCString.clone(id);

    allocatedBlocksCount++;
    mallocCallsCount++;
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
    if (!allocationData) {
        NLOGE("NMemoryProfiler", "free failed, attempted freeing an unallocated block. Id: %s%s%s", NTCOLOR(HIGHLIGHT), id, NTCOLOR(STREAM_DEFAULT));
        return ;
    }

    // Free the block,
    NSystemUtils.free(address);
    allocationData->pointer = 0;
    allocatedBlocksCount--;

    // Compact allocation data if needed,
    if (NVector.size(&allocationDatas)-allocatedBlocksCount >= COMPACTION_THRESHOLD) compactAllocationsVector();

    freeCallsCount++;
}

static void compactAllocationsVector() {

    // TODO: eradicate the need for compaction...

    NLOGE("sdf", "COMPACTING!");
    int32_t vectorSize = NVector.size(&allocationDatas);
    int32_t destinationIndex=0;
    for (int32_t sourceIndex=0; sourceIndex<vectorSize; sourceIndex++) {
        struct AllocationData* source = NVector.get(&allocationDatas, sourceIndex);
        if (source->pointer && (sourceIndex!=destinationIndex)) {
            struct AllocationData* destination = NVector.get(&allocationDatas, destinationIndex);
            *destination = *source;
            destinationIndex++;
        }
    }
    NVector.resize(&allocationDatas, allocatedBlocksCount);
}

struct AllocationDataAggregation {
    char* id;
    int32_t count;
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
                found=True;
                break;
            }
        }

        // Bin not found, create a new one,
        if (!found) {
            struct AllocationDataAggregation *aggregation = NVector.emplaceBack(&allocationDataAggregation);
            aggregation->id = NCString.clone(allocation->id);
            aggregation->count = 1;
        }
    }

    // Log and free,
    if (mallocCallsCount != freeCallsCount) NLOGE("NMemoryProfiler", "mallocs != frees. mallocs: %s%ld%s, frees: %s%ld%s", NTCOLOR(HIGHLIGHT), mallocCallsCount, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), freeCallsCount, NTCOLOR(STREAM_DEFAULT));

    for (int32_t i=NVector.size(&allocationDataAggregation)-1; i>=0; i--) {
        struct AllocationDataAggregation *aggregation = NVector.get(&allocationDataAggregation, i);
        NLOGE("NMemoryProfiler", "  ID: %s%s%s, Count: %s%d%s", NTCOLOR(HIGHLIGHT), aggregation->id, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), aggregation->count, NTCOLOR(STREAM_DEFAULT));
        NSystemUtils.free(aggregation->id);
    }
    NVector.destroy(&allocationDataAggregation);

    profilingEnabled = True;
}