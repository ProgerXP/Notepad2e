#include "Utils.h"
#include "scintilla.h"
#include "helpers.h"
#include "resource.h"
#include <cassert>
#include "Notepad2.h"
#include "HLSelection.h"
#include "Edit.h"
#include "SciCall.h"
#include "InlineProgressBarCtrl.h"
#include "MainWndHelper.h"

HANDLE g_hScintilla = NULL;

#define N2E_WHEEL_TIMER_ID	0xfefe
#define N2E_SEL_EDIT_TIMER_ID	(N2E_WHEEL_TIMER_ID + 1)
UINT_PTR	_n2e_sel_edit_timer_id = N2E_SEL_EDIT_TIMER_ID;
UINT	_n2e_wheel_timer_to = 100;
UINT	_n2e_css_property = css_prop_less;

FILE	*_n2e_log = 0;
HWND	g_hwnd = 0;

extern BOOL	bHighlightSelection;
BOOL	_n2e_skip_highlight = FALSE;
BOOL	bUsePrefixInOpenDialog = TRUE;
BOOL	bCtrlWheelScroll = TRUE;
BOOL  bMoveCaretOnRightClick = TRUE;
int iEvaluateMathExpression = 0;
int iWordNavigationMode = 0;
ELanguageIndicatorMode iShowLanguageInTitle = ELI_HIDE;

UINT	_n2e_ctx_menu_type = 0;
extern	LPMRULIST pFileMRU;
extern	WCHAR     g_wchWorkingDirectory[MAX_PATH];
//
BOOL	_n2e_wheel_timer = FALSE;
INT		_n2e_alloc_count = 0;
extern	long	_n2e_max_search_range;

int iHighlightLineIfWindowInactive = 0;
int iScrollYCaretPolicy = 0;
int iFindWordMatchCase = 0;
int iFindWordWrapAround = 0;

HWND hwndStatusProgressBar = NULL;
BOOL bShowProgressBar = FALSE;
extern HWND  hwndMain;
extern HWND  hwndEdit;

void n2e_InitInstance()
{
  InitScintillaHandle(hwndEdit);
  n2e_Init(hwndMain);
  hShellHook = SetWindowsHookEx(WH_SHELL, ShellProc, NULL, GetCurrentThreadId());
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

void* n2e_Alloc(size_t size)
{
  if (_n2e_alloc_count)
  {
    N2E_TRACE(L"WARNING !!! ALLOC mismatch : %d", _n2e_alloc_count);
  }
  ++_n2e_alloc_count;
  return GlobalAlloc(GPTR, sizeof(WCHAR) * (size + 1));
}

void n2e_Free(void* ptr)
{
  if (ptr)
  {
    --_n2e_alloc_count;
    GlobalFree(ptr);
  }
}

void* n2e_Realloc(void* ptr, size_t len)
{
  n2e_Free(ptr);
  return n2e_Alloc(len);
}

VOID CALLBACK n2e_WheelTimerProc(HWND _h, UINT _u, UINT_PTR idEvent, DWORD _t)
{
  _n2e_wheel_timer = FALSE;
  KillTimer(NULL, idEvent);
}

VOID CALLBACK n2e_SelEditTimerProc(HWND _h, UINT _u, UINT_PTR idEvent, DWORD _t)
{
}

VOID n2e_Init(HWND hWnd)
{
  g_hwnd = hWnd;
  if (IniGetInt(N2E_INI_SECTION, L"DebugLog", 0))
  {
    _n2e_log = fopen("n2e_log.log", "w");
  }
  n2e_SetWheelScroll(bCtrlWheelScroll);
  *_n2e_last_run = 0;

  HLS_init();
}

VOID n2e_LoadINI()
{
  bHighlightSelection = IniGetInt(N2E_INI_SECTION, L"HighlightSelection", bHighlightSelection);
  bCtrlWheelScroll = IniGetInt(N2E_INI_SECTION, L"WheelScroll", bCtrlWheelScroll);
  _n2e_wheel_timer_to = IniGetInt(N2E_INI_SECTION, L"WheelScrollInterval", _n2e_wheel_timer_to);
  _n2e_css_property = IniGetInt(N2E_INI_SECTION, L"CSSSettings", _n2e_css_property);
  _n2e_ctx_menu_type = IniGetInt(N2E_INI_SECTION, L"ShellMenuType", CMF_EXPLORE);
  _n2e_max_search_range = IniGetInt(N2E_INI_SECTION, L"MaxSearchDistance", 64) * 1024;
  bUsePrefixInOpenDialog = IniGetInt(N2E_INI_SECTION, L"OpenDialogByPrefix", bUsePrefixInOpenDialog);
  iHighlightLineIfWindowInactive = IniGetInt(N2E_INI_SECTION, INI_SETTING_HIGHLIGHT_LINE_IF_WINDOW_INACTIVE, iHighlightLineIfWindowInactive);
  iScrollYCaretPolicy = IniGetInt(N2E_INI_SECTION, INI_SETTING_SCROLL_Y_CARET_POLICY, iScrollYCaretPolicy);
  iFindWordMatchCase = IniGetInt(N2E_INI_SECTION, INI_SETTING_FIND_WORD_MATCH_CASE, iFindWordMatchCase);
  iFindWordWrapAround = IniGetInt(N2E_INI_SECTION, INI_SETTING_FIND_WRAP_AROUND, iFindWordWrapAround);
  bMoveCaretOnRightClick = IniGetInt(N2E_INI_SECTION, INI_SETTING_MOVE_CARET_ON_RIGHT_CLICK, bMoveCaretOnRightClick);
  iEvaluateMathExpression = IniGetInt(N2E_INI_SECTION, INI_SETTING_MATH_EVAL, iEvaluateMathExpression);
  iShowLanguageInTitle = IniGetInt(N2E_INI_SECTION, INI_SETTING_LANGUAGE_INDICATOR, iShowLanguageInTitle);
  iWordNavigationMode = IniGetInt(N2E_INI_SECTION, INI_SETTING_WORD_NAVIGATION_MODE, iWordNavigationMode);
}

VOID n2e_SaveINI()
{
  IniSetInt(N2E_INI_SECTION, L"HighlightSelection", bHighlightSelection);
  IniSetInt(N2E_INI_SECTION, L"WheelScroll", bCtrlWheelScroll);
  IniSetInt(N2E_INI_SECTION, L"WheelScrollInterval", _n2e_wheel_timer_to);
  IniSetInt(N2E_INI_SECTION, L"CSSSettings", _n2e_css_property);
  IniSetInt(N2E_INI_SECTION, L"ShellMenuType", _n2e_ctx_menu_type);
  IniSetInt(N2E_INI_SECTION, L"MaxSearchDistance", _n2e_max_search_range / 1024);
  IniSetInt(N2E_INI_SECTION, L"OpenDialogByPrefix", bUsePrefixInOpenDialog);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_HIGHLIGHT_LINE_IF_WINDOW_INACTIVE, iHighlightLineIfWindowInactive);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_SCROLL_Y_CARET_POLICY, iScrollYCaretPolicy);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_FIND_WORD_MATCH_CASE, iFindWordMatchCase);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_FIND_WRAP_AROUND, iFindWordWrapAround);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_MOVE_CARET_ON_RIGHT_CLICK, bMoveCaretOnRightClick);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_MATH_EVAL, iEvaluateMathExpression);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_LANGUAGE_INDICATOR, iShowLanguageInTitle);
  IniSetInt(N2E_INI_SECTION, INI_SETTING_WORD_NAVIGATION_MODE, iWordNavigationMode);
}

VOID n2e_Release()
{
  HLS_release();
  if (_n2e_log)
  {
    fclose(_n2e_log);
  }
  _n2e_log = 0;
  g_hwnd = 0;
}

VOID N2E_Trace(const char *fmt, ...)
{
  if (_n2e_log)
  {
    va_list vl;
    SYSTEMTIME st;
    char	buff[0xff + 1];
    char* ch = 0;
    GetLocalTime(&st);
    fprintf(_n2e_log, "- [%d:%d:%d] ", st.wMinute, st.wSecond, st.wMilliseconds);
    va_start(vl, fmt);
    vsprintf_s(buff, 0xff, fmt, vl);
    va_end(vl);
    ch = buff;
    while (*ch)
    {
      if ('\n' == *ch)
      {
        *ch = '¶';
      }
      ++ch;
    }
    fprintf(_n2e_log, "%s\r\n", buff);
    fflush(_n2e_log);
  }
}

