#pragma once

typedef enum
{
  SM_ALL,
  SM_INVERSED_ALL,
  SM_LINE
} ESelectionMode;

typedef enum
{
  SUM_INIT,
  SUM_UPDATE,
  SUM_MODIF
} ESelectionUpdateMode;

typedef enum
{
  SES_APPLY = 1 << 0,
  SES_REJECT = 1 << 1
} ESelectionEditStopMode;

void n2e_EditInit(const HWND hwnd);
void n2e_SelectionRelease();
UINT n2e_SelectionGetSciEventMask(const BOOL range_not);
void n2e_SelectionNotificationHandler(const HWND hwnd, const int code, const struct SCNotification *scn);
void n2e_SelectionUpdate(const ESelectionUpdateMode place);
BOOL n2e_IsHighlightSelectionEnabled();
BOOL n2e_IsSelectionEditModeOn();
BOOL n2e_IsPageWiseSelectionEditMode();
void n2e_SelectionEditStart(const ESelectionMode mode);
BOOL n2e_SelectionEditStop(const HWND hwnd, const ESelectionEditStopMode mode);
void n2e_SelectionEditHideToolTip();
void n2e_OnMouseVanishEvent(const BOOL showCursor);

#define SET_CURSOR_HANDLER()  case WM_SETCURSOR: n2e_OnMouseVanishEvent(TRUE); return DefWindowProc(hwnd, WM_SETCURSOR, wParam, lParam);
