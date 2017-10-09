#include <Windows.h>
#include <Strsafe.h>
#include "StrToHex.h"

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

#define MIN_VALID_CHAR_CODE 0x21
#define TEXT_BUFFER_SIZE_MIN 65536
#define TEXT_BUFFER_SIZE_MAX TEXT_BUFFER_SIZE_MIN * 10

BOOL bBreakOnError = TRUE;

LPVOID MemAlloc(const int sz)
{
  return GlobalAlloc(GPTR, sz);
}

void MemFree(LPVOID ptr)
{
  GlobalFree(ptr);
}

long TSS_GetSelectionStart(const StringSource* pSS)
{
  return pSS->hwnd
    ? SendMessage(pSS->hwnd, SCI_GETSELECTIONSTART, 0, 0)
    : 0;
}

long TSS_GetSelectionEnd(const StringSource* pSS)
{
  return pSS->hwnd
    ? SendMessage(pSS->hwnd, SCI_GETSELECTIONEND, 0, 0)
    : 0;
}

long TSS_GetLength(const StringSource* pSS)
{
  return pSS->hwnd
    ? SendMessage(pSS->hwnd, SCI_GETLENGTH, 0, 0)
    : strlen(pSS->text);
}

BOOL GetText(const StringSource* pSS, LPSTR pText, const long iStart, const long iEnd)
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
      return SendMessage(pSS->hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr) > 0;
    }
    else
    {
      strncpy_s(tr.lpstrText, tr.chrg.cpMax - tr.chrg.cpMin + 1, &pSS->text[tr.chrg.cpMin], _TRUNCATE);
      return TRUE;
    }
  }
  return FALSE;
}

int IntByHexDigit(const unsigned char ch)
{
  if (ch >= 'a')
  {
    return (ch - 'a') + 10;
  }
  else if (ch >= 'A')
  {
    return (ch - 'A') + 10;
  }
  else
  {
    return ch - '0';
  }
}

#define HEX_DIGITS_UPPER  "0123456789ABCDEF"

inline BOOL IsHexDigit(const unsigned char ch)
{
  return (isxdigit(ch) != 0);
}

BOOL IsUnicodeEncodingMode()
{
  return (mEncoding[iEncoding].uFlags & NCP_UNICODE);
}

BOOL IsReverseUnicodeEncodingMode()
{
  return (mEncoding[iEncoding].uFlags & NCP_UNICODE_REVERSE);
}

