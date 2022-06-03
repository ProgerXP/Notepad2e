#pragma once

#include <WTypes.h>
#include "StringRecoding-fwd.h"

struct CALWData;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CALWData* HCALWData;

HCALWData CALW_Create(const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3);
void CALW_Free(HCALWData);
BOOL CALW_Run(HCALWData, RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
void CALW_InitPassImpl(HCALWData h, RecodingAlgorithm* pRA);
BOOL CALW_CanUseHWNDForReadingImpl(HCALWData h, const RecodingAlgorithm* pRA);
BOOL CALW_CanUseHWNDForWritingImpl(HCALWData h, const RecodingAlgorithm* pRA);

#ifdef __cplusplus
}; // extern "C"
#endif
