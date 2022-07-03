
//////////////////////////////////////////////////////
// Created by Omar El Sayyed on 13th of March 2021.
//////////////////////////////////////////////////////

// Note: this is a generic (slow) implementation. A specialized implementation for 32bit integers
// is thus highly recommended. This can be achieved while maintaining high performance by creating
// a new set of functions and a different interface to handle the 32bit specialized case. For
// example, NByteVector and NVector32.

#pragma once

#include <NTypes.h>

struct NVector {
    // DON'T OVERWRITE. For use by the provided functions only.
    uint32_t capacity;
    uint32_t objectSize;
    uint32_t objectsCount;
    void* objects;
};

struct NVector_Interface {
    struct NVector* (*initialize)(struct NVector* outputVector, uint32_t initialCapacity, uint32_t objectSize);
    struct NVector* (*create)(uint32_t initialCapacity, uint32_t objectSize);
    void (*destroy)(struct NVector* vector);
    void (*destroyAndFree)(struct NVector* vector);
    struct NVector* (*clear)(struct NVector* vector);
    boolean (*grow)(struct NVector* vector, uint32_t newCapacity);
    void* (*emplaceBack)(struct NVector* vector);  // New structure pointer if successful, 0 otherwise.
    boolean (*pushBack)(struct NVector* vector, const void *object);  // True if successful.
    boolean (*popBack)(struct NVector* vector, void *outputObject);   // True if successful.
    void* (*get)(struct NVector* vector, uint32_t index);
    void* (*getLast)(struct NVector* vector);
    int32_t (*getFirstInstanceIndex)(struct NVector* vector, const void* object);
    void (*remove)(struct NVector* vector, int32_t index);
    uint32_t (*size)(struct NVector* vector);
    boolean (*resize)(struct NVector* vector, uint32_t newSize);
};

extern const struct NVector_Interface NVector;