inline int RequiredCharsForCode()
{
  return IsUnicodeEncodingMode() ? 4 : 2;
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

void TextBuffer_ResetPos(struct TTextBuffer* pTB, const int iMaxPos)
{
  pTB->m_iMaxPos = iMaxPos;
  pTB->m_iPos = 0;
}

void TextBuffer_Clear(struct TTextBuffer* pTB)
{
  TextBuffer_ResetPos(pTB, 0);
  pTB->m_ptr[0] = 0;
}

BOOL TextBuffer_Init(struct TTextBuffer* pTB, const int iSize)
{
  pTB->m_iSize = iSize;
  pTB->m_ptr = (LPSTR)MemAlloc(pTB->m_iSize);
  TextBuffer_ResetPos(pTB, iSize-1);
  return TRUE;
}

BOOL TextBuffer_Free(struct TTextBuffer* pTB)
{
  if (pTB->m_ptr)
  {
    MemFree((LPVOID)pTB->m_ptr);
  }
  pTB->m_iSize = 0;
  pTB->m_ptr = NULL;
  TextBuffer_ResetPos(pTB, 0);
  return TRUE;
}

BOOL TextBuffer_Update(struct TTextBuffer* pTB, LPSTR ptr, const int iSize)
{
  if (pTB->m_ptr != ptr)
  {
    TextBuffer_Free(pTB);
    pTB->m_iSize = iSize;
    pTB->m_ptr = (LPSTR)ptr;
  }
  TextBuffer_ResetPos(pTB, iSize-1);
  return TRUE;
}

BOOL TextBuffer_IsPosOKImpl(struct TTextBuffer* pTB, const int requiredChars)
{
  return pTB->m_iPos < pTB->m_iSize - 1 - requiredChars;
}

BOOL TextBuffer_IsPosOK(struct TTextBuffer* pTB)
{
  return TextBuffer_IsPosOKImpl(pTB, RequiredCharsForCode());
}

void TextBuffer_IncPos(struct TTextBuffer* pTB)
{
  ++pTB->m_iPos;
}

void TextBuffer_DecPos(struct TTextBuffer* pTB)
{
  --pTB->m_iPos;
}

char TextBuffer_GetChar(struct TTextBuffer* pTB)
{
  return pTB->m_ptr[pTB->m_iPos];
}

char TextBuffer_PopChar(struct TTextBuffer* pTB)
{
  const char ch = TextBuffer_GetChar(pTB);
  TextBuffer_IncPos(pTB);
  return ch;
}

BOOL TextBuffer_PushChar(struct TTextBuffer* pTB, const char ch)
{
  pTB->m_ptr[pTB->m_iPos] = ch;
  TextBuffer_IncPos(pTB);
  return TRUE;
}

BOOL TextBuffer_GetHexChar(struct TTextBuffer* pTB, char* pCh)
{
  *pCh = MIN_VALID_CHAR_CODE - 1;
  int charsProcessed = 0;
  while ((*pCh < MIN_VALID_CHAR_CODE) && TextBuffer_IsPosOKImpl(pTB, 1))
  {
    *pCh = TextBuffer_PopChar(pTB);
    ++charsProcessed;
  }
  const BOOL res = (*pCh >= MIN_VALID_CHAR_CODE);
  if (!res)
  {
    while (charsProcessed--)
    {
      TextBuffer_DecPos(pTB);
    }
  }
  return res;
}

BOOL TextBuffer_IsDataPortionAvailable(struct TTextBuffer* pTB, const long iRequiredChars, const BOOL bHexDigitsRequired)
{
  const long iRemainingChars = (pTB->m_iPos < pTB->m_iMaxPos) ? (pTB->m_iMaxPos - pTB->m_iPos) : 0;
  if (iRemainingChars >= iRequiredChars)
  {
    if (!bHexDigitsRequired
        || CheckRequiredHexDigitsAvailable(pTB->m_ptr, pTB->m_ptr + pTB->m_iPos, pTB->m_iMaxPos, iRequiredChars))
    {
      return TRUE;
    }
  }
  return FALSE;
}

void TextBuffer_NormalizeBeforeEncode(struct TTextBuffer* pTB)
{
  if (IsUnicodeEncodingMode())
  {
	  int cbDataWide = (pTB->m_iMaxPos * sizeof(WCHAR) + 1);
    LPWSTR lpDataWide = MemAlloc(cbDataWide);
    cbDataWide = MultiByteToWideChar(CP_UTF8, 0, pTB->m_ptr, pTB->m_iMaxPos, lpDataWide, cbDataWide/sizeof(WCHAR)) * sizeof(WCHAR);
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
    LPWSTR lpDataWide = MemAlloc(cbData * sizeof(WCHAR) + 1);
    int    cbDataWide = MultiByteToWideChar(CP_UTF8, 0, lpData, cbData, lpDataWide, (int)GlobalSize(lpDataWide) / sizeof(WCHAR));
    // Special cases: 42, 50220, 50221, 50222, 50225, 50227, 50229, 54936, 57002-11, 65000, 65001
    if (uCodePage == CP_UTF7 || uCodePage == 54936)
    {
      lpData = MemAlloc(GlobalSize(lpDataWide) * 2);
      cbData = WideCharToMultiByte(uCodePage, 0, lpDataWide, cbDataWide, lpData, (int)GlobalSize(lpData), NULL, NULL);
    }
    else
    {
      cbData = WideCharToMultiByte(uCodePage, WC_NO_BEST_FIT_CHARS, lpDataWide, cbDataWide, lpData, (int)GlobalSize(lpData), NULL, &bCancelDataLoss);
      if (!bCancelDataLoss)
      {
        cbData = WideCharToMultiByte(uCodePage, 0, lpDataWide, cbDataWide, lpData, (int)GlobalSize(lpData), NULL, NULL);
        bCancelDataLoss = FALSE;
      }
    }
    MemFree(lpDataWide);
    TextBuffer_Update(pTB, lpData, cbData);
  }
}

void TextBuffer_NormalizeAfterDecode(struct TTextBuffer* pTB)
{
  const UINT uCodePage = mEncoding[iEncoding].uCodePage;
  if (IsUnicodeEncodingMode())
  {
    LPSTR lpData = MemAlloc(pTB->m_iPos * 2 + 16);
	  const int cbData = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)pTB->m_ptr, pTB->m_iPos/sizeof(WCHAR), lpData, (int)GlobalSize(lpData), NULL, NULL);
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
    LPWSTR lpDataWide = MemAlloc(cbData);
    cbData = MultiByteToWideChar(uCodePage, 0, pTB->m_ptr, pTB->m_iPos, lpDataWide, cbData);
    pTB->m_iPos = WideCharToMultiByte(CP_UTF8, 0, lpDataWide, cbData, pTB->m_ptr, pTB->m_iPos, NULL, NULL);
    pTB->m_ptr[pTB->m_iPos] = 0;
    MemFree(lpDataWide);
  }
}

