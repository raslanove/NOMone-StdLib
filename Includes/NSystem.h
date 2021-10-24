
//////////////////////////////////////////////////////
// Created by Omar El Sayyed on 7th of August 2021.
//////////////////////////////////////////////////////

#pragma once

#include <NTypes.h>

// To be used by backend life-cycle events only.
struct NSystem_Interface {
    void (*initialize)(void (*nMain)());  // Call once at the program beginning.
    void (*terminate)();   // Call once at the program end.
};

extern const struct NSystem_Interface NSystem;
extern int64_t NSystem_mallocCallsCount, NSystem_freeCallsCount;