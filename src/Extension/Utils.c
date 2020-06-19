#include "stdafx.h"
#include <assert.h>
#include <time.h>
#include "Utils.h"
#include "CommonUtils.h"
#include "Dialogs.h"
#include "ExtSelection.h"
#include "EditHelper.h"
#include "InlineProgressBarCtrl.h"
#include "MainWndHelper.h"
#include "resource.h"
#include "Scintilla.h"
#include "Helpers.h"
#include "SciCall.h"
#include "SciLexer.h"
#include "Styles.h"
#include "Notepad2.h"
#include "Trace.h"
#include "Subclassing.h"
#include "VersionHelper.h"
#include "Externals.h"
#include "ProcessElevationUtils.h"
#include "Shell32Helper.h"

#define INI_SETTING_HIGHLIGHT_SELECTION L"HighlightSelection"
#define INI_SETTING_EDIT_SELECTION_SCOPE L"EditSelectionScope"
#define INI_SETTING_SAVE_ON_LOSE_FOCUS L"SaveOnLoseFocus"
#define INI_SETTING_WHEEL_SCROLL L"WheelScroll"
#define INI_SETTING_WHEEL_SCROLL_INTERVAL L"WheelScrollInterval"
#define INI_SETTING_CLOCK_FORMAT L"ClockFormat"
#define INI_SETTING_CSS_SETTINGS L"CSSSettings"
#define INI_SETTING_SHELL_MENU_TYPE L"ShellMenuType"
#define INI_SETTING_MAX_SEARCH_DISTANCE L"MaxSearchDistance"
#define INI_SETTING_OPEN_DIALOG_BY_PREFIX L"OpenDialogByPrefix"
#define INI_SETTING_HIGHLIGHT_LINE_IF_WINDOW_INACTIVE L"HighlightLineIfWindowInactive"
#define INI_SETTING_SCROLL_Y_CARET_POLICY L"ScrollYCaretPolicy"
#define INI_SETTING_FIND_WORD_MATCH_CASE L"FindWordMatchCase"
#define INI_SETTING_FIND_WRAP_AROUND L"FindWordWrapAround"
#define INI_SETTING_MOVE_CARET_ON_RIGHT_CLICK L"MoveCaretOnRightClick"
#define INI_SETTING_MATH_EVAL L"MathEval"
#define INI_SETTING_LANGUAGE_INDICATOR L"TitleLanguage"
#define INI_SETTING_WORD_NAVIGATION_MODE L"WordNavigationMode"
#define INI_SETTING_URL_ENCODE_MODE L"UrlEncodeMode"
#ifdef LPEG_LEXER
#define INI_SETTING_LPEG_PATH L"LPegPath"
#endif

#define N2E_WHEEL_TIMER_ID  0xFF
#define DEFAULT_WHEEL_SCROLL_INTERVAL_MS  50
#define DEFAULT_CLOCK_UPDATE_INTERVAL_MS  10000
#define DEFAULT_MAX_SEARCH_DISTANCE_KB  96
#define BYTES_IN_KB  1024

HANDLE g_hScintilla = NULL;
UINT iWheelScrollInterval = DEFAULT_WHEEL_SCROLL_INTERVAL_MS;
BOOL bWheelTimerActive = FALSE;
const UINT iClockUpdateInterval = DEFAULT_CLOCK_UPDATE_INTERVAL_MS;
WCHAR wchClockFormat[MAX_PATH] = { 0 };
UINT_PTR iClockUpdateTimerId = 0;
int iClockMenuItemIndex = -1;
ECSSSettingsMode iCSSSettings = CSS_LESS;
WCHAR wchLastRun[N2E_MAX_PATH_N_CMD_LINE];
EUsePrefixInOpenDialog iUsePrefixInOpenDialog = UPO_AUTO;
BOOL bUsePrefixInOpenDialog = FALSE;
ESaveOnLoseFocus iSaveOnLoseFocus = SLF_DISABLED;
BOOL bCtrlWheelScroll = TRUE;
BOOL bMoveCaretOnRightClick = TRUE;
EExpressionEvaluationMode iEvaluateMathExpression = EEM_LINE;
EWordNavigationMode iWordNavigationMode = WNM_STANDARD;
EUrlEncodeMode iUrlEncodeMode = UEM_IMPROVED;
ELanguageIndicatorMode iShowLanguageInTitle = LIT_SHOW_NON_US;
UINT iShellMenuType = CMF_EXPLORE;
BOOL bHighlightLineIfWindowInactive = FALSE;
long iMaxSearchDistance = DEFAULT_MAX_SEARCH_DISTANCE_KB * BYTES_IN_KB;
EScrollYCaretPolicy iScrollYCaretPolicy = SCP_LEGACY;
BOOL bFindWordMatchCase = FALSE;
BOOL bFindWordWrapAround = FALSE;
HWND hwndStatusProgressBar = NULL;
BOOL bShowProgressBar = FALSE;
BOOL bLPegEnabled = FALSE;
WCHAR wchLPegHomeOrigin[MAX_PATH] = { 0 };
WCHAR g_wchLPegHome[MAX_PATH] = { 0 };

extern HWND  hwndMain;
extern HWND  hwndEdit;
extern WCHAR szTitleExcerpt[128];
extern int iPathNameFormat;
extern WCHAR szCurFile[MAX_PATH + 40];
extern UINT uidsAppTitle;
extern int flagPasteBoard;
extern BOOL fIsElevated;
extern BOOL bModified;
extern int iEncoding;
extern int iOriginalEncoding;
extern BOOL bReadOnly;
extern long iMaxSearchDistance;
extern enum EHighlightCurrentSelectionMode iHighlightSelection;
extern BOOL bEditSelectionScope;
extern LPMRULIST pFileMRU;
extern enum ESaveSettingsMode nSaveSettingsMode;

LPVOID LoadDataFile(const UINT nResourceID, int* pLength);

void n2e_InitInstance()
{
  InitScintillaHandle(hwndEdit);
  n2e_Init(hwndEdit);
  hShellHook = SetWindowsHookEx(WH_SHELL, n2e_ShellProc, NULL, GetCurrentThreadId());
}

void n2e_ExitInstance()
{
  if (hShellHook)
  {
    UnhookWindowsHookEx(hShellHook);
  }
  n2e_Release();
}

void CALLBACK n2e_WheelTimerProc(HWND _h, UINT _u, UINT_PTR idEvent, DWORD _t)
{
  bWheelTimerActive = FALSE;
  KillTimer(NULL, idEvent);
}

BOOL n2e_UpdateClockMenuItem()
{
  time_t t;
  time(&t);
  WCHAR buf[MAX_PATH] = { 0 };
  if ((iClockMenuItemIndex < 0)
      || !lstrlen(wchClockFormat)
      || !wcsftime(buf, sizeof(buf), wchClockFormat, localtime(&t)))
  {
    return FALSE;
  }

  const HMENU hmenu = GetMenu(hwndMain);
  MENUITEMINFO mii = { 0 };
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STRING | MIIM_STATE;
  mii.fState = MFS_DISABLED | MFS_GRAYED | MFS_HILITE;
  mii.dwTypeData = buf;
  mii.cch = lstrlen(buf);
  if (SetMenuItemInfo(hmenu, iClockMenuItemIndex, TRUE, &mii))
  {
    DrawMenuBar(hwndMain);
    return TRUE;
  }
  return FALSE;
}

void CALLBACK n2e_ClockTimerProc(HWND _h, UINT _u, UINT_PTR idEvent, DWORD _t)
{
  n2e_UpdateClockMenuItem();
}

LRESULT CALLBACK n2e_ScintillaSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_CHAR:
    n2e_OnMouseVanishEvent(FALSE);
    break;
  case WM_KILLFOCUS:
  case WM_MOUSEMOVE:
    n2e_OnMouseVanishEvent(TRUE);
    break;
  default:
    break;
  }
  return n2e_CallOriginalWindowProc(hwnd, uMsg, wParam, lParam);
}

void n2e_InitClock()
{
  if (lstrlen(wchClockFormat))
  {
    const HMENU hmenu = GetMenu(hwndMain);
    iClockMenuItemIndex = GetMenuItemCount(hmenu);
    MENUITEMINFO mii = { 0 };
    mii.cbSize = sizeof(mii);
    InsertMenuItem(hmenu, iClockMenuItemIndex, TRUE, &mii);
    n2e_UpdateClockMenuItem();
    iClockUpdateTimerId = SetTimer(NULL, 0, iClockUpdateInterval, n2e_ClockTimerProc);
  }
  else
  {
    iClockUpdateTimerId = 0;
  }
}

void n2e_ReleaseClock()
{
  if (iClockUpdateTimerId)
  {
    KillTimer(NULL, iClockUpdateTimerId);
    const HMENU hmenu = GetMenu(hwndMain);
    DeleteMenu(hmenu, GetMenuItemCount(hmenu) - 1, MF_BYPOSITION);
    DrawMenuBar(hwndMain);
  }
}

BOOL n2e_SaveResourceFile(const UINT nResourceID, LPCWSTR wchTargetPath)
{
  BOOL res = FALSE;
  const HANDLE hFile = CreateFile(wchTargetPath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    return res;
  }
  int iDataLength = 0;
  LPVOID lpData = LoadDataFile(nResourceID, &iDataLength);
  if (lpData && (iDataLength > 0))
  {
    DWORD dwBytesWritten = 0;
    res = WriteFile(hFile, lpData, iDataLength, &dwBytesWritten, NULL) && (dwBytesWritten == iDataLength);
    free(lpData);
  }
  CloseHandle(hFile);
  if (!res)
  {
    DeleteFile(wchTargetPath);
  }
  return res;
}

BOOL n2e_InitLPegHomeDir()
{
#ifdef LPEG_LEXER
  if (lstrlen(g_wchLPegHome) == 0)
  {
    return FALSE;
  }

  if (!PathFileExists(g_wchLPegHome) && (SHCreateDirectoryEx(NULL, g_wchLPegHome, NULL) != ERROR_SUCCESS))
  {
    dwLastIOError = GetLastError();
    MsgBox(MBWARN, IDS_ERR_FAILED_CREATE, L"folder", g_wchLPegHome);
    return FALSE;
  }

  WCHAR wchThemesFolder[MAX_PATH] = { 0 };
  lstrcpy(wchThemesFolder, g_wchLPegHome);
  lstrcat(wchThemesFolder, L"themes");
  if (!PathFileExists(wchThemesFolder) && (SHCreateDirectoryEx(NULL, wchThemesFolder, NULL) != ERROR_SUCCESS))
  {
    dwLastIOError = GetLastError();
    MsgBox(MBWARN, IDS_ERR_FAILED_CREATE, L"folder", wchThemesFolder);
    return FALSE;
  }

  WCHAR wchLexerFile[MAX_PATH] = { 0 };
  lstrcpy(wchLexerFile, g_wchLPegHome);
  lstrcat(wchLexerFile, L"lexer.lua");
  if (!PathFileExists(wchLexerFile) && !n2e_SaveResourceFile(IDR_DATA_LUA_LEXER, wchLexerFile))
  {
    dwLastIOError = GetLastError();
    MsgBox(MBWARN, IDS_ERR_FAILED_CREATE, L"LUA file", wchLexerFile);
    return FALSE;
  }

  WCHAR wchThemeFile[MAX_PATH] = { 0 };
  lstrcpy(wchThemeFile, g_wchLPegHome);
  lstrcat(wchThemeFile, L"themes\\default.lua");
  if (!PathFileExists(wchThemeFile) && !n2e_SaveResourceFile(IDR_DATA_LUA_THEME, wchThemeFile))
  {
    dwLastIOError = GetLastError();
    MsgBox(MBWARN, IDS_ERR_FAILED_CREATE, L"LUA file", wchThemeFile);
    return FALSE;
  }
#endif LPEG_LEXER

  return TRUE;
}

#ifdef LPEG_LEXER
BOOL n2e_MatchLPEGLexer(LPCWSTR lpszExtension)
{
  extern EDITLEXER lexLPEG;

  WCHAR  tch[256 + 16];
  WCHAR  *p1, *p2;
  lstrcpy(tch, lexLPEG.szExtensions);
  p1 = tch;
  while (*p1)
  {
    if (p2 = StrChr(p1, L';'))
      *p2 = L'\0';
    else
      p2 = StrEnd(p1);
    StrTrim(p1, L" .");
    if (lstrcmpi(p1, lpszExtension) == 0)
      return TRUE;
    p1 = p2 + 1;
  }
  return FALSE;
}

WCHAR wchLuaLexerFile[MAX_PATH] = { 0 };

BOOL n2e_UseLuaLexer(LPCWSTR lpszExt, LPBOOL pbLexerFileExists)
{
  LPCWSTR lpszExtension = lpszExt + 1; // skip leading dot char
  lstrcpy(wchLuaLexerFile, g_wchLPegHome);
  lstrcat(wchLuaLexerFile, lpszExtension);
  lstrcat(wchLuaLexerFile, L".lua");

  *pbLexerFileExists = PathFileExists(wchLuaLexerFile);

  return bLPegEnabled
    && n2e_MatchLPEGLexer(lpszExtension)
    && *pbLexerFileExists;
}

char chLexerName[MAX_PATH] = { 0 };

extern PEDITLEXER pLexCurrent;

LPSTR n2e_GetLuaLexerName()
{
  if (!bLPegEnabled || (lstrlen(szCurFile) == 0))
  {
    return NULL;
  }
  LPCWSTR lpszExt = PathFindExtension(szCurFile);
  BOOL bLexerFileExists = FALSE;
  if (n2e_UseLuaLexer(lpszExt, &bLexerFileExists))
  {
    WideCharToMultiByte(CP_UTF8, 0, lpszExt + 1, -1, chLexerName, COUNTOF(chLexerName), NULL, NULL);
    return chLexerName;
  }
  if ((wcslen(wchLuaLexerFile) > 0) && !bLexerFileExists)
  {
    MsgBox(MBWARN, IDS_ERR_LEXER_FILE_NOT_FOUND, wchLuaLexerFile);
    return NULL;
  }
  if ((pLexCurrent->iLexer == SCLEX_LPEG) && (strlen(chLexerName) > 0) && bLexerFileExists)
  {
    return chLexerName;
  }
  return NULL;
}
#endif

void n2e_Init(const HWND hwndEdit)
{
  srand((UINT)GetTickCount());
  n2e_InitializeTrace();
  n2e_SetWheelScroll(bCtrlWheelScroll);
  n2e_InitClock();
  n2e_ResetLastRun();
  n2e_EditInit();
  n2e_Shell32Initialize();
  n2e_SubclassWindow(hwndEdit, n2e_ScintillaSubclassWndProc);
  bLPegEnabled = n2e_InitLPegHomeDir();
}

LPCWSTR n2e_GetLastRun(LPCWSTR lpstrDefault)
{
  LPCWSTR def = wchLastRun;
  if (lstrlen(def) == 0)
  {
    def = lpstrDefault;
  }
  return def;
}

void n2e_SetLastRun(LPCWSTR arg)
{
  lstrcpyn(wchLastRun, arg, _countof(wchLastRun) - 1);
}

void n2e_ResetLastRun()
{
  *wchLastRun = 0;
}

void n2e_ResetSaveOnLoseFocus()
{
  static BOOL bSkipFirstReset = TRUE;
  if (bSkipFirstReset)
  {
    bSkipFirstReset = FALSE;
    return;
  }
  if (iSaveOnLoseFocus == SLF_ENABLED_UNTIL_NEW_FILE)
  {
    iSaveOnLoseFocus = SLF_DISABLED;
  }
}

void n2e_LoadINI()
{
  iHighlightSelection = IniGetInt(N2E_INI_SECTION, INI_SETTING_HIGHLIGHT_SELECTION, iHighlightSelection);
  bEditSelectionScope = IniGetInt(N2E_INI_SECTION, INI_SETTING_EDIT_SELECTION_SCOPE, bEditSelectionScope);
  iSaveOnLoseFocus = IniGetInt(N2E_INI_SECTION, INI_SETTING_SAVE_ON_LOSE_FOCUS, iSaveOnLoseFocus);
  bCtrlWheelScroll = IniGetInt(N2E_INI_SECTION, INI_SETTING_WHEEL_SCROLL, bCtrlWheelScroll);
  iWheelScrollInterval = IniGetInt(N2E_INI_SECTION, INI_SETTING_WHEEL_SCROLL_INTERVAL, iWheelScrollInterval);
  IniGetString(N2E_INI_SECTION, INI_SETTING_CLOCK_FORMAT, L"", wchClockFormat, COUNTOF(wchClockFormat));
  iCSSSettings = IniGetInt(N2E_INI_SECTION, INI_SETTING_CSS_SETTINGS, iCSSSettings);
  iShellMenuType = IniGetInt(N2E_INI_SECTION, INI_SETTING_SHELL_MENU_TYPE, iShellMenuType);
  iMaxSearchDistance = IniGetInt(N2E_INI_SECTION, INI_SETTING_MAX_SEARCH_DISTANCE, DEFAULT_MAX_SEARCH_DISTANCE_KB) * BYTES_IN_KB;
  iUsePrefixInOpenDialog = IniGetInt(N2E_INI_SECTION, INI_SETTING_OPEN_DIALOG_BY_PREFIX, iUsePrefixInOpenDialog);
  bHighlightLineIfWindowInactive = IniGetInt(N2E_INI_SECTION, INI_SETTING_HIGHLIGHT_LINE_IF_WINDOW_INACTIVE, bHighlightLineIfWindowInactive);
  iScrollYCaretPolicy = IniGetInt(N2E_INI_SECTION, INI_SETTING_SCROLL_Y_CARET_POLICY, iScrollYCaretPolicy);
  bFindWordMatchCase = IniGetInt(N2E_INI_SECTION, INI_SETTING_FIND_WORD_MATCH_CASE, bFindWordMatchCase);
  bFindWordWrapAround = IniGetInt(N2E_INI_SECTION, INI_SETTING_FIND_WRAP_AROUND, bFindWordWrapAround);
  bMoveCaretOnRightClick = IniGetInt(N2E_INI_SECTION, INI_SETTING_MOVE_CARET_ON_RIGHT_CLICK, bMoveCaretOnRightClick);
  iEvaluateMathExpression = IniGetInt(N2E_INI_SECTION, INI_SETTING_MATH_EVAL, iEvaluateMathExpression);
  iShowLanguageInTitle = IniGetInt(N2E_INI_SECTION, INI_SETTING_LANGUAGE_INDICATOR, iShowLanguageInTitle);
  iWordNavigationMode = IniGetInt(N2E_INI_SECTION, INI_SETTING_WORD_NAVIGATION_MODE, iWordNavigationMode);
  iUrlEncodeMode = IniGetInt(N2E_INI_SECTION, INI_SETTING_URL_ENCODE_MODE, iUrlEncodeMode);

#ifdef LPEG_LEXER
  IniGetString(N2E_INI_SECTION, INI_SETTING_LPEG_PATH, L"", wchLPegHomeOrigin, COUNTOF(wchLPegHomeOrigin));
  if (lstrlen(wchLPegHomeOrigin) > 0)
  {
    WCHAR szBuf[MAX_PATH] = { 0 };
    if (!ExpandEnvironmentStrings(wchLPegHomeOrigin, szBuf, COUNTOF(szBuf)))
    {
      lstrcpyn(szBuf, wchLPegHomeOrigin, COUNTOF(wchLPegHomeOrigin));
    }
    if (PathIsRelative(szBuf))
    {
      lstrcpy(g_wchLPegHome, g_wchWorkingDirectory);
      PathAddBackslash(g_wchLPegHome);
      lstrcat(g_wchLPegHome, szBuf);
    }
    else
    {
      lstrcpy(g_wchLPegHome, szBuf);
    }
    PathAddBackslash(g_wchLPegHome);
    lstrcpy(szBuf, g_wchLPegHome);
    PathCanonicalize(g_wchLPegHome, szBuf);
  }
  else
  {
    lstrcpy(wchLPegHomeOrigin, g_wchLPegHome);
  }
#endif

  if (iUsePrefixInOpenDialog != UPO_AUTO)
  {
    bUsePrefixInOpenDialog = (iUsePrefixInOpenDialog != UPO_DISABLED);
  }
  else
  {
    bUsePrefixInOpenDialog = TRUE;
    if (IsWindows7OrGreater())
    {
      HKEY hKey;
      if (SUCCEEDED(RegOpenKey(HKEY_CURRENT_USER,
                               L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoComplete",
                               &hKey)))
      {
        DWORD dwType = REG_SZ;
        WCHAR wchValue[MAX_PATH] = { 0 };
        DWORD cbValue = sizeof(wchValue);
        if (SUCCEEDED(RegQueryValueEx(hKey,
                                      L"Append Completion",
                                      NULL,
                                      &dwType,
                                      (LPBYTE)&wchValue,
                                      &cbValue)))
        {
          bUsePrefixInOpenDialog = (StrStrI(wchValue, L"yes") != wchValue);
        }
        RegCloseKey(hKey);
      }
    }
  }
}

void n2e_SaveINI()
{
  IniSetInt(N2E_INI_SECTION, INI_SETTING_HIGHLIGHT_SELECTION, iHighlightSelection);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_EDIT_SELECTION_SCOPE, bEditSelectionScope);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_SAVE_ON_LOSE_FOCUS, iSaveOnLoseFocus);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_WHEEL_SCROLL, bCtrlWheelScroll);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_WHEEL_SCROLL_INTERVAL, iWheelScrollInterval);
  IniSetString(N2E_INI_SECTION, INI_SETTING_CLOCK_FORMAT, wchClockFormat);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_CSS_SETTINGS, iCSSSettings);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_SHELL_MENU_TYPE, iShellMenuType);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_MAX_SEARCH_DISTANCE, iMaxSearchDistance / BYTES_IN_KB);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_OPEN_DIALOG_BY_PREFIX, iUsePrefixInOpenDialog);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_HIGHLIGHT_LINE_IF_WINDOW_INACTIVE, bHighlightLineIfWindowInactive);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_SCROLL_Y_CARET_POLICY, iScrollYCaretPolicy);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_FIND_WORD_MATCH_CASE, bFindWordMatchCase);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_FIND_WRAP_AROUND, bFindWordWrapAround);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_MOVE_CARET_ON_RIGHT_CLICK, bMoveCaretOnRightClick);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_MATH_EVAL, iEvaluateMathExpression);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_LANGUAGE_INDICATOR, iShowLanguageInTitle);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_WORD_NAVIGATION_MODE, iWordNavigationMode);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_URL_ENCODE_MODE, iUrlEncodeMode);
#ifdef LPEG_LEXER
  IniSetString(N2E_INI_SECTION, INI_SETTING_LPEG_PATH, wchLPegHomeOrigin);
#endif
}

void n2e_Release()
{
  n2e_ReleaseClock();
  n2e_SelectionRelease();
  n2e_FinalizeTrace();
  n2e_FinalizeIPC();
}

void n2e_Reset()
{
  n2e_Release();
  n2e_Init(hwndEdit);
}

BOOL n2e_TestOffsetTail(WCHAR *wch)
{
  while (*wch)
  {
    if (isalnum(*wch) && 'h' != *wch)
    {
      return FALSE;
    }
    wch++;
  }
  return TRUE;
}

BOOL n2e_GetGotoNumber(LPTSTR temp, int *out, const BOOL hex)
{
#define RADIX_DEC 10
#define RADIX_HEX 16

  BOOL ok = 0;
  int cou = 0;
  BOOL is_hex = 0;
  WCHAR *ec = 0;
  while (lstrlen(temp))
  {
    if (hex)
    {
      if (isdigit(temp[0]))
      {
        *out = wcstol(temp, &ec, RADIX_DEC);
        if (StrChr(L"abcdefABCDEFxh", *ec))
        {
          *out = wcstol(temp, &ec, RADIX_HEX);
          if (0 == *out)
          {
            return 0;
          }
        }
        ok = n2e_TestOffsetTail(ec);
        N2E_TRACE_PLAIN("Result is  %d (%d)", *out, ok);
        return ok;
      }
      else if (StrChr(L"abcdefABCDEF", temp[0]))
      {
        is_hex = 1;
      }
      else if (L'$' == temp[0])
      {
        temp++;
        is_hex = 1;
      }
      else if (isalnum(temp[0]))
      {
        return 0;
      }
      else
      {
        temp++;
      }
      if (is_hex)
      {
        if (!isalnum(temp[0]))
        {
          return 0;
        }
        *out = wcstol(temp, &ec, RADIX_HEX);
        ok = n2e_TestOffsetTail(ec);
        N2E_TRACE_PLAIN("Result is (hex) %d (ok %d)", *out, ok);
        return ok;
      }
    }
    else
    {
      if (isdigit(temp[0]))
      {
        *out = wcstol(temp, &ec, RADIX_DEC);
        return 1;
      }
      else
      {
        temp++;
      }
    }
  }
  return 0;
}

void n2e_WheelScrollWorker(int lines)
{
  int anch, sel = 0;
  if (bWheelTimerActive)
  {
    N2E_TRACE_PLAIN("wheel timer blocked");
    return;
  }
  bWheelTimerActive = TRUE;
  SetTimer(NULL, N2E_WHEEL_TIMER_ID, iWheelScrollInterval, n2e_WheelTimerProc);
  anch = SendMessage(hwndEdit, SCI_LINESONSCREEN, 0, 0);
  if (lines > 0)
  {
    SendMessage(hwndEdit, SCI_LINESCROLL, 0, anch);
  }
  else if (lines < 0)
  {
    SendMessage(hwndEdit, SCI_LINESCROLL, 0, -anch);
  }
}

void n2e_SetWheelScroll(const BOOL enable)
{
  n2e_wheel_action = enable ? n2e_WheelScrollWorker : 0;
}

extern WCHAR wchWndClass[16];

BOOL CALLBACK n2e_EnumProc(HWND hwnd, LPARAM lParam)
{
  WCHAR szClassName[64];
  if ((hwnd != hwndMain)
      && GetClassName(hwnd, szClassName, COUNTOF(szClassName))
      && (wcsstr(szClassName, wchWndClass) != 0))
  {
    PostMessage(hwnd, WM_N2E_RELOAD_SETTINGS, 0, 0);
  }
  return TRUE;
}

void n2e_Reload_Settings()
{
  EnumWindows(n2e_EnumProc, (LPARAM)hwndMain);
}

BOOL n2e_CanSaveINISection(const BOOL bCheckSaveSettingsMode, const ESaveSettingsMode modeRequired)
{
  return !bCheckSaveSettingsMode || (nSaveSettingsMode == modeRequired);
}

BOOL n2e_IsTextEmpty(LPCWSTR txt)
{
  int t = lstrlen(txt);
  while (--t >= 0)
  {
    if (!iswspace(txt[t]))
    {
      return FALSE;
    }
  }
  return TRUE;
}

BOOL n2e_IsRectangularSelection()
{
  return SciCall_GetSelectionMode() == SC_SEL_RECTANGLE;
}

BOOL n2e_GetCurrentSelection(LPWSTR buf, const int iCount)
{
  BOOL res = FALSE;
  const int iSelStart = SciCall_GetSelStart();
  const int iSelEnd = SciCall_GetSelEnd();
  const int iSelLength = iSelEnd - iSelStart;
  if ((iSelLength > 0) && (iSelLength < iCount * 4))
  {
    LPSTR pSelText = LocalAlloc(LPTR, iSelLength + 1);
    struct TextRange tr = { { iSelStart, iSelEnd }, pSelText };
    if ((SciCall_GetTextRange(0, &tr) > 0)
      && (n2e_MultiByteToWideChar(pSelText, -1, NULL, 0) <= iCount))
    {
      res = (n2e_MultiByteToWideChar(pSelText, -1, buf, iCount) > 0);
    }
    LocalFree(pSelText);
  }
  return res;
}

int n2e_CompareFiles(LPCWSTR sz1, LPCWSTR sz2)
{
  int res1, res2;
  res1 = StrCmp(sz1, sz2);
  if (res1)
  {
    WCHAR b1[MAX_PATH], b2[MAX_PATH];
    StrCpy(b1, sz1);
    StrCpy(b2, sz2);
    PathRemoveExtension(b1);
    PathRemoveExtension(b2);
    res2 = StrCmp(b1, b2);
    if (res2)
    {
      return res2;
    }
  }
  return res1;
}

BOOL n2e_OpenFileByPrefix(LPCWSTR pref, LPWSTR dir, LPWSTR out)
{
  WIN32_FIND_DATA wfd;
  WCHAR path[MAX_PATH];
  WCHAR temp[MAX_PATH], filter[MAX_PATH];
  WCHAR _in[MAX_PATH];
  WCHAR* in = _in;
  HANDLE res;
  int len;
  lstrcpy(_in, pref);
  while ((len = lstrlen(in) - 1) >= 0)
  {
    if (L' ' == in[len])
    {
      in[len] = 0;
    }
    else if (L' ' == in[0])
    {
      ++in;
    }
    else
    {
      break;
    }
  }

  lstrcpy(filter, in);
  lstrcat(filter, L"*");
  PathStripPath(filter);

  if (!PathIsRelative(in))
  {
    lstrcpy(dir, in);
  }
  else
  {
    PathAddBackslash(dir);
    lstrcat(dir, in);
  }
  PathCanonicalize(path, dir);
  PathRemoveFileSpec(path);
  PathAddBackslash(path);
  lstrcat(path, filter);

  res = FindFirstFile(path, &wfd);
  N2E_TRACE("search file by mask '%S'", path);
  if (INVALID_HANDLE_VALUE == res)
  {
    return FALSE;
  }
  temp[0] = 0;
  do
  {
    if (0 == (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      if (0 == temp[0]
          || (PathMatchSpec(wfd.cFileName, filter) && n2e_CompareFiles(temp, wfd.cFileName) > 0))
      {
        lstrcpy(temp, wfd.cFileName);
      }
    }
  } while (FindNextFile(res, &wfd) != 0);
  FindClose(res);
  if (temp[0])
  {
    PathCanonicalize(out, path);
    PathRemoveFileSpec(out);
    PathAddBackslash(out);
    lstrcat(out, temp);
    return TRUE;
  }
  return FALSE;
}

BOOL n2e_FileIsCdUp(LPCWSTR str)
{
  int k;
  for (k = 0; k < lstrlen(str); ++k)
  {
    if (!StrChr(L". ", str[k]))
    {
      return FALSE;
    }
  }
  return TRUE;
}

WCHAR last_selected[MAX_PATH];

UINT_PTR CALLBACK n2e_OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
  static UINT file_ok = 0;
  static BOOL take_call = FALSE;
  HWND hPar = GetParent(hdlg);
  switch (uiMsg)
  {
    case WM_NOTIFY:
      {
        OFNOTIFY *ofn = (OFNOTIFY *)lParam;
        NMHDR nm = ofn->hdr;
        switch (nm.code)
        {
          case CDN_FILEOK:
            {
              WCHAR buf[MAX_PATH];
              WCHAR dir[MAX_PATH];
              //
              int len = CommDlg_OpenSave_GetSpec(hPar, buf, MAX_PATH);
              // can return -1 !!!
              *dir = L'\0';
              if (CommDlg_OpenSave_GetFolderPath(hPar, dir, COUNTOF(dir)) < 0)
              {
                if (CommDlg_OpenSave_GetFilePathW(hPar, dir, COUNTOF(dir)) > 0)
                {
                  PathRemoveFileSpec(dir);
                }
              }
              SetWindowLongPtr(hdlg, DWLP_MSGRESULT, 1);
              N2E_TRACE("OFN OK '%S' ", buf);
              if (len)
              {
                WCHAR out[MAX_PATH];
                LPWSTR final_str = buf;
                if (wcsstr(last_selected, buf))
                {
                  final_str = last_selected;
                  N2E_TRACE("OFN drop window text %S ", buf);
                }
                N2E_TRACE("OFN input (%S) ", final_str);
                if (!n2e_OpenFileByPrefix(final_str, dir, out))
                {
                  if (ofn->lpOFN)
                  {
                    WCHAR mess[1024];
                    wsprintf(mess,
                             L"%s\nFile not found.\n"
                             L"Additionally, no file name starting "
                             L"with this string exists in this folder:\n%s",
                             final_str, dir);
                    MessageBox(hdlg, mess, WC_NOTEPAD2, MB_OK | MB_ICONWARNING);
                  }
                  take_call = TRUE;
                  return 0;
                }
                else
                {
                  CommDlg_OpenSave_SetControlText(hPar, cmb13, (LPARAM)out);
                  if (ofn->lpOFN && ofn->lpOFN->lpstrFile)
                  {
                    lstrcpy(ofn->lpOFN->lpstrFile, out);
                  }
                  N2E_TRACE("OFN final result (%S) ", out);
                  SetWindowLongPtr(hdlg, DWLP_MSGRESULT, 0);
                  take_call = FALSE;
                  return 1;
                }
              }
            }
            return 1;
          case CDN_SELCHANGE:
            {
              WCHAR buf[MAX_PATH];
              N2E_TRACE("OFN sel change  ");
              if ((CommDlg_OpenSave_GetFilePath(hPar, buf, MAX_PATH) > 0) && !PathIsDirectory(buf))
              {
                if (CommDlg_OpenSave_GetSpec(hPar, buf, MAX_PATH) > 0)
                {
                  if (n2e_FileIsCdUp(buf))
                  {
                    *buf = 0;
                  }
                  N2E_TRACE("Set OFN input %S", buf);
                  CommDlg_OpenSave_SetControlText(hPar, cmb13, (LPARAM)buf);
                  lstrcpy(last_selected, buf);
                  return 1;
                }
              }
            }
            break;
          case CDN_INITDONE:
            {
              N2E_TRACE("OFN init  ");
              take_call = FALSE;
              file_ok = RegisterWindowMessage(FILEOKSTRING);
              n2e_SubclassOpenDialog(hPar);
              *last_selected = 0;
            }
            break;
          case CDN_FOLDERCHANGE:
            {
              *last_selected = 0;
            }
            break;
        }
      }
      break;
    default:
      if (file_ok == uiMsg)
      {
        N2E_TRACE("custom OK");
        SetWindowLongPtr(hdlg, DWLP_MSGRESULT, take_call);
      }
  }
  return take_call;
}

BOOL n2e_OpenMRULast(LPWSTR fn)
{
  int i;
  int count;
  WCHAR tch[MAX_PATH];
  WCHAR cd[MAX_PATH];
  BOOL open;
  open = FALSE;
  count = MRU_Enum(pFileMRU, 0, NULL, 0);
  GetCurrentDirectory(COUNTOF(cd), cd);
  for (i = 0; i < count && i < 2; i++)
  {
    MRU_Enum(pFileMRU, i, tch, COUNTOF(tch));
    N2E_WTRACE_PLAIN("mru '%s'", tch);
    PathAbsoluteFromApp(tch, NULL, 0, TRUE);
    N2E_WTRACE_PLAIN("mru full '%s'", tch);
    if (0 == i || open)
    {
      lstrcpy(fn, tch);
      if (open)
      {
        break;
      }
    }
    if (0 == lstrcmp(tch, szCurFile) && i < count - 1)
    {
      open = TRUE;
    }
  }
  N2E_WTRACE_PLAIN("check for path '%s'", fn);
  if (!PathFileExists(fn))
  {
    N2E_WTRACE_PLAIN("no path '%s'", fn);
    if (IDYES == MsgBox(MBYESNO, IDS_ERR_MRUDLG))
    {
      MRU_DeleteFileFromStore(pFileMRU, fn);
      MRU_Destroy(pFileMRU);
      pFileMRU = MRU_Create(L"Recent Files", MRU_NOCASE, 32);
      MRU_Load(pFileMRU);
    }
    return 0;
  }
  if (i > 0 && lstrcmp(fn, szCurFile))
  {
    MRU_Add(pFileMRU, fn);
    return TRUE;
  }
  return FALSE;
}

void n2e_GetLastDir(LPTSTR out)
{
  WCHAR tch[MAX_PATH];
  INT count = MRU_Enum(pFileMRU, 0, NULL, 0);
  if (count)
  {
    MRU_Enum(pFileMRU, 0, tch, COUNTOF(tch));
    N2E_WTRACE_PLAIN("OFN mru '%s'", tch);
    lstrcpy(out, tch);
    PathRemoveFileSpec(out);
    if (PathIsRelative(out))
    {
      WCHAR tchModule[MAX_PATH];
      GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));
      PathRemoveFileSpec(tchModule);
      PathAppend(tchModule, out);
      PathCanonicalize(out, tchModule);
    }
    N2E_WTRACE_PLAIN("OFN mru final '%s'", out);
  }
  else
  {
    lstrcpy(out, g_wchWorkingDirectory);
  }
}

LPCWSTR n2e_GetExePath()
{
  static WCHAR tchExePath[N2E_MAX_PATH_N_CMD_LINE] = { 0 };
  if (lstrlen(tchExePath) == 0)
  {
    int nArgs = 0;
    LPWSTR* szArglist = CommandLineToArgvW(GetCommandLine(), &nArgs);
    if (szArglist)
    {
      wcscpy_s(tchExePath, _countof(tchExePath) - 1, szArglist[0]);
      LocalFree(szArglist);
    }
  }
  return tchExePath;
}

BOOL n2e_Grep(void* _lpf, const BOOL grep)
{
  LPEDITFINDREPLACE lpf = (LPEDITFINDREPLACE)_lpf;
  int k = 0;
  int res = 0;
  int line_first, line_last;
  char szFind2[512];

  struct Sci_TextToFind ttf;
  if (!lstrlenA(lpf->szFind))
  {
    return FALSE;
  }

  if (!n2e_IsFindReplaceAvailable(lpf))
  {
    return FALSE;
  }

  const int selStart = SciCall_GetSelStart();
  const int selEnd = SciCall_GetSelEnd();
  line_first = SciCall_LineFromPosition(selStart);
  line_last = SciCall_LineFromPosition(selEnd);
  if (line_last - line_first + 1 <= 2)
  {
    line_first = 0;
    line_last = SciCall_GetLineCount() - 1;
  }

  lstrcpynA(szFind2, lpf->szFind, COUNTOF(szFind2));
  ZeroMemory(&ttf, sizeof(ttf));
  if (lpf->bTransformBS)
  {
    TransformBackslashes(szFind2, (lpf->fuFlags & SCFIND_REGEXP),
      (UINT)SendMessage(lpf->hwnd, SCI_GETCODEPAGE, 0, 0));
  }
  if (lstrlenA(szFind2) == 0)
  {
    return FALSE;
  }

  BeginWaitCursor();
  SendMessage(lpf->hwnd, SCI_BEGINUNDOACTION, 0, 0);
  ttf.lpstrText = szFind2;
  ttf.chrg.cpMin = SciCall_LineEndPosition(line_last);
  ttf.chrg.cpMax = SciCall_PositionFromLine(line_first);
  const int maxPos = ttf.chrg.cpMin;
  n2e_ShowProgressBarInStatusBar(grep ? L"Applying Grep..." : L"Applying Ungrep...", 1, maxPos);

  res = SciCall_FindText(lpf->fuFlags, &ttf);
  while (ttf.chrg.cpMin > ttf.chrg.cpMax)
  {
    int posFrom = ttf.chrg.cpMax;
    int posTo = ttf.chrg.cpMin;
    if (res >= 0)
    {
      const int lineIndex = SciCall_LineFromPosition(res);
      const int posStart = SciCall_PositionFromLine(lineIndex);
      n2e_SetProgressBarPosInStatusBar(maxPos - res);
      if (grep)
      {
        posFrom = SciCall_LineEndPosition(lineIndex);
        posTo = ttf.chrg.cpMin;
        ttf.chrg.cpMin = (lineIndex > 0) ? SciCall_LineEndPosition(lineIndex-1) : SciCall_PositionFromLine(lineIndex);
      }
      else
      {
        posFrom = (lineIndex > 0) ? SciCall_LineEndPosition(lineIndex-1) : posStart;
        posTo = (lineIndex > 0) ? SciCall_LineEndPosition(lineIndex) : SciCall_PositionFromLine(lineIndex+1);
        ttf.chrg.cpMin = posStart;
      }
    }
    else
    {
      posTo = grep ? SciCall_PositionFromLine(SciCall_LineFromPosition(ttf.chrg.cpMin) + 1) : posFrom;
      ttf.chrg.cpMin = ttf.chrg.cpMax;
    }
    if (posTo != posFrom)
    {
      SciCall_DeleteRange(posFrom, posTo - posFrom);
    }
    res = SciCall_FindText(lpf->fuFlags, &ttf);
  }

  SendMessage(lpf->hwnd, SCI_ENDUNDOACTION, 0, 0);
  UpdateLineNumberWidth();
  n2e_HideProgressBarInStatusBar();
  EndWaitCursor();
  return TRUE;
}

void n2e_InplaceRev(WCHAR * s)
{
  WCHAR t, *e = s + lstrlen(s);
  while (--e > s)
  {
    t = *s; *s++ = *e; *e = t;
  }
}

BOOL n2e_IsWordChar(const WCHAR ch)
{
  return IsCharAlphaNumericW(ch) || (ch == L'_');
}

BOOL n2e_SetClipboardText(const HWND hwnd, const wchar_t* text)
{
  if ((wcslen(text) <= 0) || !OpenClipboard(hwnd))
  {
    return FALSE;
  }

  HANDLE hNew = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(WCHAR) * (lstrlen(text) + 1));
  LPWSTR pszNew = hNew ? GlobalLock(hNew) : NULL;
  if (!hNew || !pszNew)
  {
    return FALSE;
  }

  lstrcpy(pszNew, text);
  GlobalUnlock(hNew);
  EmptyClipboard();
  SetClipboardData(CF_UNICODETEXT, hNew);
  CloseClipboard();

  return TRUE;
}

void n2e_UpdateWindowTitle(const HWND hwnd)
{
  SetWindowTitle(hwnd, uidsAppTitle, flagPasteBoard, fIsElevated, IDS_UNTITLED, szCurFile,
                 iPathNameFormat, bModified || iEncoding != iOriginalEncoding,
                 IDS_READONLY, bReadOnly, szTitleExcerpt);
}

int n2e_GetCurrentShowTitleMenuID()
{
  if (lstrlen(szTitleExcerpt))
  {
    return IDM_VIEW_SHOWEXCERPT;
  }
  else switch (iPathNameFormat)
  {
    case PNM_FILENAMEONLY:
      return IDM_VIEW_SHOWFILENAMEONLY;
    case PNM_FILENAMEFIRST:
      return IDM_VIEW_SHOWFILENAMEFIRST;
    case PNM_FULLPATH:
    default:
      return IDM_VIEW_SHOWFULLPATH;
  }
}

int n2e_GetCurrentLanguageIndicatorMenuID()
{
  switch (iShowLanguageInTitle)
  {
    case LIT_HIDE:
      return IDM_VIEW_NOLANGUAGEINDICATOR;
    case LIT_SHOW:
      return IDM_VIEW_SHOWLANGUAGEINDICATOR;
    case LIT_SHOW_NON_US:
      return IDM_VIEW_SHOWLANGUAGEINDICATORNONUS;
    default:
      assert(FALSE);
      return 0;
  }
}

int n2e_GetCurrentSaveSettingsMenuID()
{
  switch (nSaveSettingsMode)
  {
  case SSM_ALL:
    return IDM_VIEW_SAVESETTINGS_MODE_ALL;
  case SSM_RECENT:
    return IDM_VIEW_SAVESETTINGS_MODE_RECENT;
  case SSM_NO:
    return IDM_VIEW_SAVESETTINGS_MODE_NO;
  default:
    assert(FALSE);
    return 0;
  }
}


int n2e_GetCurrentSaveOnLoseFocusMenuID()
{
  switch (iSaveOnLoseFocus)
  {
  case SLF_DISABLED:
    return ID_SAVEONLOSEFOCUS_DISABLED;
  case SLF_ENABLED:
    return ID_SAVEONLOSEFOCUS_ENABLED;
  case SLF_ENABLED_UNTIL_NEW_FILE:
    return ID_SAVEONLOSEFOCUS_ENABLEDUNTILANEWFILE;
  default:
    assert(FALSE);
    return 0;
  }
}

int n2e_GetCurrentHighlightCurrentSelectionMenuID()
{
  switch (iHighlightSelection)
  {
  case HCS_DISABLED:
    return IDM_VIEW_HIGHLIGHTCURRENTSELECTION_DISABLED;
  case HCS_WORD:
    return IDM_VIEW_HIGHLIGHTCURRENTSELECTION_WORD;
  case HCS_SELECTION:
    return IDM_VIEW_HIGHLIGHTCURRENTSELECTION_SELECTION;
  case HCS_WORD_AND_SELECTION:
    return IDM_VIEW_HIGHLIGHTCURRENTSELECTION_WORDANDSELECTION;
  case HCS_WORD_IF_NO_SELECTION:
    return IDM_VIEW_HIGHLIGHTCURRENTSELECTION_WORDIFNOSELECTION;
  default:
    assert(FALSE);
    return 0;
  }
}

int n2e_GetCurrentEvalMenuID()
{
  switch (iEvaluateMathExpression)
  {
  case EEM_DISABLED:
    return ID_SETTINGS_EVAL_DISABLED;
  case EEM_SELECTION:
    return ID_SETTINGS_EVAL_SELECTION;
  case EEM_LINE:
    return ID_SETTINGS_EVAL_LINE;
  default:
    assert(FALSE);
    return 0;
  }
}

