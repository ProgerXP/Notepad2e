#pragma once
#include <wtypes.h>

void* n2e_Alloc(size_t size);
void n2e_Free(void* ptr);
void* n2e_Realloc(void* ptr, size_t len);
