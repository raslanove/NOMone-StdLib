
//////////////////////////////////////////////////////
// Created by Omar El Sayyed on 7th of August 2021.
//////////////////////////////////////////////////////

#pragma once

#include <NTypes.h>

// To be used by backend life-cycle events only.
struct NSystem_Interface {
    void (*initialize)(void (*nMain)(int argc, char *argv[]), int argc, char *argv[]);  // Call once at the program beginning.
    void (*terminate)();   // Call once at the program end.
};

extern const struct NSystem_Interface NSystem;