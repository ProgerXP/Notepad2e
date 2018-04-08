#pragma once
#include <wtypes.h>

#define WM_DPICHANGED 0x02E0
#define DEFAULT_SCREEN_DPI 96
#define DEFAULT_FONT_DPI 72

BOOL n2e_DPIInitialize();
BOOL n2e_EnableNonClientDpiScaling(HWND hwnd);
RECT n2e_DPIAdjustRect(RECT rc, const int dpiInitialX, const int dpiInitialY, const int dpiX, const int dpiY);

void n2e_ScintillaDPIInit(HWND hwnd);
void n2e_ScintillaDPIUpdate(HWND hwnd, const WPARAM dpi);
void n2e_DialogDPIInit(HWND hwnd);

DWORD n2e_GetDPIFromMonitor(HMONITOR hMonitor, HWND hwnd);

LRESULT n2e_DPIChanged_WindowProcHandler(const HWND hwnd, const WPARAM wParam, const LPARAM lParam);
LRESULT n2e_DPIChanged_DlgProcHandler(HWND hwnd, WPARAM wParam, LPARAM lParam);

#define DPI_INIT()  n2e_DialogDPIInit(hwnd);

#define DPI_CHANGED_HANDLER() \
  case WM_DPICHANGED: \
    return n2e_DPIChanged_DlgProcHandler(hwnd, wParam, lParam);
