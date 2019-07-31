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

#define DeclareGetWindowProp(prop, name)                                  \
__forceinline HANDLE GetWindow##prop(const HWND hwnd) {                   \
  return GetProp(hwnd, name);                                             \
}

#define DeclareSetWindowProp(prop, name)                                  \
__forceinline void SetWindow##prop(const HWND hwnd, const HANDLE value) { \
  SetProp(hwnd, name, value);                                             \
}

#define DeclareGetWindowPropSpec(prop, name, ret)                         \
__forceinline ret GetWindow##prop(const HWND hwnd) {                      \
  return PtrToInt(GetProp(hwnd, name));                                   \
}

#define DeclareSetWindowPropSpec(prop, name, type)                        \
__forceinline void SetWindow##prop(const HWND hwnd, const type value) {   \
  SetProp(hwnd, name, IntToPtr(value));                                   \
}

#define DeclareSetWindowProp2(prop, name, type1, type2)                                        \
__forceinline void SetWindow##prop(const HWND hwnd, const type1 value1, const type2 value2) {  \
  SetProp(hwnd, name, (HANDLE)MAKEWPARAM(value1, value2));                                     \
}

DeclareGetWindowPropSpec(DPIInitialized, PROPERTY_WINDOW_DPI_INITIALIZED, BOOL);
DeclareSetWindowPropSpec(DPIInitialized, PROPERTY_WINDOW_DPI_INITIALIZED, BOOL);
DeclareGetWindowPropSpec(SkipDPIResize, PROPERTY_WINDOW_SKIP_DPI_RESIZE, BOOL);
DeclareSetWindowPropSpec(SkipDPIResize, PROPERTY_WINDOW_SKIP_DPI_RESIZE, BOOL);
DeclareGetWindowPropSpec(FontSize, PROPERTY_WINDOW_FONT_SIZE, int);
DeclareSetWindowPropSpec(FontSize, PROPERTY_WINDOW_FONT_SIZE, int);
DeclareGetWindowProp(DynamicFont, PROPERTY_WINDOW_DYNAMIC_FONT);
DeclareSetWindowProp(DynamicFont, PROPERTY_WINDOW_DYNAMIC_FONT);
DeclareGetWindowPropSpec(DPI, PROPERTY_WINDOW_DPI, DWORD);
DeclareSetWindowProp2(DPI, PROPERTY_WINDOW_DPI, int, int);
DeclareGetWindowPropSpec(Position, PROPERTY_WINDOW_POSITION, DWORD);
DeclareSetWindowProp2(Position, PROPERTY_WINDOW_POSITION, int, int);
DeclareGetWindowPropSpec(Size, PROPERTY_WINDOW_SIZE, DWORD);
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
