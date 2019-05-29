#pragma once
#include <wtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

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
__forceinline ret GetWindow##prop(const HWND hwnd) {  \
  return((ret)GetProp(hwnd, name));                       \
}

#define DeclareSetWindowProp(prop, name, type)                              \
__forceinline void SetWindow##prop(const HWND hwnd, const type value) { \
  SetProp(hwnd, name, (HANDLE)value);                                       \
}

#define DeclareSetWindowProp2(prop, name, type1, type2)    \
__forceinline void SetWindow##prop(const HWND hwnd, const type1 value1, const type2 value2) {  \
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


BOOL DPIInitialize();
BOOL DPIEnableNonClientDpiScaling(const HWND hwnd);
RECT DPIAdjustRect(RECT rc, const int dpiXInitial, const int dpiYInitial, const int dpiX, const int dpiY);

void DialogDPIInit(const HWND hwnd);
void DialogDPIUpdate(const HWND hwnd, const BOOL bDPIFromHDC);
void DialogDPIGetMinMaxInfo(const HWND hwnd, LPARAM lParam);

DWORD GetDPIFromMonitor(const HMONITOR hMonitor, const HWND hwnd);

LRESULT DPIChanged_WindowProcHandler(const HWND hwnd, const WPARAM wParam, const LPARAM lParam);
LRESULT DPIChanged_DlgProcHandler(const HWND hwnd, const WPARAM wParam, const LPARAM lParam);

#define DPI_INIT() DialogDPIInit(hwnd);
#define DPI_ENABLE_NC_SCALING() DPIEnableNonClientDpiScaling(hwnd);
#define DPI_RESIZE() DialogDPIUpdate(hwnd, FALSE);
#define DPI_GETMINMAXINFO() DialogDPIGetMinMaxInfo(hwnd, lParam);

#define DPI_CHANGED_HANDLER() \
  case WM_DPICHANGED: \
    return DPIChanged_DlgProcHandler(hwnd, wParam, lParam);

#ifdef __cplusplus
}//end extern "C"
#endif
