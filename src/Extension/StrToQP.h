#pragma once

struct TEncodingData;
typedef struct TEncodingData EncodingData;
struct TRecodingAlgorythm;
typedef struct TRecodingAlgorythm RecodingAlgorythm;

int QP_GetExpectedResultLength(const BOOL isEncoding, const int originalLength);
BOOL QP_IsValidSequence(EncodingData* pED, const int requiredChars);
LPVOID QP_InitAlgorythmData(const BOOL isEncoding);
void QP_ReleaseAlgorythmData(LPVOID pData);
BOOL QP_Encode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL QP_Decode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed);

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

  LPCSTR EncodeStringToQP(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultLength);
  LPCSTR DecodeQPToString(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultLength);
  void EncodeStrToQP(const HWND hwnd);
  void DecodeQPToStr(const HWND hwnd);

#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus
