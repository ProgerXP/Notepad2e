#pragma once

struct TEncodingData;
typedef struct TEncodingData EncodingData;
struct TRecodingAlgorythm;
typedef struct TRecodingAlgorythm RecodingAlgorythm;

BOOL Hex_IsValidSequence(EncodingData* pED, const int requiredChars);
BOOL Hex_Encode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL Hex_Decode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed);

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

  LPCSTR EncodeStringToHex(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultSize);
  LPCSTR DecodeHexToString(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultSize);
  void EncodeStrToHex(const HWND hwnd);
  void DecodeHexToStr(const HWND hwnd);

#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus
