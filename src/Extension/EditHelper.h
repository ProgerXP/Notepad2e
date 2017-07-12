#pragma once
#include "stdafx.h"

#include "scintilla.h"
#include "helpers.h"
#include "resource.h"
#include "Notepad2.h"
#include "Dialogs.h"
#include "Edit.h"
#include "ExtSelection.h"
#include "Styles.h"
#include "StrToHex.h"
#include "Utils.h"

extern NP2ENCODING mEncoding[];
extern	int       iEncoding;
extern int iFindWordMatchCase;
extern int iFindWordWrapAround;
extern TBBUTTON  tbbMainWnd[];
extern  HWND      hwndToolbar;
extern	BOOL		bHighlightSelection;
extern	BOOL		_n2e_highlight_all;
extern	BOOL		bUsePrefixInOpenDialog;
extern	BOOL		_n2e_edit_selection;
extern	BOOL		bCtrlWheelScroll;
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

extern WCHAR	n2e_last_html_tag[0xff];
extern WCHAR	n2e_last_html_end_tag[0xff];

void n2e_StripHTMLTags(HWND hwnd);
void n2e_JumpToOffset(HWND hwnd, int iNewPos);
void n2e_EditInsertNewLine(HWND hwnd, BOOL insertAbove);
BOOL n2e_IsSelectionModeValid(HWND hwnd);
void n2e_FindNextWord(HWND hwnd, LPCEDITFINDREPLACE lpref, BOOL next);
int n2e_FindTextImpl(const HWND hwnd, const int searchFlags, struct TextToFind* pttf);
BOOL n2e_CheckTextExists(const HWND hwnd, const int searchFlags, const struct TextToFind* pttf, const int iPos);
void n2e_MsgCreate();
BOOL n2e_OpenNextFile(HWND hwnd, LPCWSTR file, BOOL next);
void n2e_UnwrapSelection(HWND hwnd, BOOL quote_mode);
void n2e_EscapeHTML(HWND hwnd);
void n2e_UpdateFindIcon(const BOOL findOK);
void n2e_ResetFindIcon();
void n2e_EditString2Hex(HWND);
void n2e_EditHex2String(HWND);

void n2e_SaveCheckboxes(HWND hwnd);
BOOL n2e_IsCheckboxChecked(HWND hwnd, const UINT nCtrlID, const BOOL bCheckRestoredState);
void n2e_EditFindReplaceUpdateCheckboxes(HWND hwnd, const UINT nCtrlID);
void n2e_EditFindReplaceInitialUpdateCheckboxes(HWND hwnd);

BOOL n2e_IsSubclassedEditInCombo(const HWND hwnd);
BOOL n2e_SubclassEditInCombo(const HWND hwnd, const UINT idCombo);
