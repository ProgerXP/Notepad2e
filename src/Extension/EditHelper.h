#pragma once

#define _WIN32_WINNT 0x501
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <string.h>
#include "scintilla.h"
#include "helpers.h"
#include "resource.h"
#include <cassert>
#include "Notepad2.h"
#include "Edit.h"
#include "HLSelection.h"
#include "StrToHex.h"

#define HL_IS_LITERAL(CH) hl_iswordchar(CH)
#define HL_IS_SPACE(CH) hl_isspace(CH)

#ifdef __cplusplus
extern "C" {
#endif
  extern NP2ENCODING mEncoding[];
  extern	int       iEncoding;
  extern int iFindWordMatchCase;
  extern int iFindWordWrapAround;
  extern TBBUTTON  tbbMainWnd[];
  extern  HWND      hwndToolbar;

  #define ICON_FIND_OK 9
  #define ICON_FIND_FAILED 26
  #define FIND_INFO_INDEX 12

  extern WCHAR	hl_last_html_tag[0xff];
  extern WCHAR	hl_last_html_end_tag[0xff];

  void HL_Strip_html_tags(HWND hwnd);
  void HL_Jump_offset(HWND hwnd, int iNewPos);
  void EditInsertNewLine(HWND hwnd, BOOL insertAbove);
  BOOL IsSelectionModeValid(HWND hwnd);
  void HL_Find_next_word(HWND hwnd, LPCEDITFINDREPLACE lpref, BOOL next);
  int FindTextImpl(const HWND hwnd, const int searchFlags, struct TextToFind* pttf);
  BOOL CheckTextExists(const HWND hwnd, const int searchFlags, const struct TextToFind* pttf, const int iPos);
  BOOL HL_Open_nextFs_file(HWND hwnd, LPCWSTR file, BOOL next);
  void HL_Unwrap_selection(HWND hwnd, BOOL quote_mode);
  void HL_Escape_html(HWND hwnd);
  void UpdateFindIcon(const BOOL findOK);
  void ResetFindIcon();

#ifdef __cplusplus
}//end extern "C"
#endif
