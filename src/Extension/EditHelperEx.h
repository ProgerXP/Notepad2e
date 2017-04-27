#pragma once
#include "stdafx.h"

#include "scintilla.h"
#include "helpers.h"
#include "resource.h"
#include <cassert>
#include "Notepad2.h"
#include "Edit.h"
#include "HLSelection.h"
#include "StrToHex.h"

#ifdef __cplusplus
extern "C" {
#endif
  extern NP2ENCODING mEncoding[];
  extern	int       iEncoding;
  extern int iFindWordMatchCase;
  extern int iFindWordWrapAround;
  extern TBBUTTON  tbbMainWnd[];
  extern  HWND      hwndToolbar;
  extern	BOOL		b_HL_highlight_selection;
  extern	BOOL		b_HL_highlight_all;
  extern	BOOL		b_Hl_use_prefix_in_open_dialog;
  extern	BOOL		b_HL_edit_selection;
  extern	BOOL		b_HL_ctrl_wheel_scroll;
  extern  BOOL    bMoveCaretOnRightClick;
  extern  int     iEvaluateMathExpression;
  extern  int     iWordNavigationMode;
  extern  BOOL    bTabsAsSpaces;
  extern  BOOL    bTabIndents;
  extern  BOOL    bBackspaceUnindents;
  extern  int     iTabWidth;
  extern  int     iIndentWidth;
  extern  BOOL    bShowIndentGuides;
  extern  int     iWordWrapMode;
  extern  int     fWordWrap;
  extern  int     iWordWrapIndent;
  extern  BOOL    bShowWordWrapSymbols;
  extern  int     iWordWrapSymbols;
  extern  BOOL    bMarkLongLines;
  extern  int     iLongLineMode;
  extern  int     iLongLinesLimit;
  extern  BOOL    bShowSelectionMargin;
  extern  BOOL    bViewWhiteSpace;
  extern  BOOL    bViewEOLs;
  
  #define ICON_FIND_OK 9
  #define ICON_FIND_FAILED 26
  #define FIND_INFO_INDEX 12

  BOOL HL_Explorer_cxt_menu(LPCWSTR path, void *parentWindow);
  int isValidRegex(LPCSTR str);

#ifdef __cplusplus
}//end extern "C"
#endif
