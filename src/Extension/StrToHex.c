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

int Hex_GetExpectedResultLength(const BOOL isEncoding, const int originalLength)
{
  if (isEncoding)
  {
    return originalLength * 2;
  }
  else
  {
    return originalLength / 2;
  }
}

BOOL Hex_IsValidSequence(EncodingData* pED, const int requiredChars)
{
  return CheckRequiredHexDigitsAvailable(pED->m_tb.m_ptr, pED->m_tb.m_ptr + pED->m_tb.m_iPos, pED->m_tb.m_iMaxPos, requiredChars);
}

BOOL Hex_Encode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
  TextBuffer_PushHexChar(pED, ch);
  if (piCharsProcessed)
  {
    (*piCharsProcessed) += 1;
  }
  return TRUE;
}

BOOL Hex_Decode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  if (IsUnicodeEncodingMode())
  {
    char chEncoded1, chEncoded2, chEncoded3, chEncoded4;
    if (TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded1, piCharsProcessed)
        && TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded2, piCharsProcessed)
        && TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded3, piCharsProcessed)
        && TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded4, piCharsProcessed))
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
      return TRUE;
    }
    return FALSE;
  }
  else
  {
    char chEncoded1, chEncoded2;
    if (TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded1, piCharsProcessed)
        && TextBuffer_GetLiteralChar(&pED->m_tb, &chEncoded2, piCharsProcessed))
    {
      const char chDecoded = IntByHexDigit(chEncoded1) * 16 + IntByHexDigit(chEncoded2);
      TextBuffer_PushChar(&pED->m_tbRes, chDecoded);
      return TRUE;
    }
    return FALSE;
  }
  return FALSE;
}

static StringSource ss = { 0 };
static RecodingAlgorythm ra = { 0 };

LPCSTR EncodeStringToHex(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorythm_Init(&ra, ERT_HEX, TRUE);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorythm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

LPCSTR DecodeHexToString(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorythm_Init(&ra, ERT_HEX, FALSE);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorythm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

void EncodeStrToHex(const HWND hwnd)
{
  RecodingAlgorythm_Init(&ra, ERT_HEX, TRUE);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorythm_Release(&ra);
}

void DecodeHexToStr(const HWND hwnd)
{
  RecodingAlgorythm_Init(&ra, ERT_HEX, FALSE);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorythm_Release(&ra);
}