BOOL TextRange_Init(const StringSource* pSS, struct TTextRange* pTR)
{
  pTR->m_hwnd = pSS->hwnd;
  pTR->m_iSelStart = TSS_GetSelectionStart(pSS);
  pTR->m_iSelEnd = TSS_GetSelectionEnd(pSS);
  if (pTR->m_iSelStart == pTR->m_iSelEnd)
  {
    pTR->m_iSelStart = 0;
    pTR->m_iSelEnd = TSS_GetLength(pSS);
  };
  pTR->m_iPositionStart = pTR->m_iSelStart;
  pTR->m_iPositionCurrent = pTR->m_iSelStart;
  return pTR->m_iSelStart != pTR->m_iSelEnd;
}

BOOL TextRange_IsDataPortionAvailable(struct TTextRange* pTR)
{
  return pTR->m_iPositionCurrent < pTR->m_iSelEnd;
}

BOOL TextRange_GetNextDataPortion(const StringSource* pSS, struct TTextRange* pTR, struct TTextBuffer* pTB)
{
  const long iEnd = min(pTR->m_iPositionCurrent + pTB->m_iSize - 1, pTR->m_iSelEnd);
  if (GetText(pSS, pTB->m_ptr, pTR->m_iPositionCurrent, iEnd))
  {
    TextBuffer_ResetPos(pTB, iEnd - pTR->m_iPositionCurrent);
    pTR->m_iPositionStart = pTR->m_iPositionCurrent;
    pTR->m_iPositionCurrent = iEnd;
    return TRUE;
  }
  return FALSE;
}

BOOL EncodingSettings_Init(const StringSource* pSS, struct TEncodingData* pED, const BOOL bChar2Hex)
{
  if (!TextRange_Init(pSS, &pED->m_tr))
  {
    return FALSE;
  }
  pED->m_bChar2Hex = bChar2Hex;
  long iBufferSize = TEXT_BUFFER_SIZE_MIN;
  while ((pED->m_tr.m_iSelEnd > iBufferSize * 10) && (iBufferSize < TEXT_BUFFER_SIZE_MAX))
  {
    iBufferSize += TEXT_BUFFER_SIZE_MIN;
  }
  TextBuffer_Init(&pED->m_tb, iBufferSize);
  TextBuffer_Init(&pED->m_tbRes, iBufferSize * 4);
  TextBuffer_Init(&pED->m_tbTmp, iBufferSize * 4);
  return TRUE;
}

void EncodingSettings_Free(struct TEncodingData* pED)
{
  TextBuffer_Free(&pED->m_tb);
  TextBuffer_Free(&pED->m_tbRes);
  TextBuffer_Free(&pED->m_tbTmp);
}

BOOL CodeStrHex_Char2Hex(struct TEncodingData* pED)
{
  const char ch = TextBuffer_PopChar(&pED->m_tb);
  TextBuffer_PushChar(&pED->m_tbRes, HEX_DIGITS_UPPER[(ch >> 4) & 0xF]);
  TextBuffer_PushChar(&pED->m_tbRes, HEX_DIGITS_UPPER[ch & 0xF]);
  return TRUE;
}

BOOL CodeStrHex_Hex2Char(struct TEncodingData* pED)
{
  if (IsUnicodeEncodingMode())
  {
    char chEncoded1, chEncoded2, chEncoded3, chEncoded4;
    if (TextBuffer_GetHexChar(&pED->m_tb, &chEncoded1)
        && TextBuffer_GetHexChar(&pED->m_tb, &chEncoded2)
        && TextBuffer_GetHexChar(&pED->m_tb, &chEncoded3)
        && TextBuffer_GetHexChar(&pED->m_tb, &chEncoded4))
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
    if (TextBuffer_GetHexChar(&pED->m_tb, &chEncoded1)
        && TextBuffer_GetHexChar(&pED->m_tb, &chEncoded2))
    {
      const char chDecoded = IntByHexDigit(chEncoded1) * 16 + IntByHexDigit(chEncoded2);
      TextBuffer_PushChar(&pED->m_tbRes, chDecoded);
      return TRUE;
    }
    return FALSE;
  }
  return FALSE;
}

BOOL CodeStrHex_ProcessDataPortion(StringSource* pSS, struct TEncodingData* pED)
{
  BOOL bRes = TRUE;
  long iCursorOffset = 0;
  BOOL charsProcessed = FALSE;
  if (pED->m_bChar2Hex)
  {
    TextBuffer_NormalizeBeforeEncode(&pED->m_tb);
    while (TextBuffer_IsDataPortionAvailable(&pED->m_tb, 1, FALSE)
           && TextBuffer_IsPosOK(&pED->m_tbRes))
    {
      charsProcessed |= CodeStrHex_Char2Hex(pED);
    }
    iCursorOffset = pED->m_tbRes.m_iPos - (pED->m_tr.m_iPositionCurrent - pED->m_tr.m_iPositionStart);
    if (!TextBuffer_IsPosOK(&pED->m_tbRes))
    {
      const int iEncodedChars = pED->m_tbRes.m_iPos / RequiredCharsForCode();
      pED->m_tr.m_iPositionCurrent = pED->m_tr.m_iPositionStart + iEncodedChars;
      iCursorOffset = pED->m_tbRes.m_iPos - iEncodedChars;
    }
  }
  else
  {
    while (TextBuffer_IsDataPortionAvailable(&pED->m_tb, RequiredCharsForCode(), FALSE))
    {
      charsProcessed |= CodeStrHex_Hex2Char(pED);
    }
    if (charsProcessed)
    {
      pED->m_tr.m_iPositionCurrent += pED->m_tb.m_iPos - pED->m_tb.m_iMaxPos;
      TextBuffer_NormalizeAfterDecode(&pED->m_tbRes);
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
          if (!IsHexDigit(TextBuffer_GetChar(&pED->m_tb))
              || !CheckRequiredHexDigitsAvailable(pED->m_tb.m_ptr, pED->m_tb.m_ptr + pED->m_tb.m_iPos, pED->m_tb.m_iMaxPos, RequiredCharsForCode()))
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
      SendMessage(pED->m_tr.m_hwnd, SCI_SETSEL, pED->m_tr.m_iPositionStart, pED->m_tr.m_iPositionCurrent);
      SendMessage(pED->m_tr.m_hwnd, SCI_REPLACESEL, 0, (LPARAM)pED->m_tbRes.m_ptr);
    }
    else
    {
      strncpy_s(pSS->result + pED->m_tr.m_iPositionStart,
                sizeof(pSS->result) - pED->m_tr.m_iPositionStart,
                pED->m_tbRes.m_ptr, _TRUNCATE);
    }
    pED->m_tr.m_iPositionCurrent += iCursorOffset;
    pED->m_tr.m_iSelEnd += iCursorOffset;
  }

  TextBuffer_Clear(&pED->m_tb);
  TextBuffer_Clear(&pED->m_tbRes);
  TextBuffer_Clear(&pED->m_tbTmp);
  n2e_AdjustProgressBarInStatusBar(pED->m_tr.m_iPositionCurrent, pED->m_tr.m_iSelEnd);

  return bRes;
}

