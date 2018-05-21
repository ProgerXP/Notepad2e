#include "DPIHelper.h"
#include "Scintilla.h"
#include "SciCall.h"
#include "VersionHelper.h"

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

BOOL n2e_DPIInitialize()
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

BOOL n2e_EnableNonClientDpiScaling(const HWND hwnd)
{
  return pfnEnableNonClientDpiScaling ? pfnEnableNonClientDpiScaling(hwnd) : FALSE;
}

BOOL n2e_GetDpiForMonitor(const HMONITOR hMonitor, UINT* dpiX, UINT* dpiY)
{
  return pfnGetDpiForMonitor ? SUCCEEDED(pfnGetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, dpiX, dpiY)) : FALSE;
}

DWORD n2e_GetDPIFromWindowHDC(const HWND hwnd)
{
  int dpiX = USER_DEFAULT_SCREEN_DPI, dpiY = USER_DEFAULT_SCREEN_DPI;
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

DWORD n2e_GetDPIFromMonitor(const HMONITOR hMonitor, const HWND hwnd)
{
  int dpiX = USER_DEFAULT_SCREEN_DPI, dpiY = USER_DEFAULT_SCREEN_DPI;
  if (n2e_GetDpiForMonitor(hMonitor, &dpiX, &dpiY))
  {
    return MAKELONG(dpiX, dpiY);
  }
  return n2e_GetDPIFromWindowHDC(hwnd);
}

DWORD n2e_GetDPIFromWindow(const HWND hwnd)
{
  return n2e_GetDPIFromMonitor(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), hwnd);
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

int n2e_GetScaledFontSize(const int fontHeight, const int fontInitialDPI, const int fontDPI)
{
  return -MulDiv(fontHeight, fontInitialDPI, fontDPI);
}

RECT n2e_GetWindowRect(const HWND hwnd)
{
  RECT rc = { 0 };
  GetWindowRect(hwnd, &rc);
  return rc;
}

BOOL CALLBACK n2e_EnumChildProc_DPIPrepare(const HWND hwnd, const LPARAM lParam)
{
  const struct TDPISettings* pDPISettings = (struct TDPISettings*)lParam;
  const HWND hwndParent = pDPISettings->hwnd;

  if (GetAncestor(hwnd, GA_PARENT) != hwndParent)
  {
    return TRUE;
  }

  RECT rc = n2e_GetWindowRect(hwnd);
  MapWindowPoints(NULL, hwndParent, (LPPOINT)&rc, 2);

  n2e_SetWindowDynamicFont(hwnd, 0);
  n2e_SetWindowDPI(hwnd, pDPISettings->dpiX, pDPISettings->dpiY);
  n2e_SetWindowPosition(hwnd, rc.left, rc.top);
  n2e_SetWindowSize(hwnd, rc.right - rc.left, rc.bottom - rc.top);

  const HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
  if (hFont)
  {
    LOGFONT lf = { 0 };
    GetObject(hFont, sizeof(LOGFONT), &lf);
    const int fontSize = n2e_GetScaledFontSize(lf.lfHeight, DEFAULT_FONT_DPI, pDPISettings->dpiY);
    n2e_SetWindowFontSize(hwnd, fontSize);
  }

  return TRUE;
};

void n2e_DPIPrepare(const HWND hwnd, const int dpiX, const int dpiY)
{
  const struct TDPISettings dpiSettings = CreateDPISettings(hwnd, NULL, dpiX, dpiY);
  EnumChildWindows(hwnd, n2e_EnumChildProc_DPIPrepare, (LPARAM)&dpiSettings);
}

RECT n2e_DPIAdjustRect(RECT rc, const int dpiXInitial, const int dpiYInitial, const int dpiX, const int dpiY)
{
  rc.left = MulDiv(rc.left, dpiX, dpiXInitial);
  rc.top = MulDiv(rc.top, dpiY, dpiYInitial);
  rc.right = MulDiv(rc.right, dpiX, dpiXInitial);
  rc.bottom = MulDiv(rc.bottom, dpiY, dpiYInitial);
  return rc;
}

BOOL n2e_IsWindowVisibleByStyle(const HWND hwnd)
{
  return GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE;
}

BOOL n2e_DPIApply(const HWND hwnd, const HWND hwndParent, HDWP* lpHDWP, const int dpiX, const int dpiY)
{
  const DWORD dpiInitial = n2e_GetWindowDPI(hwnd);
  const DWORD windowPos = n2e_GetWindowPosition(hwnd);
  const DWORD windowSize = n2e_GetWindowSize(hwnd);

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
    rc = n2e_DPIAdjustRect(rc, LOWORD(dpiInitial), HIWORD(dpiInitial), dpiX, dpiY);
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
                 SWP_NOZORDER | SWP_NOOWNERZORDER | (n2e_IsWindowVisibleByStyle(hwnd) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOACTIVATE);
  }  

  const HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
  const int fontSizeOriginal = n2e_GetWindowFontSize(hwnd);
  if (hFont && (fontSizeOriginal > 0))
  {
    LOGFONT lf = { 0 };
    if (GetObject(hFont, sizeof(LOGFONT), &lf))
    {
      lf.lfHeight = n2e_GetScaledFontSize(fontSizeOriginal, dpiY, DEFAULT_FONT_DPI);
      const HFONT hFontNew = CreateFontIndirect(&lf);
      if (hFontNew)
      {
        const HANDLE hDynamicFont = n2e_GetWindowDynamicFont(hwnd);
        if (hDynamicFont)
        {
          DeleteObject(hDynamicFont);
        }
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hFontNew, 0);
        n2e_SetWindowDynamicFont(hwnd, hFontNew);
      }
    }
  }
  return TRUE;
}

