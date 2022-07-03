
///////////////////////////////////////////////////////
// Created by Omar El Sayyed on 5th of August 2021.
///////////////////////////////////////////////////////

#pragma once

#include <NTypes.h>

struct NCString_Interface {
    int32_t (*length)(const char* string);
    boolean (*startsWith)(const char* string, const char* value);
    boolean (*endsWith)(const char* string, const char* value);
    boolean (*contains)(const char* string, const char* value);
    int32_t (*lastIndexOf)(const char* string, const char* value); // Returns last occurrence index, -1 if not found.
    boolean (*equals)(const char* string, const char* value);
    char* (*copy)(char* destination, const char* source); // Returns destination.
    char* (*clone)(const char* source);
    int32_t (*parseInteger)(const char* string);
    int64_t (*parse64BitInteger)(const char* string);
};

extern const struct NCString_Interface NCString;
