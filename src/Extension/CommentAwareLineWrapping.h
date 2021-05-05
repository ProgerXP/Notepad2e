#pragma once

struct TEncodingData;
typedef struct TEncodingData EncodingData;
struct TRecodingAlgorithm;
typedef struct TRecodingAlgorithm RecodingAlgorithm;

BOOL CALW_IsValidSequence(EncodingData* pED, const int requiredChars);
LPVOID CALW_InitAlgorithmData(const int iAdditionalData);
void CALW_ReleaseAlgorithmData(LPVOID pData);
BOOL CALW_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL CALW_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

  LPCSTR EncodeStringWithCALW(LPCSTR text, const int textLength, const int encoding, const int additionalData, const int bufferSize, int* pResultSize);
  void EncodeStrWithCALW(const HWND hwnd);
  
#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus
