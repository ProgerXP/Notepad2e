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
};

struct TEncodingData
{
  struct TTextRange m_tr;
  struct TTextBuffer m_tb;
  struct TTextBuffer m_tbRes;
  struct TTextBuffer m_tbTmp;
  BOOL m_bChar2Hex;
};

#define MAX_INPUT_STRING_LENGTH 512*1024

struct TStringSource
{
  HWND hwnd;
  char text[MAX_INPUT_STRING_LENGTH];
  char result[MAX_INPUT_STRING_LENGTH*4];
};

typedef struct TStringSource StringSource;

LPCSTR EncodeStringToHex(LPCSTR text, const int encoding);
//LPCSTR EncodeStringToHex(LPCWSTR text, const int encoding);
void DecodeHexToString(LPCSTR text);
void EncodeStrToHex(const HWND hwnd);
void DecodeHexToStr(const HWND hwnd);

#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus