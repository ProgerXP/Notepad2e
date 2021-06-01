#include <Windows.h>
#include <assert.h>
#include "Externals.h"
#include "StrToBase64.h"
#include "StringRecoding.h"

extern StringSource ss = { 0 };
extern RecodingAlgorithm ra = { 0 };

static const unsigned char base64_table[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

BOOL CheckRequiredBase64DigitsAvailable(LPCSTR pCurTextOrigin, LPCSTR pCurText, const long iCurTextLength, const long nChars)
{
  BOOL res = FALSE;
  if (iCurTextLength - (pCurText - pCurTextOrigin) >= nChars)
  {
    long i = 0;
    res = TRUE;
    for (i = 0; i < nChars; ++i)
    {
      res &= (strchr(base64_table, pCurText[i]) != 0);
      if (!res)
      {
        break;
      }
    }
  }
  return res;
}

BOOL Base64_IsValidSequence(EncodingData* pED, const int requiredChars)
{
  return CheckRequiredBase64DigitsAvailable(pED->m_tb.m_ptr, pED->m_tb.m_ptr + pED->m_tb.m_iPos, pED->m_tb.m_iMaxPos, requiredChars);
}

static Base64Data b64data = { 0 };

LPVOID Base64_InitAlgorithmData(const BOOL isEncoding)
{
  if (!isEncoding)
  {
    if (!b64data.initialized)
    {
      memset(b64data.dtable, 0x80, 256);
      for (int i = 0; i < sizeof(base64_table) - 1; i++)
      {
        b64data.dtable[base64_table[i]] = (unsigned char)i;
      }
      b64data.dtable['='] = 0;
      b64data.initialized = 1;
    }
    return (LPVOID)&b64data;
  }
  else
  {
    return NULL;
  }
}

void Base64_ReleaseAlgorithmData(LPVOID pData)
{
}

long Base64_GetEncodedCharLength(unsigned char ch)
{
  if (Is8BitEncodingMode())
  {
    return isascii(ch) ? 1 : 2;
  }
  return 1;
}

BOOL Base64_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  unsigned char chInput[4] = { 0 };
  for (int i = 0; i < _countof(chInput) - 1; ++i)
  {
    chInput[i] = TextBuffer_PopChar(&pED->m_tb);
  }
  TextBuffer_PushChar(&pED->m_tbRes, base64_table[chInput[0] >> 2]);
  TextBuffer_PushChar(&pED->m_tbRes, base64_table[((chInput[0] & 0x03) << 4) | (chInput[1] >> 4)]);
  TextBuffer_PushChar(&pED->m_tbRes, base64_table[((chInput[1] & 0x0f) << 2) | (chInput[2] >> 6)]);
  TextBuffer_PushChar(&pED->m_tbRes, base64_table[chInput[2] & 0x3f]);

  if (piCharsProcessed)
  {
    int iCharsProcessed = 0;
    if (Is8BitEncodingMode())
    {
      for (int i = 0; i < _countof(chInput) - 1; ++i)
      {
        iCharsProcessed += Base64_GetEncodedCharLength(chInput[i]);
      }
    }
    else
    {
      iCharsProcessed = _countof(chInput) - 1;
    }
    (*piCharsProcessed) += iCharsProcessed;
  }
  return TRUE;
}

BOOL Base64_EncodeTail(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  long iCharsProcessed = 0;
  unsigned char chInput[4] = { 0 };
  int i = 0;

  chInput[0] = TextBuffer_PopChar(&pED->m_tb);
  iCharsProcessed += Base64_GetEncodedCharLength(chInput[0]);
  TextBuffer_PushChar(&pED->m_tbRes, base64_table[chInput[0] >> 2]);

  if (TextBuffer_GetTailLength(&pED->m_tb) == 0)
  {
    TextBuffer_PushChar(&pED->m_tbRes, base64_table[(chInput[0] & 0x03) << 4]);
    TextBuffer_PushChar(&pED->m_tbRes, '=');
  }
  else
  {
    chInput[1] = TextBuffer_PopChar(&pED->m_tb);
    iCharsProcessed += Base64_GetEncodedCharLength(chInput[1]);
    TextBuffer_PushChar(&pED->m_tbRes, base64_table[((chInput[0] & 0x03) << 4) | (chInput[1] >> 4)]);
    TextBuffer_PushChar(&pED->m_tbRes, base64_table[(chInput[1] & 0x0f) << 2]);
  }
  TextBuffer_PushChar(&pED->m_tbRes, '=');

  if (piCharsProcessed)
  {
    (*piCharsProcessed) += iCharsProcessed;
  }
  return TRUE;
}

BOOL Base64_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  Base64Data* pData = (Base64Data*)pRA->data;
  assert(pData);

  int iCharsProcessed = 0;
  unsigned char chInput[5] = { 0 };
  unsigned char block[5] = { 0 };
  int pad = 0;
  for (int i = 0; i < _countof(chInput) - 1; ++i)
  {
    if (!TextBuffer_GetLiteralChar(&pED->m_tb, &chInput[i], &iCharsProcessed))
    {
      return FALSE;
    }
    block[i] = pData->dtable[chInput[i]];
    if (chInput[i] == '=')
    {
      pad++;
    }
  }
  TextBuffer_PushChar(&pED->m_tbRes, (block[0] << 2) | (block[1] >> 4));
  switch (pad)
  {
  case 0:
    TextBuffer_PushChar(&pED->m_tbRes, (block[1] << 4) | (block[2] >> 2));
    TextBuffer_PushChar(&pED->m_tbRes, (block[2] << 6) | block[3]);
    break;
  case 1:
    TextBuffer_PushChar(&pED->m_tbRes, (block[1] << 4) | (block[2] >> 2));
    break;
  case 2:
    break;
  default:
    assert(FALSE);
    return FALSE;   // Invalid padding
  }

  if (piCharsProcessed)
  {
    (*piCharsProcessed) += iCharsProcessed;
  }
  return TRUE;
}

LPCSTR EncodeStringToBase64(LPCSTR text, const int textLength, const int encoding,
  const int additionalData1, const int additionalData2, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorithm_Init(&ra, ERT_BASE64, TRUE, additionalData1, additionalData2);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorithm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

LPCSTR DecodeBase64ToString(LPCSTR text, const int textLength, const int encoding,
  const int additionalData1, const int additionalData2, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorithm_Init(&ra, ERT_BASE64, FALSE, additionalData1, additionalData2);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorithm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

void EncodeStrToBase64(const HWND hwnd)
{
  RecodingAlgorithm_Init(&ra, ERT_BASE64, TRUE, 0, 0);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorithm_Release(&ra);
}

void DecodeBase64ToStr(const HWND hwnd)
{
  RecodingAlgorithm_Init(&ra, ERT_BASE64, FALSE, 0, 0);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorithm_Release(&ra);
}
