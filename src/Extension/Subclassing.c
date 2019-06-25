#include "Subclassing.h"

#define PROPERTY_ORIGINAL_WINDOW_PROC L"OriginalWindowProc"

BOOL n2e_IsSubclassedWindow(const HWND hwnd)
{
  return (GetProp(hwnd, PROPERTY_ORIGINAL_WINDOW_PROC) != 0);
}

BOOL n2e_SubclassWindow(const HWND hwnd, const WNDPROC lpWndProc)
{
  if (!n2e_IsSubclassedWindow(hwnd))
  {
    return SetProp(hwnd, PROPERTY_ORIGINAL_WINDOW_PROC, (HANDLE)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)lpWndProc));
  }
  return FALSE;
}

WNDPROC n2e_GetOriginalWindowProc(const HWND hwnd)
{
  return (WNDPROC)GetProp(hwnd, PROPERTY_ORIGINAL_WINDOW_PROC);
}

LRESULT n2e_CallOriginalWindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
  return CallWindowProc(n2e_GetOriginalWindowProc(hwnd), hwnd, uMsg, wParam, lParam);
}
