#include "Utils.h"
#include "Dialogs.h"
#include "ExtSelection.h"
#include "EditHelper.h"
#include "InlineProgressBarCtrl.h"
#include "MainWndHelper.h"
#include "resource.h"
#include "Scintilla.h"
#include "Helpers.h"
#include "SciCall.h"
#include "Notepad2.h"
#include "Trace.h"

#define INI_SETTING_HIGHLIGHT_SELECTION L"HighlightSelection"
#define INI_SETTING_WHEEL_SCROLL L"WheelScroll"
#define INI_SETTING_WHEEL_SCROLL_INTERVAL L"WheelScrollInterval"
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

#define N2E_WHEEL_TIMER_ID  0xFF
#define DEFAULT_WHEEL_SCROLL_INTERVAL_MS  50
#define DEFAULT_MAX_SEARCH_DISTANCE_KB  96
#define BYTES_IN_KB  1024

HANDLE g_hScintilla = NULL;
UINT iWheelScrollInterval = DEFAULT_WHEEL_SCROLL_INTERVAL_MS;
BOOL bWheelTimerActive = FALSE;
ECSSSettingsMode iCSSSettings = CSS_LESS;
WCHAR wchLastRun[N2E_MAX_PATH_N_CMD_LINE];
BOOL bUsePrefixInOpenDialog = TRUE;
BOOL bCtrlWheelScroll = TRUE;
BOOL bMoveCaretOnRightClick = TRUE;
EExpressionEvaluationMode iEvaluateMathExpression = EEM_DISABLED;
EWordNavigationMode iWordNavigationMode = 0;
ELanguageIndicatorMode iShowLanguageInTitle = LIT_HIDE;
UINT iShellMenuType = CMF_EXPLORE;
BOOL bHighlightLineIfWindowInactive = FALSE;
long iMaxSearchDistance = DEFAULT_MAX_SEARCH_DISTANCE_KB * BYTES_IN_KB;
EScrollYCaretPolicy iScrollYCaretPolicy = SCP_LEGACY;
BOOL bFindWordMatchCase = FALSE;
BOOL bFindWordWrapAround = FALSE;
HWND hwndStatusProgressBar = NULL;
BOOL bShowProgressBar = FALSE;

extern HWND  hwndMain;
extern HWND  hwndEdit;
extern WCHAR szTitleExcerpt[128];
extern int iPathNameFormat;
extern WCHAR szCurFile[MAX_PATH + 40];
extern UINT uidsAppTitle;
extern BOOL fIsElevated;
extern BOOL bModified;
extern int iEncoding;
extern int iOriginalEncoding;
extern BOOL bReadOnly;
extern long iMaxSearchDistance;
extern BOOL bHighlightSelection;
extern LPMRULIST pFileMRU;
extern WCHAR g_wchWorkingDirectory[MAX_PATH];
extern enum ESaveSettingsMode nSaveSettingsMode;

void n2e_InitInstance()
{
  InitScintillaHandle(hwndEdit);
  n2e_Init();
  hShellHook = SetWindowsHookEx(WH_SHELL, n2e_ShellProc, NULL, GetCurrentThreadId());
}

void n2e_ExitInstance()
{
  if (hShellHook)
  {
    UnhookWindowsHookEx(hShellHook);
  }
  n2e_Release();
  n2e_SaveINI();
}

void CALLBACK n2e_WheelTimerProc(HWND _h, UINT _u, UINT_PTR idEvent, DWORD _t)
{
  bWheelTimerActive = FALSE;
  KillTimer(NULL, idEvent);
}