BOOL CALLBACK n2e_EnumChildProc_DPIApply(const HWND hwnd, const LPARAM lParam)
{
  struct TDPISettings* pDPISettings = (struct TDPISettings*)lParam;
  n2e_DPIApply(hwnd, pDPISettings->hwnd, &pDPISettings->hdwp, pDPISettings->dpiX, pDPISettings->dpiY);
  return TRUE;
};

void n2e_ScintillaDPIInit(const HWND hwnd)
{
  if (!IsWindowsVistaOrGreater())
  {
    return;
  }
  const RECT rc = n2e_GetWindowRect(hwnd);
  const POINT pt = { rc.left, rc.top };
  SciCall_SetDPI(n2e_GetDPIFromMonitor(MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY), hwnd));
}

void n2e_ScintillaDPIUpdate(const HWND hwnd, const WPARAM dpi)
{
  if (!IsWindowsVistaOrGreater())
  {
    return;
  }
  SciCall_SetDPI(dpi);
}

void n2e_DialogDPIInit(const HWND hwnd)
{
  if (!IsWindowsVistaOrGreater())
  {
    return;
  }
  n2e_DialogDPIUpdate(hwnd, TRUE);

  RECT rc = n2e_GetWindowRect(hwnd);
  const DWORD dpiHDC = n2e_GetDPIFromWindowHDC(hwnd);
  const DWORD dpi = n2e_GetDPIFromWindow(hwnd);
  n2e_SetWindowDPI(hwnd, LOWORD(dpiHDC), HIWORD(dpiHDC));
  rc = n2e_DPIAdjustRect(rc, LOWORD(dpiHDC), HIWORD(dpiHDC), LOWORD(dpi), HIWORD(dpi));

  n2e_SetWindowDPIInitialized(hwnd, TRUE);
  n2e_DPIChanged_DlgProcHandler(hwnd, dpi, (LPARAM)&rc);
}

void n2e_DialogDPIUpdate(const HWND hwnd, const BOOL bDPIFromHDC)
{
  if (!IsWindowsVistaOrGreater() || n2e_GetWindowSkipDPIResize(hwnd))
  {
    return;
  }

  const DWORD dpi = bDPIFromHDC ? n2e_GetDPIFromWindowHDC(hwnd) : n2e_GetDPIFromWindow(hwnd);
  n2e_DPIPrepare(hwnd, LOWORD(dpi), HIWORD(dpi));
}

void n2e_DialogDPIGetMinMaxInfo(const HWND hwnd, LPARAM lParam)
{
  const DWORD dpiInitial = n2e_GetWindowDPI(hwnd);
  if (!dpiInitial || !IsWindowsVistaOrGreater())
  {
    return;
  }
  LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
  const DWORD dpi = n2e_GetDPIFromWindow(hwnd);
  RECT rc = {
    0,
    0,
    lpmmi->ptMinTrackSize.x,
    lpmmi->ptMinTrackSize.y
  };
  rc = n2e_DPIAdjustRect(rc, LOWORD(dpiInitial), HIWORD(dpiInitial), LOWORD(dpi), HIWORD(dpi));
  lpmmi->ptMinTrackSize.x = rc.right;
  lpmmi->ptMinTrackSize.y = rc.bottom;
}

LRESULT n2e_DPIChanged_WindowProcHandler(const HWND hwnd, const WPARAM wParam, const LPARAM lParam)
{
  n2e_SetWindowSkipDPIResize(hwnd, TRUE);
  LPCRECT lpRect = (LPCRECT)lParam;
  SetWindowPos(hwnd, NULL, lpRect->left, lpRect->top,
                lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_FRAMECHANGED);
  n2e_SetWindowSkipDPIResize(hwnd, FALSE);
  return 0;
}

LRESULT n2e_DPIChanged_DlgProcHandler(const HWND hwnd, const WPARAM wParam, const LPARAM lParam)
{
  if (!n2e_GetWindowDPIInitialized(hwnd))
  {
    return FALSE;
  }

  n2e_DPIChanged_WindowProcHandler(hwnd, wParam, lParam);

  const struct TDPISettings dpiSettings = CreateDPISettings(hwnd, BeginDeferWindowPos(10), LOWORD(wParam), HIWORD(wParam));
  EnumChildWindows(hwnd, n2e_EnumChildProc_DPIApply, (LPARAM)&dpiSettings);
  EndDeferWindowPos(dpiSettings.hdwp);

  InvalidateRect(hwnd, NULL, TRUE);
  UpdateWindow(hwnd);

  return 0;
}
