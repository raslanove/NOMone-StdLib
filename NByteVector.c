#include <NByteVector.h>
#include <NSystemUtils.h>
#include <NError.h>

#define NBYTEVECTOR_BOUNDARY_CHECK 1

static struct NByteVector* initialize(struct NByteVector* outputVector, uint32_t initialCapacity) {

    NSystemUtils.memset(outputVector, 0, sizeof(struct NByteVector));

    if ((initialCapacity>0) && (initialCapacity<4)) initialCapacity = 4; // Make sure that expands can accommodate a 32bit push.

    outputVector->capacity = initialCapacity;
    outputVector->size = 0;

    if (initialCapacity > 0) outputVector->objects = NMALLOC(initialCapacity, "NByteVector.initialize() outputVector->objects");

    return outputVector;
}

static struct NByteVector* create(uint32_t initialCapacity) {
    struct NByteVector* vector = NMALLOC(sizeof(struct NByteVector), "NByteVector.create() vector");
    return initialize(vector, initialCapacity);
}

static void destroy(struct NByteVector* vector) {
    if (vector->objects) NFREE(vector->objects, "NByteVector.destroy() vector->objects");
    NSystemUtils.memset(vector, 0, sizeof(struct NByteVector));
}

static void destroyAndFree(struct NByteVector* vector) {
    destroy(vector);
    NFREE(vector, "NByteVector.destroyAndFree() vector");
}

static struct NByteVector* clear(struct NByteVector* vector) {
    vector->size = 0;
    return vector;
}

static boolean grow(struct NByteVector* vector, uint32_t newCapacity) {

    if (newCapacity <= vector->capacity) return True;

    void *newArray = NMALLOC(newCapacity, "NByteVector.grow() newArray");
    if (!newArray) return False;

    if (vector->objects) {
        NSystemUtils.memcpy(newArray, vector->objects, vector->size);
        NFREE(vector->objects, "NByteVector.grow() vector->objects");
    }

    vector->objects = newArray;
    vector->capacity = newCapacity;

    return True;
}

static boolean expand(struct NByteVector* vector) {
    if (vector->capacity == 0) {
        vector->objects = NMALLOC(4, "NByteVector.expand() vector->objects");    // It's a waste to allocate less than 1 word, this also makes
                                                                                 // sure we can push 32bits after a single expansion.
        if (!vector->objects) return False;
        vector->capacity = 4;
        return True;
    }

    return grow(vector, vector->capacity<<1u);
}

static boolean pushBack(struct NByteVector* vector, uint8_t value) {

    // Double the vector capacity if needed,
    if ((vector->size == vector->capacity) && !expand(vector)) return False;

    // Push the value,
    vector->objects[vector->size++] = value;
    return True;
}

static boolean popBack(struct NByteVector* vector, uint8_t *output) {
    if (vector->size == 0) return False;

    *output = vector->objects[--(vector->size)];
    return True;
}

static boolean pushBack32Bit(struct NByteVector* vector, int32_t value) {

    // Double the vector capacity if needed,
    if ((vector->size + 4 > vector->capacity) && !expand(vector)) return False;

    // Push the value,
    *((int32_t *) &vector->objects[vector->size]) = value;
    vector->size += 4;
    return True;
}

static boolean popBack32Bit(struct NByteVector* vector, int32_t *output) {
    if (vector->size < 4) return False;

    vector->size -= 4;
    *output = *((int32_t *) &vector->objects[vector->size]);
    return True;
}

static boolean pushBackBulk(struct NByteVector* vector, void* data, uint32_t size) {

    // Double the vector capacity if needed,
    uint32_t newSize = vector->size + size;
    if (newSize > vector->capacity) {

        // Check if an expansion would do,
        if (newSize <= (vector->capacity<<1u)) {
            if (!expand(vector)) return False;
        } else {
            // Grow to the required size directly,
            if (!grow(vector, newSize)) return False;
        }
    }

    // Push the block,
    NSystemUtils.memcpy(&vector->objects[vector->size], data, size);
    vector->size = newSize;
    return True;
}

static boolean popBackBulk(struct NByteVector* vector, void* outputData, uint32_t size) {

    if (vector->size < size) return False;

    vector->size -= size;
    NSystemUtils.memcpy(outputData, &vector->objects[vector->size], size);
    return True;
}

static uint8_t get(struct NByteVector* vector, uint32_t index) {
#if NBYTEVECTOR_BOUNDARY_CHECK
    if (index >= vector->size) {
        NERROR("NByteVector.get()", "Index out of bound: %d", index);
        return 0;
    }
#endif

    return vector->objects[index];
}

static boolean set(struct NByteVector* vector, uint32_t index, uint8_t value) {
#if NBYTEVECTOR_BOUNDARY_CHECK
    if (index >= vector->size) {
        NERROR("NByteVector.get()", "Index out of bound: %d", index);
        return False;
    }
#endif

    vector->objects[index] = value;
    return True;
}

static uint32_t size(struct NByteVector* vector) {
    return vector->size;
}

static boolean resize(struct NByteVector* vector, uint32_t newSize) {
    if (newSize > vector->capacity && !grow(vector, newSize)) return False;
    vector->size = newSize;
    return True;
}

const struct NByteVector_Interface NByteVector = {
    .initialize = initialize,
    .create = create,
    .destroy = destroy,
    .destroyAndFree = destroyAndFree,
    .clear = clear,
    .pushBack = pushBack,
    .popBack = popBack,
    .pushBack32Bit = pushBack32Bit,
    .popBack32Bit = popBack32Bit,
    .pushBackBulk = pushBackBulk,
    .popBackBulk = popBackBulk,
    .get = get,
    .set = set,
    .size = size,
    .resize = resize
};