void n2e_Init()
{
  n2e_InitializeTrace();
  n2e_SetWheelScroll(bCtrlWheelScroll);
  n2e_ResetLastRun();
  n2e_EditInit();
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

void n2e_LoadINI()
{
  bHighlightSelection = IniGetInt(N2E_INI_SECTION, INI_SETTING_HIGHLIGHT_SELECTION, bHighlightSelection);
  bCtrlWheelScroll = IniGetInt(N2E_INI_SECTION, INI_SETTING_WHEEL_SCROLL, bCtrlWheelScroll);
  iWheelScrollInterval = IniGetInt(N2E_INI_SECTION, INI_SETTING_WHEEL_SCROLL_INTERVAL, iWheelScrollInterval);
  iCSSSettings = IniGetInt(N2E_INI_SECTION, INI_SETTING_CSS_SETTINGS, iCSSSettings);
  iShellMenuType = IniGetInt(N2E_INI_SECTION, INI_SETTING_SHELL_MENU_TYPE, iShellMenuType);
  iMaxSearchDistance = IniGetInt(N2E_INI_SECTION, INI_SETTING_MAX_SEARCH_DISTANCE, DEFAULT_MAX_SEARCH_DISTANCE_KB) * BYTES_IN_KB;
  bUsePrefixInOpenDialog = IniGetInt(N2E_INI_SECTION, INI_SETTING_OPEN_DIALOG_BY_PREFIX, bUsePrefixInOpenDialog);
  bHighlightLineIfWindowInactive = IniGetInt(N2E_INI_SECTION, INI_SETTING_HIGHLIGHT_LINE_IF_WINDOW_INACTIVE, bHighlightLineIfWindowInactive);
  iScrollYCaretPolicy = IniGetInt(N2E_INI_SECTION, INI_SETTING_SCROLL_Y_CARET_POLICY, iScrollYCaretPolicy);
  bFindWordMatchCase = IniGetInt(N2E_INI_SECTION, INI_SETTING_FIND_WORD_MATCH_CASE, bFindWordMatchCase);
  bFindWordWrapAround = IniGetInt(N2E_INI_SECTION, INI_SETTING_FIND_WRAP_AROUND, bFindWordWrapAround);
  bMoveCaretOnRightClick = IniGetInt(N2E_INI_SECTION, INI_SETTING_MOVE_CARET_ON_RIGHT_CLICK, bMoveCaretOnRightClick);
  iEvaluateMathExpression = IniGetInt(N2E_INI_SECTION, INI_SETTING_MATH_EVAL, iEvaluateMathExpression);
  iShowLanguageInTitle = IniGetInt(N2E_INI_SECTION, INI_SETTING_LANGUAGE_INDICATOR, iShowLanguageInTitle);
  iWordNavigationMode = IniGetInt(N2E_INI_SECTION, INI_SETTING_WORD_NAVIGATION_MODE, iWordNavigationMode);
}

void n2e_SaveINI()
{
  IniSetInt(N2E_INI_SECTION, INI_SETTING_HIGHLIGHT_SELECTION, bHighlightSelection);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_WHEEL_SCROLL, bCtrlWheelScroll);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_WHEEL_SCROLL_INTERVAL, iWheelScrollInterval);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_CSS_SETTINGS, iCSSSettings);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_SHELL_MENU_TYPE, iShellMenuType);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_MAX_SEARCH_DISTANCE, iMaxSearchDistance / BYTES_IN_KB);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_OPEN_DIALOG_BY_PREFIX, bUsePrefixInOpenDialog);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_HIGHLIGHT_LINE_IF_WINDOW_INACTIVE, bHighlightLineIfWindowInactive);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_SCROLL_Y_CARET_POLICY, iScrollYCaretPolicy);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_FIND_WORD_MATCH_CASE, bFindWordMatchCase);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_FIND_WRAP_AROUND, bFindWordWrapAround);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_MOVE_CARET_ON_RIGHT_CLICK, bMoveCaretOnRightClick);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_MATH_EVAL, iEvaluateMathExpression);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_LANGUAGE_INDICATOR, iShowLanguageInTitle);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_WORD_NAVIGATION_MODE, iWordNavigationMode);
}

void n2e_Release()
{
  n2e_SelectionRelease();
  n2e_FinalizeTrace();
}

void n2e_Reset()
{
  n2e_Release();
  n2e_Init();
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
              SetWindowLong(hdlg, DWL_MSGRESULT, 1);
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
                  SetWindowLong(hdlg, DWL_MSGRESULT, 0);
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
        SetWindowLong(hdlg, DWL_MSGRESULT, take_call);
      }
  }
  return take_call;
}

UINT_PTR CALLBACK n2e_OFNHookSaveProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
  const HWND hPar = GetParent(hdlg);
  switch (uiMsg)
  {
    case WM_NOTIFY:
      {
        OFNOTIFY *ofn = (OFNOTIFY *)lParam;
        NMHDR nm = ofn->hdr;
        switch (nm.code)
        {
          case CDN_FILEOK:
            if (ofn->lpOFN && ofn->lpOFN->lpstrFile)
            {
              WCHAR buf[MAX_PATH] = { 0 };
              if (CommDlg_OpenSave_GetFilePath(hPar, buf, COUNTOF(buf)) > 0)
              {
                WCHAR ext[MAX_PATH] = { 0 };
                lstrcpy(ext, L".");
                lstrcat(ext, ofn->lpOFN->lpstrDefExt);
                PathAddExtension(buf, ext);
              }
              if (lstrcmpi(ofn->lpOFN->lpstrFile, buf) == 0)
              {
                lstrcpy(ofn->lpOFN->lpstrFile, buf);
                ofn->lpOFN->dwReserved = TRUE;
              }
              return 0;
            }
            return 1;
          default:
            break;
        }
      }
      break;
    default:
      break;
  }
  return FALSE;
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
  return  i > 0 && lstrcmp(fn, szCurFile);
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

void n2e_Grep(void* _lpf, const BOOL grep)
{
  LPEDITFINDREPLACE lpf = (LPEDITFINDREPLACE)_lpf;
  int k = 0;
  int res = 0;
  int line_first, line_last;
  char szFind2[512];

  struct Sci_TextToFind ttf;
  if (!lstrlenA(lpf->szFind))
  {
    return;
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
    return;
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
  SetWindowTitle(hwnd, uidsAppTitle, fIsElevated, IDS_UNTITLED, szCurFile,
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
  n2e_ProcessPendingMessages();
}

void n2e_IncProgressBarPosInStatusBar(const long nOffset)
{
  InlineProgressBarCtrl_IncPos(hwndStatusProgressBar, nOffset);
  n2e_ProcessPendingMessages();
}

void n2e_ProcessPendingMessages()
{
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

int n2e_JoinLines_GetSelEnd(int iSelEnd)
{
  int iLastLine = SciCall_LineFromPosition(iSelEnd);
  while ((iLastLine > 0) && (SciCall_PositionFromLine(iLastLine) == iSelEnd))
  {
    --iLastLine;
    iSelEnd = SciCall_LineEndPosition(iLastLine);
  }
  return iSelEnd;
}

BOOL n2e_RenameFileToTemporary(LPCWSTR szFile, LPWSTR szMaxPathTmpFile)
{
  WCHAR szDirectory[MAX_PATH] = { 0 };
  lstrcpy(szDirectory, szFile);
  PathRemoveFileSpec(szDirectory);
  return GetTempFileName(szDirectory, L"tmp", 0, szMaxPathTmpFile)
    ? MoveFileEx(szCurFile, szMaxPathTmpFile, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)
    : FALSE;
}