void CodeStrHex(StringSource* pSS, const BOOL bChar2Hex)
{
  if (pSS->hwnd)
  {
    SendMessage(pSS->hwnd, WM_SETREDRAW, (WPARAM)FALSE, 0);
    SendMessage(pSS->hwnd, SCI_BEGINUNDOACTION, 0, 0);
  }
  struct TEncodingData ed;
  if (!EncodingSettings_Init(pSS, &ed, bChar2Hex))
  {
    return;
  }
  n2e_ShowProgressBarInStatusBar(bChar2Hex ? L"String to Hex..." : L"Hex to String...", 0, ed.m_tr.m_iSelEnd - ed.m_tr.m_iSelStart);
  BOOL bProcessFailed = FALSE;
  while (TextRange_IsDataPortionAvailable(&ed.m_tr))
  {
    if (TextRange_GetNextDataPortion(pSS, &ed.m_tr, &ed.m_tb))
    {
      if (!CodeStrHex_ProcessDataPortion(pSS, &ed))
      {
        bProcessFailed = TRUE;
        break;
      }
    }
  }
  if (bProcessFailed)
  {
    ed.m_tr.m_iSelEnd = ed.m_tr.m_iPositionCurrent;
  }
  if (pSS->hwnd)
  {
    SendMessage(pSS->hwnd, SCI_LINESCROLL, -ed.m_tr.m_iSelEnd, 0);
    SendMessage(pSS->hwnd, SCI_SETSEL, ed.m_tr.m_iSelStart, ed.m_tr.m_iSelEnd);
    SendMessage(pSS->hwnd, SCI_ENDUNDOACTION, 0, 0);
    SendMessage(pSS->hwnd, WM_SETREDRAW, (WPARAM)TRUE, 0);
    InvalidateRect(pSS->hwnd, NULL, FALSE);
    UpdateWindow(pSS->hwnd);
  }
  EncodingSettings_Free(&ed);
  n2e_HideProgressBarInStatusBar();
}

static StringSource ss = { 0 };

LPCSTR EncodeStringToHex(LPCSTR text, const int encoding)
{
  iEncoding = encoding;
  strncpy_s(ss.text, sizeof(ss.text), text, _TRUNCATE);  
  CodeStrHex(&ss, TRUE);
  return ss.result;
}

// LPCSTR EncodeStringToHex(LPCWSTR text, const int encoding)
// {
//   iEncoding = encoding;
//   strncpy_s(ss.text, sizeof(ss.text), text, _TRUNCATE);
//   CodeStrHex(&ss, TRUE);
//   return ss.result;
// }

void DecodeHexToString(LPCSTR text)
{
  strncpy_s(ss.text, sizeof(ss.text), text, _TRUNCATE);
  CodeStrHex(&ss, FALSE);
}

void EncodeStrToHex(const HWND hwnd)
{
  ss.hwnd = hwnd;
  CodeStrHex(&ss, TRUE);
}

void DecodeHexToStr(const HWND hwnd)
{
  ss.hwnd = hwnd;
  CodeStrHex(&ss, FALSE);
}

#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus