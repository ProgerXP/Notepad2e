#include "StringRecoding.h"
#include <assert.h>
#include <Shlwapi.h>
#include "CommonUtils.h"
#include "Externals.h"
#include "Scintilla.h"
#include "SciCall.h"
#include "StrToBase64.h"
#include "StrToHex.h"
#include "StrToQP.h"
#include "StrToURL.h"
#include "CommentAwareLineWrapping.h"

#define MIN_RECODING_BUFFER_SIZE 8
#define DEFAULT_RECODING_BUFFER_SIZE 65536
#define DEFAULT_RECODING_BUFFER_SIZE_MAX 5 * 1024 * 1024

int iRecodingBufferSize = DEFAULT_RECODING_BUFFER_SIZE;
int iRecodingBufferSizeMax = DEFAULT_RECODING_BUFFER_SIZE_MAX;

#define HEX_DIGITS_UPPER  "0123456789ABCDEF"

BOOL bBreakOnError = TRUE;

BOOL n2e_IsUnicodeEncodingMode()
{
  return (mEncoding[iEncoding].uFlags & NCP_UNICODE);
}

BOOL n2e_IsUTF8EncodingMode()
{
  return (mEncoding[iEncoding].uFlags & NCP_UTF8);
}

BOOL Is8BitEncodingMode()
{
  return (mEncoding[iEncoding].uFlags & NCP_8BIT);
}

BOOL IsReverseUnicodeEncodingMode()
{
  return (mEncoding[iEncoding].uFlags & NCP_UNICODE_REVERSE);
}

#define MIN_HEX_DIGIT_LOWER 'a'
#define MIN_HEX_DIGIT_UPPER 'A'
#define MIN_HEX_DIGIT_VALUE 10

BOOL IsHexDigit(const unsigned char ch)
{
  return (isxdigit(ch) != 0);
}

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

BOOL DecodeHexDigits(const unsigned char chEncoded1, const unsigned char chEncoded2, unsigned char* pchDecoded)
{
  if (IsHexDigit(chEncoded1) && IsHexDigit(chEncoded2))
  {
    assert(pchDecoded);
    if (pchDecoded)
    {
      *pchDecoded = IntByHexDigit(chEncoded1) * 16 + IntByHexDigit(chEncoded2);
      return TRUE;
    }
  }
  return FALSE;
}

void TextBuffer_ResetPos(TextBuffer* pTB, const int iMaxPos)
{
  pTB->m_iMaxPos = iMaxPos;
  pTB->m_iPos = 0;
}

void TextBuffer_Clear(TextBuffer* pTB)
{
  TextBuffer_ResetPos(pTB, 0);
  pTB->m_ptr[0] = 0;
}

BOOL TextBuffer_Init(TextBuffer* pTB, const int iSize)
{
  pTB->m_iSize = iSize;
  pTB->m_ptr = (LPSTR)n2e_Alloc(pTB->m_iSize);
  TextBuffer_ResetPos(pTB, iSize - 1);
  return TRUE;
}

BOOL TextBuffer_Free(TextBuffer* pTB)
{
  if (pTB->m_ptr)
  {
    n2e_Free((LPVOID)pTB->m_ptr);
  }
  pTB->m_iSize = 0;
  pTB->m_ptr = NULL;
  TextBuffer_ResetPos(pTB, 0);
  return TRUE;
}

BOOL TextBuffer_Update(TextBuffer* pTB, LPSTR ptr, const int iSize)
{
  if (pTB->m_ptr != ptr)
  {
    TextBuffer_Free(pTB);
    pTB->m_iSize = iSize;
    pTB->m_ptr = (LPSTR)ptr;
  }
  TextBuffer_ResetPos(pTB, iSize - 1);
  return TRUE;
}

int TextBuffer_GetHeadLength(TextBuffer* pTB)
{
  return pTB->m_iPos;
}

int TextBuffer_GetTailLength(TextBuffer* pTB)
{
  return pTB->m_iMaxPos - pTB->m_iPos;
}

int TextLengthInChars(LPCSTR lpStr1, LPCSTR lpStr2, const int _iEncoding, int *piByteCount)
{
  *piByteCount = lpStr2 - lpStr1;
  if (lpStr1 == lpStr2)
  {
    *piByteCount = 1;
    return 1;
  }
  if (_iEncoding == CPI_UTF8)
  {
    int len = 0;
    LPCSTR ptr = lpStr1;
    while (ptr != lpStr2)
    {
      len += (*ptr++ & 0xc0) != 0x80;
    }
    return len;
  }
  else
  {
    return lpStr2 - lpStr1;
  }
}

int TextBuffer_GetWordLength(TextBuffer* pTB, const int _iEncoding, int *piByteCount)
{
  const LPSTR pSpace = strpbrk(pTB->m_ptr + pTB->m_iPos, " \t\r\n\a\b");
  if (pSpace)
  {
    return TextLengthInChars(pTB->m_ptr + pTB->m_iPos, pSpace, _iEncoding, piByteCount);
  }
  else
  {
    return TextLengthInChars(pTB->m_ptr + pTB->m_iPos, pTB->m_ptr + pTB->m_iMaxPos, _iEncoding, piByteCount);
  }
}

int TextBuffer_GetCharSequenceLength(TextBuffer* pTB, const char ch, const int iOffsetFrom)
{
  int res = 0;
  while ((pTB->m_iPos + iOffsetFrom + res < pTB->m_iMaxPos)
    && (pTB->m_ptr[pTB->m_iPos + iOffsetFrom + res] == ch))
  {
    ++res;
  }
  return res;
}

int TextBuffer_Find(TextBuffer* pTB, const LPCSTR lpstr, const int iOffsetFrom)
{
  LPCSTR pSrc = pTB->m_ptr + pTB->m_iPos + iOffsetFrom;
  LPCSTR pRes = StrStrA(pSrc, lpstr);
  return pRes ? pRes - pSrc : -1;
}

BOOL TextBuffer_IsAnyCharAtPos_IgnoreSpecial(TextBuffer* pTB, LPCSTR lpChars, LPCSTR lpstrIgnored, const int iOffsetFrom)
{
  int res = 0;
  while (pTB->m_iPos + iOffsetFrom + res < pTB->m_iMaxPos)
  {
    const char _ch = pTB->m_ptr[pTB->m_iPos + iOffsetFrom + res];
    if (strchr(lpChars, _ch))
    {
      return TRUE;
    }
    else if (!strchr(lpstrIgnored, _ch))
    {
      return FALSE;
    }
    ++res;
  }
  return res;
}

BOOL TextBuffer_IsCharAtPos_IgnoreSpecial(TextBuffer* pTB, const char ch, LPCSTR lpstrIgnored, const int iOffsetFrom)
{
  const CHAR chars[2] = { ch, 0 };
  return TextBuffer_IsAnyCharAtPos_IgnoreSpecial(pTB, &chars[0], lpstrIgnored, iOffsetFrom);
}

int TextBuffer_CountWhiteSpaces(TextBuffer* pTB, const int iOffsetFrom)
{
  int res = 0;
  while (pTB->m_iPos + iOffsetFrom + res < pTB->m_iMaxPos)
  {
    const char _ch = pTB->m_ptr[pTB->m_iPos + iOffsetFrom + res];
    if (strchr(" \t", _ch))
    {
      ++res;
      continue;
    }
    else
    {
      break;
    }
  }
  return res;
}

BOOL TextBuffer_IsWhiteSpaceLine(TextBuffer* pTB, const int iOffsetFrom, int* piLineLength)
{
  int res = 0;
  while (pTB->m_iPos + iOffsetFrom + res < pTB->m_iMaxPos)
  {
    const char _ch = pTB->m_ptr[pTB->m_iPos + iOffsetFrom + res];
    if (IsEOLChar(_ch))
    {
      if (piLineLength)
      {
        *piLineLength = res;
      }
      return TRUE;
    }
    ++res;
    if (strchr(" \t", _ch))
    {
      continue;
    }
    return FALSE;
  }
  if (piLineLength)
  {
    *piLineLength = res;
  }
  return TRUE;
}

BOOL TextBuffer_IsTextAtPos(TextBuffer* pTB, const LPCSTR lpstr, const int iOffsetFrom)
{
  LPCSTR pSrc = pTB->m_ptr + pTB->m_iPos + iOffsetFrom;
  return StrCmpNA(pSrc, lpstr, strlen(lpstr)) == 0;
}

BOOL TextBuffer_IsPosOKImpl(TextBuffer* pTB, const int requiredChars)
{
  return pTB->m_iPos <= pTB->m_iSize - 1 - requiredChars;
}

BOOL TextBuffer_IsPosOK(TextBuffer* pTB, RecodingAlgorithm* pRA)
{
  return TextBuffer_IsPosOKImpl(pTB, pRA->iRequiredCharsForEncode);
}

void TextBuffer_IncPos(TextBuffer* pTB)
{
  ++pTB->m_iPos;
}

void TextBuffer_DecPos(TextBuffer* pTB)
{
  --pTB->m_iPos;
}

void TextBuffer_OffsetPos(TextBuffer* pTB, const int iOffset)
{
  pTB->m_iPos += iOffset;
}

char TextBuffer_GetChar(TextBuffer* pTB)
{
  return TextBuffer_GetCharAt(pTB, 0);
}

char TextBuffer_GetCharAt(TextBuffer* pTB, const int iOffset)
{
  return pTB->m_ptr[pTB->m_iPos + iOffset];
}

char TextBuffer_PopChar(TextBuffer* pTB)
{
  const char ch = TextBuffer_GetChar(pTB);
  TextBuffer_IncPos(pTB);
  return ch;
}

BOOL TextBuffer_PushChar(TextBuffer* pTB, const char ch)
{
  pTB->m_ptr[pTB->m_iPos] = ch;
  TextBuffer_IncPos(pTB);
  return TRUE;
}

BOOL TextBuffer_PushHexChar(EncodingData* pED, const unsigned char ch)
{
  TextBuffer_PushChar(&pED->m_tbRes, HEX_DIGITS_UPPER[(ch >> 4) & 0xF]);
  TextBuffer_PushChar(&pED->m_tbRes, HEX_DIGITS_UPPER[ch & 0xF]);
  return TRUE;
}

BOOL TextBuffer_PushNonZeroChar(TextBuffer* pTB, const char ch)
{
  return ch ? TextBuffer_PushChar(pTB, ch) : FALSE;
}

BOOL TextBuffer_GetLiteralChar(TextBuffer* pTB, char* pCh, long* piCharsProcessed)
{
#define MIN_VALID_CHAR_CODE 0x21

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

BOOL TextBuffer_IsEOL(TextBuffer* pTB, const int iEOLMode)
{
  switch (iEOLMode)
  {
  case SC_EOL_CRLF:
    return (TextBuffer_GetChar(pTB) == '\r')
      && (TextBuffer_GetCharAt(pTB, 1) == '\n');
  case SC_EOL_LF:
    return TextBuffer_GetChar(pTB) == '\n';
  case SC_EOL_CR:
    return TextBuffer_GetChar(pTB) == '\r';
  }
  assert(0);
  return FALSE;
}

void TextBuffer_AddEOL(TextBuffer* pTB, const int iEOLMode)
{
  switch (iEOLMode)
  {
  case SC_EOL_CRLF:
    TextBuffer_PushChar(pTB, '\r');
    TextBuffer_PushChar(pTB, '\n');
    return;
  case SC_EOL_LF:
    TextBuffer_PushChar(pTB, '\n');
    return;
  case SC_EOL_CR:
    TextBuffer_PushChar(pTB, '\r');
    return;
  }
  assert(0);
}

BOOL TextBuffer_IsDataPortionAvailable(TextBuffer* pTB, const long iRequiredChars)
{
  const long iRemainingChars = (pTB->m_iPos < pTB->m_iMaxPos) ? (pTB->m_iMaxPos - pTB->m_iPos) : 0;
  if (iRemainingChars >= iRequiredChars)
  {
    return TRUE;
  }
  return FALSE;
}

void TextBuffer_NormalizeBeforeEncode(RecodingAlgorithm* pRA, TextBuffer* pTB, long* piPositionCurrent, long* piUnicodeProcessedChars)
{
  if (n2e_IsUnicodeEncodingMode() && (pRA->recodingType == ERT_HEX))
  {
    int cbDataWide = (pTB->m_iMaxPos + 1) * sizeof(WCHAR);
    LPWSTR lpDataWide = n2e_Alloc(cbDataWide);
    cbDataWide = MultiByteToWideChar(CP_UTF8, 0, pTB->m_ptr, pTB->m_iMaxPos, lpDataWide, cbDataWide / sizeof(WCHAR)) * sizeof(WCHAR);
    lpDataWide[cbDataWide / sizeof(WCHAR)] = 0;

#define INVALID_UNICODE_CHAR    0xFFFD
    LPWSTR lpInvalidCharPos = wcschr(lpDataWide, INVALID_UNICODE_CHAR);
    while (lpInvalidCharPos)
    {
      // block starts with specific code
      if (lpInvalidCharPos == lpDataWide)
      {
        ++lpInvalidCharPos;
        cbDataWide = (lpInvalidCharPos - lpDataWide) * sizeof(WCHAR);
        lpInvalidCharPos[0] = 0;
        if (piPositionCurrent)
        {
          (*piPositionCurrent) -= pTB->m_iMaxPos - WideCharToMultiByte(CP_UTF8, 0, lpDataWide, wcslen(lpDataWide), NULL, 0, NULL, NULL);
        }
        break;
      }
      // invalid char is a part of truncated UTF-8 sequence
      if (lpInvalidCharPos && (lpInvalidCharPos < lpDataWide + pTB->m_iMaxPos))
      {
        cbDataWide = (lpInvalidCharPos - lpDataWide) * sizeof(WCHAR);
        lpInvalidCharPos[0] = 0;
        if (piPositionCurrent)
        {
          (*piPositionCurrent) -= pTB->m_iMaxPos - WideCharToMultiByte(CP_UTF8, 0, lpDataWide, wcslen(lpDataWide), NULL, 0, NULL, NULL);
        }
        break;
      }
      lpInvalidCharPos = wcschr(lpInvalidCharPos + 1, INVALID_UNICODE_CHAR);
    }
    if (piUnicodeProcessedChars)
    {
      *piUnicodeProcessedChars = WideCharToMultiByte(CP_UTF8, 0, lpDataWide, wcslen(lpDataWide), NULL, 0, NULL, NULL);
    }
    if (!IsReverseUnicodeEncodingMode())
    {
      _swab((char *)lpDataWide, (char *)lpDataWide, cbDataWide);
    }
    TextBuffer_Update(pTB, (LPSTR)lpDataWide, cbDataWide + 1);
  }
  else if (mEncoding[iEncoding].uFlags & NCP_UTF8)
  {
    // nothing to do
  }
  else if (mEncoding[iEncoding].uFlags & NCP_8BIT)
  {
    BOOL bCancelDataLoss = FALSE;
    UINT uCodePage = mEncoding[iEncoding].uCodePage;
    int cbData = pTB->m_iMaxPos + 1;
    LPSTR lpData = pTB->m_ptr;
    const int iOriginalLength = strlen(lpData);
    LPWSTR lpDataWide = n2e_Alloc(cbData * sizeof(WCHAR) + 16);
    int    cbDataWide = MultiByteToWideChar(CP_UTF8, 0, lpData, cbData, lpDataWide, cbData);
    // Special cases: 42, 50220, 50221, 50222, 50225, 50227, 50229, 54936, 57002-11, 65000, 65001
    if (uCodePage == CP_UTF7 || uCodePage == 54936)
    {
      cbData = cbDataWide * 2;
      lpData = n2e_Alloc(cbData);
      cbData = WideCharToMultiByte(uCodePage, 0, lpDataWide, cbDataWide, lpData, cbData, NULL, NULL);
    }
    else
    {
      cbData = WideCharToMultiByte(uCodePage, WC_NO_BEST_FIT_CHARS, lpDataWide, cbDataWide, lpData, cbData, NULL, &bCancelDataLoss);
      if (!bCancelDataLoss)
      {
        cbData = WideCharToMultiByte(uCodePage, 0, lpDataWide, cbDataWide, lpData, cbData, NULL, NULL);
        bCancelDataLoss = FALSE;
      }
    }
    n2e_Free(lpDataWide);
    TextBuffer_Update(pTB, lpData, cbData);
  }
}

void TextBuffer_NormalizeAfterDecode(RecodingAlgorithm* pRA, TextBuffer* pTB)
{
  const UINT uCodePage = mEncoding[iEncoding].uCodePage;
  if (n2e_IsUnicodeEncodingMode() && (pRA->recodingType == ERT_HEX))
  {
    int cbData = pTB->m_iPos * sizeof(WCHAR) + 16;
    LPSTR lpData = n2e_Alloc(cbData);
    cbData = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)pTB->m_ptr, pTB->m_iPos / sizeof(WCHAR), lpData, cbData, NULL, NULL);
    lpData[cbData] = 0;
    TextBuffer_Update(pTB, lpData, cbData);
    pTB->m_iPos = cbData;
  }
  else if (mEncoding[iEncoding].uFlags & NCP_UTF8)
  {
    // nothing to do
  }
  else if (mEncoding[iEncoding].uFlags & NCP_8BIT)
  {
    int cbData = pTB->m_iPos * 2 + 16;
    LPWSTR lpDataWide = n2e_Alloc(cbData);
    cbData = MultiByteToWideChar(uCodePage, 0, pTB->m_ptr, pTB->m_iPos, lpDataWide, cbData);
    const int requiredBufferSize = WideCharToMultiByte(CP_UTF8, 0, lpDataWide, cbData, NULL, 0, NULL, NULL);
    if (requiredBufferSize >= pTB->m_iMaxPos)
    {
      TextBuffer_Init(pTB, requiredBufferSize + 1);
    }
    pTB->m_iPos = WideCharToMultiByte(CP_UTF8, 0, lpDataWide, cbData, pTB->m_ptr, pTB->m_iMaxPos, NULL, NULL);
    pTB->m_ptr[pTB->m_iPos] = 0;
    n2e_Free(lpDataWide);
  }
}

