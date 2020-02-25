#pragma once
#include <wtypes.h>

LPVOID n2e_Alloc(size_t size);
void n2e_Free(LPVOID ptr);
LPVOID n2e_Realloc(LPVOID ptr, size_t size);
