#pragma once
#include "StringRecoding-fwd.h"

BOOL QP_IsValidSequence(EncodingData* pED, const int requiredChars);
LPVOID QP_InitAlgorithmData(const BOOL isEncoding);
void QP_ReleaseAlgorithmData(LPVOID pData);
BOOL QP_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL QP_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

  LPCSTR EncodeStringToQP(LPCSTR text, const int textLength, const int encoding,
    const int additionalData1, const int additionalData2, const int additionalData3, const int bufferSize, int* pResultLength);
  LPCSTR DecodeQPToString(LPCSTR text, const int textLength, const int encoding,
    const int additionalData1, const int additionalData2, const int additionalData3, const int bufferSize, int* pResultLength);
  void EncodeStrToQP(const HWND hwnd);
  void DecodeQPToStr(const HWND hwnd);

#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus
