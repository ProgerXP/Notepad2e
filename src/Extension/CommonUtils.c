#include "CommonUtils.h"
#include "Trace.h"

INT iAllocCount = 0;

void* n2e_Alloc(size_t size)
{
  if (iAllocCount)
  {
    N2E_TRACE(L"WARNING !!! ALLOC mismatch : %d", iAllocCount);
  }
  ++iAllocCount;
  return GlobalAlloc(GPTR, sizeof(WCHAR) * (size + 1));
}

void n2e_Free(void* ptr)
{
  if (ptr)
  {
    --iAllocCount;
    GlobalFree(ptr);
  }
}

void* n2e_Realloc(void* ptr, size_t len)
{
  n2e_Free(ptr);
  return n2e_Alloc(len);
}
