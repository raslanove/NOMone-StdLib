#include <NVector.h>
#include <NSystemUtils.h>

#define NVECTOR_BOUNDARY_CHECK 1

static struct NVector* initialize(struct NVector* outputVector, uint32_t initialCapacity, uint32_t objectSize) {

    if (objectSize==0) return 0;

    NSystemUtils.memset(outputVector, 0, sizeof(struct NVector));

    outputVector->capacity = initialCapacity;
    outputVector->objectSize = objectSize;
    outputVector->objectsCount = 0;

    if (initialCapacity > 0) outputVector->objects = NMALLOC(initialCapacity * objectSize, "NVector.initialize() outputVector->objects");

    return outputVector;
}

static struct NVector* create(uint32_t initialCapacity, uint32_t objectSize) {

    if (objectSize==0) return 0;

    struct NVector* newVector = NMALLOC(sizeof(struct NVector), "NVector.create() newVector");
    return initialize(newVector, initialCapacity, objectSize);
}

static struct NVector* initializeFrom(struct NVector* outputVector, struct NVector* vectorToCopy) {

    // Clone the vector,
    *outputVector = *vectorToCopy;

    //  Allocate separate storage (if needed),
    if (vectorToCopy->capacity) {
        int32_t vectorSizeBytes = vectorToCopy->capacity * vectorToCopy->objectSize;
        outputVector->objects = NMALLOC(vectorSizeBytes, "NVector.initializeFrom() outputVector->objects");

        // Copy data,
        NSystemUtils.memcpy(outputVector->objects, vectorToCopy->objects, vectorSizeBytes);
    }

    return outputVector;
}

static void destroy(struct NVector* vector) {
    if (vector->objects) NFREE(vector->objects, "NVector.destroy() vector->objects");
    NSystemUtils.memset(vector, 0, sizeof(struct NVector));
}

static void destroyAndFree(struct NVector* vector) {
    destroy(vector);
    NFREE(vector, "NVector.destroyAndFree() vector");
}

static struct NVector* clear(struct NVector* vector) {
    vector->objectsCount = 0;
    return vector;
}

static boolean grow(struct NVector* vector, uint32_t newCapacity) {

    if (newCapacity <= vector->capacity) return True;

    uint32_t newSizeBytes = newCapacity * vector->objectSize;
    void *newArray = NMALLOC(newSizeBytes, "NVector.grow() newArray");
    if (!newArray) return False;

    if (vector->objects) {
        uint32_t originalSizeBytes = vector->objectsCount * vector->objectSize;
        NSystemUtils.memcpy(newArray, vector->objects, originalSizeBytes);
        NFREE(vector->objects, "NVector.grow() vector->objects");
    }

    vector->objects = newArray;
    vector->capacity = newCapacity;

    return True;
}

static boolean expand(struct NVector* vector) {
    if (vector->capacity == 0) {
        vector->objects = NMALLOC(vector->objectSize, "NVector.expand() vector->objects");
        if (!vector->objects) return False;
        vector->capacity = 1;
        return True;
    }
    return grow(vector, vector->capacity<<1);
}

static void* emplaceBack(struct NVector* vector) {

    // Double the vector capacity if needed,
    if ((vector->objectsCount == vector->capacity) && !expand(vector)) return 0;

    // Make way for a new entry,
    void *newObjectPointer = (void *)(((intptr_t) vector->objects) + (vector->objectsCount * vector->objectSize));
    vector->objectsCount++;

    return newObjectPointer;
}

static boolean pushBack(struct NVector* vector, const void *object) {

    // Double the vector capacity if needed,
    if ((vector->objectsCount == vector->capacity) && !expand(vector)) return False;

    // Push the value,
    void *newObjectPointer = (void *)(((intptr_t) vector->objects) + (vector->objectsCount * vector->objectSize));
    NSystemUtils.memcpy(newObjectPointer, object, vector->objectSize);
    vector->objectsCount++;

    return True;
}

static boolean popBack(struct NVector* vector, void *outputObject) {
    if (vector->objectsCount==0) return False;

    vector->objectsCount--;
    void *lastObjectPointer = (void *)(((intptr_t) vector->objects) + (vector->objectsCount * vector->objectSize));
    NSystemUtils.memcpy(outputObject, lastObjectPointer, vector->objectSize);

    return True;
}

static void* get(struct NVector* vector, uint32_t index) {
    #if NVECTOR_BOUNDARY_CHECK
    if (index >= vector->objectsCount) return 0;
    #endif

    return (void *)(((intptr_t) vector->objects) + (index                    * vector->objectSize));
}

static void* getLast(struct NVector* vector) {
    if (!vector->objectsCount) return 0;
    return (void *)(((intptr_t) vector->objects) + ((vector->objectsCount-1) * vector->objectSize));
}

static int32_t getFirstInstanceIndex(struct NVector* vector, const void* object) {
    intptr_t currentLocation = (intptr_t) vector->objects;
    for (int32_t i=0; i<vector->objectsCount; i++) {
        if (!NSystemUtils.memcmp(object, (void*) currentLocation, vector->objectSize)) return i;
        currentLocation += vector->objectSize;
    }
    return -1;
}

static void remove(struct NVector* vector, int32_t index) {
    #if NVECTOR_BOUNDARY_CHECK
    if (index < 0 | index >= vector->objectsCount) return;
    #endif

    vector->objectsCount--;
    intptr_t dest = ((intptr_t) vector->objects) + (index*vector->objectSize);
    NSystemUtils.memmove((void*) dest, (void*)(dest + vector->objectSize), (vector->objectsCount - index)*vector->objectSize);
}

static uint32_t size(struct NVector* vector) {
    return vector->objectsCount;
}

static boolean resize(struct NVector* vector, uint32_t newSize) {
    if (newSize > vector->capacity && !grow(vector, newSize)) return False;
    vector->objectsCount = newSize;
    return True;
}

const struct NVector_Interface NVector = {
    .initialize = initialize,
    .create = create,
    .initializeFrom = initializeFrom,
    .destroy = destroy,
    .destroyAndFree = destroyAndFree,
    .clear = clear,
    .grow = grow,
    .emplaceBack = emplaceBack,
    .pushBack = pushBack,
    .popBack = popBack,
    .get = get,
    .getLast = getLast,
    .getFirstInstanceIndex = getFirstInstanceIndex,
    .remove = remove,
    .size = size,
    .resize = resize
};
