#pragma once
#include "StringRecoding-fwd.h"

BOOL CALW_IsValidSequence(EncodingData* pED, const int requiredChars);
LPVOID CALW_InitAlgorithmData(const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3);
void CALW_ReleaseAlgorithmData(LPVOID pData);
BOOL CALW_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
BOOL CALW_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed);
void CALW_InitPass(RecodingAlgorithm* pRA, const int iPassIndex);

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

  LPCSTR EncodeStringWithCALW(LPCSTR text, const int textLength, const int encoding,
    const int additionalData1, const int additionalData2, const int additionalData3, const int bufferSize, int* pResultSize);
  void EncodeStrWithCALW(const HWND hwnd);

  struct TPrefixData
  {
    char data[MAX_PATH];
    BOOL isInitialized;
    BOOL isComment;
    BOOL isEmptyLineComment;
  };
  typedef struct TPrefixData PrefixData;

  void PrefixData_SetEmpty(PrefixData* pd);
  BOOL PrefixData_IsInitialized(PrefixData* pd);
  void PrefixData_SetInitialized(PrefixData* pd, const BOOL isInitialized);
  BOOL PrefixData_IsComment(PrefixData* pd);
  BOOL PrefixData_IsEmptyLineComment(PrefixData* pd);
  void PrefixData_SetComment(PrefixData* pd, const BOOL isComment, const BOOL isEmptyLineComment);
  int PrefixData_GetLength(PrefixData* pd);
  BOOL PrefixData_IsEmpty(PrefixData* pd);
  void PrefixData_PushChar(PrefixData* pd, const unsigned char ch);
  const unsigned char PrefixData_GetChar(PrefixData* pd, const int pos);
  const int PrefixData_CountTrailingWhiteSpaces(PrefixData* pd);


#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus
