#include "EditHelperEx.h"
#include <boost/regex.hpp>
#include "../scintilla/src/UniConversion.h"
#include "Scintilla.h"

extern "C"
{
  #include "CommonUtils.h"

  extern UINT iShellMenuType;

  #include "Trace.h"
  
  LPCONTEXTMENU2  g_IContext2 = NULL;
  LPCONTEXTMENU3  g_IContext3 = NULL;

  void Invoke(const int cmd, LPCONTEXTMENU menu, const HWND win, LPCWSTR path)
  {
    if (cmd > 0)
    {
      std::wstring pathFolder = path;
      PathRemoveFileSpec((LPWSTR)pathFolder.data());

      CMINVOKECOMMANDINFOEX info = {0};
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

  LRESULT CALLBACK HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    switch (message)
    {
      case WM_MENUCHAR: // only supported by IContextMenu3
        if (g_IContext3)
        {
          LRESULT lResult = 0;
          g_IContext3->HandleMenuMsg2(message, wParam, lParam, &lResult);
          return (lResult);
        }
        break;
      case WM_DRAWITEM:
      case WM_MEASUREITEM:
        if (wParam)
        {
          break;    // if wParam != 0 then the message is not menu-related
        }
      case WM_INITMENUPOPUP:
        if (g_IContext2)
        {
          g_IContext2->HandleMenuMsg(message, wParam, lParam);
        }
        else
        {  // version 3
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
    if (!SUCCEEDED(result) || !id)
    {
      return false;
    }
    result = SHBindToParent(id, IID_IShellFolder, (void **)&ifolder, &idChild);
    ifolder->GetUIObjectOf(NULL,
                           1,
                           (LPCITEMIDLIST *)&idChild,
                           IID_IContextMenu,
                           NULL,
                           (void **)&icm1);
    if (icm1)
    {
        // since we got an IContextMenu interface we can
        // now obtain the higher version interfaces via that
      if (icm1->QueryInterface(IID_IContextMenu3, ppContextMenu) == NOERROR)
      {
        iMenuType = 3;
      }
      else if (icm1->QueryInterface(IID_IContextMenu2, ppContextMenu) == NOERROR)
      {
        iMenuType = 2;
      }
      if (*ppContextMenu)
      {
        icm1->Release();    // we can now release version 1 interface,
      }
      // cause we got a higher one
      else
      {
        iMenuType = 1;
        *ppContextMenu = icm1;    // since no higher versions were found
      }  // redirect ppContextMenu to version 1 interface
    }
    else
    {
      return FALSE;    // something went wrong
    }
    return TRUE; // success
  }

  BOOL n2e_ExplorerCxtMenu(LPCWSTR path, const HWND hwndParent)
  {
    int iMenuType = 0;
    // to know which version of IContextMenu is supported
    LPCONTEXTMENU pContextMenu;
    // common pointer to IContextMenu and higher version interface
    if (!GetContextMenu(path, (void **)&pContextMenu, iMenuType))
    {
      return FALSE;    // something went wrong
    }

    HMENU h_menu = CreatePopupMenu();
    pContextMenu->QueryContextMenu(h_menu, 0, 1, 0x7FFF, iShellMenuType);
    LONG_PTR OldWndProc = NULL;

    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *)&osvi);
    BOOL bIsWindowsXPorLater =
      ((osvi.dwMajorVersion > 5) ||
       ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1)));
    N2E_TRACE_PLAIN("win version %d (%d - %d) . XP ? : %d", WINVER, osvi.dwMajorVersion, osvi.dwMinorVersion, !bIsWindowsXPorLater);
    if (iMenuType > 1)  // only version 2 and 3 supports menu messages
    {
      OldWndProc = SetWindowLongPtr(hwndParent,
                                          GWLP_WNDPROC, (LONG_PTR)HookWndProc);
      if (iMenuType == 2)
      {
        g_IContext2 = (LPCONTEXTMENU2)pContextMenu;
      }
      else
      {
        g_IContext3 = (LPCONTEXTMENU3)pContextMenu;
      }
    }
    else
    {
      OldWndProc = NULL;
    }
    POINT pt;
    GetCursorPos(&pt);
    int iCmd = TrackPopupMenuEx(h_menu, TPM_RETURNCMD, pt.x, pt.y, hwndParent, NULL);
    Invoke(iCmd, pContextMenu, hwndParent, path);
    if (OldWndProc)
    {
      SetWindowLongPtr(hwndParent, GWLP_WNDPROC, OldWndProc);
    }
    pContextMenu->Release();
    return TRUE;
  }

  int n2e_isValidRegex(LPCSTR str)
  {
    try
    {
      boost::regex reg(str, boost::regex_constants::ECMAScript);
      return 1;
    }
    catch (...)
    {
      return 0;
    }
  }

  int n2e_GetUTF8CharLength(const unsigned char ch)
  {
    return (int)Scintilla::UTF8CharLength(ch);
  }

  void n2e_ReplaceSubstring(LPSTR buf, LPCSTR from, LPCSTR to)
  {
    std::string str(buf);
    const std::string strFrom(from);
    const std::string strTo(to);
    size_t index = 0;
    while (1)
    {
      index = str.find(strFrom, index);
      if (index == std::string::npos)
      {
        break;
      }
      str.replace(index, strFrom.length(), strTo);
      index += std::min<size_t>(strFrom.length(), strTo.length());
    }
    lstrcpyA(buf, str.c_str());
  }

  void n2e_ReplaceSubstringFormat(LPSTR buf, LPCSTR from, LPCSTR formatTo, const int value)
  {
    char to[10];
    wsprintfA(to, formatTo, value);
    n2e_ReplaceSubstring(buf, from, to);
  }

  std::vector<SE_DATA> vectorEditSelections;

  int n2e_GetEditSelectionCount()
  {
    return (int)vectorEditSelections.size();
  }

  void n2e_ClearEditSelections()
  {
    for (size_t i = 0; i < vectorEditSelections.size(); ++i)
    {
      SE_DATA* se = &vectorEditSelections[i];
      if (se->original)
      {
        n2e_Free(se->original);
        se->original = NULL;
      }
    }
    vectorEditSelections.clear();
  }

  void n2e_AddEditSelection(LPSE_DATA pData)
  {
    vectorEditSelections.push_back(*pData);
  }

  LPSE_DATA n2e_GetEditSelection(const int index)
  {
    return &vectorEditSelections[index];
  }

  std::map<HWND, std::tuple<int, int, int, int, int>> g_mapViewSettings;

  void n2e_SaveViewState(HWND hwnd)
  {
    const int iVisTopLine = (int)SendMessage(hwnd, SCI_GETFIRSTVISIBLELINE, 0, 0);
    g_mapViewSettings[hwnd] = std::make_tuple(
      SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0),
      SendMessage(hwnd, SCI_GETANCHOR, 0, 0),
      iVisTopLine,
      SendMessage(hwnd, SCI_DOCLINEFROMVISIBLE, (WPARAM)iVisTopLine, 0),
      SendMessage(hwnd, SCI_GETXOFFSET, 0, 0));
  }

  void n2e_LoadViewState(HWND hwnd)
  {
    if (g_mapViewSettings.find(hwnd) == g_mapViewSettings.cend())
      return;

    const auto settings = g_mapViewSettings.at(hwnd);
    const auto iCurPos = std::get<0>(settings);
    const auto iAnchorPos = std::get<1>(settings);
    const auto iVisTopLine = std::get<2>(settings);
    const auto iDocTopLine = std::get<3>(settings);
    const auto iXOffset = std::get<4>(settings);

    SendMessage(hwnd, SCI_SETSEL, iAnchorPos, iCurPos);
    SendMessage(hwnd, SCI_ENSUREVISIBLE, (WPARAM)iDocTopLine, 0);
    const auto iNewTopLine = (int)SendMessage(hwnd, SCI_GETFIRSTVISIBLELINE, 0, 0);
    SendMessage(hwnd, SCI_LINESCROLL, 0, (LPARAM)iVisTopLine - iNewTopLine);
    SendMessage(hwnd, SCI_SETXOFFSET, (WPARAM)iXOffset, 0);
  }
}