BOOL TextRange_Init(const StringSource* pSS, const RecodingAlgorithm* pRA, struct TTextRange* pTR)
{
  pTR->m_hwnd = pSS->hwnd;
  pTR->m_iSelStart = StringSource_GetSelectionStart(pSS);
  pTR->m_iSelEnd = StringSource_GetSelectionEnd(pSS);
  if (pTR->m_iSelStart == pTR->m_iSelEnd)
  {
    if (pRA->recodingType == ERT_CALW)
    {
      pTR->m_iSelStart = StringSource_GetLineStart(pSS, pTR->m_iSelStart);
      pTR->m_iSelEnd = StringSource_GetLineEnd(pSS, pTR->m_iSelStart);
    }
    else
    {
      pTR->m_iSelStart = 0;
      pTR->m_iSelEnd = StringSource_GetLength(pSS);
    }
  };
  if (pRA->recodingType == ERT_CALW)
  {
    while ((pTR->m_iSelStart < pTR->m_iSelEnd) && strchr("\r\n\t ", StringSource_GetCharAt(pSS, pTR->m_iSelEnd - 1)))
    {
      if (!pSS->hwnd)
        break;
      --pTR->m_iSelEnd;
    }
  }
  pTR->m_iPositionCurrent = pTR->m_iSelStart;
  pTR->m_iExpectedProcessedChars = 0;

  return pTR->m_iSelStart != pTR->m_iSelEnd;
}

BOOL TextRange_GetNextDataPortion(StringSource* pSS, struct TTextRange* pTR, struct TTextBuffer* pTB)
{
  const long iEnd = min(pTR->m_iPositionCurrent + pTB->m_iSize - 1, pTR->m_iSelEnd);
  if (StringSource_GetText(pSS, pTB->m_ptr, pTR->m_iPositionCurrent, iEnd))
  {
    TextBuffer_ResetPos(pTB, iEnd - pTR->m_iPositionCurrent);
    pTR->m_iPositionStart = pTR->m_iPositionCurrent;
    pTR->m_iPositionCurrent = iEnd;
    return TRUE;
  }
  return FALSE;
}

BOOL RecodingAlgorithm_Init(RecodingAlgorithm* pRA, const ERecodingType rt, const BOOL isEncoding,
  const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3)
{
  pRA->recodingType = rt;
  pRA->isEncoding = isEncoding;
  pRA->iPassCount = 1;
  pRA->iAdditionalData1 = iAdditionalData1;
  pRA->iAdditionalData2 = iAdditionalData2;
  pRA->iAdditionalData3 = iAdditionalData3;
  switch (pRA->recodingType)
  {
  case ERT_HEX:
    lstrcpy(pRA->statusText, isEncoding ? L"String to Hex..." : L"Hex to String...");
    pRA->iRequiredCharsForEncode = 1;
    pRA->iRequiredCharsForDecode = n2e_IsUnicodeEncodingMode() ? 4 : 2;
    pRA->pIsValidStrSequence = Hex_IsValidSequence;
    pRA->pEncodeMethod = Hex_Encode;
    pRA->pEncodeTailMethod = NULL;
    pRA->pDecodeMethod = Hex_Decode;
    pRA->pDecodeTailMethod = NULL;
    pRA->data = NULL;
    return TRUE;
  case ERT_BASE64:
    lstrcpy(pRA->statusText, isEncoding ? L"String to Base64..." : L"Base64 to String...");
    pRA->iRequiredCharsForEncode = 3;
    pRA->iRequiredCharsForDecode = 4;
    pRA->pIsValidStrSequence = Base64_IsValidSequence;
    pRA->pEncodeMethod = Base64_Encode;
    pRA->pEncodeTailMethod = Base64_EncodeTail;
    pRA->pDecodeMethod = Base64_Decode;
    pRA->pDecodeTailMethod = NULL;
    pRA->data = Base64_InitAlgorithmData(pRA->isEncoding);
    return TRUE;
  case ERT_QP:
    lstrcpy(pRA->statusText, isEncoding ? L"String to QP..." : L"QP to String...");
    pRA->iRequiredCharsForEncode = 1;
    pRA->iRequiredCharsForDecode = 3;   // specify max correctly encoded sequence length as
                                        // minimal required length to prevent buffer transition problem
                                        // when encoded sequence is splitted between buffers
    pRA->pIsValidStrSequence = QP_IsValidSequence;
    pRA->pEncodeMethod = QP_Encode;
    pRA->pEncodeTailMethod = NULL;
    pRA->pDecodeMethod = QP_Decode;
    pRA->pDecodeTailMethod = QP_Decode; // use regular decode proc for tail
    pRA->data = QP_InitAlgorithmData(pRA->isEncoding);
    return TRUE;
  case ERT_URL:
    lstrcpy(pRA->statusText, isEncoding ? L"String to URL..." : L"URL to String...");
    pRA->iRequiredCharsForEncode = 1;
    pRA->iRequiredCharsForDecode = 3;
    pRA->pIsValidStrSequence = URL_IsValidSequence;
    pRA->pEncodeMethod = URL_Encode;
    pRA->pEncodeTailMethod = NULL;
    pRA->pDecodeMethod = URL_Decode;
    pRA->pDecodeTailMethod = URL_Decode;
    pRA->data = NULL;
    return TRUE;
  case ERT_CALW:
    lstrcpy(pRA->statusText, L"Comment-aware line wrapping...");
    pRA->iPassCount = 2;
    pRA->iRequiredCharsForEncode = 1;
    pRA->iRequiredCharsForDecode = 3;
    pRA->pIsValidStrSequence = CALW_IsValidSequence;
    pRA->pEncodeMethod = CALW_Encode;
    pRA->pEncodeTailMethod = NULL;
    pRA->pDecodeMethod = CALW_Decode;
    pRA->pDecodeTailMethod = NULL;
    pRA->data = CALW_InitAlgorithmData(pRA->iAdditionalData1, pRA->iAdditionalData2, pRA->iAdditionalData3);
    return TRUE;
  default:
    assert(FALSE);
    return FALSE;
  }
}

BOOL RecodingAlgorithm_Release(RecodingAlgorithm* pRA)
{
  switch (pRA->recodingType)
  {
  case ERT_HEX:
    return TRUE;
  case ERT_BASE64:
    Base64_ReleaseAlgorithmData(pRA->data);
    return TRUE;
  case ERT_QP:
    QP_ReleaseAlgorithmData(pRA->data);
    return TRUE;
  case ERT_URL:
    return TRUE;
  case ERT_CALW:
    CALW_ReleaseAlgorithmData(pRA->data);
    return TRUE;
  default:
    assert(FALSE);
    return FALSE;
  }
}

void StringSource_InitFromString(StringSource* pSS, LPCSTR text, const int textLength)
{
  pSS->iTextLength = (textLength <= 0) ? strlen(text) : textLength;
  pSS->iProcessedChars = 0;
  pSS->hwnd = NULL;
  memcpy(pSS->text, text, min(sizeof(pSS->text), textLength));
  pSS->iResultLength = 0;
}

void StringSource_InitFromHWND(StringSource* pSS, const HWND hwnd)
{
  pSS->iTextLength = 0;
  pSS->iProcessedChars = 0;
  pSS->hwnd = hwnd;
  pSS->iResultLength = 0;
}

long StringSource_GetSelectionStart(const StringSource* pSS)
{
  return pSS->hwnd
    ? SciCall_GetSelStart()
    : 0;
}

long StringSource_GetSelectionEnd(const StringSource* pSS)
{
  return pSS->hwnd
    ? SciCall_GetSelEnd()
    : pSS->iTextLength;
}

long StringSource_GetLineStart(const StringSource* pSS, const int iPos)
{
  return pSS->hwnd
    ? SciCall_PositionFromLine(SciCall_LineFromPosition(iPos))
    : 0;
}

long StringSource_GetLineEnd(const StringSource* pSS, const int iPos)
{
  return pSS->hwnd
    ? SciCall_LineEndPosition(SciCall_LineFromPosition(iPos))
    : pSS->iTextLength;
}

long StringSource_GetLength(const StringSource* pSS)
{
  return pSS->hwnd
    ? SciCall_GetLength()
    : pSS->iTextLength;
}

char StringSource_GetCharAt(const StringSource* pSS, const int iPos)
{
  return pSS->hwnd
    ? SciCall_GetCharAt(iPos)
    : pSS->text[iPos];
}

long StringSource_IsDataPortionAvailable(const StringSource* pSS, EncodingData* pED)
{
  if (pSS->hwnd)
  {
    return pED->m_tr.m_iPositionCurrent < pED->m_tr.m_iSelEnd;
  }
  else
  {
    return pSS->iProcessedChars < pSS->iTextLength;
  }
}

BOOL StringSource_GetText(StringSource* pSS, LPSTR pText, const long iStart, const long iEnd)
{
  LPSTR res = NULL;
  const long length = iEnd - iStart;
  if (length > 0)
  {
    struct TextRange tr = { 0 };
    tr.chrg.cpMin = iStart;
    tr.chrg.cpMax = iEnd;
    tr.lpstrText = pText;
    if (pSS->hwnd)
    {
      return SciCall_GetTextRange(0, &tr) > 0;
    }
    else
    {
      memcpy(tr.lpstrText, &pSS->text[pSS->iProcessedChars], (iEnd - iStart) + 1);
      return TRUE;
    }
  }
  return FALSE;
}

BOOL EncodingSettings_Init(const StringSource* pSS, const RecodingAlgorithm* pRA, EncodingData* pED)
{
  if (!TextRange_Init(pSS, pRA, &pED->m_tr))
  {
    return FALSE;
  }
  pED->m_bIsEncoding = pRA->isEncoding;
  int iBufferSize = iRecodingBufferSize;
  if (pED->m_tr.m_iSelEnd > iRecodingBufferSizeMax)
  {
    iBufferSize = iRecodingBufferSizeMax;
  }
  else
  {
    while ((pED->m_tr.m_iSelEnd > iBufferSize * 10) && (iBufferSize < iRecodingBufferSizeMax))
    {
      iBufferSize += iRecodingBufferSize;
    }
  }
  TextBuffer_Init(&pED->m_tb, iBufferSize);
  TextBuffer_Init(&pED->m_tbRes, iBufferSize * 4);
  TextBuffer_Init(&pED->m_tbTmp, iBufferSize * 4);
  return TRUE;
}

void EncodingSettings_Free(EncodingData* pED)
{
  TextBuffer_Free(&pED->m_tb);
  TextBuffer_Free(&pED->m_tbRes);
  TextBuffer_Free(&pED->m_tbTmp);
}

void Recode_Run(RecodingAlgorithm* pRA, StringSource* pSS, const int bufferSize)
{
  if (bufferSize > MIN_RECODING_BUFFER_SIZE)
  {
    iRecodingBufferSize = bufferSize;
    iRecodingBufferSizeMax = bufferSize;
  }
  else
  {
    iRecodingBufferSize = DEFAULT_RECODING_BUFFER_SIZE;
    iRecodingBufferSizeMax = DEFAULT_RECODING_BUFFER_SIZE_MAX;
  }
  struct TEncodingData ed;
  if (!EncodingSettings_Init(pSS, pRA, &ed))
  {
    return;
  }
  if (pSS->hwnd)
  {
    SciCall_SetSkipUIUpdate(1);
    SciCall_BeginUndoAction();
  }

  for (int i = 0; i < pRA->iPassCount; ++i)
  {
    pRA->iPassIndex = i;
    if (i != 0)
    {
      if (!pSS->hwnd)
      {
        memcpy_s(pSS->text, pSS->iResultLength, pSS->result, pSS->iResultLength);
        pSS->text[pSS->iResultLength] = 0;
        pSS->iTextLength = pSS->iResultLength;
        memset(pSS->result, 0, pSS->iResultLength);
        ed.m_tr.m_iSelEnd = pSS->iTextLength;
      }
      pSS->iProcessedChars = 0;
      pSS->iResultLength = 0;
      ed.m_tr.m_iPositionCurrent = ed.m_tr.m_iSelStart;
    }
    n2e_ShowProgressBarInStatusBar(pRA->statusText, 0, ed.m_tr.m_iSelEnd - ed.m_tr.m_iSelStart);
    BOOL bProcessFailed = FALSE;
    while (StringSource_IsDataPortionAvailable(pSS, &ed))
    {
      if (ed.m_tb.m_iSize < iRecodingBufferSize)
      {
        TextBuffer_Init(&ed.m_tb, iRecodingBufferSize);
      }
      if (TextRange_GetNextDataPortion(pSS, &ed.m_tr, &ed.m_tb))
      {
        if (!Recode_ProcessDataPortion(pRA, pSS, &ed))
        {
          bProcessFailed = TRUE;
          break;
        }
      }
    }
    if (bProcessFailed)
    {
      ed.m_tr.m_iSelEnd = ed.m_tr.m_iPositionCurrent;
      break;
    }
  }
  const int iSelStart = ed.m_tr.m_iSelStart;
  const int iSelEnd = ed.m_tr.m_iSelEnd;
  EncodingSettings_Free(&ed);

  if (pSS->hwnd)
  {
    SciCall_LineScroll(-iSelEnd, 0);
    SciCall_SetSel(iSelStart, iSelEnd);
    SciCall_EndUndoAction();
    SciCall_SetSkipUIUpdate(0);
    UpdateWindow(pSS->hwnd);
  }
  
  n2e_HideProgressBarInStatusBar();
}

long UTF8StringLength(LPCSTR text)
{
  long len = 0;
  LPCSTR s = text;
  while (*s)
  {
    len += (*s++ & 0xc0) != 0x80;
  }
  return len;
}


