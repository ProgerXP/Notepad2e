#pragma once

struct TBase64Data
{
  unsigned char initialized;
  unsigned char dtable[256];
};
typedef struct TBase64Data Base64Data;

struct TEncodingData;
typedef struct TEncodingData EncodingData;
struct TRecodingAlgorithm;
typedef struct TRecodingAlgorithm RecodingAlgorithm;

BOOL Base64_IsValidSequence(EncodingData* pED, const int requiredChars);
LPVOID Base64_InitAlgorithmData(const BOOL isEncoding);
void Base64_ReleaseAlgorithmData(LPVOID pData);
BOOL Base64_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL Base64_EncodeTail(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL Base64_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

LPCSTR EncodeStringToBase64(LPCSTR text, const int textLength, const int encoding, const int additionalData, const int bufferSize, int* pResultLength);
LPCSTR DecodeBase64ToString(LPCSTR text, const int textLength, const int encoding, const int additionalData, const int bufferSize, int* pResultLength);
void EncodeStrToBase64(const HWND hwnd);
void DecodeBase64ToStr(const HWND hwnd);

#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus
