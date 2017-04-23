#pragma once

#include <windows.h>
#include <shlwapi.h>

#define HL_IS_LITERAL(CH) hl_iswordchar(CH)
#define HL_IS_SPACE(CH) hl_isspace(CH)

#ifdef __cplusplus
extern "C" {
#endif
  extern WCHAR	hl_last_html_tag[0xff];
  extern WCHAR	hl_last_html_end_tag[0xff];

  void HL_Strip_html_tags(HWND hwnd);
  void EditInsertNewLine(HWND hwnd, BOOL insertAbove);
  BOOL IsSelectionModeValid(HWND hwnd);
#ifdef __cplusplus
}//end extern "C"
#endif
