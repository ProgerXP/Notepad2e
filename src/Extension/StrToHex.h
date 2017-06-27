#pragma once
#include "stdafx.h"

#define MIN_VALID_CHAR_CODE 0x21

#define TEXT_BUFFER_SIZE_MIN  65536
#define TEXT_BUFFER_SIZE_MAX  TEXT_BUFFER_SIZE_MIN * 10

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

void EncodeStrToHex(const HWND hwnd);
void DecodeHexToStr(const HWND hwnd);