VOID N2E_WTrace(const char *fmt, LPCWSTR word)
{
  if (_n2e_log)
  {
    int size;
    char *temp = 0;
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(_n2e_log, "- [%d:%d:%d] ", st.wMinute, st.wSecond, st.wMilliseconds);
    temp = n2e_Alloc(size = WideCharToMultiByte(CP_UTF8, 0, word, -1, NULL, 0, NULL, NULL));
    WideCharToMultiByte(CP_UTF8, 0, word, -1, temp, size, NULL, NULL);
    fprintf(_n2e_log, fmt, temp);
    n2e_Free(temp);
    fprintf(_n2e_log, "\r\n");
    fflush(_n2e_log);
  }
}

VOID N2E_WTrace2(const char *fmt, LPCWSTR word1, LPCWSTR word2)
{
  if (_n2e_log)
  {
    int size;
    char *temp, *temp2;
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(_n2e_log, "- [%d:%d:%d] ", st.wMinute, st.wSecond, st.wMilliseconds);
    temp = n2e_Alloc(size = WideCharToMultiByte(CP_UTF8, 0, word1, -1, NULL, 0, NULL, NULL));
    WideCharToMultiByte(CP_UTF8, 0, word1, -1, temp, size, NULL, NULL);
    temp2 = n2e_Alloc(size = WideCharToMultiByte(CP_UTF8, 0, word2, -1, NULL, 0, NULL, NULL));
    WideCharToMultiByte(CP_UTF8, 0, word2, -1, temp2, size, NULL, NULL);
    fprintf(_n2e_log, fmt, temp, temp2);
    n2e_Free(temp);
    n2e_Free(temp2);
    fprintf(_n2e_log, "\r\n");
    fflush(_n2e_log);
  }
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

BOOL n2e_GetGotoNumber(LPTSTR temp, int *out, BOOL hex)
{
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
        *out = wcstol(temp, &ec, 10);
        if (StrChr(L"abcdefABCDEFxh", *ec))
        {
          *out = wcstol(temp, &ec, 16);
          if (0 == *out)
          {
            return 0;
          }
        }
        ok = n2e_TestOffsetTail(ec);
        N2E_Trace("Result is  %d (%d)", *out, ok);
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
        *out = wcstol(temp, &ec, 16);
        ok = n2e_TestOffsetTail(ec);
        N2E_Trace("Result is (hex) %d (ok %d)", *out, ok);
        return ok;
      }
    }
    else
    {
      if (isdigit(temp[0]))
      {
        *out = wcstol(temp, &ec, 10);
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

VOID n2e_WheelScrollWorker(int lines)
{
  int anch, sel = 0;
  if (_n2e_wheel_timer)
  {
    N2E_Trace("wheel timer blocked");
    return;
  }
  _n2e_wheel_timer = TRUE;
  SetTimer(NULL, N2E_WHEEL_TIMER_ID, _n2e_wheel_timer_to, n2e_WheelTimerProc);
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

VOID n2e_SetWheelScroll(BOOL on)
{
  if (on)
  {
    n2e_wheel_action = n2e_WheelScrollWorker;
  }
  else
  {
    n2e_wheel_action = 0;
  }
}

BOOL CALLBACK n2e_EnumProc(
  HWND hwnd,
  LPARAM lParam
)
{
  WCHAR title[0xff + 1];
  GetWindowText(hwnd, title, 0xff);
  if (wcsstr(title, WC_NOTEPAD2)
      && g_hwnd != hwnd
      )
  {
    PostMessage(hwnd, HWM_RELOAD_SETTINGS, 0, 0);
  }
  return TRUE;
}

VOID n2e_Reload_Settings()
{
  EnumWindows(n2e_EnumProc, (LPARAM)g_hwnd);
}

BOOL n2e_Is_Empty(LPCWSTR txt)
{
  int t = lstrlen(txt);
  while (--t >= 0)
  {
    if (!isspace(txt[t]))
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

BOOL	n2e_OpenFileByPrefix(LPCWSTR pref, LPWSTR dir, LPWSTR out)
{
  WIN32_FIND_DATA	wfd;
  WCHAR	path[MAX_PATH];
  WCHAR	temp[MAX_PATH];
  WCHAR	_in[MAX_PATH];
  WCHAR*	in = _in;
  HANDLE res;
  int		len;
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

  if (!PathIsRelative(in))
  {
    lstrcpy(dir, in);
    PathRemoveFileSpec(dir);
    lstrcpy(path, in);
    PathStripPath(in);
  }
  else
  {
    lstrcpy(path, dir);
    lstrcat(path, L"\\");
    lstrcat(path, in);
  }
  lstrcat(path, L"*");
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
      N2E_TRACE("file: '%S'", wfd.cFileName);
      if (_wcsnicmp(wfd.cFileName, in, 1))
      {
        N2E_TRACE("skip");
        continue;
      }
      if (0 == temp[0] || n2e_CompareFiles(temp, wfd.cFileName) > 0)
      {
        lstrcpy(temp, wfd.cFileName);
      }
    }
  } while (FindNextFile(res, &wfd) != 0);
  FindClose(res);
  if (temp[0])
  {
    lstrcpy(out, dir);
    if (L'\\' != out[lstrlen(out) - 1])
    {
      lstrcat(out, L"\\");
    }
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

UINT_PTR CALLBACK n2e_OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
  static UINT file_ok = 0;
  static WCHAR last_selected[MAX_PATH];
  static BOOL take_call = FALSE;
  HWND hPar = GetParent(hdlg);
  switch (uiMsg)
  {
    case WM_NOTIFY: {
        OFNOTIFY *ofn = (OFNOTIFY *)lParam;
        NMHDR nm = ofn->hdr;
        switch (nm.code)
        {
          case CDN_FILEOK: {
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
                WCHAR	out[MAX_PATH];
                LPWSTR	final_str = buf;
                if (wcsstr(last_selected, buf))
                {
                  final_str = last_selected;
                  N2E_TRACE("OFN drop window text %S ", buf);
                }
                N2E_TRACE("OFN input (%S) ", final_str);
                if (!n2e_OpenFileByPrefix(final_str, dir, out))
                {
                  WCHAR mess[1024];
                  wsprintf(mess,
                           L"%s\nFile not found.\n"
                           L"Additionally, no file name starting "
                           L"with this string exists in this folder\n%s",
                           final_str, dir);
                  MessageBox(hdlg, mess, WC_NOTEPAD2, MB_OK | MB_ICONWARNING);
                  take_call = TRUE;
                  return 0;
                }
                else
                {
                  CommDlg_OpenSave_SetControlText(hPar, cmb13, (LPARAM)out);
                  lstrcpy(ofn->lpOFN->lpstrFile, out);
                  N2E_TRACE("OFN final result (%S) ", out);
                  SetWindowLong(hdlg, DWL_MSGRESULT, 0);
                  take_call = FALSE;
                }
              }
            }
                           return 1;
          case CDN_SELCHANGE: {
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
          case CDN_INITDONE: {
              N2E_TRACE("OFN init  ");
              take_call = FALSE;
              file_ok = RegisterWindowMessage(FILEOKSTRING);
              *last_selected = 0;
            }
                             break;
          case CDN_FOLDERCHANGE: {
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

VOID n2e_GetLastDir(LPTSTR out)
{
  WCHAR	tch[MAX_PATH];
  INT count = MRU_Enum(pFileMRU, 0, NULL, 0);
  if (count)
  {
    MRU_Enum(pFileMRU, 0, tch, COUNTOF(tch));
    N2E_WTrace("OFN mru '%s'", tch);
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
    N2E_WTrace("OFN mru final '%s'", out);
  }
  else
  {
    lstrcpy(out, g_wchWorkingDirectory);
  }
}

VOID n2e_Grep(VOID* _lpf, BOOL grep)
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
  ShowProgressBarInStatusBar(grep ? L"Applying Grep..." : L"Applying Ungrep...", 1, maxPos);

  BOOL bIsLastLine = TRUE;
  res = SciCall_FindText(lpf->fuFlags, &ttf);
  while (ttf.chrg.cpMin > ttf.chrg.cpMax)
  {
    const int lineIndex = (res >= 0) ? SciCall_LineFromPosition(res) : 0;
    const int lineStart = SciCall_PositionFromLine(lineIndex);
    int lineEnd = 0;
    if (res >= 0)
    {
      UpdateProgressBarInStatusBar(maxPos - res);
      BOOL bDone = FALSE;
      if (grep && bIsLastLine)
      {
        if (lineIndex + 2 == SciCall_GetLineCount())
        {
          lineEnd = SciCall_LineEndPosition(lineIndex);
          bIsLastLine = FALSE;
          bDone = TRUE;
        }
      }
      if (!bDone)
      {
        lineEnd = SciCall_PositionFromLine(lineIndex + 1);
      }
    }
    else
    {
      lineEnd = ttf.chrg.cpMax;
    }
    SciCall_DeleteRange(grep ? lineEnd : lineStart,
                        grep ? (ttf.chrg.cpMin - lineEnd) : (lineEnd - lineStart));

    ttf.chrg.cpMin = lineStart;
    res = SciCall_FindText(lpf->fuFlags, &ttf);
  }

  SendMessage(lpf->hwnd, SCI_ENDUNDOACTION, 0, 0);
  UpdateLineNumberWidth();
  HideProgressBarInStatusBar();
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

BOOL n2e_IsWordChar(WCHAR ch)
{
  return	IsCharAlphaNumericW(ch) || NULL != StrChr(L"_", ch);
}

BOOL n2e_IsSpace(WCHAR ch)
{
  return isspace(ch);
}

BOOL SetClipboardText(const HWND hwnd, const wchar_t* text)
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

// recent window title params
UINT _uIDAppName;
BOOL _bIsElevated;
UINT _uIDUntitled;
WCHAR _lpszFile[MAX_PATH * 2];
int _iFormat;
BOOL _bModified;
UINT _uIDReadOnly;
BOOL _bReadOnly;
WCHAR _lpszExcerpt[MAX_PATH * 2];

void SaveWindowTitleParams(UINT uIDAppName, BOOL bIsElevated, UINT uIDUntitled,
                           LPCWSTR lpszFile, int iFormat, BOOL bModified,
                           UINT uIDReadOnly, BOOL bReadOnly, LPCWSTR lpszExcerpt)
{
  _uIDAppName = uIDAppName;
  _bIsElevated = bIsElevated;
  _uIDUntitled = uIDUntitled;
  StrCpyW(_lpszFile, lpszFile);
  _iFormat = iFormat;
  _bModified = bModified;
  _uIDReadOnly = uIDReadOnly;
  _bReadOnly = bReadOnly;
  StrCpyW(_lpszExcerpt, lpszExcerpt);
}

void UpdateWindowTitle(HWND hwnd)
{
  SetWindowTitle(hwnd, _uIDAppName, _bIsElevated, _uIDUntitled, _lpszFile, _iFormat, _bModified, _uIDReadOnly, _bReadOnly, _lpszExcerpt);
}

void CreateProgressBarInStatusBar()
{
  hwndStatusProgressBar = InlineProgressBarCtrl_Create(hwndStatus, 0, 100, TRUE, STATUS_LEXER);
}

void DestroyProgressBarInStatusBar()
{
  DestroyWindow(hwndStatusProgressBar);
  hwndStatusProgressBar = NULL;
}

void ShowProgressBarInStatusBar(LPCWSTR pProgressText, const long nCurPos, const long nMaxPos)
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

void HideProgressBarInStatusBar()
{
  if (hwndStatusProgressBar)
  {
    bShowProgressBar = FALSE;
    wcscpy_s(tchProgressBarTaskName, _countof(tchProgressBarTaskName), L"");
    ShowWindow(hwndStatusProgressBar, SW_HIDE);
    UpdateStatusbar();
  }
}

void UpdateProgressBarInStatusBar(const long nCurPos)
{
  InlineProgressBarCtrl_SetPos(hwndStatusProgressBar, nCurPos);
  InvalidateRect(hwndStatusProgressBar, NULL, FALSE);
}

void AdjustProgressBarInStatusBar(const long nCurPos, const long nMaxPos)
{
  InlineProgressBarCtrl_SetRange(hwndStatusProgressBar, 0, nMaxPos, 1);
  UpdateProgressBarInStatusBar(nCurPos);
}
