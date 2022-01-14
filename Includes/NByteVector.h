
//////////////////////////////////////////////////////
// Created by Omar El Sayyed on 17th of July 2021.
//////////////////////////////////////////////////////

#pragma once

#include <NTypes.h>

struct NByteVector {
    // DON'T OVERWRITE. For use by the provided functions only.
    uint32_t capacity;
    uint32_t size;
    uint8_t* objects;
};

struct NByteVector_Interface {
    struct NByteVector* (*initialize)(struct NByteVector* outputVector, uint32_t initialCapacity);
    struct NByteVector* (*create)(uint32_t initialCapacity);
    void (*destroy)(struct NByteVector* vector);
    void (*destroyAndFree)(struct NByteVector* vector);
    struct NByteVector* (*clear)(struct NByteVector* vector);
    boolean (*pushBack)(struct NByteVector* vector, uint8_t value);  // True if successful.
    boolean (*popBack)(struct NByteVector* vector, uint8_t* output);   // True if successful.
    boolean (*pushBack32Bit)(struct NByteVector* vector, int32_t value);
    boolean (*popBack32Bit)(struct NByteVector* vector, int32_t *output);
    boolean (*pushBackBulk)(struct NByteVector* vector, void* data, uint32_t size);
    boolean (*popBackBulk)(struct NByteVector* vector, void* outputData, uint32_t size);
    uint8_t (*get)(struct NByteVector* vector, uint32_t index);
    boolean (*set)(struct NByteVector* vector, uint32_t index, uint8_t value);
    uint32_t (*size)(struct NByteVector* vector);
    boolean (*resize)(struct NByteVector* vector, uint32_t newSize);
};

extern const struct NByteVector_Interface NByteVector;
