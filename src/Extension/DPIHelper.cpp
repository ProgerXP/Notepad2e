#include "DPIHelper.h"

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

static BOOL isWindowsVistaOrGreater();

BOOL DPIInitialize()
{
  if (isWindowsVistaOrGreater())
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

void DialogDPIInit(const HWND hwnd)
{
  if (!isWindowsVistaOrGreater())
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
  if (!isWindowsVistaOrGreater() || GetWindowSkipDPIResize(hwnd))
  {
    return;
  }

  const DWORD dpi = bDPIFromHDC ? GetDPIFromWindowHDC(hwnd) : GetDPIFromWindow(hwnd);
  DPIPrepare(hwnd, LOWORD(dpi), HIWORD(dpi));
}

void DialogDPIGetMinMaxInfo(const HWND hwnd, LPARAM lParam)
{
  const DWORD dpiInitial = GetWindowDPI(hwnd);
  if (!dpiInitial || !isWindowsVistaOrGreater())
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

static BOOL isWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
  OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0,{ 0 }, 0, 0 };
  DWORDLONG        const dwlConditionMask = VerSetConditionMask(
    VerSetConditionMask(
      VerSetConditionMask(
        0, VER_MAJORVERSION, VER_GREATER_EQUAL),
      VER_MINORVERSION, VER_GREATER_EQUAL),
    VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

  osvi.dwMajorVersion = wMajorVersion;
  osvi.dwMinorVersion = wMinorVersion;
  osvi.wServicePackMajor = wServicePackMajor;

  return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

static BOOL isWindowsVistaOrGreater()
{
  static BOOL bInitialized = FALSE;
  static BOOL bWindowsVistaOrGreater = FALSE;
  if (!bInitialized)
  {
    bWindowsVistaOrGreater = isWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
    bInitialized = TRUE;
  }
  return bWindowsVistaOrGreater;
}
}; // extern "C"