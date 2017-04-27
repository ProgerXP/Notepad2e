#pragma once
#include "stdafx.h"

#include "scintilla.h"
#include "helpers.h"
#include "resource.h"
#include "Notepad2.h"
#include "Dialogs.h"
#include "Edit.h"
#include "HLSelection.h"
#include "Styles.h"
#include "StrToHex.h"
#include "Utils.h"

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
//extern  ELanguageIndicatorMode iShowLanguageInTitle;
//extern	WCHAR		_hl_last_run[HL_MAX_PATH_N_CMD_LINE];

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
void HL_Msg_create();
BOOL HL_Open_nextFs_file(HWND hwnd, LPCWSTR file, BOOL next);
void HL_Unwrap_selection(HWND hwnd, BOOL quote_mode);
void HL_Escape_html(HWND hwnd);
void UpdateFindIcon(const BOOL findOK);
void ResetFindIcon();
void  EditString2Hex(HWND);
void  EditHex2String(HWND);
