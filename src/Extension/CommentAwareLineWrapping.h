#pragma once
#include "StringRecoding-fwd.h"

BOOL CALW_IsValidSequence(EncodingData* pED, const int requiredChars);
LPVOID CALW_InitAlgorithmData(const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3);
void CALW_ReleaseAlgorithmData(LPVOID pData);
BOOL CALW_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL CALW_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

  LPCSTR EncodeStringWithCALW(LPCSTR text, const int textLength, const int encoding,
    const int additionalData1, const int additionalData2, const int additionalData3, const int bufferSize, int* pResultSize);
  void EncodeStrWithCALW(const HWND hwnd);
  
#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus
