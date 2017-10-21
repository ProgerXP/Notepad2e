#pragma once
#include <WTypes.h>
#include "Externals.h"

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

struct TTextBuffer
{
  int m_iSize;
  LPSTR m_ptr;
  int m_iMaxPos;
  int m_iPos;
};

struct TTextRange
{
  HWND m_hwnd;
  long m_iSelStart;
  long m_iSelEnd;
  long m_iPositionStart;
  long m_iPositionCurrent;
  long m_iExpectedProcessedChars;
};

struct TEncodingData
{
  struct TTextRange m_tr;
  struct TTextBuffer m_tb;
  struct TTextBuffer m_tbRes;
  struct TTextBuffer m_tbTmp;
  BOOL m_bChar2Hex;
};

#define MAX_TEST_STRING_LENGTH 1200000

struct TStringSource
{
  HWND hwnd;
  char text[MAX_TEST_STRING_LENGTH];
  char result[MAX_TEST_STRING_LENGTH];
  int iTextLength;
  int iProcessedChars;
};

typedef struct TStringSource StringSource;

LPCSTR EncodeStringToHex(LPCSTR text, const int encoding, const int bufferSize);
LPCSTR DecodeHexToString(LPCSTR text, const int encoding, const int bufferSize);
void EncodeStrToHex(const HWND hwnd);
void DecodeHexToStr(const HWND hwnd);

#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus