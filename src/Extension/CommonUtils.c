#include "CommonUtils.h"
#include "Trace.h"

INT iAllocCount = 0;

LPVOID n2e_Alloc(size_t size)
{
  ++iAllocCount;
  return malloc(size);
}

void n2e_Free(LPVOID ptr)
{ 
  --iAllocCount;
  free(ptr);
}

LPVOID n2e_Realloc(LPVOID ptr, size_t size)
{
  LPVOID res = realloc(ptr, size);
  if (!res && ptr)
  {
    n2e_Free(ptr);
  }
  return res;
}
