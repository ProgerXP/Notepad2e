#include "DPIHelper.h"
#include "Scintilla.h"
#include "SciCall.h"

#define PROPERTY_WINDOW_DYNAMIC_FONT L"WindowDynamicFont"
#define PROPERTY_WINDOW_DPI L"WindowDPI"
#define PROPERTY_WINDOW_POS L"WindowPosition"
#define PROPERTY_WINDOW_SIZE L"WindowSize"
#define PROPERTY_WINDOW_FONT_SIZE L"WindowFontSize"

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

BOOL IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
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

BOOL IsWindowsVistaOrGreater()
{
  static BOOL bInitialized = FALSE;
  static BOOL bWindowsVistaOrGreater = FALSE;
  if (!bInitialized)
  {
    bWindowsVistaOrGreater = IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
    bInitialized = TRUE;
  }
  return bWindowsVistaOrGreater;
}

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

BOOL n2e_EnableNonClientDpiScaling(HWND hwnd)
{
  return pfnEnableNonClientDpiScaling ? pfnEnableNonClientDpiScaling(hwnd) : FALSE;
}

BOOL n2e_GetDpiForMonitor(HMONITOR hMonitor, UINT* dpiX, UINT* dpiY)
{
  return pfnGetDpiForMonitor ? SUCCEEDED(pfnGetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, dpiX, dpiY)) : FALSE;
}

DWORD n2e_GetDPIFromMonitor(HMONITOR hMonitor, HWND hwnd)
{
  int dpiX = USER_DEFAULT_SCREEN_DPI, dpiY = USER_DEFAULT_SCREEN_DPI;
  if (!n2e_GetDpiForMonitor(hMonitor, &dpiX, &dpiY) && hwnd)
  {
    const HDC hdc = GetDC(hwnd);
    dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(hwnd, hdc);
  }
  return MAKELONG(dpiX, dpiY);
}

DWORD n2e_GetDPIFromWindow(HWND hwnd)
{
  return n2e_GetDPIFromMonitor(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), hwnd);
}

struct TDPISettings
{
  HANDLE hData;
  int dpiX, dpiY;
};

struct TDPISettings DPISettings_FromData(const HANDLE hData, const int dpiX, const int dpiY)
{
  struct TDPISettings dpiSettings;
  dpiSettings.hData = hData;
  dpiSettings.dpiX = dpiX;
  dpiSettings.dpiY = dpiY;
  return dpiSettings;
}

int n2e_GetScaledFontSize(const int fontHeight, const int fontInitialDPI, const int fontDPI)
{
  return -MulDiv(fontHeight, fontInitialDPI, fontDPI);
}

BOOL CALLBACK n2e_EnumChildProc_DPIPrepare(HWND hwnd, LPARAM lParam)
{
  const struct TDPISettings* pDPISettings = (struct TDPISettings*)lParam;
  const HWND hwndParent = (HWND)pDPISettings->hData;

  RECT rc = { 0 };
  GetWindowRect(hwnd, &rc);
  POINT ptLeftTop = { rc.left, rc.top };
  POINT ptRightBottom = { rc.right, rc.bottom };
  ScreenToClient(hwndParent, &ptLeftTop);
  ScreenToClient(hwndParent, &ptRightBottom);

  SetProp(hwnd, PROPERTY_WINDOW_DYNAMIC_FONT, (HANDLE)0);
  SetProp(hwnd, PROPERTY_WINDOW_DPI, (HANDLE)MAKEWPARAM(pDPISettings->dpiX, pDPISettings->dpiY));
  SetProp(hwnd, PROPERTY_WINDOW_POS, (HANDLE)MAKEWPARAM(ptLeftTop.x, ptLeftTop.y));
  SetProp(hwnd, PROPERTY_WINDOW_SIZE, (HANDLE)MAKEWPARAM(ptRightBottom.x - ptLeftTop.x, ptRightBottom.y - ptLeftTop.y));

  int fontSize = 0;
  const HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
  if (hFont)
  {
    LOGFONT lf = { 0 };
    GetObject(hFont, sizeof(LOGFONT), &lf);
    fontSize = n2e_GetScaledFontSize(lf.lfHeight, DEFAULT_FONT_DPI, pDPISettings->dpiY);
    SetProp(hwnd, PROPERTY_WINDOW_FONT_SIZE, (HANDLE)fontSize);
  }

  return TRUE;
};

void n2e_DPIPrepare(const HWND hwnd, const int dpiX, const int dpiY)
{
  const struct TDPISettings dpiSettings = DPISettings_FromData(hwnd, dpiX, dpiY);
  EnumChildWindows(hwnd, n2e_EnumChildProc_DPIPrepare, (LPARAM)&dpiSettings);
}

RECT n2e_DPIAdjustRect(RECT rc, const int dpiInitialX, const int dpiInitialY, const int dpiX, const int dpiY)
{
  rc.left = MulDiv(rc.left, dpiX, dpiInitialX);
  rc.top = MulDiv(rc.top, dpiY, dpiInitialY);
  rc.right = MulDiv(rc.right, dpiX, dpiInitialX);
  rc.bottom = MulDiv(rc.bottom, dpiY, dpiInitialY);
  return rc;
}

