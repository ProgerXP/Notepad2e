#pragma once
#include "StringRecoding-fwd.h"

BOOL URL_IsValidSequence(EncodingData* pED, const int requiredChars);
BOOL URL_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL URL_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

  LPCSTR EncodeStringToURL(LPCSTR text, const int textLength, const int encoding,
    const int additionalData1, const int additionalData2, const int bufferSize, int* pResultSize);
  LPCSTR DecodeURLToString(LPCSTR text, const int textLength, const int encoding,
    const int additionalData1, const int additionalData2, const int bufferSize, int* pResultSize);
  void EncodeStrToURL(const HWND hwnd);
  void DecodeURLToStr(const HWND hwnd);

#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus
