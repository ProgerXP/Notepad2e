#pragma once
#include <wtypes.h>
#include "Scintilla.h"

extern HWND hwndEdit;
extern int iEncoding;
extern int iEOLMode;
extern DWORD dwLastIOError;
BOOL FileIO(BOOL, LPCWSTR, BOOL, int*, int*, BOOL*, BOOL*, BOOL*, BOOL);

#ifndef N2E_TESTING

#include "Edit.h"
#include "Utils.h"
extern NP2ENCODING mEncoding[];
extern HANDLE g_hScintilla;

#else // ifndef N2E_TESTING

#include <wtypes.h>

#define NCP_DEFAULT            1
#define NCP_UTF8               2
#define NCP_UTF8_SIGN          4
#define NCP_UNICODE            8
#define NCP_UNICODE_REVERSE   16
#define NCP_UNICODE_BOM       32
#define NCP_8BIT              64
#define NCP_INTERNAL          (NCP_DEFAULT|NCP_UTF8|NCP_UTF8_SIGN|NCP_UNICODE|NCP_UNICODE_REVERSE|NCP_UNICODE_BOM)
#define NCP_RECODE           128
#define CPI_NONE              -1
#define CPI_DEFAULT            0
#define CPI_OEM                1
#define CPI_UNICODEBOM         2
#define CPI_UNICODEBEBOM       3
#define CPI_UNICODE            4
#define CPI_UNICODEBE          5
#define CPI_UTF8               6
#define CPI_UTF8SIGN           7
#define CPI_UTF7               8


typedef struct _np2encoding
{
  UINT    uFlags;
  UINT    uCodePage;
  char*   pszParseNames;
  int     idsName;
  WCHAR   wchLabel[32];
} NP2ENCODING;

extern NP2ENCODING mEncoding[];

void n2e_ShowProgressBarInStatusBar(LPCWSTR pProgressText, const long nCurPos, const long nMaxPos);
void n2e_HideProgressBarInStatusBar();
void n2e_IncProgressBarPosInStatusBar(const long nOffset);

extern WCHAR szIniFile[MAX_PATH];

#endif
