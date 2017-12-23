#include <Windows.h>
#include "Externals.h"
#include "StrToBase64.h"
#include "StringRecoding.h"
#include "base64.h"

extern StringSource ss = { 0 };
extern RecodingAlgorythm ra = { 0 };

LPCSTR EncodeStringToBase64(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorythm_Init(&ra, ERT_BASE64, TRUE);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorythm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

LPCSTR DecodeBase64ToString(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorythm_Init(&ra, ERT_BASE64, FALSE);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorythm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

void EncodeStrToBase64(const HWND hwnd)
{
  RecodingAlgorythm_Init(&ra, ERT_BASE64, TRUE);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorythm_Release(&ra);
}

void DecodeBase64ToStr(const HWND hwnd)
{
  RecodingAlgorythm_Init(&ra, ERT_BASE64, FALSE);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorythm_Release(&ra);
}