void n2e_CreateProgressBarInStatusBar()
{
  hwndStatusProgressBar = InlineProgressBarCtrl_Create(hwndStatus, 0, 100, TRUE, STATUS_LEXER);
}

void n2e_DestroyProgressBarInStatusBar()
{
  DestroyWindow(hwndStatusProgressBar);
  hwndStatusProgressBar = NULL;
}

void n2e_ShowProgressBarInStatusBar(LPCWSTR pProgressText, const long nCurPos, const long nMaxPos)
{
  if (hwndStatusProgressBar)
  {
    wcscpy_s(tchProgressBarTaskName, _countof(tchProgressBarTaskName), pProgressText);
    bShowProgressBar = TRUE;
    InlineProgressBarCtrl_SetRange(hwndStatusProgressBar, nCurPos, nMaxPos, 1);
    InlineProgressBarCtrl_SetPos(hwndStatusProgressBar, nCurPos);
    InlineProgressBarCtrl_Resize(hwndStatusProgressBar);
    ShowWindow(hwndStatusProgressBar, SW_SHOW);
    UpdateStatusbar();
  }
}

void n2e_HideProgressBarInStatusBar()
{
  if (hwndStatusProgressBar)
  {
    bShowProgressBar = FALSE;
    wcscpy_s(tchProgressBarTaskName, _countof(tchProgressBarTaskName), L"");
    ShowWindow(hwndStatusProgressBar, SW_HIDE);
    UpdateStatusbar();
  }
}

void n2e_SetProgressBarPosInStatusBar(const long nCurPos)
{
  InlineProgressBarCtrl_SetPos(hwndStatusProgressBar, nCurPos);
}

void n2e_IncProgressBarPosInStatusBar(const long nOffset)
{
  InlineProgressBarCtrl_IncPos(hwndStatusProgressBar, nOffset);
}

int n2e_JoinParagraphs_GetSelEnd(const int iSelEnd)
{
  int res = iSelEnd;
  int iLastLine = SciCall_LineFromPosition(iSelEnd);
  while ((iLastLine > 0) && (SciCall_PositionFromLine(iLastLine) == res))
  {
    --iLastLine;
    res = SciCall_LineEndPosition(iLastLine);
  }
  return res;
}

int n2e_GetNonSpaceCharPos(const int iLine, const BOOL bFromLineStart)
{
  int res = -1;
  struct TextRange tr = { 0 };
  tr.chrg.cpMin = SciCall_PositionFromLine(iLine);
  tr.chrg.cpMax = SciCall_LineEndPosition(iLine);
  const int iLineLength = tr.chrg.cpMax - tr.chrg.cpMin;
  if (iLineLength > 0)
  {
    tr.lpstrText = n2e_Alloc(iLineLength + 1);
    if (SciCall_GetTextRange(0, &tr) > 0)
    {
      int i = bFromLineStart ? 0 : iLineLength - 1;
      while (bFromLineStart ? (i < iLineLength) : (i >= 0))
      {
        if (!isspace(tr.lpstrText[i]))
        {
          res = i;
          break;
        }
        bFromLineStart ? ++i : --i;
      }
    }
    n2e_Free(tr.lpstrText);
  }
  return res;
}

int n2e_GetFirstNonSpaceCharPos(const int iLine)
{
  return n2e_GetNonSpaceCharPos(iLine, TRUE);
}

int n2e_GetLastNonSpaceCharPos(const int iLine)
{
  return n2e_GetNonSpaceCharPos(iLine, FALSE);
}

BOOL n2e_IsEmptyLine(const int iLine)
{
  return (n2e_GetNonSpaceCharPos(iLine, TRUE) < 0);
}

int n2e_JoinLines_GetSelEnd(const int iSelStart, const int iSelEnd, BOOL *pbContinueProcessing)
{
  int res = iSelEnd;
  if (iSelStart != iSelEnd)
  {
    res = n2e_JoinParagraphs_GetSelEnd(iSelEnd);
  }
  else
  {
    const int iLineCount = SciCall_GetLineCount();
    const int iLineStart = SciCall_LineFromPosition(iSelStart);
    int iLine = iLineStart;
    BOOL bContinue = TRUE;
    while (bContinue && (iLine < iLineCount))
    {
      ++iLine;
      if (!n2e_IsEmptyLine(iLine))
      {
        const int _iSelStart = SciCall_PositionFromLine(iLineStart) + n2e_GetLastNonSpaceCharPos(iLineStart) + 1;
        const int _iSelEnd = SciCall_PositionFromLine(iLine) + n2e_GetFirstNonSpaceCharPos(iLine);
        SciCall_SetSel(_iSelStart, _iSelEnd);
        SciCall_ReplaceSel(0, " ");
        if (pbContinueProcessing)
        {
          *pbContinueProcessing = FALSE;
        }
        bContinue = FALSE;
        break;
      }
    }
  }
  return res;
}

LRESULT CALLBACK n2e_About3rdPartyRicheditWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_GETDLGCODE)
  {
    return n2e_CallOriginalWindowProc(hwnd, uMsg, wParam, lParam) & ~DLGC_HASSETSEL;
  }
  return n2e_CallOriginalWindowProc(hwnd, uMsg, wParam, lParam);
}

LPVOID LoadDataFile(const UINT nResourceID, int* pLength)
{
  HGLOBAL hGlob = NULL;
  LPCVOID lpData = NULL;
  LPVOID lpResult = NULL;
  HRSRC hRes = FindResource(g_hInstance, MAKEINTRESOURCE(nResourceID), L"DATA");
  if (hRes)
  {
    hGlob = LoadResource(g_hInstance, hRes);
  }
  if (hGlob)
  {
    lpData = LockResource(hGlob);
  }
  if (lpData)
  {
    const int size = hRes ? SizeofResource(g_hInstance, hRes) : 0;
    if (pLength)
    {
      *pLength = size;
    }
    lpResult = (size > 0) ? malloc(*pLength) : NULL;
    if (lpResult)
    {
      memcpy(lpResult, lpData, size);
    }
  }
  if (hGlob)
  {
    FreeLibrary(g_hInstance);
  }
  return lpResult;
}

LPCWSTR LoadAbout3rdPartyText(int* pLength)
{
  static HRSRC hRes = NULL;
  static HGLOBAL hGlob = NULL;
  static LPCWSTR lpRTF = NULL;
  if (!hRes)
  {
    hRes = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_ABOUT_3RD_PARTY), L"RTF");
  }
  if (hRes && !hGlob)
  {
    hGlob = LoadResource(g_hInstance, hRes);
  }
  if (hGlob && !lpRTF)
  {
    lpRTF = (LPCWSTR)LockResource(hGlob);
  }
  if (pLength)
  {
    *pLength = hRes ? SizeofResource(g_hInstance, hRes) : 0;
  }
  return lpRTF;
}

struct TRTFData
{
  LPCWSTR lpData;
  LONG nLength;
  LONG nOffset;
};
typedef struct TRTFData RTFData;

RTFData rtfData = { 0 };

DWORD CALLBACK EditStreamCallBack(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
  RTFData* prtfData = (RTFData*)dwCookie;
  if (prtfData->nLength < cb)
  {
    *pcb = prtfData->nLength;
    memcpy(pbBuff, (LPCSTR)prtfData->lpData, *pcb);
  }
  else
  {
    *pcb = cb;
    memcpy(pbBuff, (LPCSTR)(prtfData->lpData + prtfData->nOffset), *pcb);
    prtfData->nOffset += cb;
  }
  return 0;
}

