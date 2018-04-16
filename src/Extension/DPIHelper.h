#pragma once
#include <wtypes.h>

#define WM_DPICHANGED 0x02E0
#define DEFAULT_SCREEN_DPI 96
#define DEFAULT_FONT_DPI 72

#define PROPERTY_WINDOW_DPI_INITIALIZED L"WindowDPIInitialized"
#define PROPERTY_WINDOW_SKIP_DPI_RESIZE L"WindowSkipDPIResize"
#define PROPERTY_WINDOW_FONT_SIZE L"WindowFontSize"
#define PROPERTY_WINDOW_DYNAMIC_FONT L"WindowDynamicFont"
#define PROPERTY_WINDOW_DPI L"WindowDPI"
#define PROPERTY_WINDOW_POSITION L"WindowPosition"
#define PROPERTY_WINDOW_SIZE L"WindowSize"

#define DeclareGetWindowProp(prop, name, ret)             \
__forceinline ret n2e_GetWindow##prop(const HWND hwnd) {  \
  return((ret)GetProp(hwnd, name));                       \
}

#define DeclareSetWindowProp(prop, name, type)                              \
__forceinline void n2e_SetWindow##prop(const HWND hwnd, const type value) { \
  SetProp(hwnd, name, (HANDLE)value);                                       \
}

#define DeclareSetWindowProp2(prop, name, type1, type2)    \
__forceinline void n2e_SetWindow##prop(const HWND hwnd, const type1 value1, const type2 value2) {  \
  SetProp(hwnd, name, (HANDLE)MAKEWPARAM(value1, value2));               \
}

DeclareGetWindowProp(DPIInitialized, PROPERTY_WINDOW_DPI_INITIALIZED, BOOL);
DeclareSetWindowProp(DPIInitialized, PROPERTY_WINDOW_DPI_INITIALIZED, BOOL);
DeclareGetWindowProp(SkipDPIResize, PROPERTY_WINDOW_SKIP_DPI_RESIZE, BOOL);
DeclareSetWindowProp(SkipDPIResize, PROPERTY_WINDOW_SKIP_DPI_RESIZE, BOOL);
DeclareGetWindowProp(FontSize, PROPERTY_WINDOW_FONT_SIZE, int);
DeclareSetWindowProp(FontSize, PROPERTY_WINDOW_FONT_SIZE, int);
DeclareGetWindowProp(DynamicFont, PROPERTY_WINDOW_DYNAMIC_FONT, HANDLE);
DeclareSetWindowProp(DynamicFont, PROPERTY_WINDOW_DYNAMIC_FONT, HANDLE);
DeclareGetWindowProp(DPI, PROPERTY_WINDOW_DPI, DWORD);
DeclareSetWindowProp2(DPI, PROPERTY_WINDOW_DPI, int, int);
DeclareGetWindowProp(Position, PROPERTY_WINDOW_POSITION, DWORD);
DeclareSetWindowProp2(Position, PROPERTY_WINDOW_POSITION, int, int);
DeclareGetWindowProp(Size, PROPERTY_WINDOW_SIZE, DWORD);
DeclareSetWindowProp2(Size, PROPERTY_WINDOW_SIZE, int, int);


BOOL n2e_DPIInitialize();
BOOL n2e_EnableNonClientDpiScaling(const HWND hwnd);
RECT n2e_DPIAdjustRect(RECT rc, const int dpiXInitial, const int dpiYInitial, const int dpiX, const int dpiY);

void n2e_ScintillaDPIInit(const HWND hwnd);
void n2e_ScintillaDPIUpdate(const HWND hwnd, const WPARAM dpi);
void n2e_DialogDPIInit(const HWND hwnd);
void n2e_DialogDPIUpdate(const HWND hwnd, const BOOL bDPIFromHDC);
void n2e_DialogDPIGetMinMaxInfo(const HWND hwnd, LPARAM lParam);

DWORD n2e_GetDPIFromMonitor(const HMONITOR hMonitor, const HWND hwnd);

LRESULT n2e_DPIChanged_WindowProcHandler(const HWND hwnd, const WPARAM wParam, const LPARAM lParam);
LRESULT n2e_DPIChanged_DlgProcHandler(const HWND hwnd, const WPARAM wParam, const LPARAM lParam);

#define N2E_DPI_INIT() n2e_DialogDPIInit(hwnd);
#define N2E_DPI_RESIZE() n2e_DialogDPIUpdate(hwnd, FALSE);
#define N2E_DPI_GETMINMAXINFO() n2e_DialogDPIGetMinMaxInfo(hwnd, lParam);

#define N2E_DPI_CHANGED_HANDLER() \
  case WM_DPICHANGED: \
    return n2e_DPIChanged_DlgProcHandler(hwnd, wParam, lParam);
