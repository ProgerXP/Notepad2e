#pragma once
/******************************************************************************
*
*
* Notepad2
*
* Styles.h
*   Scintilla Style Management
*
* See Readme.txt for more information about this source code.
* Please send me your comments to this work.
*
* See License.txt for details about distribution and modification.
*
*                                              (c) Florian Balmer 1996-2011
*                                                  florian.balmer@gmail.com
*                                               http://www.flos-freeware.ch
*
*
******************************************************************************/

#include "Extension/Lexers.h"

void   Style_Load();
void   Style_Save();
BOOL   Style_Import(HWND);
BOOL   Style_Export(HWND);
void   Style_SetLexer(HWND, PEDITLEXER);
void   Style_SetLongLineColors(HWND);
void   Style_SetCurrentLineBackground(HWND);
void   Style_SetLexerFromFile(HWND, LPCWSTR);
void   Style_SetLexerFromName(HWND, LPCWSTR, LPCWSTR);
void   Style_SetDefaultLexer(HWND);
void   Style_SetHTMLLexer(HWND);
void   Style_SetXMLLexer(HWND);
void   Style_SetLexerFromID(HWND, int);
void   Style_SetDefaultFont(HWND);
void   Style_ToggleUse2ndDefault(HWND);
BOOL   Style_GetUse2ndDefault(HWND);
void   Style_SetIndentGuides(HWND);
BOOL   Style_GetOpenDlgFilterStr(LPWSTR, int);
BOOL   Style_StrGetFont(LPCWSTR, LPWSTR, int);
BOOL   Style_StrGetFontQuality(LPCWSTR, LPWSTR, int);
BOOL   Style_StrGetCharSet(LPCWSTR, int*);
BOOL   Style_StrGetSize(LPCWSTR, int*);
BOOL   Style_StrGetSizeStr(LPCWSTR, LPWSTR, int);
BOOL   Style_StrGetColor(BOOL, LPCWSTR, int*);
BOOL   Style_StrGetCase(LPCWSTR, int*);
BOOL   Style_StrGetAlpha(LPCWSTR, int*);
BOOL   Style_SelectFont(HWND, LPWSTR, int, BOOL);
BOOL   Style_SelectColor(HWND, BOOL, LPWSTR, int);
void   Style_SetStyles(HWND, int, int, LPCWSTR);
void   Style_SetFontQuality(HWND, LPCWSTR);
void   Style_GetCurrentLexerName(LPWSTR, int);
int    Style_GetLexerIconId(PEDITLEXER);
void   Style_AddLexerToTreeView(HWND, PEDITLEXER);
INT_PTR CALLBACK Styles_ConfigDlgProc(HWND, UINT, WPARAM, LPARAM);
void   Style_ConfigDlg(HWND);
INT_PTR CALLBACK Style_SelectLexerDlgProc(HWND, UINT, WPARAM, LPARAM);
void   Style_SelectLexerDlg(HWND);