void n2e_InitAbout3rdPartyText(const HWND hwndRichedit)
{
  n2e_SubclassWindow(hwndRichedit, n2e_About3rdPartyRicheditWndProc);

  SendMessage(hwndRichedit, EM_SETEVENTMASK, 0,
    SendMessage(hwndRichedit, EM_GETEVENTMASK, 0, 0) | ENM_LINK);
  SendMessage(hwndRichedit, EM_AUTOURLDETECT, TRUE, 0);

  if (!rtfData.lpData)
  {
    rtfData.lpData = LoadAbout3rdPartyText(&rtfData.nLength);
  }
  rtfData.nOffset = 0;

  EDITSTREAM es = { (DWORD_PTR)&rtfData, 0, EditStreamCallBack };
  SendMessage(hwndRichedit, EM_STREAMIN, SF_RTF, (LPARAM)&es);
  SendMessage(hwndRichedit, EM_SETTARGETDEVICE, (WPARAM)NULL, 0);
}

void n2e_ProcessAbout3rdPartyUrl(const HWND hwndRichedit, ENLINK* pENLink)
{
  if (pENLink->msg == WM_LBUTTONUP)
  {
    LPWSTR pUrl = (LPWSTR)n2e_Alloc((pENLink->chrg.cpMax - pENLink->chrg.cpMin + 1) * sizeof(WCHAR));
    TEXTRANGE tr = { pENLink->chrg, pUrl };
    if (SendMessage(hwndRichedit, EM_GETTEXTRANGE, 0, (LPARAM)&tr) > 0)
    {
      ShellExecute(GetParent(hwndRichedit), L"open", pUrl, NULL, NULL, SW_SHOWNORMAL);
    }
    n2e_Free(pUrl);
  }
}

long n2e_GenerateRandom()
{
  const long MIN_RANDOM = 1;
  const long MAX_DECIMAL_DIGITS = 5;
  long factor = 1;
  long res = 0;
  for (int i = 0; i < MAX_DECIMAL_DIGITS; ++i)
  {
    res += (rand() % 10) * factor;
    factor *= 10;
  }
  return max(MIN_RANDOM, res);
}

void n2e_SetCheckedRadioButton(const HWND hwnd, const int idFirst, const int idLast, const int selectedIndex)
{
  CheckRadioButton(hwnd, idFirst, idLast, idFirst + selectedIndex);
}

int n2e_GetCheckedRadioButton(const HWND hwnd, const int idFirst, const int idLast)
{
  int res = -1;
  for (int id = idFirst; id <= idLast; ++id)
  {
    if (IsDlgButtonChecked(hwnd, id) & BST_CHECKED)
    {
      res = id - idFirst;
      break;
    }
  }
  return res;
}

void n2e_UpdateFavLnkParams(TADDFAVPARAMS* lpParams)
{
  const BOOL bQuoteSelection = (wcschr(lpParams->pszCurrentSelection, L' ') != NULL);
  switch (lpParams->cursorPosition)
  {
  case FCP_FIRST_LINE:
    break;
  case FCP_LAST_LINE:
  case FCP_CURRENT_LINE:
    PathQuoteSpaces(lpParams->pszTarget);
    _swprintf(lpParams->pszArguments, L"/g %d %s",
              (lpParams->cursorPosition == FCP_LAST_LINE) ? -1 : SciCall_LineFromPosition(SciCall_GetSelStart()) + 1,
              lpParams->pszTarget);
    _swprintf(lpParams->pszTarget, L"%s", n2e_GetExePath());
    break;
  case FCP_CURRENT_SELECTION:
    PathQuoteSpaces(lpParams->pszTarget);
    _swprintf(lpParams->pszArguments, L"/gs %d:%d %s", SciCall_GetSelStart(), SciCall_GetSelEnd(), lpParams->pszTarget);
    _swprintf(lpParams->pszTarget, L"%s", n2e_GetExePath());
    break;
  case FCP_FIRST_SUBSTRING:
    PathQuoteSpaces(lpParams->pszTarget);
    _swprintf(lpParams->pszArguments,
              bQuoteSelection ? L"/m \"%s\" %s" : L"/m %s %s",
              lpParams->pszCurrentSelection,
              lpParams->pszTarget);
    _swprintf(lpParams->pszTarget, L"%s", n2e_GetExePath());
    break;
  case FCP_LAST_SUBSTRING:
    PathQuoteSpaces(lpParams->pszTarget);
    _swprintf(lpParams->pszArguments,
              bQuoteSelection ? L"/m- \"%s\" %s" : L"/m- %s %s",
              lpParams->pszCurrentSelection,
              lpParams->pszTarget);
    _swprintf(lpParams->pszTarget, L"%s", n2e_GetExePath());
    break;
  default:
    break;
  }
}

void n2e_EditJumpTo(const HWND hwnd, const int iNewLine, const int iNewCol, const int iNewSelStart, const int iNewSelEnd)
{
  if (iNewSelStart == -1)
  {
    EditJumpTo(hwnd, iNewLine, iNewCol);
  }
  else
  {
    SciCall_SetXCaretPolicy(CARET_SLOP | CARET_STRICT | CARET_EVEN, 50);
    SciCall_SetYCaretPolicy(CARET_SLOP | CARET_STRICT | CARET_EVEN, 5);

    SciCall_GotoPos(iNewSelStart + 1);
    SciCall_CharLeftExtEnd();
    SciCall_SetSel(SciCall_GetCurrentPos(), iNewSelEnd - 1);
    SciCall_CharRightExtEnd();
    SciCall_ChooseCaretX();

    SciCall_SetXCaretPolicy(CARET_SLOP | CARET_EVEN, 50);
    SciCall_SetYCaretPolicy(CARET_EVEN, 0);
  }
}

HWND n2e_ToolTipCreate(const HWND hwndParent)
{
  return CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP,
          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndParent, NULL, NULL, NULL);
}

BOOL n2e_ToolTipAddControl(const HWND hwndToolTip, const HWND hwndControl, LPTSTR pszText)
{
  const HWND hwndParent = GetParent(hwndToolTip);
  if (!hwndToolTip || !IsWindow(hwndToolTip) || !hwndParent)
  {
    return FALSE;
  }

  TOOLINFO ti = { 0 };
  ti.cbSize = sizeof(ti);
  ti.hwnd = hwndParent;
  ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
  ti.uId = (UINT_PTR)hwndControl;
  ti.lpszText = pszText;

  return SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti) == TRUE;
}

BOOL n2e_ToolTipAddToolInfo(const HWND hwndToolTip, LPVOID lpToolInfo)
{
  const HWND hwndParent = GetParent(hwndToolTip);
  if (!hwndToolTip || !IsWindow(hwndToolTip) || !hwndParent)
  {
    return FALSE;
  }

  return SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)lpToolInfo) == TRUE;
}

BOOL n2e_ToolTipSetToolInfo(const HWND hwndToolTip, LPVOID lpToolInfo)
{
  const HWND hwndParent = GetParent(hwndToolTip);
  if (!hwndToolTip || !IsWindow(hwndToolTip) || !hwndParent)
  {
    return FALSE;
  }

  return SendMessage(hwndToolTip, TTM_SETTOOLINFO, 0, (LPARAM)lpToolInfo) == TRUE;
}

void n2e_ToolTipTrackPosition(const HWND hwndToolTip, const POINT pt)
{
  const HWND hwndParent = GetParent(hwndToolTip);
  if (!hwndToolTip || !IsWindow(hwndToolTip) || !hwndParent)
  {
    return;
  }

  SendMessage(hwndToolTip, TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));
}

void n2e_ToolTipTrackActivate(const HWND hwndToolTip, const BOOL bActivate, LPVOID lpToolInfo)
{
  const HWND hwndParent = GetParent(hwndToolTip);
  if (!hwndToolTip || !IsWindow(hwndToolTip) || !hwndParent)
  {
    return;
  }

  SendMessage(hwndToolTip, TTM_TRACKACTIVATE, bActivate, (LPARAM)lpToolInfo);
}
