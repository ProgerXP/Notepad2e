#include <algorithm>
#include <map>

#include "DPIHelper.h"
#include "resource.h"
#include "VersionHelper.h"

extern "C"
{
#define USER32DLL L"User32.dll"
#define SHCOREDLL L"Shcore.dll"

typedef enum _MONITOR_DPI_TYPE
{
  MDT_EFFECTIVE_DPI = 0,
  MDT_ANGULAR_DPI = 1,
  MDT_RAW_DPI = 2,
  MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;

typedef BOOL(WINAPI *EnableNonClientDpiScalingProc)(HWND hwnd);
typedef HRESULT(WINAPI *GetDpiForMonitorProc)(HMONITOR hMonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

EnableNonClientDpiScalingProc pfnEnableNonClientDpiScaling;
GetDpiForMonitorProc pfnGetDpiForMonitor;

static HMODULE hUser32Module = NULL;
static HMODULE hSHCOREModule = NULL;

BOOL DPIInitialize()
{
  if (IsWindowsVistaOrGreater())
  {
    hUser32Module = GetModuleHandle(USER32DLL);
    if (hUser32Module)
    {
      pfnEnableNonClientDpiScaling = (EnableNonClientDpiScalingProc)GetProcAddress(hUser32Module, "EnableNonClientDpiScaling");
    }
    hSHCOREModule = GetModuleHandle(SHCOREDLL);
    if (hSHCOREModule)
    {
      pfnGetDpiForMonitor = (GetDpiForMonitorProc)GetProcAddress(hSHCOREModule, "GetDpiForMonitor");
    }
    return TRUE;
  }
  return FALSE;
}

BOOL DPIEnableNonClientDpiScaling(const HWND hwnd)
{
  return pfnEnableNonClientDpiScaling ? pfnEnableNonClientDpiScaling(hwnd) : FALSE;
}

BOOL GetDpiForMonitor(const HMONITOR hMonitor, UINT* dpiX, UINT* dpiY)
{
  return pfnGetDpiForMonitor ? SUCCEEDED(pfnGetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, dpiX, dpiY)) : FALSE;
}

DWORD GetDPIFromWindowHDC(const HWND hwnd)
{
  int dpiX = USER_DEFAULT_SCREEN_DPI;
  int dpiY = USER_DEFAULT_SCREEN_DPI;
  if (hwnd)
  {
    const HDC hdc = GetDC(hwnd);
    if (hdc)
    {
      dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
      dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
      ReleaseDC(hwnd, hdc);
    }
  }
  return MAKELONG(dpiX, dpiY);
}

DWORD GetDPIFromMonitor(const HMONITOR hMonitor, const HWND hwnd)
{
  UINT dpiX = USER_DEFAULT_SCREEN_DPI;
  UINT dpiY = USER_DEFAULT_SCREEN_DPI;
  if (GetDpiForMonitor(hMonitor, &dpiX, &dpiY))
  {
    return MAKELONG(dpiX, dpiY);
  }
  return GetDPIFromWindowHDC(hwnd);
}

DWORD GetDPIFromWindow(const HWND hwnd)
{
  return GetDPIFromMonitor(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), hwnd);
}

struct TDPISettings
{
  HWND hwnd;
  HDWP hdwp;
  int dpiX, dpiY;
};

struct TDPISettings CreateDPISettings(const HWND hwnd, const HDWP hdwp, const int dpiX, const int dpiY)
{
  struct TDPISettings dpiSettings;
  dpiSettings.hwnd = hwnd;
  dpiSettings.hdwp = hdwp;
  dpiSettings.dpiX = dpiX;
  dpiSettings.dpiY = dpiY;
  return dpiSettings;
}

int GetScaledFontSize(const int fontHeight, const int fontInitialDPI, const int fontDPI)
{
  return -MulDiv(fontHeight, fontInitialDPI, fontDPI);
}

RECT GetWindowRectangle(const HWND hwnd)
{
  RECT rc = { 0 };
  GetWindowRect(hwnd, &rc);
  return rc;
}

BOOL CALLBACK EnumChildProc_DPIPrepare(const HWND hwnd, const LPARAM lParam)
{
  const struct TDPISettings* pDPISettings = (struct TDPISettings*)lParam;
  const HWND hwndParent = pDPISettings->hwnd;

  if (GetAncestor(hwnd, GA_PARENT) != hwndParent)
  {
    return TRUE;
  }

  RECT rc = GetWindowRectangle(hwnd);
  MapWindowPoints(NULL, hwndParent, (LPPOINT)&rc, 2);

  SetWindowDynamicFont(hwnd, 0);
  SetWindowDPI(hwnd, pDPISettings->dpiX, pDPISettings->dpiY);
  SetWindowPosition(hwnd, rc.left, rc.top);
  SetWindowSize(hwnd, rc.right - rc.left, rc.bottom - rc.top);

  const HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
  if (hFont)
  {
    LOGFONT lf = { 0 };
    GetObject(hFont, sizeof(LOGFONT), &lf);
    const int fontSize = GetScaledFontSize(lf.lfHeight, DEFAULT_FONT_DPI, pDPISettings->dpiY);
    SetWindowFontSize(hwnd, fontSize);
  }

  return TRUE;
};

void DPIPrepare(const HWND hwnd, const int dpiX, const int dpiY)
{
  const struct TDPISettings dpiSettings = CreateDPISettings(hwnd, NULL, dpiX, dpiY);
  EnumChildWindows(hwnd, EnumChildProc_DPIPrepare, (LPARAM)&dpiSettings);
}

RECT DPIAdjustRect(RECT rc, const int dpiXInitial, const int dpiYInitial, const int dpiX, const int dpiY)
{
  rc.left = MulDiv(rc.left, dpiX, dpiXInitial);
  rc.top = MulDiv(rc.top, dpiY, dpiYInitial);
  rc.right = MulDiv(rc.right, dpiX, dpiXInitial);
  rc.bottom = MulDiv(rc.bottom, dpiY, dpiYInitial);
  return rc;
}

BOOL IsWindowVisibleByStyle(const HWND hwnd)
{
  return GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE;
}

BOOL DPIApply(const HWND hwnd, const HWND hwndParent, HDWP* lpHDWP, const int dpiX, const int dpiY)
{
  const DWORD dpiInitial = GetWindowDPI(hwnd);
  const DWORD windowPos = GetWindowPosition(hwnd);
  const DWORD windowSize = GetWindowSize(hwnd);

  if (!dpiInitial || !windowPos || !windowSize)
  {
    return FALSE;
  }

  RECT rc = {
    LOWORD(windowPos),
    HIWORD(windowPos),
    LOWORD(windowPos) + LOWORD(windowSize),
    HIWORD(windowPos) + HIWORD(windowSize)
  };
  const BOOL isSizeGripCtrl = (GetWindowLongPtr(hwnd, GWL_STYLE) & SBS_SIZEGRIP);
  if (isSizeGripCtrl)
  {
    GetClientRect(hwndParent, &rc);
    const int iSizeGrip = GetSystemMetrics(SM_CXHTHUMB);
    rc.left = rc.right - iSizeGrip;
    rc.top = rc.bottom - iSizeGrip;
  }
  else
  {
    rc = DPIAdjustRect(rc, LOWORD(dpiInitial), HIWORD(dpiInitial), dpiX, dpiY);
  }
  if (lpHDWP && *lpHDWP)
  {
    *lpHDWP = DeferWindowPos(*lpHDWP, hwnd, NULL, rc.left, rc.top,
                   rc.right - rc.left, rc.bottom - rc.top,
                   SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
  }
  else
  {
    SetWindowPos(hwnd, NULL, rc.left, rc.top,
                 rc.right - rc.left, rc.bottom - rc.top,
                 SWP_NOZORDER | SWP_NOOWNERZORDER | (IsWindowVisibleByStyle(hwnd) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOACTIVATE);
  }  

  const HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
  const int fontSizeOriginal = GetWindowFontSize(hwnd);
  if (hFont && (fontSizeOriginal > 0))
  {
    LOGFONT lf = { 0 };
    if (GetObject(hFont, sizeof(LOGFONT), &lf))
    {
      lf.lfHeight = GetScaledFontSize(fontSizeOriginal, dpiY, DEFAULT_FONT_DPI);
      const HFONT hFontNew = CreateFontIndirect(&lf);
      if (hFontNew)
      {
        const HANDLE hDynamicFont = GetWindowDynamicFont(hwnd);
        if (hDynamicFont)
        {
          DeleteObject(hDynamicFont);
        }
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hFontNew, 0);
        SetWindowDynamicFont(hwnd, hFontNew);
      }
    }
  }
  return TRUE;
}

BOOL CALLBACK EnumChildProc_DPIApply(const HWND hwnd, const LPARAM lParam)
{
  struct TDPISettings* pDPISettings = (struct TDPISettings*)lParam;
  DPIApply(hwnd, pDPISettings->hwnd, &pDPISettings->hdwp, pDPISettings->dpiX, pDPISettings->dpiY);
  return TRUE;
};

HBITMAP StretchBitmap(const HBITMAP hbmp, const int newSizeX, const int newSizeY)
{
  HBITMAP hbmpScaled = NULL;
  const HDC hdcScreen = GetDC(NULL);
  if (const HDC hdcSource = CreateCompatibleDC(hdcScreen))
  {
    SelectObject(hdcSource, hbmp);
    if (const HDC hdcScaled = CreateCompatibleDC(hdcScreen))
    {
      hbmpScaled = CreateCompatibleBitmap(hdcScreen, newSizeX, newSizeY);
      if (hbmpScaled)
      {
        SelectObject(hdcScaled, hbmpScaled);
        BITMAP bmp = { 0 };
        if (!GetObject(hbmp, sizeof(BITMAP), &bmp)
          || !StretchBlt(hdcScaled, 0, 0, newSizeX, newSizeY, hdcSource, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY))
        {
          DeleteObject(hbmpScaled);
          hbmpScaled = NULL;
        }
      }
      DeleteDC(hdcScaled);
    }
    DeleteDC(hdcSource);
  }
  ReleaseDC(NULL, hdcScreen);
  return hbmpScaled;
}

HBITMAP DPICreateToolbarBitmap(const HWND hwnd, const HINSTANCE hInstance)
{
  const DWORD DPI_MAP_STEP_SIZE = DWORD(DEFAULT_SCREEN_DPI * 0.25);
  const std::map<DWORD, UINT> c_mapDPIToToolbarID =
  {
    { DWORD(DEFAULT_SCREEN_DPI), IDR_MAINWND },
    { DWORD(DEFAULT_SCREEN_DPI * 1.25), IDB_TOOLBAR_125 },
    { DWORD(DEFAULT_SCREEN_DPI * 1.5), IDB_TOOLBAR_150 },
    { DWORD(DEFAULT_SCREEN_DPI * 1.75), IDB_TOOLBAR_175 },
    { DWORD(DEFAULT_SCREEN_DPI * 2), IDB_TOOLBAR_200 }
  };

  const DWORD dpiY = HIWORD(GetDPIFromWindow(hwnd));
  const auto it = std::find_if(c_mapDPIToToolbarID.cbegin(), c_mapDPIToToolbarID.cend(), [&](const std::pair<DWORD, UINT>& v) {
      return (dpiY < v.first + DPI_MAP_STEP_SIZE / 2);
    });
  const bool stretchBitmap = (it == c_mapDPIToToolbarID.cend());
  const auto& toolbarParams = !stretchBitmap ? *it : *c_mapDPIToToolbarID.crbegin();

  HBITMAP hbmp = LoadBitmap(hInstance, MAKEINTRESOURCE(toolbarParams.second));
  if (stretchBitmap)
  {
    BITMAP bmp = { 0 };
    GetObject(hbmp, sizeof(BITMAP), &bmp);
    const int newSizeY = (bmp.bmHeight * dpiY) / toolbarParams.first;
    const int newSizeX = (bmp.bmWidth * newSizeY) / bmp.bmHeight;
    if (const HBITMAP hbmpScaled = StretchBitmap(hbmp, newSizeX, newSizeY))
    {
      DeleteObject(hbmp);
      hbmp = hbmpScaled;
    }
  }
  return hbmp;
}

void DialogDPIInit(const HWND hwnd)
{
  if (!IsWindowsVistaOrGreater())
  {
    return;
  }
  DialogDPIUpdate(hwnd, TRUE);

  RECT rc = GetWindowRectangle(hwnd);
  const DWORD dpiHDC = GetDPIFromWindowHDC(hwnd);
  const DWORD dpi = GetDPIFromWindow(hwnd);
  SetWindowDPI(hwnd, LOWORD(dpiHDC), HIWORD(dpiHDC));
  rc = DPIAdjustRect(rc, LOWORD(dpiHDC), HIWORD(dpiHDC), LOWORD(dpi), HIWORD(dpi));

  SetWindowDPIInitialized(hwnd, TRUE);
  DPIChanged_DlgProcHandler(hwnd, dpi, (LPARAM)&rc);
}

void DialogDPIUpdate(const HWND hwnd, const BOOL bDPIFromHDC)
{
  if (!IsWindowsVistaOrGreater() || GetWindowSkipDPIResize(hwnd))
  {
    return;
  }

  const DWORD dpi = bDPIFromHDC ? GetDPIFromWindowHDC(hwnd) : GetDPIFromWindow(hwnd);
  DPIPrepare(hwnd, LOWORD(dpi), HIWORD(dpi));
}

void DialogDPIGetMinMaxInfo(const HWND hwnd, LPARAM lParam)
{
  const DWORD dpiInitial = GetWindowDPI(hwnd);
  if (!dpiInitial || !IsWindowsVistaOrGreater())
  {
    return;
  }
  LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
  const DWORD dpi = GetDPIFromWindow(hwnd);
  RECT rc = {
    0,
    0,
    lpmmi->ptMinTrackSize.x,
    lpmmi->ptMinTrackSize.y
  };
  rc = DPIAdjustRect(rc, LOWORD(dpiInitial), HIWORD(dpiInitial), LOWORD(dpi), HIWORD(dpi));
  lpmmi->ptMinTrackSize.x = rc.right;
  lpmmi->ptMinTrackSize.y = rc.bottom;
}

LRESULT DPIChanged_WindowProcHandler(const HWND hwnd, const WPARAM /*wParam*/, const LPARAM lParam)
{
  SetWindowSkipDPIResize(hwnd, TRUE);
  LPCRECT lpRect = (LPCRECT)lParam;
  SetWindowPos(hwnd, NULL, lpRect->left, lpRect->top,
                lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_FRAMECHANGED);
  SetWindowSkipDPIResize(hwnd, FALSE);
  return 0;
}

LRESULT DPIChanged_DlgProcHandler(const HWND hwnd, const WPARAM wParam, const LPARAM lParam)
{
  if (!GetWindowDPIInitialized(hwnd))
  {
    return FALSE;
  }

  DPIChanged_WindowProcHandler(hwnd, wParam, lParam);

  const struct TDPISettings dpiSettings = CreateDPISettings(hwnd, BeginDeferWindowPos(10), LOWORD(wParam), HIWORD(wParam));
  EnumChildWindows(hwnd, EnumChildProc_DPIApply, (LPARAM)&dpiSettings);
  EndDeferWindowPos(dpiSettings.hdwp);

  InvalidateRect(hwnd, NULL, TRUE);
  UpdateWindow(hwnd);

  return 0;
}

}; // extern "C"