BOOL Recode_ProcessDataPortion(RecodingAlgorithm* pRA, StringSource* pSS, EncodingData* pED)
{
  BOOL bRes = TRUE;
  long iCursorOffset = 0;
  BOOL charsProcessed = FALSE;
  long iCharsProcessed = 0;
  if (pED->m_bIsEncoding)
  {
    TextBuffer_NormalizeBeforeEncode(pRA, &pED->m_tb, &pED->m_tr.m_iPositionCurrent, &pED->m_tr.m_iExpectedProcessedChars);
    if (pED->m_tr.m_iExpectedProcessedChars
        && (pED->m_tbRes.m_iSize < pED->m_tr.m_iExpectedProcessedChars * 4))
    {
      TextBuffer_Init(&pED->m_tbRes, (pED->m_tr.m_iExpectedProcessedChars + 1) * 4);
    }
    while (TextBuffer_IsDataPortionAvailable(&pED->m_tb, pRA->iRequiredCharsForEncode)
           && TextBuffer_IsPosOK(&pED->m_tbRes, pRA))
    {
      charsProcessed |= pRA->pEncodeMethod(pRA, pED, &iCharsProcessed);
    }
    if (pRA->pEncodeTailMethod && !charsProcessed && !TextBuffer_IsDataPortionAvailable(&pED->m_tb, pRA->iRequiredCharsForEncode))
    {
      charsProcessed |= pRA->pEncodeTailMethod(pRA, pED, &iCharsProcessed);
    }
    if (TextBuffer_IsPosOK(&pED->m_tbRes, pRA))
    {
      if (pRA->recodingType == ERT_HEX)
      {
        iCursorOffset = pED->m_tbRes.m_iPos - (pED->m_tr.m_iPositionCurrent - pED->m_tr.m_iPositionStart);
      }
      else
      {
        pED->m_tr.m_iPositionCurrent = pED->m_tr.m_iPositionStart + iCharsProcessed;
        iCursorOffset = pED->m_tbRes.m_iPos - (pED->m_tr.m_iPositionCurrent - pED->m_tr.m_iPositionStart);
      }
    }
    else
    {
      const int iEncodedChars = pED->m_tbRes.m_iPos / pRA->iRequiredCharsForEncode;
      pED->m_tr.m_iPositionCurrent = pED->m_tr.m_iPositionStart + iEncodedChars;
      iCursorOffset = pED->m_tbRes.m_iPos - iEncodedChars;
    }
  }
  else
  {
    while (TextBuffer_IsDataPortionAvailable(&pED->m_tb, pRA->iRequiredCharsForDecode))
    {
      auto res = pRA->pDecodeMethod(pRA, pED, &iCharsProcessed);
      charsProcessed |= res;
      if (!res)
        break;
    }
    if (pRA->pDecodeTailMethod && !charsProcessed && !TextBuffer_IsDataPortionAvailable(&pED->m_tb, pRA->iRequiredCharsForDecode))
    {
      charsProcessed |= pRA->pDecodeTailMethod(pRA, pED, &iCharsProcessed);
    }
    if (charsProcessed)
    {
      pED->m_tr.m_iPositionCurrent += pED->m_tb.m_iPos - pED->m_tb.m_iMaxPos;
      TextBuffer_NormalizeAfterDecode(pRA, &pED->m_tbRes);
      iCursorOffset = pED->m_tbRes.m_iPos - pED->m_tb.m_iPos;
    }
    else
    {
      pED->m_tr.m_iPositionCurrent = pED->m_tr.m_iPositionStart;
      if (bBreakOnError && !isspace(pED->m_tb.m_ptr[pED->m_tb.m_iPos]))
      {
        bRes = FALSE;
      }
      else
      {
        // skip unsupported characters
        while (pED->m_tb.m_iPos < pED->m_tb.m_iMaxPos)
        {
          if (!pRA->pIsValidStrSequence(pED, pRA->iRequiredCharsForDecode))
          {
            TextBuffer_IncPos(&pED->m_tb);
            ++pED->m_tr.m_iPositionCurrent;
            continue;
          }
          break;
        }
        // skip trailing characters if there was no activity in this round
        if ((pED->m_tb.m_iMaxPos < pED->m_tb.m_iSize - 1) && (pED->m_tb.m_iPos == 0))
        {
          while (pED->m_tb.m_iPos < pED->m_tb.m_iMaxPos)
          {
            TextBuffer_IncPos(&pED->m_tb);
            ++pED->m_tr.m_iPositionCurrent;
          }
        }
      }
    }
  }

  if (charsProcessed)
  {
    pED->m_tbRes.m_ptr[pED->m_tbRes.m_iPos] = 0;
    if (pSS->hwnd)
    {
      SciCall_SetSel(pED->m_tr.m_iPositionStart, pED->m_tr.m_iPositionCurrent);
      SciCall_ReplaceSel(0, "");
      SciCall_AddText(pED->m_tbRes.m_iPos, pED->m_tbRes.m_ptr);
      pSS->iResultLength += pED->m_tbRes.m_iPos;
      pED->m_tr.m_iSelEnd += iCursorOffset;
      pED->m_tr.m_iPositionCurrent = SciCall_GetCurrentPos();
      iCursorOffset = 0;
    }
    else
    {
      const int length = pED->m_tbRes.m_iPos;
      memcpy_s(pSS->result + pED->m_tr.m_iPositionStart,
                MAX_TEST_STRING_LENGTH - pED->m_tr.m_iPositionStart,
                pED->m_tbRes.m_ptr, length);
      pSS->iResultLength += length;
    }
    if (pED->m_bIsEncoding && n2e_IsUnicodeEncodingMode())
    {
      if (pED->m_tr.m_iExpectedProcessedChars)
      {
        pSS->iProcessedChars += pED->m_tr.m_iExpectedProcessedChars;
      }
      else
      {
        pSS->iProcessedChars += iCharsProcessed / 2;
      }
      pED->m_tr.m_iExpectedProcessedChars = 0;
    }
    else
    {
      pSS->iProcessedChars += iCharsProcessed;
    }
    if (iCursorOffset != 0)
    {
      pED->m_tr.m_iPositionCurrent += iCursorOffset;
      pED->m_tr.m_iSelEnd += iCursorOffset;
    }
  }

  n2e_IncProgressBarPosInStatusBar(pED->m_tb.m_iPos);
  TextBuffer_Clear(&pED->m_tb);
  TextBuffer_Clear(&pED->m_tbRes);
  TextBuffer_Clear(&pED->m_tbTmp);

  return bRes;
}
