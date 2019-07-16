#include <wtypes.h>
#include "Externals.h"
#include "StrToHex.h"
#include "StringRecoding.h"

BOOL CheckRequiredHexDigitsAvailable(LPCSTR pCurTextOrigin, LPCSTR pCurText, const long iCurTextLength, const long nChars)
{
  BOOL res = FALSE;
  if (iCurTextLength - (pCurText - pCurTextOrigin) >= nChars)
  {
    long i = 0;
    res = TRUE;
    for (i = 0; i < nChars; ++i)
    {
      res &= IsHexDigit(pCurText[i]);
      if (!res)
      {
        break;
      }
    }
  }
  return res;
}

BOOL Hex_IsValidSequence(EncodingData* pED, const int requiredChars)
{
  return CheckRequiredHexDigitsAvailable(pED->m_tb.m_ptr, pED->m_tb.m_ptr + pED->m_tb.m_iPos, pED->m_tb.m_iMaxPos, requiredChars);
}

BOOL Hex_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
  TextBuffer_PushHexChar(pED, ch);
  if (piCharsProcessed)
  {
    (*piCharsProcessed) += 1;
  }
  return TRUE;
}

BOOL Hex_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  long iCharsProcessed = 0;
  if (n2e_IsUnicodeEncodingMode())
  {
    char chEncoded1, chEncoded2, chEncoded3, chEncoded4;
    if (TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded1, &iCharsProcessed)
        && TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded2, &iCharsProcessed)
        && TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded3, &iCharsProcessed)
        && TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded4, &iCharsProcessed))
    {
      const char chDecoded1 = IntByHexDigit(chEncoded1) * 16 + IntByHexDigit(chEncoded2);
      const char chDecoded2 = IntByHexDigit(chEncoded3) * 16 + IntByHexDigit(chEncoded4);
      if (IsReverseUnicodeEncodingMode())
      {
        TextBuffer_PushChar(&pED->m_tbRes, chDecoded1);
        TextBuffer_PushChar(&pED->m_tbRes, chDecoded2);
      }
      else
      {
        TextBuffer_PushChar(&pED->m_tbRes, chDecoded2);
        TextBuffer_PushChar(&pED->m_tbRes, chDecoded1);
      }
      if (piCharsProcessed)
      {
        (*piCharsProcessed) += iCharsProcessed;
      }
      return TRUE;
    }
    return FALSE;
  }
  else
  {
    char chEncoded1, chEncoded2;
    if (TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded1, &iCharsProcessed)
        && TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded2, &iCharsProcessed))
    {
      const char chDecoded = IntByHexDigit(chEncoded1) * 16 + IntByHexDigit(chEncoded2);
      TextBuffer_PushChar(&pED->m_tbRes, chDecoded);
      if (piCharsProcessed)
      {
        (*piCharsProcessed) += iCharsProcessed;
      }
      return TRUE;
    }
    return FALSE;
  }
  return FALSE;
}

static StringSource ss = { 0 };
static RecodingAlgorithm ra = { 0 };

LPCSTR EncodeStringToHex(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorithm_Init(&ra, ERT_HEX, TRUE);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorithm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

LPCSTR DecodeHexToString(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorithm_Init(&ra, ERT_HEX, FALSE);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorithm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

void EncodeStrToHex(const HWND hwnd)
{
  RecodingAlgorithm_Init(&ra, ERT_HEX, TRUE);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorithm_Release(&ra);
}

void DecodeHexToStr(const HWND hwnd)
{
  RecodingAlgorithm_Init(&ra, ERT_HEX, FALSE);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorithm_Release(&ra);
}
