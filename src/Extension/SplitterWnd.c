#include "SplitterWnd.h"
#include <windowsx.h>
#include "CommonUtils.h"

LRESULT CALLBACK SplitterProc(HWND hWndSplitter, UINT Message, WPARAM wParam, LPARAM lParam);

const wchar_t UC_SPLITTER[] = { L"SplitterWnd" };
const wchar_t SETTINGS[] = { L"SplitterSettings" };

#define HTSPLITTER_MOVE 25
const int SPLITTER_WIDTH = 5;

ATOM InitSplitter()
{
  WNDCLASS wc = { 0, SplitterProc, 0, 0, GetModuleHandle(NULL), NULL, NULL, GetSysColorBrush(COLOR_BTNFACE), NULL, UC_SPLITTER };
  return RegisterClass(&wc);
}

struct TWndInfo
{
  HWND hwnd;
  RECT rc;
};
typedef struct TWndInfo WndInfo;

struct TSplitterSettings
{
  HWND hwndSelf;
  BOOL bHorizontal;
  RECT rcSplitter;
  WndInfo children[2];
  POINT ptResizingCurrent;
  BOOL bResizing;
  POINT ptResizingStart;
  POINT ptMoving;
};
typedef struct TSplitterSettings SplitterSettings;

SplitterSettings* GetSettings(HWND hwndSplitter)
{
  return (SplitterSettings*)GetProp(hwndSplitter, SETTINGS);
}

HWND CreateSplitterWnd(const HWND hwndParent, const HWND hwndChild1, const HWND hwndChild2, const BOOL bHorizontal)
{
  ATOM atomSplitter = InitSplitter();
  SplitterSettings* params = n2e_Alloc(sizeof(SplitterSettings));
  ZeroMemory(params, sizeof(SplitterSettings));
  params->bHorizontal = bHorizontal;
  params->children[0].hwnd = hwndChild1;
  params->children[1].hwnd = hwndChild2;
  CreateWindow(UC_SPLITTER, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, 200, 200, hwndParent, NULL, GetModuleHandle(NULL), params);
  return params->hwndSelf;
}

void UpdateChildrenPos(SplitterSettings* pSettings, const int offsetX, const int offsetY)
{
  RECT rc;
  GetClientRect(pSettings->hwndSelf, &rc);
  RECT rc1 = rc;
  RECT rc2 = rc;
  if (pSettings->bHorizontal)
  {
    rc1.right = (rc.right - rc.left) / 2 - SPLITTER_WIDTH / 2 - offsetX;
    rc2.left = rc1.right + SPLITTER_WIDTH;
    pSettings->rcSplitter.left = rc1.right;
    pSettings->rcSplitter.top = rc1.top;
    pSettings->rcSplitter.right = rc2.left;
    pSettings->rcSplitter.bottom = rc2.bottom;
  }
  else
  {
    rc1.bottom = (rc.bottom - rc.top) / 2 - SPLITTER_WIDTH / 2 - offsetY;
    rc2.top = rc1.bottom + SPLITTER_WIDTH;
    pSettings->rcSplitter.left = rc1.left;
    pSettings->rcSplitter.top = rc1.bottom;
    pSettings->rcSplitter.right = rc2.right;
    pSettings->rcSplitter.bottom = rc2.top;
  };
  pSettings->children[0].rc = rc1;
  pSettings->children[1].rc = rc2;
  SetWindowPos(pSettings->children[0].hwnd, NULL, rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top, SWP_NOACTIVATE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
  SetWindowPos(pSettings->children[1].hwnd, pSettings->children[0].hwnd, rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top, SWP_NOACTIVATE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

LRESULT CALLBACK SplitterProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_CREATE:
    {
      LPCREATESTRUCT lpCreate = (LPCREATESTRUCT)lParam;
      SetProp(hWnd, SETTINGS, (HANDLE)lpCreate->lpCreateParams);
      SplitterSettings* pSettings = GetSettings(hWnd);
      if (pSettings)
      {
        SetParent(pSettings->children[0].hwnd, hWnd);
        SetParent(pSettings->children[1].hwnd, hWnd);
        pSettings->hwndSelf = hWnd;
        UpdateChildrenPos(pSettings, pSettings->ptResizingCurrent.x, pSettings->ptResizingCurrent.y);
      }
    }
    return 0;
  case WM_NCHITTEST:
    {
      const LRESULT lRes = DefWindowProc(hWnd, uMsg, wParam, lParam);
      const SplitterSettings* pSettings = GetSettings(hWnd);
      POINT ptMouse = { LOWORD(lParam), HIWORD(lParam) };
      ScreenToClient(hWnd, &ptMouse);
      return ((lRes == HTCLIENT) && PtInRect(&pSettings->rcSplitter, ptMouse)) ? HTSPLITTER_MOVE : lRes;
    }
    break;
  case WM_SETCURSOR:
    if (LOWORD(lParam) == HTSPLITTER_MOVE)
    {
      const SplitterSettings* pSettings = GetSettings(hWnd);
      if (pSettings)
      {
        SetCursor(LoadCursor(NULL, pSettings->bHorizontal ? IDC_SIZEWE : IDC_SIZENS));
        return TRUE;
      }
    }
    break;
  case WM_NCLBUTTONDOWN:
    {
      SplitterSettings* pSettings = GetSettings(hWnd);
      if (pSettings)
      {
        if (wParam == HTSPLITTER_MOVE)
        {
          SetCapture(hWnd);
          POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
          ScreenToClient(hWnd, &pt);
          pSettings->bResizing = TRUE;
          pSettings->ptResizingStart = pt;
        }
      }
    }
    break;
  case WM_MOUSEMOVE:
    {
      SplitterSettings* pSettings = GetSettings(hWnd);
      if (pSettings)
      {
        pSettings->ptMoving.x = pSettings->ptResizingCurrent.x + pSettings->ptResizingStart.x - GET_X_LPARAM(lParam);
        pSettings->ptMoving.y = pSettings->ptResizingCurrent.y + pSettings->ptResizingStart.y - GET_Y_LPARAM(lParam);
        UpdateChildrenPos(GetSettings(hWnd), pSettings->ptMoving.x, pSettings->ptMoving.y);
        return 0;
      }
    }
    break;
  case WM_LBUTTONUP:
    {
      SplitterSettings* pSettings = GetSettings(hWnd);
      if (pSettings && pSettings->bResizing)
      {
        ReleaseCapture();
        pSettings->ptResizingCurrent = pSettings->ptMoving;
        pSettings->bResizing = FALSE;
      }
    }
    break;
  case WM_SIZE:
    {
      SplitterSettings* pSettings = GetSettings(hWnd);
      if (pSettings)
      {
        UpdateChildrenPos(GetSettings(hWnd), pSettings->ptResizingCurrent.x, pSettings->ptResizingCurrent.y);
        return 0;
      }
    }
    break;
  default:
    break;
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
