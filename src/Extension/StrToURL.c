#include <wtypes.h>
#include "Externals.h"
#include "StrToURL.h"
#include "StringRecoding.h"

BOOL CheckRequiredHexDigitsAvailable(LPCSTR pCurTextOrigin, LPCSTR pCurText, const long iCurTextLength, const long nChars);

BOOL URL_IsValidSequence(EncodingData* pED, const int requiredChars)
{
  LPCSTR pCurText = pED->m_tb.m_ptr + pED->m_tb.m_iPos;
  return (pCurText[0] != '%') || CheckRequiredHexDigitsAvailable(pED->m_tb.m_ptr, pCurText + 1, pED->m_tb.m_iMaxPos, 2);
}

BOOL IsURLChar(TextBuffer* pTB, const unsigned char ch)
{
  return !(((ch >= 'A') && (ch <= 'Z'))
    || ((ch >= 'a') && (ch <= 'z'))
    || ((ch >= '0') && (ch <= '9'))
    || (ch == '-') || (ch == '_') || (ch == '.') || (ch == '~'));
}

BOOL URL_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
  int iCharsProcessed = 0;
  if (IsURLChar(&pED->m_tb, ch))
  {
    TextBuffer_PushChar(&pED->m_tbRes, '%');
    TextBuffer_PushHexChar(pED, ch);
    ++iCharsProcessed;
  }
  else
  {
    TextBuffer_PushChar(&pED->m_tbRes, ch);
    ++iCharsProcessed;
  }
  if (piCharsProcessed)
  {
    (*piCharsProcessed) += iCharsProcessed;
  }
  return TRUE;
}

BOOL URL_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  const char ch = TextBuffer_PopChar(&pED->m_tb);
  unsigned char chNext = TextBuffer_GetChar(&pED->m_tb);
  if ((ch == '%') && (TextBuffer_GetTailLength(&pED->m_tb) >= 2))
  {
    const char chEncoded1 = TextBuffer_PopChar(&pED->m_tb);
    const char chEncoded2 = TextBuffer_PopChar(&pED->m_tb);
    char chDecoded = 0;
    if (DecodeHexDigits(chEncoded1, chEncoded2, &chDecoded))
    {
      TextBuffer_PushChar(&pED->m_tbRes, chDecoded);
    }
    else
    {
      TextBuffer_PushChar(&pED->m_tbRes, ch);
      TextBuffer_PushChar(&pED->m_tbRes, chEncoded1);
      TextBuffer_PushChar(&pED->m_tbRes, chEncoded2);
    }
    if (piCharsProcessed)
    {
      (*piCharsProcessed) += 3;
    }
    return TRUE;
  }
  else
  {
    TextBuffer_PushChar(&pED->m_tbRes, ch);
    if (piCharsProcessed)
    {
      (*piCharsProcessed) += 1;
    }
    return TRUE;
  }
  return FALSE;
}

static StringSource ss = { 0 };
static RecodingAlgorithm ra = { 0 };

LPCSTR EncodeStringToURL(LPCSTR text, const int textLength, const int encoding, const int additionalData, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorithm_Init(&ra, ERT_URL, TRUE, additionalData);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorithm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

LPCSTR DecodeURLToString(LPCSTR text, const int textLength, const int encoding, const int additionalData, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorithm_Init(&ra, ERT_URL, FALSE, additionalData);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorithm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

void EncodeStrToURL(const HWND hwnd)
{
  RecodingAlgorithm_Init(&ra, ERT_URL, TRUE, 0);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorithm_Release(&ra);
}

void DecodeURLToStr(const HWND hwnd)
{
  RecodingAlgorithm_Init(&ra, ERT_URL, FALSE, 0);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorithm_Release(&ra);
}
