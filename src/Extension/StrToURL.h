#pragma once

struct TEncodingData;
typedef struct TEncodingData EncodingData;
struct TRecodingAlgorythm;
typedef struct TRecodingAlgorythm RecodingAlgorythm;

BOOL URL_IsValidSequence(EncodingData* pED, const int requiredChars);
BOOL URL_Encode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL URL_Decode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed);

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

  LPCSTR EncodeStringToURL(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultSize);
  LPCSTR DecodeURLToString(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultSize);
  void EncodeStrToURL(const HWND hwnd);
  void DecodeURLToStr(const HWND hwnd);

#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus
