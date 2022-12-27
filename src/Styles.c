/******************************************************************************
*
*
* Notepad2
*
* Styles.c
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
#define _WIN32_WINNT 0x501
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include "scintilla.h"
#include "scilexer.h"
#include "notepad2.h"
#include "edit.h"
#include "styles.h"
#include "dialogs.h"
#include "helpers.h"
#include "resource.h"
#include "Extension/DPIHelper.h"
#include "Extension/SciCall.h"
#include "Extension/SplitterWnd.h"
#include "Extension/Utils.h"


COLORREF crCustom[16];
BOOL bUse2ndDefaultStyle;
BOOL fStylesModified = FALSE;
BOOL fWarnedNoIniFile = FALSE;
BOOL fIsConsolasAvailable = FALSE;
int iBaseFontSize = 10;
int iDefaultLexer;
BOOL bAutoSelect;
int cxStyleSelectDlg;
int cyStyleSelectDlg;
extern int  iDefaultCodePage;
extern int  iDefaultCharSet;
extern BOOL bHighlightCurrentLine;


//=============================================================================
//
//  Style_Load()
//
void Style_Load()
{
  int i, iLexer;
  WCHAR tch[32];
  WCHAR *pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * 32 * 1024);
  int   cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);

  // Custom colors
  crCustom[0] = RGB(0x00, 0x00, 0x00);
  crCustom[1] = RGB(0x0A, 0x24, 0x6A);
  crCustom[2] = RGB(0x3A, 0x6E, 0xA5);
  crCustom[3] = RGB(0x00, 0x3C, 0xE6);
  crCustom[4] = RGB(0x00, 0x66, 0x33);
  crCustom[5] = RGB(0x60, 0x80, 0x20);
  crCustom[6] = RGB(0x64, 0x80, 0x00);
  crCustom[7] = RGB(0xA4, 0x60, 0x00);
  crCustom[8] = RGB(0xFF, 0xFF, 0xFF);
  crCustom[9] = RGB(0xFF, 0xFF, 0xE2);
  crCustom[10] = RGB(0xFF, 0xF1, 0xA8);
  crCustom[11] = RGB(0xFF, 0xC0, 0x00);
  crCustom[12] = RGB(0xFF, 0x40, 0x00);
  crCustom[13] = RGB(0xC8, 0x00, 0x00);
  crCustom[14] = RGB(0xB0, 0x00, 0xB0);
  crCustom[15] = RGB(0xB2, 0x8B, 0x40);

  LoadIniSection(L"Custom Colors", pIniSection, cchIniSection);
  for (i = 0; i < 16; i++)
  {
    int itok;
    int irgb;
    WCHAR wch[32];
    wsprintf(tch, L"%02i", i + 1);
    if (IniSectionGetString(pIniSection, tch, L"", wch, COUNTOF(wch)))
    {
      if (wch[0] == L'#')
      {
        itok = swscanf(CharNext(wch), L"%x", &irgb);
        if (itok == 1)
          crCustom[i] = RGB((irgb & 0xFF0000) >> 16, (irgb & 0xFF00) >> 8, irgb & 0xFF);
      }
    }
  }

  LoadIniSection(L"Styles", pIniSection, cchIniSection);
  // 2nd default
  bUse2ndDefaultStyle = (IniSectionGetInt(pIniSection, L"Use2ndDefaultStyle", 0)) ? 1 : 0;

  // default scheme
  iDefaultLexer = IniSectionGetInt(pIniSection, L"DefaultScheme", 0);
  iDefaultLexer = min(max(iDefaultLexer, 0), NUMLEXERS - 1);

  // auto select
  bAutoSelect = (IniSectionGetInt(pIniSection, L"AutoSelect", 1)) ? 1 : 0;

  // scheme select dlg dimensions
  cxStyleSelectDlg = IniSectionGetInt(pIniSection, L"SelectDlgSizeX", 304);
  cxStyleSelectDlg = max(cxStyleSelectDlg, 0);

  cyStyleSelectDlg = IniSectionGetInt(pIniSection, L"SelectDlgSizeY", 0);
  cyStyleSelectDlg = max(cyStyleSelectDlg, 324);

  for (iLexer = 0; iLexer < NUMLEXERS; iLexer++)
  {
    LoadIniSection(pLexArray[iLexer]->pszName, pIniSection, cchIniSection);
    if (!IniSectionGetString(pIniSection, L"FileNameExtensions", pLexArray[iLexer]->pszDefExt,
                             pLexArray[iLexer]->szExtensions, COUNTOF(pLexArray[iLexer]->szExtensions)))
      lstrcpyn(pLexArray[iLexer]->szExtensions, pLexArray[iLexer]->pszDefExt,
               COUNTOF(pLexArray[iLexer]->szExtensions));
    i = 0;
    while (pLexArray[iLexer]->Styles[i].iStyle != -1)
    {
      IniSectionGetString(pIniSection, pLexArray[iLexer]->Styles[i].pszName,
                          pLexArray[iLexer]->Styles[i].pszDefault,
                          pLexArray[iLexer]->Styles[i].szValue,
                          COUNTOF(pLexArray[iLexer]->Styles[i].szValue));
      i++;
    }
  }
  LocalFree(pIniSection);
}


//=============================================================================
//
//  Style_Save()
//
void Style_Save()
{
  int i, iLexer;
  WCHAR tch[32];
  WCHAR *pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * 32 * 1024);
  int   cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);

  // Custom colors
  for (i = 0; i < 16; i++)
  {
    WCHAR wch[32];
    wsprintf(tch, L"%02i", i + 1);
    wsprintf(wch, L"#%02X%02X%02X",
             (int)GetRValue(crCustom[i]), (int)GetGValue(crCustom[i]), (int)GetBValue(crCustom[i]));
    IniSectionSetString(pIniSection, tch, wch);
  }
  SaveIniSection(L"Custom Colors", pIniSection);
  ZeroMemory(pIniSection, cchIniSection);

  // auto select
  IniSectionSetInt(pIniSection, L"Use2ndDefaultStyle", bUse2ndDefaultStyle);

  // default scheme
  IniSectionSetInt(pIniSection, L"DefaultScheme", iDefaultLexer);

  // auto select
  IniSectionSetInt(pIniSection, L"AutoSelect", bAutoSelect);

  // scheme select dlg dimensions
  IniSectionSetInt(pIniSection, L"SelectDlgSizeX", cxStyleSelectDlg);
  IniSectionSetInt(pIniSection, L"SelectDlgSizeY", cyStyleSelectDlg);

  SaveIniSection(L"Styles", pIniSection);

  if (!fStylesModified)
  {
    LocalFree(pIniSection);
    return;
  }

  ZeroMemory(pIniSection, cchIniSection);
  for (iLexer = 0; iLexer < NUMLEXERS; iLexer++)
  {
    IniSectionSetString(pIniSection, L"FileNameExtensions", pLexArray[iLexer]->szExtensions);
    i = 0;
    while (pLexArray[iLexer]->Styles[i].iStyle != -1)
    {
      IniSectionSetString(pIniSection, pLexArray[iLexer]->Styles[i].pszName, pLexArray[iLexer]->Styles[i].szValue);
      i++;
    }
    SaveIniSection(pLexArray[iLexer]->pszName, pIniSection);
    ZeroMemory(pIniSection, cchIniSection);
  }
  LocalFree(pIniSection);
}


//=============================================================================
//
//  Style_Import()
//
BOOL Style_Import(HWND hwnd)
{
  WCHAR szFile[MAX_PATH * 2] = L"";
  WCHAR szFilter[256];
  OPENFILENAME ofn;

  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  GetString(IDS_FILTER_INI, szFilter, COUNTOF(szFilter));
  PrepareFilterStr(szFilter);

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrFile = szFile;
  ofn.lpstrDefExt = L"ini";
  ofn.nMaxFile = COUNTOF(szFile);
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
            | OFN_PATHMUSTEXIST | OFN_SHAREAWARE;

  if (GetOpenFileName(&ofn))
  {
    int i, iLexer;
    WCHAR *pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * 32 * 1024);
    int   cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);
    for (iLexer = 0; iLexer < NUMLEXERS; iLexer++)
    {
      if (GetPrivateProfileSection(pLexArray[iLexer]->pszName, pIniSection, cchIniSection, szFile))
      {
        if (!IniSectionGetString(pIniSection, L"FileNameExtensions", pLexArray[iLexer]->pszDefExt,
                                 pLexArray[iLexer]->szExtensions, COUNTOF(pLexArray[iLexer]->szExtensions)))
          lstrcpyn(pLexArray[iLexer]->szExtensions, pLexArray[iLexer]->pszDefExt,
                   COUNTOF(pLexArray[iLexer]->szExtensions));
        i = 0;
        while (pLexArray[iLexer]->Styles[i].iStyle != -1)
        {
          IniSectionGetString(pIniSection, pLexArray[iLexer]->Styles[i].pszName,
                              pLexArray[iLexer]->Styles[i].pszDefault,
                              pLexArray[iLexer]->Styles[i].szValue,
                              COUNTOF(pLexArray[iLexer]->Styles[i].szValue));
          i++;
        }
      }
    }
    LocalFree(pIniSection);
    return (TRUE);
  }
  return (FALSE);
}

//=============================================================================
//
//  Style_Export()
//
BOOL Style_Export(HWND hwnd)
{
  WCHAR szFile[MAX_PATH * 2] = L"";
  WCHAR szFilter[256];
  OPENFILENAME ofn;
  DWORD dwError = ERROR_SUCCESS;

  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  GetString(IDS_FILTER_INI, szFilter, COUNTOF(szFilter));
  PrepareFilterStr(szFilter);

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrFile = szFile;
  ofn.lpstrDefExt = L"ini";
  ofn.nMaxFile = COUNTOF(szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
            | OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_OVERWRITEPROMPT;

  if (GetSaveFileName(&ofn))
  {
    int i, iLexer;
    WCHAR *pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * 32 * 1024);
    int   cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);
    for (iLexer = 0; iLexer < NUMLEXERS; iLexer++)
    {
      IniSectionSetString(pIniSection, L"FileNameExtensions", pLexArray[iLexer]->szExtensions);
      i = 0;
      while (pLexArray[iLexer]->Styles[i].iStyle != -1)
      {
        IniSectionSetString(pIniSection, pLexArray[iLexer]->Styles[i].pszName, pLexArray[iLexer]->Styles[i].szValue);
        i++;
      }
      if (!WritePrivateProfileSection(pLexArray[iLexer]->pszName, pIniSection, szFile))
        dwError = GetLastError();
      ZeroMemory(pIniSection, cchIniSection);
    }
    LocalFree(pIniSection);

    if (dwError != ERROR_SUCCESS)
    {
      MsgBox(MBWARN, IDS_EXPORT_FAIL, szFile);
    }
    return (TRUE);
  }
  return (FALSE);
}


//=============================================================================
//
//  Style_SetLexer()
//
void _Style_SetLexer(HWND hwnd, PEDITLEXER pLexNew)
{
  int i;
  int rgb;
  int iValue;
  int iIdx;
  int iStyleBits;
  WCHAR wchCaretStyle[64] = L"";

  // Select default if NULL is specified
  if (!pLexNew)
    pLexNew = pLexArray[iDefaultLexer];

  // [2e]: Lua LPeg Lexers #251
#ifdef LPEG_LEXER
  LPSTR pszLuaLexer = NULL;
  if (pLexNew->iLexer == SCLEX_LPEG)
  {
    pszLuaLexer = n2e_GetLuaLexerName();
    if (!pszLuaLexer)
    {
      pLexNew = &lexDefault;
    }
  }
#endif
  // [/2e]

  // Lexer
  SendMessage(hwnd, SCI_SETLEXER, pLexNew->iLexer, 0);

  iStyleBits = (int)SendMessage(hwnd, SCI_GETSTYLEBITSNEEDED, 0, 0);
  SendMessage(hwnd, SCI_SETSTYLEBITS, (WPARAM)iStyleBits, 0);

  // [2e]: Lua LPeg Lexers #251
#ifdef LPEG_LEXER
  if (pLexNew->iLexer == SCLEX_LPEG)
  {
    char chLPegHome[MAX_PATH] = { 0 };
    WideCharToMultiByte(CP_UTF8, 0, g_wchLPegHome, COUNTOF(g_wchLPegHome), chLPegHome, COUNTOF(chLPegHome), NULL, NULL);

    SciCall_SetLexerLanguage(0, "lpeg");
    SciCall_SetProperty("lexer.lpeg.home", chLPegHome);
    SciCall_SetProperty("lexer.lpeg.color.theme", "default");
    SciCall_PrivateLexerCall(SCI_GETDIRECTFUNCTION, SciCall_GetDirectFunction());
    SciCall_PrivateLexerCall(SCI_SETDOCPOINTER, SciCall_GetDirectPointer());
  }
  else
#endif
  // [/2e]
  {
    if (pLexNew->iLexer == SCLEX_XML)
      SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "lexer.xml.allow.scripts", (LPARAM) "1");
    else if (pLexNew->iLexer == SCLEX_CPP)
    {
      SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "styling.within.preprocessor", (LPARAM) "1");
      SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "lexer.cpp.track.preprocessor", (LPARAM) "0");
      SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "lexer.cpp.update.preprocessor", (LPARAM) "0");
      // [2e]: Highlight JS templates #207
      SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "lexer.cpp.backquoted.strings", (LPARAM) "1");
    }
    else if (pLexNew->iLexer == SCLEX_PASCAL)
      SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "lexer.pascal.smart.highlighting", (LPARAM) "1");
    else if (pLexNew->iLexer == SCLEX_SQL)
    {
      SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "sql.backslash.escapes", (LPARAM) "1");
      SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "lexer.sql.backticks.identifier", (LPARAM) "1");
      SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "lexer.sql.numbersign.comment", (LPARAM) "1");
    }
    // [2e]: #2, #10 ???
    else if (pLexNew->iLexer == SCLEX_NSIS)
    {
      SciCall_SetProperty("nsis.ignorecase", "1");
    }
    else if (pLexNew->iLexer == SCLEX_CSS)
    {
      if (iCSSSettings & CSS_SASSY)
      {
        SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "lexer.css.scss.language", (LPARAM) "1");
      }
      if (iCSSSettings & CSS_LESS)
      {
        SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "lexer.css.less.language", (LPARAM) "1");
      }
      if (iCSSSettings & CSS_HSS)
      {
        SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM) "lexer.css.hss.language", (LPARAM) "1");
      }
    }
    // [/2e]

    // Add KeyWord Lists
    for (i = 0; i < 9; i++)
      SendMessage(hwnd, SCI_SETKEYWORDS, i, (LPARAM)pLexNew->pKeyWords->pszKeyWords[i]);
  }

  // Use 2nd default style
  iIdx = (bUse2ndDefaultStyle) ? DLO_2ND_DEFAULT_STYLE : 0;

  // Font quality setup, check availability of Consolas
  Style_SetFontQuality(hwnd, lexDefault.Styles[0 + iIdx].szValue);
  fIsConsolasAvailable = IsFontAvailable(L"Consolas");

  // Clear
  SendMessage(hwnd, SCI_CLEARDOCUMENTSTYLE, 0, 0);

  // Default Values are always set
  SendMessage(hwnd, SCI_STYLERESETDEFAULT, 0, 0);
  SendMessage(hwnd, SCI_STYLESETCHARACTERSET, STYLE_DEFAULT, (LPARAM)iDefaultCharSet);
  iBaseFontSize = 10;
  Style_SetStyles(hwnd, lexDefault.iLexer, lexDefault.Styles[0 + iIdx].iStyle, lexDefault.Styles[0 + iIdx].szValue); // default
  Style_StrGetSize(lexDefault.Styles[0 + iIdx].szValue, &iBaseFontSize);                  // base size

  if (!Style_StrGetColor(TRUE, lexDefault.Styles[0 + iIdx].szValue, &iValue))
    SendMessage(hwnd, SCI_STYLESETFORE, STYLE_DEFAULT, (LPARAM)GetSysColor(COLOR_WINDOWTEXT));    // default text color
  if (!Style_StrGetColor(FALSE, lexDefault.Styles[0 + iIdx].szValue, &iValue))
    SendMessage(hwnd, SCI_STYLESETBACK, STYLE_DEFAULT, (LPARAM)GetSysColor(COLOR_WINDOW));    // default window color

  if (pLexNew->iLexer != SCLEX_NULL)
    Style_SetStyles(hwnd, pLexNew->iLexer, pLexNew->Styles[0].iStyle, pLexNew->Styles[0].szValue);    // lexer default
  SendMessage(hwnd, SCI_STYLECLEARALL, 0, 0);

  // [2e]: Lua LPeg Lexers #251
#ifdef LPEG_LEXER
  if (pLexNew->iLexer == SCLEX_LPEG)
  {
    SciCall_PrivateLexerCall(SCI_SETLEXERLANGUAGE, pszLuaLexer);
  }
#endif
  // [/2e]

  Style_SetStyles(hwnd, lexDefault.iLexer, lexDefault.Styles[1 + iIdx].iStyle, lexDefault.Styles[1 + iIdx].szValue); // linenumber
  Style_SetStyles(hwnd, lexDefault.iLexer, lexDefault.Styles[2 + iIdx].iStyle, lexDefault.Styles[2 + iIdx].szValue); // brace light
  Style_SetStyles(hwnd, lexDefault.iLexer, lexDefault.Styles[3 + iIdx].iStyle, lexDefault.Styles[3 + iIdx].szValue); // brace bad
  Style_SetStyles(hwnd, lexDefault.iLexer, lexDefault.Styles[4 + iIdx].iStyle, lexDefault.Styles[4 + iIdx].szValue);    // control char
  Style_SetStyles(hwnd, lexDefault.iLexer, lexDefault.Styles[5 + iIdx].iStyle, lexDefault.Styles[5 + iIdx].szValue); // indent guide

  // More default values...
  if (Style_StrGetColor(TRUE, lexDefault.Styles[6 + iIdx].szValue, &rgb))
  { // selection fore
    SendMessage(hwnd, SCI_SETSELFORE, TRUE, rgb);
    SendMessage(hwnd, SCI_SETADDITIONALSELFORE, rgb, 0);
  }
  else
  {
    SendMessage(hwnd, SCI_SETSELFORE, 0, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELFORE, 0, 0);
  }
  if (Style_StrGetColor(FALSE, lexDefault.Styles[6 + iIdx].szValue, &iValue))
  { // selection back
    SendMessage(hwnd, SCI_SETSELBACK, TRUE, iValue);
    SendMessage(hwnd, SCI_SETADDITIONALSELBACK, iValue, 0);
  }
  else
  {
    SendMessage(hwnd, SCI_SETSELBACK, TRUE, RGB(0xC0, 0xC0, 0xC0)); // use a default value...
    SendMessage(hwnd, SCI_SETADDITIONALSELBACK, RGB(0xC0, 0xC0, 0xC0), 0);
  }
  if (Style_StrGetAlpha(lexDefault.Styles[6 + iIdx].szValue, &iValue))
  { // selection alpha
    SendMessage(hwnd, SCI_SETSELALPHA, iValue, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELALPHA, iValue, 0);
  }
  else
  {
    SendMessage(hwnd, SCI_SETSELALPHA, SC_ALPHA_NOALPHA, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELALPHA, SC_ALPHA_NOALPHA, 0);
  }
  if (StrStrI(lexDefault.Styles[6 + iIdx].szValue, L"eolfilled"))
  { // selection eolfilled
    SendMessage(hwnd, SCI_SETSELEOLFILLED, 1, 0);
  }
  else
  {
    SendMessage(hwnd, SCI_SETSELEOLFILLED, 0, 0);
  }
  if (Style_StrGetColor(TRUE, lexDefault.Styles[7 + iIdx].szValue, &rgb))
  { // whitespace fore
    SendMessage(hwnd, SCI_SETWHITESPACEFORE, TRUE, rgb);
  }
  else
  {
    SendMessage(hwnd, SCI_SETWHITESPACEFORE, 0, 0);
  }
  if (Style_StrGetColor(FALSE, lexDefault.Styles[7 + iIdx].szValue, &rgb))
  { // whitespace back
    SendMessage(hwnd, SCI_SETWHITESPACEBACK, TRUE, rgb);
  }
  else
  {
    SendMessage(hwnd, SCI_SETWHITESPACEBACK, 0, 0);    // use a default value...
  }

  // whitespace dot size
  iValue = 1;
  if (Style_StrGetSize(lexDefault.Styles[7 + iIdx].szValue, &iValue))
  {

    WCHAR tch[32];
    WCHAR wchStyle[COUNTOF(lexDefault.Styles[0].szValue)];
    lstrcpyn(wchStyle, lexDefault.Styles[7 + iIdx].szValue, COUNTOF(lexDefault.Styles[0].szValue));

    iValue = max(min(iValue, 5), 0);
    wsprintf(lexDefault.Styles[7 + iIdx].szValue, L"size:%i", iValue);

    if (Style_StrGetColor(TRUE, wchStyle, &rgb))
    {
      wsprintf(tch, L"; fore:#%02X%02X%02X",
               (int)GetRValue(rgb),
               (int)GetGValue(rgb),
               (int)GetBValue(rgb));
      lstrcat(lexDefault.Styles[7 + iIdx].szValue, tch);
    }

    if (Style_StrGetColor(FALSE, wchStyle, &rgb))
    {
      wsprintf(tch, L"; back:#%02X%02X%02X",
               (int)GetRValue(rgb),
               (int)GetGValue(rgb),
               (int)GetBValue(rgb));
      lstrcat(lexDefault.Styles[7 + iIdx].szValue, tch);
    }
  }
  SendMessage(hwnd, SCI_SETWHITESPACESIZE, iValue, 0);

  Style_SetCurrentLineBackground(hwnd);

  // caret style and width
  if (StrStr(lexDefault.Styles[DLO_CARET_COLOR + iIdx].szValue, L"block"))
  {
    SendMessage(hwnd, SCI_SETCARETSTYLE, CARETSTYLE_BLOCK, 0);
    lstrcpy(wchCaretStyle, L"block");
  }
  else
  {
    WCHAR wch[32];
    iValue = 1;
    if (Style_StrGetSize(lexDefault.Styles[DLO_CARET_COLOR + iIdx].szValue, &iValue))
    {
      iValue = max(min(iValue, 3), 1);
      wsprintf(wch, L"size:%i", iValue);
      lstrcat(wchCaretStyle, wch);
    }
    SendMessage(hwnd, SCI_SETCARETSTYLE, CARETSTYLE_LINE, 0);
    SendMessage(hwnd, SCI_SETCARETWIDTH, iValue, 0);
  }
  if (StrStr(lexDefault.Styles[DLO_CARET_COLOR + iIdx].szValue, L"noblink"))
  {
    SendMessage(hwnd, SCI_SETCARETPERIOD, (WPARAM)0, 0);
    if (lstrlen(wchCaretStyle))
      lstrcat(wchCaretStyle, L"; ");
    lstrcat(wchCaretStyle, L"noblink");
  }
  else
    SendMessage(hwnd, SCI_SETCARETPERIOD, (WPARAM)GetCaretBlinkTime(), 0);

  // caret fore
  if (!Style_StrGetColor(TRUE, lexDefault.Styles[DLO_CARET_COLOR + iIdx].szValue, &rgb))
  {
    rgb = GetSysColor(COLOR_WINDOWTEXT);
  }
  else
  {
    WCHAR wch[32];
    wsprintf(wch, L"fore:#%02X%02X%02X",
             (int)GetRValue(rgb),
             (int)GetGValue(rgb),
             (int)GetBValue(rgb));
    if (lstrlen(wchCaretStyle))
      lstrcat(wchCaretStyle, L"; ");
    lstrcat(wchCaretStyle, wch);
  }
  if (!VerifyContrast(rgb, (COLORREF)SendMessage(hwnd, SCI_STYLEGETBACK, 0, 0)))
  {
    rgb = (int)SendMessage(hwnd, SCI_STYLEGETFORE, 0, 0);
  }
  SendMessage(hwnd, SCI_SETCARETFORE, rgb, 0);
  SendMessage(hwnd, SCI_SETADDITIONALCARETFORE, rgb, 0);
  lstrcpy(lexDefault.Styles[DLO_CARET_COLOR + iIdx].szValue, wchCaretStyle);
  if (SendMessage(hwnd, SCI_GETEDGEMODE, 0, 0) == EDGE_LINE)
  {
    if (Style_StrGetColor(TRUE, lexDefault.Styles[DLO_LONG_LINE_MARKER + iIdx].szValue, &rgb)) // edge fore
      SendMessage(hwnd, SCI_SETEDGECOLOUR, rgb, 0);
    else
      SendMessage(hwnd, SCI_SETEDGECOLOUR, GetSysColor(COLOR_3DLIGHT), 0);
  }
  else
  {
    if (Style_StrGetColor(FALSE, lexDefault.Styles[DLO_LONG_LINE_MARKER + iIdx].szValue, &rgb)) // edge back
      SendMessage(hwnd, SCI_SETEDGECOLOUR, rgb, 0);
    else
      SendMessage(hwnd, SCI_SETEDGECOLOUR, GetSysColor(COLOR_3DLIGHT), 0);
  }

  // Extra Line Spacing
  if (Style_StrGetSize(lexDefault.Styles[DLO_EXTRA_LINE_SPACING + iIdx].szValue, &iValue))
  {
    int iAscent = 0;
    int iDescent = 0;
    iValue = min(max(iValue, 0), 64);
    wsprintf(lexDefault.Styles[DLO_EXTRA_LINE_SPACING + iIdx].szValue, L"size:%i", iValue);
    if (iValue % 2)
    {
      iAscent++;
      iValue--;
    }
    iAscent += iValue / 2;
    iDescent += iValue / 2;
    SendMessage(hwnd, SCI_SETEXTRAASCENT, (WPARAM)iAscent, 0);
    SendMessage(hwnd, SCI_SETEXTRADESCENT, (WPARAM)iDescent, 0);
  }
  else
  {
    SendMessage(hwnd, SCI_SETEXTRAASCENT, 0, 0);
    SendMessage(hwnd, SCI_SETEXTRADESCENT, 0, 0);
  }

  if (SendMessage(hwnd, SCI_GETINDENTATIONGUIDES, 0, 0) != SC_IV_NONE)
  {
    // [2e]: Split view #316
    extern BOOL bShowIndentGuides;
    BOOL bShowIndentGuidesOrigin = bShowIndentGuides;
    bShowIndentGuides = TRUE;
    Style_SetIndentGuides(hwnd);
    bShowIndentGuides = bShowIndentGuidesOrigin;
  }

  if (pLexNew->iLexer != SCLEX_NULL)
  {
    int j;
    i = 1;
    while (pLexNew->Styles[i].iStyle != -1)
    {
      for (j = 0; j < 4 && (pLexNew->Styles[i].iStyle8[j] != 0 || j == 0); ++j)
        Style_SetStyles(hwnd, pLexNew->iLexer, pLexNew->Styles[i].iStyle8[j], pLexNew->Styles[i].szValue);

      if (pLexNew->iLexer == SCLEX_HTML && pLexNew->Styles[i].iStyle8[0] == SCE_HPHP_DEFAULT)
      {
        int iRelated[] = { SCE_HPHP_COMMENT, SCE_HPHP_COMMENTLINE, SCE_HPHP_WORD, SCE_HPHP_HSTRING, SCE_HPHP_SIMPLESTRING, SCE_HPHP_NUMBER,
                           SCE_HPHP_OPERATOR, SCE_HPHP_VARIABLE, SCE_HPHP_HSTRING_VARIABLE, SCE_HPHP_COMPLEX_VARIABLE
        };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd, pLexNew->iLexer, iRelated[j], pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_HTML && pLexNew->Styles[i].iStyle8[0] == SCE_HJ_DEFAULT)
      {
        int iRelated[] = { SCE_HJ_COMMENT, SCE_HJ_COMMENTLINE, SCE_HJ_COMMENTDOC, SCE_HJ_KEYWORD, SCE_HJ_WORD, SCE_HJ_DOUBLESTRING,
                           SCE_HJ_SINGLESTRING, SCE_HJ_STRINGEOL, SCE_HJ_REGEX, SCE_HJ_NUMBER, SCE_HJ_SYMBOLS
        };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd, pLexNew->iLexer, iRelated[j], pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_HTML && pLexNew->Styles[i].iStyle8[0] == SCE_HJA_DEFAULT)
      {
        int iRelated[] = { SCE_HJA_COMMENT, SCE_HJA_COMMENTLINE, SCE_HJA_COMMENTDOC, SCE_HJA_KEYWORD, SCE_HJA_WORD, SCE_HJA_DOUBLESTRING,
                           SCE_HJA_SINGLESTRING, SCE_HJA_STRINGEOL, SCE_HJA_REGEX, SCE_HJA_NUMBER, SCE_HJA_SYMBOLS
        };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd, pLexNew->iLexer, iRelated[j], pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_HTML && pLexNew->Styles[i].iStyle8[0] == SCE_HB_DEFAULT)
      {
        int iRelated[] = { SCE_HB_COMMENTLINE, SCE_HB_WORD, SCE_HB_IDENTIFIER, SCE_HB_STRING, SCE_HB_STRINGEOL, SCE_HB_NUMBER };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd, pLexNew->iLexer, iRelated[j], pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_HTML && pLexNew->Styles[i].iStyle8[0] == SCE_HBA_DEFAULT)
      {
        int iRelated[] = { SCE_HBA_COMMENTLINE, SCE_HBA_WORD, SCE_HBA_IDENTIFIER, SCE_HBA_STRING, SCE_HBA_STRINGEOL, SCE_HBA_NUMBER };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd, pLexNew->iLexer, iRelated[j], pLexNew->Styles[i].szValue);
      }

      if ((pLexNew->iLexer == SCLEX_HTML || pLexNew->iLexer == SCLEX_XML) && pLexNew->Styles[i].iStyle8[0] == SCE_H_SGML_DEFAULT)
      {
        int iRelated[] = { SCE_H_SGML_COMMAND, SCE_H_SGML_1ST_PARAM, SCE_H_SGML_DOUBLESTRING, SCE_H_SGML_SIMPLESTRING, SCE_H_SGML_ERROR,
                           SCE_H_SGML_SPECIAL, SCE_H_SGML_ENTITY, SCE_H_SGML_COMMENT, SCE_H_SGML_1ST_PARAM_COMMENT, SCE_H_SGML_BLOCK_DEFAULT
        };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd, pLexNew->iLexer, iRelated[j], pLexNew->Styles[i].szValue);
      }

      if ((pLexNew->iLexer == SCLEX_HTML || pLexNew->iLexer == SCLEX_XML) && pLexNew->Styles[i].iStyle8[0] == SCE_H_CDATA)
      {
        int iRelated[] = { SCE_HP_START, SCE_HP_DEFAULT, SCE_HP_COMMENTLINE, SCE_HP_NUMBER, SCE_HP_STRING,
                           SCE_HP_CHARACTER, SCE_HP_WORD, SCE_HP_TRIPLE, SCE_HP_TRIPLEDOUBLE, SCE_HP_CLASSNAME,
                           SCE_HP_DEFNAME, SCE_HP_OPERATOR, SCE_HP_IDENTIFIER, SCE_HPA_START, SCE_HPA_DEFAULT,
                           SCE_HPA_COMMENTLINE, SCE_HPA_NUMBER, SCE_HPA_STRING, SCE_HPA_CHARACTER, SCE_HPA_WORD,
                           SCE_HPA_TRIPLE, SCE_HPA_TRIPLEDOUBLE, SCE_HPA_CLASSNAME, SCE_HPA_DEFNAME, SCE_HPA_OPERATOR,
                           SCE_HPA_IDENTIFIER
        };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd, pLexNew->iLexer, iRelated[j], pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_XML && pLexNew->Styles[i].iStyle8[0] == SCE_H_CDATA)
      {
        int iRelated[] = { SCE_H_SCRIPT, SCE_H_ASP, SCE_H_ASPAT, SCE_H_QUESTION,
                           SCE_HPHP_DEFAULT, SCE_HPHP_COMMENT, SCE_HPHP_COMMENTLINE, SCE_HPHP_WORD, SCE_HPHP_HSTRING,
                           SCE_HPHP_SIMPLESTRING, SCE_HPHP_NUMBER, SCE_HPHP_OPERATOR, SCE_HPHP_VARIABLE,
                           SCE_HPHP_HSTRING_VARIABLE, SCE_HPHP_COMPLEX_VARIABLE, SCE_HJ_START, SCE_HJ_DEFAULT,
                           SCE_HJ_COMMENT, SCE_HJ_COMMENTLINE, SCE_HJ_COMMENTDOC, SCE_HJ_KEYWORD, SCE_HJ_WORD,
                           SCE_HJ_DOUBLESTRING, SCE_HJ_SINGLESTRING, SCE_HJ_STRINGEOL, SCE_HJ_REGEX, SCE_HJ_NUMBER,
                           SCE_HJ_SYMBOLS, SCE_HJA_START, SCE_HJA_DEFAULT, SCE_HJA_COMMENT, SCE_HJA_COMMENTLINE,
                           SCE_HJA_COMMENTDOC, SCE_HJA_KEYWORD, SCE_HJA_WORD, SCE_HJA_DOUBLESTRING, SCE_HJA_SINGLESTRING,
                           SCE_HJA_STRINGEOL, SCE_HJA_REGEX, SCE_HJA_NUMBER, SCE_HJA_SYMBOLS, SCE_HB_START, SCE_HB_DEFAULT,
                           SCE_HB_COMMENTLINE, SCE_HB_WORD, SCE_HB_IDENTIFIER, SCE_HB_STRING, SCE_HB_STRINGEOL,
                           SCE_HB_NUMBER, SCE_HBA_START, SCE_HBA_DEFAULT, SCE_HBA_COMMENTLINE, SCE_HBA_WORD,
                           SCE_HBA_IDENTIFIER, SCE_HBA_STRING, SCE_HBA_STRINGEOL, SCE_HBA_NUMBER, SCE_HP_START,
                           SCE_HP_DEFAULT, SCE_HP_COMMENTLINE, SCE_HP_NUMBER, SCE_HP_STRING, SCE_HP_CHARACTER, SCE_HP_WORD,
                           SCE_HP_TRIPLE, SCE_HP_TRIPLEDOUBLE, SCE_HP_CLASSNAME, SCE_HP_DEFNAME, SCE_HP_OPERATOR,
                           SCE_HP_IDENTIFIER, SCE_HPA_START, SCE_HPA_DEFAULT, SCE_HPA_COMMENTLINE, SCE_HPA_NUMBER,
                           SCE_HPA_STRING, SCE_HPA_CHARACTER, SCE_HPA_WORD, SCE_HPA_TRIPLE, SCE_HPA_TRIPLEDOUBLE,
                           SCE_HPA_CLASSNAME, SCE_HPA_DEFNAME, SCE_HPA_OPERATOR, SCE_HPA_IDENTIFIER
        };
        for (j = 0; j < COUNTOF(iRelated); j++)
           Style_SetStyles(hwnd, pLexNew->iLexer, iRelated[j], pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_CPP && pLexNew->Styles[i].iStyle8[0] == SCE_C_COMMENT)
      {
        int iRelated[] = { SCE_C_COMMENTLINE, SCE_C_COMMENTDOC, SCE_C_COMMENTLINEDOC, SCE_C_COMMENTDOCKEYWORD, SCE_C_COMMENTDOCKEYWORDERROR };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd, pLexNew->iLexer, iRelated[j], pLexNew->Styles[i].szValue);
      }

      // [2e]: Highlight JS templates #207
      if (pLexNew->iLexer == SCLEX_CPP && pLexNew->rid == 63010 && pLexNew->Styles[i].iStyle8[0] == SCE_C_STRING)
      {
        Style_SetStyles(hwnd, pLexNew->iLexer, SCE_C_STRINGRAW, pLexNew->Styles[i].szValue);
      }
      // [/2e]

      if (pLexNew->iLexer == SCLEX_SQL && pLexNew->Styles[i].iStyle8[0] == SCE_SQL_COMMENT)
      {
        int iRelated[] = { SCE_SQL_COMMENTLINE, SCE_SQL_COMMENTDOC, SCE_SQL_COMMENTLINEDOC, SCE_SQL_COMMENTDOCKEYWORD, SCE_SQL_COMMENTDOCKEYWORDERROR };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd, pLexNew->iLexer, iRelated[j], pLexNew->Styles[i].szValue);
      }
      i++;
    }
  }

  SendMessage(hwnd, SCI_COLOURISE, 0, (LPARAM)-1);

  // Save current lexer
  pLexCurrent = pLexNew;
}


//=============================================================================
//
//  Style_SetLongLineColors()
//
void Style_SetLongLineColors(HWND hwnd)
{
  int rgb;

  // Use 2nd default style
  int iIdx = (bUse2ndDefaultStyle) ? DLO_2ND_DEFAULT_STYLE : 0;
  if (SendMessage(hwnd, SCI_GETEDGEMODE, 0, 0) == EDGE_LINE)
  {
    if (Style_StrGetColor(TRUE, lexDefault.Styles[DLO_LONG_LINE_MARKER + iIdx].szValue, &rgb)) // edge fore
      SendMessage(hwnd, SCI_SETEDGECOLOUR, rgb, 0);
    else
      SendMessage(hwnd, SCI_SETEDGECOLOUR, GetSysColor(COLOR_3DLIGHT), 0);
  }
  else
  {
    if (Style_StrGetColor(FALSE, lexDefault.Styles[DLO_LONG_LINE_MARKER + iIdx].szValue, &rgb)) // edge back
      SendMessage(hwnd, SCI_SETEDGECOLOUR, rgb, 0);
    else
      SendMessage(hwnd, SCI_SETEDGECOLOUR, GetSysColor(COLOR_3DLIGHT), 0);
  }
}


//=============================================================================
//
//  Style_SetCurrentLineBackground()
//
void Style_SetCurrentLineBackground(HWND hwnd)
{
  int rgb, iValue;

  // Use 2nd default style
  const int iIdx = (bUse2ndDefaultStyle) ? DLO_2ND_DEFAULT_STYLE : 0;
  const int iCurrentLineBackgroundStyle = (GetFocus() == hwnd) ? DLO_CURRENT_LINE_BACKGROUND : DLO_CURRENT_LINE_BACKGROUND_INACTIVE;

  if (bHighlightCurrentLine)
  {

    if (Style_StrGetColor(FALSE, lexDefault.Styles[iCurrentLineBackgroundStyle + iIdx].szValue, &rgb))
    { // caret line back
      SendMessage(hwnd, SCI_SETCARETLINEVISIBLE, TRUE, 0);
      SendMessage(hwnd, SCI_SETCARETLINEBACK, rgb, 0);

      if (Style_StrGetAlpha(lexDefault.Styles[iCurrentLineBackgroundStyle + iIdx].szValue, &iValue))
        SendMessage(hwnd, SCI_SETCARETLINEBACKALPHA, iValue, 0);
      else
        SendMessage(hwnd, SCI_SETCARETLINEBACKALPHA, SC_ALPHA_NOALPHA, 0);
    }
    else
      SendMessage(hwnd, SCI_SETCARETLINEVISIBLE, FALSE, 0);
  }
  else
    SendMessage(hwnd, SCI_SETCARETLINEVISIBLE, FALSE, 0);
}


//=============================================================================
//
//  Style_SniffShebang()
//
PEDITLEXER __fastcall Style_SniffShebang(char *pchText)
{
  if (StrCmpNA(pchText, "#!", 2) == 0)
  {
    char *pch = pchText + 2;
    while (*pch == ' ' || *pch == '\t')
      pch++;
    while (*pch && *pch != ' ' && *pch != '\t' && *pch != '\r' && *pch != '\n')
      pch++;
    if ((pch - pchText) >= 3 && StrCmpNA(pch - 3, "env", 3) == 0)
    {
      while (*pch == ' ')
        pch++;
      while (*pch && *pch != ' ' && *pch != '\t' && *pch != '\r' && *pch != '\n')
        pch++;
    }
    if ((pch - pchText) >= 3 && StrCmpNIA(pch - 3, "php", 3) == 0)
      return (&lexHTML);
    else if ((pch - pchText) >= 4 && StrCmpNIA(pch - 4, "perl", 4) == 0)
      return (&lexPL);
    else if ((pch - pchText) >= 6 && StrCmpNIA(pch - 6, "python", 6) == 0)
      return (&lexPY);
  }

  return (NULL);
}


//=============================================================================
//
//  Style_MatchLexer()
//
PEDITLEXER __fastcall Style_MatchLexer(LPCWSTR lpszMatch, BOOL bCheckNames)
{
  int i;
  WCHAR  tch[256 + 16];
  WCHAR  *p1, *p2;
  if (!bCheckNames)
  {
    for (i = 0; i < NUMLEXERS; i++)
    {
      ZeroMemory(tch, sizeof(WCHAR) *COUNTOF(tch));
      lstrcpy(tch, pLexArray[i]->szExtensions);
      p1 = tch;
      while (*p1)
      {
        if (p2 = StrChr(p1, L';'))
          *p2 = L'\0';
        else
          p2 = StrEnd(p1);
        StrTrim(p1, L" .");
        if (lstrcmpi(p1, lpszMatch) == 0)
          return (pLexArray[i]);
        p1 = p2 + 1;
      }
    }
  }
  else
  {
    int cch = lstrlen(lpszMatch);
    if (cch >= 3)
    {
      for (i = 0; i < NUMLEXERS; i++)
      {
        if (StrCmpNI(pLexArray[i]->pszName, lpszMatch, cch) == 0)
          return (pLexArray[i]);
      }
    }
  }
  return (NULL);
}


//=============================================================================
//
//  Style_SetLexerFromFile()
//
extern int fNoHTMLGuess;
extern int fNoCGIGuess;
extern FILEVARS fvCurFile;

void Style_SetLexerFromFile(HWND hwnd, LPCWSTR lpszFile)
{
  LPWSTR lpszExt;
  BOOL  bFound = FALSE;
  PEDITLEXER pLexNew = pLexArray[iDefaultLexer];
  PEDITLEXER pLexSniffed;
  if ((fvCurFile.mask & FV_MODE) && fvCurFile.tchMode[0])
  {
    WCHAR wchMode[32];
    PEDITLEXER pLexMode;
    UINT cp = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);
    MultiByteToWideChar(cp, 0, fvCurFile.tchMode, -1, wchMode, COUNTOF(wchMode));
    if (!fNoCGIGuess && (lstrcmpi(wchMode, L"cgi") == 0 || lstrcmpi(wchMode, L"fcgi") == 0))
    {
      char tchText[256];
      SendMessage(hwnd, SCI_GETTEXT, (WPARAM)COUNTOF(tchText) - 1, (LPARAM)tchText);
      StrTrimA(tchText, " \t\n\r");
      if (pLexSniffed = Style_SniffShebang(tchText))
      {
        pLexNew = pLexSniffed;
        bFound = TRUE;
      }
    }
    if (!bFound)
    {
      if (pLexMode = Style_MatchLexer(wchMode, FALSE))
      {
        pLexNew = pLexMode;
        bFound = TRUE;
      }
      else if (pLexMode = Style_MatchLexer(wchMode, TRUE))
      {
        pLexNew = pLexMode;
        bFound = TRUE;
      }
    }
  }

  lpszExt = PathFindExtension(lpszFile);

  if (!bFound && bAutoSelect &&
    (lpszFile && lstrlen(lpszFile) > 0 && *lpszExt))
  {
    if (*lpszExt == L'.')
      lpszExt++;

    if (!fNoCGIGuess && (lstrcmpi(lpszExt, L"cgi") == 0 || lstrcmpi(lpszExt, L"fcgi") == 0))
    {
      char tchText[256];
      SendMessage(hwnd, SCI_GETTEXT, (WPARAM)COUNTOF(tchText) - 1, (LPARAM)tchText);
      StrTrimA(tchText, " \t\n\r");
      if (pLexSniffed = Style_SniffShebang(tchText))
      {
        pLexNew = pLexSniffed;
        bFound = TRUE;
      }
    }

    // check associated extensions
    if (!bFound)
    {
      if (pLexSniffed = Style_MatchLexer(lpszExt, FALSE))
      {
        pLexNew = pLexSniffed;
        bFound = TRUE;
      }
    }
  }

  if (!bFound && bAutoSelect &&
      lstrcmpi(PathFindFileName(lpszFile), L"makefile") == 0)
  {
    pLexNew = &lexMAK;
    bFound = TRUE;
  }

  if (!bFound && bAutoSelect && (!fNoHTMLGuess || !fNoCGIGuess))
  {
    char tchText[512];
    SendMessage(hwnd, SCI_GETTEXT, (WPARAM)COUNTOF(tchText) - 1, (LPARAM)tchText);
    StrTrimA(tchText, " \t\n\r");
    if (!fNoHTMLGuess && tchText[0] == '<')
    {
      if (StrStrIA(tchText, "<html"))
        pLexNew = &lexHTML;
      else
        pLexNew = &lexXML;
      bFound = TRUE;
    }
    else if (!fNoCGIGuess && (pLexSniffed = Style_SniffShebang(tchText)))
    {
      pLexNew = pLexSniffed;
      bFound = TRUE;
    }
  }

  // Apply the new lexer
  Style_SetLexer(hwnd, pLexNew);
}


//=============================================================================
//
//  Style_SetLexerFromName()
//
void Style_SetLexerFromName(HWND hwnd, LPCWSTR lpszFile, LPCWSTR lpszName)
{
  PEDITLEXER pLexNew;
  if (pLexNew = Style_MatchLexer(lpszName, FALSE))
    Style_SetLexer(hwnd, pLexNew);
  else if (pLexNew = Style_MatchLexer(lpszName, TRUE))
    Style_SetLexer(hwnd, pLexNew);
  else
    Style_SetLexerFromFile(hwnd, lpszFile);
}


//=============================================================================
//
//  Style_SetDefaultLexer()
//
void Style_SetDefaultLexer(HWND hwnd)
{
  Style_SetLexer(hwnd, pLexArray[0]);
}


//=============================================================================
//
//  Style_SetHTMLLexer()
//
void Style_SetHTMLLexer(HWND hwnd)
{
  Style_SetLexer(hwnd, Style_MatchLexer(L"Web Source Code", TRUE));
}


//=============================================================================
//
//  Style_SetXMLLexer()
//
void Style_SetXMLLexer(HWND hwnd)
{
  Style_SetLexer(hwnd, Style_MatchLexer(L"XML", TRUE));
}


//=============================================================================
//
//  Style_SetLexerFromID()
//
void Style_SetLexerFromID(HWND hwnd, int id)
{
  if (id >= 0 && id < NUMLEXERS)
    Style_SetLexer(hwnd, pLexArray[id]);
}


//=============================================================================
//
//  Style_ToggleUse2ndDefault()
//
void Style_ToggleUse2ndDefault(HWND hwnd)
{
  bUse2ndDefaultStyle = (bUse2ndDefaultStyle) ? 0 : 1;
  Style_SetLexer(hwnd, pLexCurrent);
}


//=============================================================================
//
//  Style_SetDefaultFont()
//
void Style_SetDefaultFont(HWND hwnd)
{
  int iIdx = (bUse2ndDefaultStyle) ? DLO_2ND_DEFAULT_STYLE : 0;
  if (Style_SelectFont(hwnd,
                       lexDefault.Styles[0 + iIdx].szValue,
                       COUNTOF(lexDefault.Styles[0].szValue),
                       TRUE))
  {
    fStylesModified = TRUE;
    Style_SetLexer(hwnd, pLexCurrent);
  }
}


//=============================================================================
//
//  Style_GetUse2ndDefault()
//
BOOL Style_GetUse2ndDefault(HWND hwnd)
{
  return (bUse2ndDefaultStyle);
}


//=============================================================================
//
//  Style_SetIndentGuides()
//
extern BOOL bShowIndentGuides;
extern int flagSimpleIndentGuides;

void Style_SetIndentGuides(HWND hwnd)
{
  int iIndentView = SC_IV_NONE;
  if (bShowIndentGuides)
  {
    if (!flagSimpleIndentGuides)
    {
      if (SendMessage(hwnd, SCI_GETLEXER, 0, 0) == SCLEX_PYTHON)
        iIndentView = SC_IV_LOOKFORWARD;
      else
        iIndentView = SC_IV_LOOKBOTH;
    }
    else
      iIndentView = SC_IV_REAL;
  }
  SendMessage(hwnd, SCI_SETINDENTATIONGUIDES, iIndentView, 0);
}


//=============================================================================
//
//  Style_GetFileOpenDlgFilter()
//
extern WCHAR tchFileDlgFilters[5 * 1024];

BOOL Style_GetOpenDlgFilterStr(LPWSTR lpszFilter, int cchFilter)
{
  if (lstrlen(tchFileDlgFilters) == 0)
    GetString(IDS_FILTER_ALL, lpszFilter, cchFilter);
  else
  {
    lstrcpyn(lpszFilter, tchFileDlgFilters, cchFilter - 2);
    lstrcat(lpszFilter, L"||");
  }
  PrepareFilterStr(lpszFilter);
  return TRUE;
}


//=============================================================================
//
//  Style_StrGetFont()
//
BOOL Style_StrGetFont(LPCWSTR lpszStyle, LPWSTR lpszFont, int cchFont)
{
  WCHAR tch[256];
  WCHAR *p;
  if (p = StrStrI(lpszStyle, L"font:"))
  {
    lstrcpy(tch, p + CSTRLEN(L"font:"));
    if (p = StrChr(tch, L';'))
      *p = L'\0';
    TrimString(tch);
    lstrcpyn(lpszFont, tch, cchFont);
    return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetFontQuality()
//
BOOL Style_StrGetFontQuality(LPCWSTR lpszStyle, LPWSTR lpszQuality, int cchQuality)
{
  WCHAR tch[256];
  WCHAR *p;

  if (p = StrStrI(lpszStyle, L"smoothing:"))
  {
    lstrcpy(tch, p + CSTRLEN(L"smoothing:"));
    if (p = StrChr(tch, L';'))
      *p = L'\0';
    TrimString(tch);
    if (lstrcmpi(tch, L"none") == 0 ||
        lstrcmpi(tch, L"standard") == 0 ||
        lstrcmpi(tch, L"cleartype") == 0 ||
        lstrcmpi(tch, L"default") == 0)
    {
      lstrcpyn(lpszQuality, tch, cchQuality);
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetCharSet()
//
BOOL Style_StrGetCharSet(LPCWSTR lpszStyle, int *i)
{
  WCHAR tch[256];
  WCHAR *p;
  int  iValue;
  int  itok;

  if (p = StrStrI(lpszStyle, L"charset:"))
  {
    lstrcpy(tch, p + CSTRLEN(L"charset:"));
    if (p = StrChr(tch, L';'))
      *p = L'\0';
    TrimString(tch);
    itok = swscanf(tch, L"%i", &iValue);
    if (itok == 1)
    {
      *i = iValue;
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetSize()
//
BOOL Style_StrGetSize(LPCWSTR lpszStyle, int *i)
{
  WCHAR tch[256];
  WCHAR *p;
  int  iValue;
  int  iSign = 0;
  int  itok;

  if (p = StrStrI(lpszStyle, L"size:"))
  {
    lstrcpy(tch, p + CSTRLEN(L"size:"));
    if (tch[0] == L'+')
    {
      iSign = 1;
      tch[0] = L' ';
    }
    else if (tch[0] == L'-')
    {
      iSign = -1;
      tch[0] = L' ';
    }
    if (p = StrChr(tch, L';'))
      *p = L'\0';
    TrimString(tch);
    itok = swscanf(tch, L"%i", &iValue);
    if (itok == 1)
    {
      if (iSign == 0)
        *i = iValue;
      else
        *i = max(0, iBaseFontSize + iValue * iSign);    // size must be +
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetSizeStr()
//
BOOL Style_StrGetSizeStr(LPCWSTR lpszStyle, LPWSTR lpszSize, int cchSize)
{
  WCHAR tch[256];
  WCHAR *p;
  if (p = StrStrI(lpszStyle, L"size:"))
  {
    lstrcpy(tch, p + CSTRLEN(L"size:"));
    if (p = StrChr(tch, L';'))
      *p = L'\0';
    TrimString(tch);
    lstrcpyn(lpszSize, tch, cchSize);
    return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetColor()
//
BOOL Style_StrGetColor(BOOL bFore, LPCWSTR lpszStyle, int *rgb)
{
  WCHAR tch[256];
  WCHAR *p;
  int  iValue;
  int  itok;
  WCHAR *pItem = (bFore) ? L"fore:" : L"back:";

  if (p = StrStrI(lpszStyle, pItem))
  {
    lstrcpy(tch, p + lstrlen(pItem));
    if (tch[0] == L'#')
      tch[0] = L' ';
    if (p = StrChr(tch, L';'))
      *p = L'\0';
    TrimString(tch);
    itok = swscanf(tch, L"%x", &iValue);
    if (itok == 1)
    {
      *rgb = RGB((iValue & 0xFF0000) >> 16, (iValue & 0xFF00) >> 8, iValue & 0xFF);
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetCase()
//
BOOL Style_StrGetCase(LPCWSTR lpszStyle, int *i)
{
  WCHAR tch[256];
  WCHAR *p;
  if (p = StrStrI(lpszStyle, L"case:"))
  {
    lstrcpy(tch, p + CSTRLEN(L"case:"));
    if (p = StrChr(tch, L';'))
      *p = L'\0';
    TrimString(tch);
    if (tch[0] == L'u' || tch[0] == L'U')
    {
      *i = SC_CASE_UPPER;
      return TRUE;
    }
    else if (tch[0] == L'l' || tch[0] == L'L')
    {
      *i = SC_CASE_LOWER;
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetAlpha()
//
BOOL Style_StrGetAlpha(LPCWSTR lpszStyle, int *i)
{
  WCHAR tch[256];
  WCHAR *p;
  int  iValue;
  int  itok;
  if (p = StrStrI(lpszStyle, L"alpha:"))
  {
    lstrcpy(tch, p + CSTRLEN(L"alpha:"));
    if (p = StrChr(tch, L';'))
      *p = L'\0';
    TrimString(tch);
    itok = swscanf(tch, L"%i", &iValue);
    if (itok == 1)
    {
      *i = min(max(SC_ALPHA_TRANSPARENT, iValue), SC_ALPHA_OPAQUE);
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_SelectFont()
//
BOOL Style_SelectFont(HWND hwnd, LPWSTR lpszStyle, int cchStyle, BOOL bDefaultStyle)
{
  CHOOSEFONT cf;
  LOGFONT lf;
  WCHAR szNewStyle[512];
  int  iValue;
  WCHAR tch[32];
  HDC hdc;

  ZeroMemory(&cf, sizeof(CHOOSEFONT));
  ZeroMemory(&lf, sizeof(LOGFONT));

  // Map lpszStyle to LOGFONT
  if (Style_StrGetFont(lpszStyle, tch, COUNTOF(tch)))
    lstrcpyn(lf.lfFaceName, tch, COUNTOF(lf.lfFaceName));
  if (Style_StrGetCharSet(lpszStyle, &iValue))
    lf.lfCharSet = iValue;
  if (Style_StrGetSize(lpszStyle, &iValue))
  {
    hdc = GetDC(hwnd);
    lf.lfHeight = -MulDiv(iValue, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(hwnd, hdc);
  }
  lf.lfWeight = (StrStrI(lpszStyle, L"bold")) ? FW_BOLD : FW_NORMAL;
  lf.lfItalic = (StrStrI(lpszStyle, L"italic")) ? 1 : 0;

  // Init cf
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.hwndOwner = hwnd;
  cf.lpLogFont = &lf;
  cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;

  if (HIBYTE(GetKeyState(VK_SHIFT)))
    cf.Flags |= CF_FIXEDPITCHONLY;

  if (!ChooseFont(&cf) || !lstrlen(lf.lfFaceName))
    return FALSE;

  // Map back to lpszStyle
  lstrcpy(szNewStyle, L"font:");
  lstrcat(szNewStyle, lf.lfFaceName);
  if (Style_StrGetFontQuality(lpszStyle, tch, COUNTOF(tch)))
  {
    lstrcat(szNewStyle, L"; smoothing:");
    lstrcat(szNewStyle, tch);
  }
  if (bDefaultStyle &&
      lf.lfCharSet != DEFAULT_CHARSET &&
      lf.lfCharSet != iDefaultCharSet)
  {
    lstrcat(szNewStyle, L"; charset:");
    wsprintf(tch, L"%i", lf.lfCharSet);
    lstrcat(szNewStyle, tch);
  }
  lstrcat(szNewStyle, L"; size:");
  wsprintf(tch, L"%i", cf.iPointSize / 10);
  lstrcat(szNewStyle, tch);
  if (cf.nFontType & BOLD_FONTTYPE)
    lstrcat(szNewStyle, L"; bold");
  if (cf.nFontType & ITALIC_FONTTYPE)
    lstrcat(szNewStyle, L"; italic");

  if (StrStrI(lpszStyle, L"underline"))
    lstrcat(szNewStyle, L"; underline");

  // save colors
  if (Style_StrGetColor(TRUE, lpszStyle, &iValue))
  {
    wsprintf(tch, L"; fore:#%02X%02X%02X",
             (int)GetRValue(iValue),
             (int)GetGValue(iValue),
             (int)GetBValue(iValue));
    lstrcat(szNewStyle, tch);
  }
  if (Style_StrGetColor(FALSE, lpszStyle, &iValue))
  {
    wsprintf(tch, L"; back:#%02X%02X%02X",
             (int)GetRValue(iValue),
             (int)GetGValue(iValue),
             (int)GetBValue(iValue));
    lstrcat(szNewStyle, tch);
  }

  if (StrStrI(lpszStyle, L"eolfilled"))
    lstrcat(szNewStyle, L"; eolfilled");

  if (Style_StrGetCase(lpszStyle, &iValue))
  {
    lstrcat(szNewStyle, L"; case:");
    lstrcat(szNewStyle, (iValue == SC_CASE_UPPER) ? L"u" : L"");
  }

  if (Style_StrGetAlpha(lpszStyle, &iValue))
  {
    lstrcat(szNewStyle, L"; alpha:");
    wsprintf(tch, L"%i", iValue);
    lstrcat(szNewStyle, tch);
  }

  lstrcpyn(lpszStyle, szNewStyle, cchStyle);
  return TRUE;
}


//=============================================================================
//
//  Style_SelectColor()
//
BOOL Style_SelectColor(HWND hwnd, BOOL bFore, LPWSTR lpszStyle, int cchStyle)
{
  CHOOSECOLOR cc;
  WCHAR szNewStyle[512];
  int  iRGBResult;
  int  iValue;
  WCHAR tch[32];

  ZeroMemory(&cc, sizeof(CHOOSECOLOR));

  iRGBResult = (bFore) ? GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_WINDOW);
  Style_StrGetColor(bFore, lpszStyle, &iRGBResult);

  cc.lStructSize = sizeof(CHOOSECOLOR);
  cc.hwndOwner = hwnd;
  cc.rgbResult = iRGBResult;
  cc.lpCustColors = crCustom;
  cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;

  if (!ChooseColor(&cc))
    return FALSE;

  iRGBResult = cc.rgbResult;

  // Rebuild style string
  lstrcpy(szNewStyle, L"");
  if (Style_StrGetFont(lpszStyle, tch, COUNTOF(tch)))
  {
    lstrcat(szNewStyle, L"font:");
    lstrcat(szNewStyle, tch);
  }
  if (Style_StrGetFontQuality(lpszStyle, tch, COUNTOF(tch)))
  {
    if (lstrlen(szNewStyle))
      lstrcat(szNewStyle, L"; ");
    lstrcat(szNewStyle, L"smoothing:");
    lstrcat(szNewStyle, tch);
  }
  if (Style_StrGetCharSet(lpszStyle, &iValue))
  {
    if (lstrlen(szNewStyle))
      lstrcat(szNewStyle, L"; ");
    wsprintf(tch, L"charset:%i", iValue);
    lstrcat(szNewStyle, tch);
  }
  if (Style_StrGetSizeStr(lpszStyle, tch, COUNTOF(tch)))
  {
    if (lstrlen(szNewStyle))
      lstrcat(szNewStyle, L"; ");
    lstrcat(szNewStyle, L"size:");
    lstrcat(szNewStyle, tch);
  }

  if (StrStrI(lpszStyle, L"bold"))
  {
    if (lstrlen(szNewStyle))
      lstrcat(szNewStyle, L"; ");
    lstrcat(szNewStyle, L"bold");
  }
  if (StrStrI(lpszStyle, L"italic"))
  {
    if (lstrlen(szNewStyle))
      lstrcat(szNewStyle, L"; ");
    lstrcat(szNewStyle, L"italic");
  }
  if (StrStrI(lpszStyle, L"underline"))
  {
    if (lstrlen(szNewStyle))
      lstrcat(szNewStyle, L"; ");
    lstrcat(szNewStyle, L"underline");
  }

  if (bFore)
  {
    if (lstrlen(szNewStyle))
      lstrcat(szNewStyle, L"; ");
    wsprintf(tch, L"fore:#%02X%02X%02X",
             (int)GetRValue(iRGBResult),
             (int)GetGValue(iRGBResult),
             (int)GetBValue(iRGBResult));
    lstrcat(szNewStyle, tch);
    if (Style_StrGetColor(FALSE, lpszStyle, &iValue))
    {
      wsprintf(tch, L"; back:#%02X%02X%02X",
               (int)GetRValue(iValue),
               (int)GetGValue(iValue),
               (int)GetBValue(iValue));
      lstrcat(szNewStyle, tch);
    }
  }
  else
  {
    if (lstrlen(szNewStyle))
      lstrcat(szNewStyle, L"; ");
    if (Style_StrGetColor(TRUE, lpszStyle, &iValue))
    {
      wsprintf(tch, L"fore:#%02X%02X%02X; ",
               (int)GetRValue(iValue),
               (int)GetGValue(iValue),
               (int)GetBValue(iValue));
      lstrcat(szNewStyle, tch);
    }
    wsprintf(tch, L"back:#%02X%02X%02X",
             (int)GetRValue(iRGBResult),
             (int)GetGValue(iRGBResult),
             (int)GetBValue(iRGBResult));
    lstrcat(szNewStyle, tch);
  }

  if (StrStrI(lpszStyle, L"eolfilled"))
    lstrcat(szNewStyle, L"; eolfilled");

  if (Style_StrGetCase(lpszStyle, &iValue))
  {
    lstrcat(szNewStyle, L"; case:");
    lstrcat(szNewStyle, (iValue == SC_CASE_UPPER) ? L"u" : L"");
  }

  if (Style_StrGetAlpha(lpszStyle, &iValue))
  {
    lstrcat(szNewStyle, L"; alpha:");
    wsprintf(tch, L"%i", iValue);
    lstrcat(szNewStyle, tch);
  }

  if (StrStrI(lpszStyle, L"block"))
    lstrcat(szNewStyle, L"; block");

  if (StrStrI(lpszStyle, L"noblink"))
    lstrcat(szNewStyle, L"; noblink");

  lstrcpyn(lpszStyle, szNewStyle, cchStyle);
  return TRUE;
}


//=============================================================================
//
//  Style_SetStyles()
//
void Style_SetStyles(HWND hwnd, int iLexer, int iStyle, LPCWSTR lpszStyle)
{

  WCHAR tch[256];
  WCHAR *p;
  int  iValue;

  // Font
  if (Style_StrGetFont(lpszStyle, tch, COUNTOF(tch)))
  {
    char mch[256] = "Lucida Console";
    if (fIsConsolasAvailable || lstrcmpi(tch, L"Consolas"))
      WideCharToMultiByte(CP_UTF8, 0, tch, -1, mch, COUNTOF(mch), NULL, NULL);
    SendMessage(hwnd, SCI_STYLESETFONT, iStyle, (LPARAM)mch);
  }

  // Size
  if (Style_StrGetSize(lpszStyle, &iValue))
    SendMessage(hwnd, SCI_STYLESETSIZE, iStyle, (LPARAM)iValue);

  // Fore
  if (Style_StrGetColor(TRUE, lpszStyle, &iValue))
    SendMessage(hwnd, SCI_STYLESETFORE, iStyle, (LPARAM)iValue);

  // Back
  if (Style_StrGetColor(FALSE, lpszStyle, &iValue))
    SendMessage(hwnd, SCI_STYLESETBACK, iStyle, (LPARAM)iValue);

  // Bold
  if (p = StrStrI(lpszStyle, L"bold"))
    SendMessage(hwnd, SCI_STYLESETBOLD, iStyle, (LPARAM)TRUE);
  else
    SendMessage(hwnd, SCI_STYLESETBOLD, iStyle, (LPARAM)FALSE);

  // Italic
  if (p = StrStrI(lpszStyle, L"italic"))
    SendMessage(hwnd, SCI_STYLESETITALIC, iStyle, (LPARAM)TRUE);
  else
    SendMessage(hwnd, SCI_STYLESETITALIC, iStyle, (LPARAM)FALSE);

  // Underline
  if (p = StrStrI(lpszStyle, L"underline"))
    SendMessage(hwnd, SCI_STYLESETUNDERLINE, iStyle, (LPARAM)TRUE);
  else
    SendMessage(hwnd, SCI_STYLESETUNDERLINE, iStyle, (LPARAM)FALSE);

  // EOL Filled
  if (p = StrStrI(lpszStyle, L"eolfilled"))
    SendMessage(hwnd, SCI_STYLESETEOLFILLED, iStyle, (LPARAM)TRUE);
  else
    SendMessage(hwnd, SCI_STYLESETEOLFILLED, iStyle, (LPARAM)FALSE);

  // Case
  if (Style_StrGetCase(lpszStyle, &iValue))
    SendMessage(hwnd, SCI_STYLESETCASE, iStyle, (LPARAM)iValue);

  // Character Set
  if (Style_StrGetCharSet(lpszStyle, &iValue))
    SendMessage(hwnd, SCI_STYLESETCHARACTERSET, iStyle, (LPARAM)iValue);
  // [2e]: Default charset for styles #315
  else if (iLexer == SCLEX_NULL)
    SendMessage(hwnd, SCI_STYLESETCHARACTERSET, iStyle, 0);
  // [2e]: New command: Show Outline #432
  else if (iLexer == SCLEX_HTML)
    SciCall_SetProperty("fold.html", "1");
  // [/2e]
}


//=============================================================================
//
//  Style_SetFontQuality()
//
void Style_SetFontQuality(HWND hwnd, LPCWSTR lpszStyle)
{

  WPARAM wQuality = SC_EFF_QUALITY_DEFAULT;
  WCHAR tch[32];
  if (Style_StrGetFontQuality(lpszStyle, tch, COUNTOF(tch)))
  {
    if (lstrcmpi(tch, L"none") == 0)
      wQuality = SC_EFF_QUALITY_NON_ANTIALIASED;
    else if (lstrcmpi(tch, L"standard") == 0)
      wQuality = SC_EFF_QUALITY_ANTIALIASED;
    else if (lstrcmpi(tch, L"cleartype") == 0)
      wQuality = SC_EFF_QUALITY_LCD_OPTIMIZED;
    else
      wQuality = SC_EFF_QUALITY_DEFAULT;
  }
  else
  {
    WCHAR tch[32];
    if (Style_StrGetFont(lpszStyle, tch, COUNTOF(tch)))
    {
      if (lstrcmpi(tch, L"Calibri") == 0 ||
          lstrcmpi(tch, L"Cambria") == 0 ||
          lstrcmpi(tch, L"Candara") == 0 ||
          lstrcmpi(tch, L"Consolas") == 0 ||
          lstrcmpi(tch, L"Constantia") == 0 ||
          lstrcmpi(tch, L"Corbel") == 0 ||
          lstrcmpi(tch, L"Segoe UI") == 0)
        wQuality = SC_EFF_QUALITY_LCD_OPTIMIZED;
    }
    else
      wQuality = SC_EFF_QUALITY_DEFAULT;
  }
  SendMessage(hwnd, SCI_SETFONTQUALITY, wQuality, 0);
}


//=============================================================================
//
//  Style_GetCurrentLexerName()
//
void Style_GetCurrentLexerName(LPWSTR lpszName, int cchName)
{
  if (!GetString(pLexCurrent->rid, lpszName, cchName))
    lstrcpyn(lpszName, pLexCurrent->pszName, cchName);
}


//=============================================================================
//
//  Style_GetLexerIconId()
//
int Style_GetLexerIconId(PEDITLEXER plex)
{
  WCHAR *p;
  WCHAR *pszExtensions;
  WCHAR *pszFile;

  SHFILEINFO shfi;

  if (lstrlen(plex->szExtensions))
    pszExtensions = plex->szExtensions;
  else
    pszExtensions = plex->pszDefExt;

  pszFile = GlobalAlloc(GPTR, sizeof(WCHAR) * (lstrlen(pszExtensions) + CSTRLEN(L"*.txt") + 16));
  lstrcpy(pszFile, L"*.");
  lstrcat(pszFile, pszExtensions);
  if (p = StrChr(pszFile, L';'))
    *p = L'\0';

  // check for ; at beginning
  if (lstrlen(pszFile) < 3)
    lstrcat(pszFile, L"txt");

  SHGetFileInfo(pszFile, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO),
                SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
  GlobalFree(pszFile);

  return (shfi.iIcon);
}


//=============================================================================
//
//  Style_AddLexerToTreeView()
//
void Style_AddLexerToTreeView(HWND hwnd, PEDITLEXER plex)
{
  int i = 0;
  WCHAR tch[128];

  HTREEITEM hTreeNode;

  TVINSERTSTRUCT tvis;
  ZeroMemory(&tvis, sizeof(TVINSERTSTRUCT));

  tvis.hInsertAfter = TVI_LAST;

  tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
  if (GetString(plex->rid, tch, COUNTOF(tch)))
    tvis.item.pszText = tch;
  else
    tvis.item.pszText = plex->pszName;
  tvis.item.iImage = Style_GetLexerIconId(plex);
  tvis.item.iSelectedImage = tvis.item.iImage;
  tvis.item.lParam = (LPARAM)plex;

  hTreeNode = (HTREEITEM)TreeView_InsertItem(hwnd, &tvis);

  tvis.hParent = hTreeNode;

  tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

  while (plex->Styles[i].iStyle != -1)
  {
    if (GetString(plex->Styles[i].rid, tch, COUNTOF(tch)))
      tvis.item.pszText = tch;
    else
      tvis.item.pszText = plex->Styles[i].pszName;
    tvis.item.lParam = (LPARAM)(&plex->Styles[i]);
    TreeView_InsertItem(hwnd, &tvis);
    i++;
  }
}


//=============================================================================
//
//  Style_AddLexerToListView()
//
void Style_AddLexerToListView(HWND hwnd, PEDITLEXER plex)
{
  WCHAR tch[128];
  LVITEM lvi;
  ZeroMemory(&lvi, sizeof(LVITEM));

  lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
  lvi.iItem = ListView_GetItemCount(hwnd);
  if (GetString(plex->rid, tch, COUNTOF(tch)))
    lvi.pszText = tch;
  else
    lvi.pszText = plex->pszName;
  lvi.iImage = Style_GetLexerIconId(plex);
  lvi.lParam = (LPARAM)plex;

  ListView_InsertItem(hwnd, &lvi);
}


//=============================================================================
//
//  Style_ConfigDlgProc()
//
INT_PTR CALLBACK Style_ConfigDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  static HWND hwndTV;
  static BOOL fDragging;
  static PEDITLEXER pCurrentLexer;
  static PEDITSTYLE pCurrentStyle;
  static HFONT hFontTitle;
  static HBRUSH hbrFore;
  static HBRUSH hbrBack;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        int i;
        SHFILEINFO shfi;
        LOGFONT lf;

        hwndTV = GetDlgItem(hwnd, IDC_STYLELIST);
        fDragging = FALSE;

        TreeView_SetImageList(hwndTV,
                              (HIMAGELIST)SHGetFileInfo(L"C:\\", 0, &shfi, sizeof(SHFILEINFO),
                                                          SHGFI_SMALLICON | SHGFI_SYSICONINDEX), TVSIL_NORMAL);

        // Add lexers
        for (i = 0; i < NUMLEXERS; i++)
          Style_AddLexerToTreeView(hwndTV, pLexArray[i]);

        pCurrentLexer = 0;
        pCurrentStyle = 0;

        TreeView_Select(hwndTV, TreeView_GetRoot(hwndTV), TVGN_CARET);

        SendDlgItemMessage(hwnd, IDC_STYLEEDIT, EM_LIMITTEXT, COUNTOF(lexDefault.Styles[0].szValue) - 1, 0);

        MakeBitmapButton(hwnd, IDC_PREVSTYLE, g_hInstance, IDB_PREV);
        MakeBitmapButton(hwnd, IDC_NEXTSTYLE, g_hInstance, IDB_NEXT);

        // Setup title font
        if (hFontTitle)
          DeleteObject(hFontTitle);
        if (NULL == (hFontTitle = (HFONT)SendDlgItemMessage(hwnd, IDC_TITLE, WM_GETFONT, 0, 0)))
          hFontTitle = GetStockObject(DEFAULT_GUI_FONT);
        GetObject(hFontTitle, sizeof(LOGFONT), &lf);
        lf.lfHeight += lf.lfHeight / 5;
        lf.lfWeight = FW_BOLD;
        hFontTitle = CreateFontIndirect(&lf);
        SendDlgItemMessage(hwnd, IDC_TITLE, WM_SETFONT, (WPARAM)hFontTitle, TRUE);

        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      DeleteBitmapButton(hwnd, IDC_STYLEFORE);
      DeleteBitmapButton(hwnd, IDC_STYLEBACK);
      DeleteBitmapButton(hwnd, IDC_PREVSTYLE);
      DeleteBitmapButton(hwnd, IDC_NEXTSTYLE);
      return FALSE;


    case WM_NOTIFY:

      if (((LPNMHDR)(lParam))->idFrom == IDC_STYLELIST)
      {
        LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;

        switch (lpnmtv->hdr.code)
        {

          case TVN_SELCHANGED: {
              if (pCurrentStyle)
                GetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue, COUNTOF(pCurrentStyle->szValue));

              else if (pCurrentLexer)
              {
                if (!GetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentLexer->szExtensions, COUNTOF(pCurrentLexer->szExtensions)))
                  lstrcpy(pCurrentLexer->szExtensions, pCurrentLexer->pszDefExt);
              }

              // a lexer has been selected
              if (!TreeView_GetParent(hwndTV, lpnmtv->itemNew.hItem))
              {
                WCHAR wch[128];

                GetDlgItemText(hwnd, IDC_STYLELABELS, wch, COUNTOF(wch));
                if (StrChr(wch, L'|'))
                  *StrChr(wch, L'|') = 0;

                pCurrentStyle = 0;
                if (pCurrentLexer = (PEDITLEXER)lpnmtv->itemNew.lParam)
                {
                  SetDlgItemText(hwnd, IDC_STYLELABEL, wch);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEEDIT), TRUE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEFONT), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEFORE), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEBACK), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEDEFAULT), TRUE);
                  SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentLexer->szExtensions);
                }
                else
                {
                  SetDlgItemText(hwnd, IDC_STYLELABEL, L"");
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEEDIT), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEFONT), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEFORE), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEBACK), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEDEFAULT), FALSE);
                  SetDlgItemText(hwnd, IDC_STYLEEDIT, L"");
                }
              }

              // a style has been selected
              else
              {
                WCHAR wch[128];

                GetDlgItemText(hwnd, IDC_STYLELABELS, wch, COUNTOF(wch));
                if (StrChr(wch, L'|'))
                  *StrChr(wch, L'|') = 0;


                pCurrentLexer = 0;
                if (pCurrentStyle = (PEDITSTYLE)lpnmtv->itemNew.lParam)
                {
                  SetDlgItemText(hwnd, IDC_STYLELABEL, StrEnd(wch) + 1);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEEDIT), TRUE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEFONT), TRUE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEFORE), TRUE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEBACK), TRUE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEDEFAULT), TRUE);
                  SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue);
                }
                else
                {
                  SetDlgItemText(hwnd, IDC_STYLELABEL, L"");
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEEDIT), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEFONT), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEFORE), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEBACK), FALSE);
                  EnableWindow(GetDlgItem(hwnd, IDC_STYLEDEFAULT), FALSE);
                  SetDlgItemText(hwnd, IDC_STYLEEDIT, L"");
                }
              }
            }
            break;

          case TVN_BEGINDRAG: {

              TreeView_Select(hwndTV, lpnmtv->itemNew.hItem, TVGN_CARET);

              if (pCurrentStyle)
                DestroyCursor(SetCursor(LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_COPY))));
              else
                DestroyCursor(SetCursor(LoadCursor(NULL, IDC_NO)));

              SetCapture(hwnd);
              fDragging = TRUE;
            }

        }
      }

      break;


    case WM_MOUSEMOVE: {
        HTREEITEM htiTarget;
        TVHITTESTINFO tvht;

        if (fDragging && pCurrentStyle)
        {
          LONG xCur = LOWORD(lParam);
          LONG yCur = HIWORD(lParam);

          tvht.pt.x = xCur;
          tvht.pt.y = yCur;

          MapWindowPoints(hwnd, hwndTV, &tvht.pt, 1);

          if ((htiTarget = TreeView_HitTest(hwndTV, &tvht)) != NULL &&
              TreeView_GetParent(hwndTV, htiTarget) != NULL)
          {
            TreeView_SelectDropTarget(hwndTV, htiTarget);
            TreeView_EnsureVisible(hwndTV, htiTarget);
          }
          else
            TreeView_SelectDropTarget(hwndTV, NULL);
        }
      }
      break;


    case WM_LBUTTONUP: {
        if (fDragging)
        {
          HTREEITEM htiTarget;

          if (htiTarget = TreeView_GetDropHilight(hwndTV))
          {
            WCHAR tchCopy[256];
            TreeView_SelectDropTarget(hwndTV, NULL);
            GetDlgItemText(hwnd, IDC_STYLEEDIT, tchCopy, COUNTOF(tchCopy));
            TreeView_Select(hwndTV, htiTarget, TVGN_CARET);

            // after select, this is new current item
            if (pCurrentStyle)
            {
              lstrcpy(pCurrentStyle->szValue, tchCopy);
              SetDlgItemText(hwnd, IDC_STYLEEDIT, tchCopy);
            }
          }
          ReleaseCapture();
          DestroyCursor(SetCursor(LoadCursor(NULL, IDC_ARROW)));
          fDragging = FALSE;
        }
      }
      break;


    case WM_CANCELMODE: {
        if (fDragging)
        {
          TreeView_SelectDropTarget(hwndTV, NULL);
          ReleaseCapture();
          DestroyCursor(SetCursor(LoadCursor(NULL, IDC_ARROW)));
          fDragging = FALSE;
        }
      }
      break;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        case IDC_PREVSTYLE:
          if (TreeView_GetSelection(hwndTV))
            TreeView_Select(hwndTV, TreeView_GetPrevVisible(hwndTV,
                                                            TreeView_GetSelection(hwndTV)), TVGN_CARET);
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          break;

        case IDC_NEXTSTYLE:
          if (TreeView_GetSelection(hwndTV))
            TreeView_Select(hwndTV, TreeView_GetNextVisible(hwndTV,
                                                            TreeView_GetSelection(hwndTV)), TVGN_CARET);
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          break;

        case IDC_STYLEFONT:
          if (pCurrentStyle)
          {
            WCHAR tch[256];
            GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));
            if (Style_SelectFont(hwnd, tch, COUNTOF(tch),
                                 lstrcmpi(pCurrentStyle->pszName, L"Default Style") == 0 ||
                                 lstrcmpi(pCurrentStyle->pszName, L"2nd Default Style") == 0))
            {
              SetDlgItemText(hwnd, IDC_STYLEEDIT, tch);
            }
          }
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          break;

        case IDC_STYLEFORE:
          if (pCurrentStyle)
          {
            WCHAR tch[256];
            GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));
            if (Style_SelectColor(hwnd, TRUE, tch, COUNTOF(tch)))
            {
              SetDlgItemText(hwnd, IDC_STYLEEDIT, tch);
            }
          }
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          break;

        case IDC_STYLEBACK:
          if (pCurrentStyle)
          {
            WCHAR tch[256];
            GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));
            if (Style_SelectColor(hwnd, FALSE, tch, COUNTOF(tch)))
            {
              SetDlgItemText(hwnd, IDC_STYLEEDIT, tch);
            }
          }
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          break;

        case IDC_STYLEDEFAULT:
          if (pCurrentStyle)
          {
            lstrcpy(pCurrentStyle->szValue, pCurrentStyle->pszDefault);
            SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue);
          }
          else if (pCurrentLexer)
          {
            lstrcpy(pCurrentLexer->szExtensions, pCurrentLexer->pszDefExt);
            SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentLexer->szExtensions);
          }
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          break;
        case IDC_STYLEEDIT: {
            if (HIWORD(wParam) == EN_CHANGE)
            {
              int cr;
              WCHAR tch[256];

              GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));

              cr = -1;
              Style_StrGetColor(TRUE, tch, &cr);
              MakeColorPickButton(hwnd, IDC_STYLEFORE, g_hInstance, cr);

              cr = -1;
              Style_StrGetColor(FALSE, tch, &cr);
              MakeColorPickButton(hwnd, IDC_STYLEBACK, g_hInstance, cr);
            }
          }
          break;

        case IDC_IMPORT: {
            HWND hwndTV = GetDlgItem(hwnd, IDC_STYLELIST);

            if (pCurrentStyle)
              GetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue, COUNTOF(pCurrentStyle->szValue));
            else if (pCurrentLexer)
            {
              if (!GetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentLexer->szExtensions, COUNTOF(pCurrentLexer->szExtensions)))
                lstrcpy(pCurrentLexer->szExtensions, pCurrentLexer->pszDefExt);
            }

            if (Style_Import(hwnd))
            {

              if (pCurrentStyle)
                SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue);
              else if (pCurrentLexer)
                SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentLexer->szExtensions);

              TreeView_Select(hwndTV, TreeView_GetRoot(hwndTV), TVGN_CARET);
            }
          }
          break;

        case IDC_EXPORT: {
            if (pCurrentStyle)
              GetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue, COUNTOF(pCurrentStyle->szValue));
            else if (pCurrentLexer)
            {
              if (!GetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentLexer->szExtensions, COUNTOF(pCurrentLexer->szExtensions)))
                lstrcpy(pCurrentLexer->szExtensions, pCurrentLexer->pszDefExt);
            }

            Style_Export(hwnd);
          }
          break;

        case IDC_PREVIEW: {
            // Hack from outside
            extern HWND hwndEdit;
            extern void UpdateLineNumerWidth();

            if (pCurrentStyle)
              GetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue, COUNTOF(pCurrentStyle->szValue));
            else if (pCurrentLexer)
            {
              if (!GetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentLexer->szExtensions, COUNTOF(pCurrentLexer->szExtensions)))
                lstrcpy(pCurrentLexer->szExtensions, pCurrentLexer->pszDefExt);
            }

            Style_SetLexer(hwndEdit, pLexCurrent);
            UpdateLineNumberWidth(hwndEdit);
            UpdateSplitterWndColorAndSize(hwndEdit, TRUE);
          }
          break;

        case IDOK:
          if (pCurrentStyle)
            GetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue, COUNTOF(pCurrentStyle->szValue));
          else if (pCurrentLexer)
          {
            if (!GetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentLexer->szExtensions, COUNTOF(pCurrentLexer->szExtensions)))
              lstrcpy(pCurrentLexer->szExtensions, pCurrentLexer->pszDefExt);
          }
          EndDialog(hwnd, IDOK);
          break;

        case IDCANCEL:
          if (fDragging)
            SendMessage(hwnd, WM_CANCELMODE, 0, 0);
          else
            EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  Style_ConfigDlg()
//
void Style_ConfigDlg(HWND hwnd)
{

  WCHAR *StyleBackup[1024];
  int c, cItems, i, iLexer;

  // Backup Styles
  c = 0;
  for (iLexer = 0; iLexer < NUMLEXERS; iLexer++)
  {
    StyleBackup[c++] = StrDup(pLexArray[iLexer]->szExtensions);
    i = 0;
    while (pLexArray[iLexer]->Styles[i].iStyle != -1)
    {
      StyleBackup[c++] = StrDup(pLexArray[iLexer]->Styles[i].szValue);
      i++;
    }
  }
  cItems = c;

  if (IDCANCEL == ThemedDialogBoxParam(g_hInstance,
                                       MAKEINTRESOURCE(IDD_STYLECONFIG),
                                       GetParent(hwnd), Style_ConfigDlgProc,
                                       (LPARAM)&StyleBackup))
  {
    // Restore Styles
    c = 0;
    for (iLexer = 0; iLexer < NUMLEXERS; iLexer++)
    {
      lstrcpy(pLexArray[iLexer]->szExtensions, StyleBackup[c++]);
      i = 0;
      while (pLexArray[iLexer]->Styles[i].iStyle != -1)
      {
        lstrcpy(pLexArray[iLexer]->Styles[i].szValue, StyleBackup[c++]);
        i++;
      }
    }
  }
  else
  {
    fStylesModified = TRUE;
    if (lstrlen(szIniFile) == 0 && !fWarnedNoIniFile)
    {
      MsgBox(MBWARN, IDS_SETTINGSNOTSAVED);
      fWarnedNoIniFile = TRUE;
    }
  }

  for (c = 0; c < cItems; c++)
    LocalFree(StyleBackup[c]);

  // Apply new (or previous) Styles
  Style_SetLexer(hwnd,pLexCurrent);
  UpdateSplitterWndColorAndSize(hwnd, TRUE);
}


//=============================================================================
//
//  Style_SelectLexerDlgProc()
//
INT_PTR CALLBACK Style_SelectLexerDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  static int cxClient;
  static int cyClient;
  static int mmiPtMaxY;
  static int mmiPtMinX;

  static HWND hwndLV;
  static int  iInternalDefault;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        int i;
        int lvItems;
        LVITEM lvi;
        SHFILEINFO shfi;
        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        RECT rc;
        WCHAR tch[MAX_PATH];
        int cGrip;

        GetClientRect(hwnd, &rc);
        cxClient = rc.right - rc.left;
        cyClient = rc.bottom - rc.top;

        AdjustWindowRectEx(&rc, GetWindowLong(hwnd, GWL_STYLE) | WS_THICKFRAME, FALSE, 0);
        mmiPtMinX = rc.right - rc.left;
        mmiPtMaxY = rc.bottom - rc.top;

        if (cxStyleSelectDlg < (rc.right - rc.left))
          cxStyleSelectDlg = rc.right - rc.left;
        if (cyStyleSelectDlg < (rc.bottom - rc.top))
          cyStyleSelectDlg = rc.bottom - rc.top;
        SetWindowPos(hwnd, NULL, rc.left, rc.top, cxStyleSelectDlg, cyStyleSelectDlg, SWP_NOZORDER);

        SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_THICKFRAME);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

        GetMenuString(GetSystemMenu(GetParent(hwnd), FALSE), SC_SIZE, tch, COUNTOF(tch), MF_BYCOMMAND);
        InsertMenu(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_STRING | MF_ENABLED, SC_SIZE, tch);
        InsertMenu(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);

        SetWindowLongPtr(GetDlgItem(hwnd, IDC_RESIZEGRIP3), GWL_STYLE,
                         GetWindowLongPtr(GetDlgItem(hwnd, IDC_RESIZEGRIP3), GWL_STYLE) | SBS_SIZEGRIP | WS_CLIPSIBLINGS);

        cGrip = GetSystemMetrics(SM_CXHTHUMB);
        SetWindowPos(GetDlgItem(hwnd, IDC_RESIZEGRIP3), NULL, cxClient - cGrip,
                     cyClient - cGrip, cGrip, cGrip, SWP_NOZORDER);

        hwndLV = GetDlgItem(hwnd, IDC_STYLELIST);

        ListView_SetImageList(hwndLV,
                              (HIMAGELIST)SHGetFileInfo(L"C:\\", 0, &shfi, sizeof(SHFILEINFO), SHGFI_SMALLICON | SHGFI_SYSICONINDEX),
                              LVSIL_SMALL);

        ListView_SetImageList(hwndLV,
                              (HIMAGELIST)SHGetFileInfo(L"C:\\", 0, &shfi, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_SYSICONINDEX),
                              LVSIL_NORMAL);

        ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV, 0, &lvc);

        // Add lexers
        for (i = 0; i < NUMLEXERS; i++)
          Style_AddLexerToListView(hwndLV, pLexArray[i]);

        ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE_USEHEADER);

        // Select current lexer
        lvItems = ListView_GetItemCount(hwndLV);
        lvi.mask = LVIF_PARAM;
        for (i = 0; i < lvItems; i++)
        {
          lvi.iItem = i;
          ListView_GetItem(hwndLV, &lvi);;
          if (lstrcmp(((PEDITLEXER)lvi.lParam)->pszName, pLexCurrent->pszName) == 0)
          {
            ListView_SetItemState(hwndLV, i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            ListView_EnsureVisible(hwndLV, i, FALSE);
            if (iDefaultLexer == i)
              CheckDlgButton(hwnd, IDC_DEFAULTSCHEME, BST_CHECKED);
          }
        }

        iInternalDefault = iDefaultLexer;

        if (bAutoSelect)
          CheckDlgButton(hwnd, IDC_AUTOSELECT, BST_CHECKED);

        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY: {
        RECT rc;

        GetWindowRect(hwnd, &rc);
        cxStyleSelectDlg = rc.right - rc.left;
        cyStyleSelectDlg = rc.bottom - rc.top;
      }
      return FALSE;


    case WM_SIZE: {
        RECT rc;

        int dxClient = LOWORD(lParam) - cxClient;
        int dyClient = HIWORD(lParam) - cyClient;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        GetWindowRect(GetDlgItem(hwnd, IDC_RESIZEGRIP3), &rc);
        MapWindowPoints(NULL, hwnd, (LPPOINT)&rc, 2);
        SetWindowPos(GetDlgItem(hwnd, IDC_RESIZEGRIP3), NULL, rc.left + dxClient, rc.top + dyClient, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd, IDC_RESIZEGRIP3), NULL, TRUE);

        GetWindowRect(GetDlgItem(hwnd, IDOK), &rc);
        MapWindowPoints(NULL, hwnd, (LPPOINT)&rc, 2);
        SetWindowPos(GetDlgItem(hwnd, IDOK), NULL, rc.left + dxClient, rc.top + dyClient, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd, IDOK), NULL, TRUE);

        GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rc);
        MapWindowPoints(NULL, hwnd, (LPPOINT)&rc, 2);
        SetWindowPos(GetDlgItem(hwnd, IDCANCEL), NULL, rc.left + dxClient, rc.top + dyClient, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd, IDCANCEL), NULL, TRUE);

        GetWindowRect(GetDlgItem(hwnd, IDC_STYLELIST), &rc);
        MapWindowPoints(NULL, hwnd, (LPPOINT)&rc, 2);
        SetWindowPos(GetDlgItem(hwnd, IDC_STYLELIST), NULL, 0, 0, rc.right - rc.left + dxClient, rc.bottom - rc.top + dyClient,
                     SWP_NOZORDER | SWP_NOMOVE);
        ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_STYLELIST), 0, LVSCW_AUTOSIZE_USEHEADER);
        InvalidateRect(GetDlgItem(hwnd, IDC_STYLELIST), NULL, TRUE);

        GetWindowRect(GetDlgItem(hwnd, IDC_AUTOSELECT), &rc);
        MapWindowPoints(NULL, hwnd, (LPPOINT)&rc, 2);
        SetWindowPos(GetDlgItem(hwnd, IDC_AUTOSELECT), NULL, rc.left, rc.top + dyClient, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd, IDC_AUTOSELECT), NULL, TRUE);

        GetWindowRect(GetDlgItem(hwnd, IDC_DEFAULTSCHEME), &rc);
        MapWindowPoints(NULL, hwnd, (LPPOINT)&rc, 2);
        SetWindowPos(GetDlgItem(hwnd, IDC_DEFAULTSCHEME), NULL, rc.left, rc.top + dyClient, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd, IDC_DEFAULTSCHEME), NULL, TRUE);
        DPI_RESIZE();
      }
      return TRUE;


    case WM_GETMINMAXINFO: {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = mmiPtMinX;
        lpmmi->ptMinTrackSize.y = mmiPtMaxY;
        DPI_GETMINMAXINFO();
      }
      return TRUE;


    case WM_NOTIFY: {
        if (((LPNMHDR)(lParam))->idFrom == IDC_STYLELIST)
        {

          switch (((LPNMHDR)(lParam))->code)
          {

            case NM_DBLCLK:
              SendMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, 1), 0);
              break;

            case LVN_ITEMCHANGED:
            case LVN_DELETEITEM: {
                int i = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
                if (iInternalDefault == i)
                  CheckDlgButton(hwnd, IDC_DEFAULTSCHEME, BST_CHECKED);
                else
                  CheckDlgButton(hwnd, IDC_DEFAULTSCHEME, BST_UNCHECKED);
                EnableWindow(GetDlgItem(hwnd, IDC_DEFAULTSCHEME), i != -1);
                EnableWindow(GetDlgItem(hwnd, IDOK), i != -1);
              }
              break;
          }
        }
      }

      return TRUE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        case IDC_DEFAULTSCHEME:
          if (IsDlgButtonChecked(hwnd, IDC_DEFAULTSCHEME) == BST_CHECKED)
            iInternalDefault = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
          else
            iInternalDefault = 0;
          break;


        case IDOK: {
            LVITEM lvi;

            lvi.mask = LVIF_PARAM;
            lvi.iItem = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
            if (ListView_GetItem(hwndLV, &lvi))
            {
              pLexCurrent = (PEDITLEXER)lvi.lParam;
              iDefaultLexer = iInternalDefault;
              bAutoSelect = (IsDlgButtonChecked(hwnd, IDC_AUTOSELECT) == BST_CHECKED) ? 1 : 0;
              EndDialog(hwnd, IDOK);
            }
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  Style_SelectLexerDlg()
//
void Style_SelectLexerDlg(HWND hwnd)
{
  if (IDOK == ThemedDialogBoxParam(g_hInstance,
                                   MAKEINTRESOURCE(IDD_STYLESELECT),
                                   GetParent(hwnd), Style_SelectLexerDlgProc, 0))
    Style_SetLexer(hwnd, pLexCurrent);
}
