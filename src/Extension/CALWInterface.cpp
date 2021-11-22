#include "CALWInterface.h"
#include "CALWImpl.h"
#include "StringRecoding.h"

extern "C" {

typedef CALWData* HCALWData;

HCALWData CALW_Create(const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3)
{
  return new CALWData(iAdditionalData1, iAdditionalData2, iAdditionalData3);
}

void CALW_Free(HCALWData h)
{
  delete h;
}

BOOL CALW_Run(HCALWData h, RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  switch (pRA->iPassIndex)
  {
  case 0:
    return h->RunPass0(pRA, pED, piCharsProcessed);
  case 1:
    return h->RunPass1(pRA, pED, piCharsProcessed);
  case 2:
    return h->RunPass2(pRA, pED, piCharsProcessed);
  default:
    return FALSE;
  }
}

}; // extern "C"