BOOL n2e_IsWindowVisibleByStyle(const HWND hwnd)
{
  return GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE;
}

void n2e_DPIApply(HWND hwnd, HDWP hWinPosInfo, const int dpiX, const int dpiY)
{
  const DWORD dpiInitial = (DWORD)GetProp(hwnd, PROPERTY_WINDOW_DPI);
  const DWORD windowPos = (DWORD)GetProp(hwnd, PROPERTY_WINDOW_POS);
  const DWORD windowSize = (DWORD)GetProp(hwnd, PROPERTY_WINDOW_SIZE);

  RECT rc = {
    LOWORD(windowPos),
    HIWORD(windowPos),
    LOWORD(windowPos) + LOWORD(windowSize),
    HIWORD(windowPos) + HIWORD(windowSize)
  };
  rc = n2e_DPIAdjustRect(rc, LOWORD(dpiInitial), HIWORD(dpiInitial), dpiX, dpiY);
  if (hWinPosInfo)
  {
    DeferWindowPos(hWinPosInfo, hwnd, NULL, rc.left, rc.top,
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
  const int fontSizeOriginal = (int)GetProp(hwnd, PROPERTY_WINDOW_FONT_SIZE);
  if (hFont && (fontSizeOriginal > 0))
  {
    LOGFONT lf = { 0 };
    if (GetObject(hFont, sizeof(LOGFONT), &lf))
    {
      lf.lfHeight = n2e_GetScaledFontSize(fontSizeOriginal, dpiY, DEFAULT_FONT_DPI);
      const HFONT hFontNew = CreateFontIndirect(&lf);
      if (hFontNew)
      {
        const HANDLE hDynamicFont = (HANDLE)GetProp(hwnd, PROPERTY_WINDOW_DYNAMIC_FONT);
        if (hDynamicFont)
        {
          DeleteObject(hDynamicFont);
        }
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hFontNew, 0);
        SetProp(hwnd, PROPERTY_WINDOW_DYNAMIC_FONT, (HANDLE)hFontNew);
      }
    }
  }
}

BOOL CALLBACK n2e_EnumChildProc_DPIApply(HWND hwnd, LPARAM lParam)
{
  const struct TDPISettings* pDPISettings = (struct TDPISettings*)lParam;
  n2e_DPIApply(hwnd, (HDWP)pDPISettings->hData, pDPISettings->dpiX, pDPISettings->dpiY);
  return TRUE;
};

void n2e_ScintillaSetDPI(HWND hwnd, const WPARAM dpi)
{
  if (IsWindowsVistaOrGreater())
  {
    SciCall_SetDPI(dpi);
  }
}

void n2e_ScintillaDPIInit(HWND hwnd)
{
  RECT rc = { 0 };
  GetWindowRect(hwnd, &rc);
  POINT pt = { rc.left, rc.top };
  n2e_ScintillaSetDPI(hwnd, n2e_GetDPIFromMonitor(MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY), hwnd));
}

void n2e_ScintillaDPIUpdate(HWND hwnd, const WPARAM dpi)
{
  if (IsWindowsVistaOrGreater())
  {
    n2e_ScintillaSetDPI(hwnd, dpi);
  }
}

void n2e_DialogDPIInit(HWND hwnd)
{
  if (!IsWindowsVistaOrGreater())
  {
    return;
  }

  HDC hdc = GetDC(hwnd);
  const WPARAM dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
  const WPARAM dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
  ReleaseDC(hwnd, hdc);

  n2e_DPIPrepare(hwnd, dpiX, dpiY);

  RECT rc = { 0 };
  GetWindowRect(hwnd, &rc);
  const DWORD dpi = n2e_GetDPIFromWindow(hwnd);

  rc = n2e_DPIAdjustRect(rc, dpiX, dpiY, LOWORD(dpi), HIWORD(dpi));
  SendMessage(hwnd, WM_DPICHANGED, dpi, (LPARAM)&rc);
}

LRESULT n2e_DPIChanged_WindowProcHandler(const HWND hwnd, const WPARAM wParam, const LPARAM lParam)
{
  LPCRECT lpRect = (LPCRECT)lParam;
  SetWindowPos(hwnd, NULL, lpRect->left, lpRect->top,
                lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
  return 0;
}

LRESULT n2e_DPIChanged_DlgProcHandler(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  n2e_DPIChanged_WindowProcHandler(hwnd, wParam, lParam);

  const struct TDPISettings dpiSettings = DPISettings_FromData(BeginDeferWindowPos(10), LOWORD(wParam), HIWORD(wParam));
  EnumChildWindows(hwnd, n2e_EnumChildProc_DPIApply, (LPARAM)&dpiSettings);
  EndDeferWindowPos((HDWP)dpiSettings.hData);

  InvalidateRect(hwnd, NULL, TRUE);
  UpdateWindow(hwnd);

  return 0;
}
