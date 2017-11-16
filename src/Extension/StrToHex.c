#include <wtypes.h>
#include "Externals.h"
#include "StrToHex.h"
#include "StringRecoding.h"

#define MIN_VALID_CHAR_CODE 0x21

#define HEX_DIGITS_UPPER  "0123456789ABCDEF"
#define MIN_HEX_DIGIT_LOWER 'a'
#define MIN_HEX_DIGIT_UPPER 'A'
#define MIN_HEX_DIGIT_VALUE 10

int IntByHexDigit(const unsigned char ch)
{
  if (ch >= MIN_HEX_DIGIT_LOWER)
  {
    return (ch - MIN_HEX_DIGIT_LOWER) + MIN_HEX_DIGIT_VALUE;
  }
  else if (ch >= MIN_HEX_DIGIT_UPPER)
  {
    return (ch - MIN_HEX_DIGIT_UPPER) + MIN_HEX_DIGIT_VALUE;
  }
  else
  {
    return ch - '0';
  }
}

BOOL IsHexDigit(const unsigned char ch)
{
  return (isxdigit(ch) != 0);
}

BOOL TextBuffer_GetHexChar(TextBuffer* pTB, char* pCh, long* piCharsProcessed)
{
  *pCh = MIN_VALID_CHAR_CODE - 1;
  int charsProcessed = 0;
  while ((*pCh < MIN_VALID_CHAR_CODE) && TextBuffer_IsPosOKImpl(pTB, 1))
  {
    *pCh = TextBuffer_PopChar(pTB);
    ++charsProcessed;
    if (piCharsProcessed)
    {
      ++(*piCharsProcessed);
    }
  }
  const BOOL res = (*pCh >= MIN_VALID_CHAR_CODE);
  if (!res)
  {
    while (charsProcessed--)
    {
      TextBuffer_DecPos(pTB);
      if (piCharsProcessed)
      {
        --(*piCharsProcessed);
      }
    }
  }
  return res;
}

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

BOOL Hex_Encode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  const char ch = TextBuffer_PopChar(&pED->m_tb);
  TextBuffer_PushChar(&pED->m_tbRes, HEX_DIGITS_UPPER[(ch >> 4) & 0xF]);
  TextBuffer_PushChar(&pED->m_tbRes, HEX_DIGITS_UPPER[ch & 0xF]);
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
    if (TextBuffer_GetHexChar(&pED->m_tb, &chEncoded1, piCharsProcessed)
        && TextBuffer_GetHexChar(&pED->m_tb, &chEncoded2, piCharsProcessed)
        && TextBuffer_GetHexChar(&pED->m_tb, &chEncoded3, piCharsProcessed)
        && TextBuffer_GetHexChar(&pED->m_tb, &chEncoded4, piCharsProcessed))
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
    if (TextBuffer_GetHexChar(&pED->m_tb, &chEncoded1, piCharsProcessed)
        && TextBuffer_GetHexChar(&pED->m_tb, &chEncoded2, piCharsProcessed))
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

LPCSTR EncodeStringToHex(LPCSTR text, const int encoding, const int bufferSize)
{
  iEncoding = encoding;
  RecodingAlgorythm_Init(&ra, ERT_HEX, TRUE);
  StringSource_Init(&ss, text, NULL);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorythm_Release(&ra);
  return ss.result;
}

LPCSTR DecodeHexToString(LPCSTR text, const int encoding, const int bufferSize)
{
  iEncoding = encoding;
  RecodingAlgorythm_Init(&ra, ERT_HEX, FALSE);
  StringSource_Init(&ss, text, NULL);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorythm_Release(&ra);
  return ss.result;
}

void EncodeStrToHex(const HWND hwnd)
{
  RecodingAlgorythm_Init(&ra, ERT_HEX, TRUE);
  StringSource_Init(&ss, "", hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorythm_Release(&ra);
}

void DecodeHexToStr(const HWND hwnd)
{
  RecodingAlgorythm_Init(&ra, ERT_HEX, FALSE);
  StringSource_Init(&ss, "", hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorythm_Release(&ra);
}
