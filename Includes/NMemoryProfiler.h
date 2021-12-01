
//////////////////////////////////////////////////////
// Created by Omar El Sayyed on 26th of November 2021.
//////////////////////////////////////////////////////

#pragma once

#include <NTypes.h>

void NMemoryProfiler_initialize();
void* NMemoryProfiler_malloc(int32_t size, const char* id);
void NMemoryProfiler_free(void* address, const char* id);
void NMemoryProfiler_logOnExitReport();
