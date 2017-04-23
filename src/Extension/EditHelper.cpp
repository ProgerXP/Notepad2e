#include "EditHelper.h"
#include "Dialogs.h"
#include <string>
#include <Shlobj.h>
#include <WTypes.h>
#include <regex>
#include "scintilla.h"
#include "resource.h"
#include "HLSelection.h"

extern "C"
{
  WCHAR	hl_last_html_tag[0xff] = L"<tag>";
  WCHAR	hl_last_html_end_tag[0xff] = L"</tag>";

  extern UINT		_hl_ctx_menu_type;
  extern	VOID	HL_Trace(const char *fmt, ...);

  LPCONTEXTMENU2	g_IContext2 = NULL;
  LPCONTEXTMENU3	g_IContext3 = NULL;
  //
  VOID Invoke(int cmd, LPCONTEXTMENU menu, HWND win, LPCWSTR path)
  {
    if (cmd > 0) {
      std::wstring pathFolder = path;
      PathRemoveFileSpec((LPWSTR)pathFolder.data());

      CMINVOKECOMMANDINFOEX info ={0};
      info.cbSize = sizeof(info);
      info.fMask = CMIC_MASK_UNICODE;
      info.hwnd = win;
      info.lpVerb  = MAKEINTRESOURCEA(cmd - 1);
      info.lpVerbW = MAKEINTRESOURCEW(cmd - 1);
      info.nShow = SW_SHOWNORMAL;
      info.lpDirectoryW = pathFolder.c_str();
      menu->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
    }
  }
  //
  LRESULT CALLBACK HookWndProc(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam)
  {
    switch (message) {
      case WM_MENUCHAR:	// only supported by IContextMenu3
        if (g_IContext3) {
          LRESULT lResult = 0;
          g_IContext3->HandleMenuMsg2(message, wParam, lParam, &lResult);
          return (lResult);
        }
        break;
      case WM_DRAWITEM:
      case WM_MEASUREITEM:
        if (wParam) {
          break;    // if wParam != 0 then the message is not menu-related
        }
      case WM_INITMENUPOPUP:
        if (g_IContext2) {
          g_IContext2->HandleMenuMsg(message, wParam, lParam);
        }
        else {	// version 3
          g_IContext3->HandleMenuMsg(message, wParam, lParam);
        }
        return (message == WM_INITMENUPOPUP ? 0 : TRUE); // inform caller that
        // we handled WM_INITPOPUPMENU by ourself
        break;
      default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  BOOL GetContextMenu(LPCWSTR path, void **ppContextMenu, int &iMenuType)
  {
    *ppContextMenu = NULL;
    LPCONTEXTMENU icm1 = NULL;
    // first we retrieve the normal IContextMenu
    // interface (every object should have it)
    LPCITEMIDLIST idChild = 0;
    IShellFolder *ifolder = 0;
    ITEMIDLIST *id = 0;
    std::wstring windowsPath = path;
    std::replace(windowsPath.begin(), windowsPath.end(), '/', '\\');
    HRESULT result = SHParseDisplayName(windowsPath.c_str(), 0, &id, 0, 0);
    if (!SUCCEEDED(result) || !id) {
      return false;
    }
    result = SHBindToParent(id, IID_IShellFolder, (void **)&ifolder, &idChild);
    ifolder->GetUIObjectOf(NULL,
                           1,
                           (LPCITEMIDLIST *)&idChild,
                           IID_IContextMenu,
                           NULL,
                           (void **)&icm1);
    if (icm1) {
        // since we got an IContextMenu interface we can
        // now obtain the higher version interfaces via that
      if (icm1->QueryInterface(IID_IContextMenu3, ppContextMenu) == NOERROR) {
        iMenuType = 3;
      }
      else if (icm1->QueryInterface(IID_IContextMenu2,
               ppContextMenu) == NOERROR) {
        iMenuType = 2;
      }
      if (*ppContextMenu) {
        icm1->Release();    // we can now release version 1 interface,
      }
      // cause we got a higher one
      else {
        iMenuType = 1;
        *ppContextMenu = icm1;    // since no higher versions were found
      }  // redirect ppContextMenu to version 1 interface
    }
    else {
      return (FALSE);    // something went wrong
    }
    return (TRUE); // success
  }

  BOOL HL_Explorer_cxt_menu(LPCWSTR path, void *parentWindow)
  {
    int iMenuType = 0;
    // to know which version of IContextMenu is supported
    LPCONTEXTMENU pContextMenu;
    // common pointer to IContextMenu and higher version interface
    if (!GetContextMenu(path, (void **)&pContextMenu, iMenuType)) {
      return FALSE;    // something went wrong
    }
    //
    HMENU	h_menu = CreatePopupMenu();
    // lets fill the our popupmenu
    pContextMenu->QueryContextMenu(h_menu,
                                   0, 1, 0x7FFF, _hl_ctx_menu_type);
    WNDPROC OldWndProc = NULL;
    //
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *)&osvi);
    BOOL	bIsWindowsXPorLater =
      ((osvi.dwMajorVersion > 5) ||
       ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1)));
    HL_Trace("win version %d (%d - %d) . XP ? : %d", WINVER, osvi.dwMajorVersion, osvi.dwMinorVersion, !bIsWindowsXPorLater);
    if (iMenuType > 1) { // only version 2 and 3 supports menu messages
      OldWndProc = (WNDPROC)SetWindowLong((HWND)parentWindow,
                                          GWL_WNDPROC, (DWORD)HookWndProc);
      if (iMenuType == 2) {
        g_IContext2 = (LPCONTEXTMENU2)pContextMenu;
      }
      else {	// version 3
        g_IContext3 = (LPCONTEXTMENU3)pContextMenu;
      }
    }
    else {
      OldWndProc = NULL;
    }
    POINT pt;
    GetCursorPos(&pt);
    int iCmd = TrackPopupMenuEx(h_menu, TPM_RETURNCMD, pt.x, pt.y, (HWND)parentWindow, NULL);
    Invoke(iCmd, pContextMenu, (HWND)parentWindow, path);
    if (OldWndProc) {
      SetWindowLong((HWND)parentWindow, GWL_WNDPROC, (DWORD)OldWndProc);
    }
    pContextMenu->Release();
    return (TRUE);
  }

  void HL_Strip_html_tags(HWND hwnd)
  {
    struct Sci_TextToFind ttf1, ttf2;
    int selbeg, selend, res, len;

    //
    selbeg = SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    selend = SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
    len = SendMessage(hwnd, SCI_GETTEXTLENGTH, 0, 0);
    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
    //
    if (0 == selend - selbeg)
    {
      ttf1.chrg.cpMin = selbeg;
      ttf1.chrg.cpMax = 0;
      ttf1.lpstrText = "<";
      res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf1);
      if (-1 != res)
      {
        ttf2.chrg.cpMin = ttf1.chrgText.cpMax;
        ttf2.chrg.cpMax = len;
        ttf2.lpstrText = ">";
        res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf2);
        if (-1 != res)
        {
          SendMessage(hwnd, SCI_DELETERANGE, ttf1.chrgText.cpMin, ttf2.chrgText.cpMax - ttf1.chrgText.cpMin);
          SendMessage(hwnd, SCI_SETSEL, ttf1.chrgText.cpMin, ttf1.chrgText.cpMin);
        }
      }
    }
    else
    {
      while (1)
      {
        ttf1.chrg.cpMin = selbeg;
        ttf1.chrg.cpMax = selend;
        ttf1.lpstrText = "<";
        res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf1);
        if (-1 != res)
        {
          ttf2.chrg.cpMin = ttf1.chrgText.cpMax;
          ttf2.chrg.cpMax = selend;
          ttf2.lpstrText = ">";
          res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf2);
          if (-1 != res)
          {
            int dlen = ttf2.chrgText.cpMax - ttf1.chrgText.cpMin;
            SendMessage(hwnd, SCI_DELETERANGE, ttf1.chrgText.cpMin, dlen);
            selend -= dlen;
          }
          else
          {
            break;
          }
        }
        else
        {
          break;
        }
      }
      SendMessage(hwnd, SCI_SETSEL, selbeg, selend);
    }
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
  }

  BOOL IsSelectionModeValid(HWND hwnd)
  {
    if (SC_SEL_RECTANGLE == SendMessage(hwnd, SCI_GETSELECTIONMODE, 0, 0))
    {
      MsgBox(MBINFO, IDS_SELRECT);
      return FALSE;
    }
    return TRUE;
  }

  extern BOOL bAutoIndent;

  LPSTR GetLinePrefix(HWND hwnd, int iLine, LPBOOL pbLineEmpty)
  {
    *pbLineEmpty = TRUE;
    LPSTR pszPrefix = NULL;

    const int iCurPos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
    const int iLineStart = SendMessage(hwnd, SCI_POSITIONFROMLINE, iLine, 0);
    const int iLinePrefixLength = iCurPos - iLineStart;
    pszPrefix = (LPSTR)GlobalAlloc(GPTR, iLinePrefixLength + 1);
    struct TextRange tr = { { iLineStart, iCurPos }, pszPrefix };
    SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

    for (size_t i = 0; i < strlen(pszPrefix); ++i)
    {
      const char chCurrent = pszPrefix[i];
      if (!isspace(chCurrent) || (chCurrent == '\r') || (chCurrent == '\n'))
      {
        pszPrefix[i] = 0;
        *pbLineEmpty = (i == 0);
        break;
      }
    }

    return pszPrefix;
  }

  void FreeLinePrefix(LPSTR pszPrefix)
  {
    if (pszPrefix)
    {
      GlobalFree(pszPrefix);
    }
  }

  void InsertNewLineWithPrefix(HWND hwnd, LPSTR pszPrefix, BOOL bInsertAbove)
  {
    const BOOL bAutoIndentOrigin = bAutoIndent;
    bAutoIndent = 0;
    SendMessage(hwnd, SCI_NEWLINE, 0, 0);
    bAutoIndent = bAutoIndentOrigin;
    if (bInsertAbove)
    {
      SendMessage(hwnd, SCI_CHARLEFT, 0, 0);
    }
    if (pszPrefix && (strlen(pszPrefix) > 0))
    {
      const int iCurrentPos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
      SendMessage(hwnd, SCI_INSERTTEXT, iCurrentPos, (LPARAM)pszPrefix);
      SendMessage(hwnd, SCI_LINEEND, 0, 0);
    }
  }

  void EditInsertNewLine(HWND hwnd, BOOL insertAbove)
  {
    if (HLS_Edit_selection_stop(HL_SE_APPLY))
    {
      return;
    }

    const int iCurPos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
    const int iCurLine = SendMessage(hwnd, SCI_LINEFROMPOSITION, iCurPos, 0);
    const int iCurLineEndPos = SendMessage(hwnd, SCI_GETLINEENDPOSITION, iCurLine, 0);
    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
    const int iPrevLine = (iCurLine > 0) ? iCurLine - 1 : 0;
    LPSTR pszPrefixText = NULL;
    BOOL bIsEmptyPrefix = FALSE;

    if (insertAbove)
    {
      const int iPrevLineEndPos = (iPrevLine == iCurLine) ? 0 : SendMessage(hwnd, SCI_GETLINEENDPOSITION, iPrevLine, 0);
      if (bAutoIndent)
      {
        const int iLineStart = SendMessage(hwnd, SCI_POSITIONFROMLINE, iCurLine, 0);
        const int iLinePrefixLength = iCurPos - iLineStart;
        bIsEmptyPrefix = (iLinePrefixLength == 0);
        if (!bIsEmptyPrefix)
        {
          pszPrefixText = GetLinePrefix(hwnd, iCurLine, &bIsEmptyPrefix);
        }
        if (bIsEmptyPrefix)
        {
          SendMessage(hwnd, SCI_SETSEL, iPrevLineEndPos, iPrevLineEndPos);
          SendMessage(hwnd, SCI_NEWLINE, 0, 0);
          if (iPrevLine == iCurLine)
          {
            const int iNewPrevLineEndPos = SendMessage(hwnd, SCI_GETLINEENDPOSITION, iPrevLine, 0);
            SendMessage(hwnd, SCI_SETSEL, iNewPrevLineEndPos, iNewPrevLineEndPos);
          }
        }
        else
        {
          SendMessage(hwnd, SCI_SETSEL, iPrevLineEndPos, iPrevLineEndPos);
          InsertNewLineWithPrefix(hwnd, pszPrefixText, (iPrevLine == iCurLine));
        }
      }
      else
      {
        SendMessage(hwnd, SCI_SETSEL, iPrevLineEndPos, iPrevLineEndPos);
        SendMessage(hwnd, SCI_NEWLINE, 0, 0);
      }
    }
    else
    {
      const BOOL isLineEnd = (iCurPos == iCurLineEndPos);
      if (isLineEnd && bAutoIndent)
      {
        SendMessage(hwnd, SCI_SETSEL, iCurLineEndPos, iCurLineEndPos);
        pszPrefixText = GetLinePrefix(hwnd, iCurLine - 1, &bIsEmptyPrefix);
        InsertNewLineWithPrefix(hwnd, pszPrefixText, FALSE);
      }
      else
      {
        SendMessage(hwnd, SCI_SETSEL, iCurLineEndPos, iCurLineEndPos);
        SendMessage(hwnd, SCI_NEWLINE, 0, 0);
      }
    }
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
    FreeLinePrefix(pszPrefixText);
  }

  int isValidRegex(LPCSTR str)
  {
    try
    {
      std::regex reg(str);
      return 1;
    }
    catch (...)
    {
      return 0;
    }
  }
}