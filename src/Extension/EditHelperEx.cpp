#include "EditHelperEx.h"
#include <regex>
#include "../scintilla/src/UniConversion.h"

extern "C"
{
  extern UINT		_n2e_ctx_menu_type;
  VOID	N2E_Trace(const char *fmt, ...);
  
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

  BOOL n2e_ExplorerCxtMenu(LPCWSTR path, void *parentWindow)
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
                                   0, 1, 0x7FFF, _n2e_ctx_menu_type);
    WNDPROC OldWndProc = NULL;
    //
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *)&osvi);
    BOOL	bIsWindowsXPorLater =
      ((osvi.dwMajorVersion > 5) ||
       ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1)));
    N2E_Trace("win version %d (%d - %d) . XP ? : %d", WINVER, osvi.dwMajorVersion, osvi.dwMinorVersion, !bIsWindowsXPorLater);
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

  int GetUTF8CharLength(unsigned char ch)
  {
    return (int)UTF8CharLength(ch);
  }
}