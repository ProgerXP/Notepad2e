#include "MainWndHelper.h"
#include "Notepad2.h"
#include "Utils.h"

HHOOK hShellHook = NULL;

ExpressionValueMode modePrevExpressionValue = EVM_DEC;
char arrchPrevExpressionText[MAX_EXPRESSION_LENGTH] = { 0 };
ExpressionValueMode modeExpressionValue = EVM_DEC;
WCHAR arrwchExpressionValue[MAX_PATH] = { 0 };

#define       STATUS_PANE_SIZE_CLICK_TIMER  0x1000
#define       STATUS_PANE_SIZE_DBLCLICK_TIMER 0x1001
UINT_PTR      timerIDPaneSizeClick = 0;
UINT_PTR      timerIDPaneSizeDblClick = 0;

extern HWND  hwndMain;

void OnPaneSizeClick(const HWND hwnd, const BOOL singleClick, const BOOL runHandler)
{
  if (timerIDPaneSizeClick > 0)
  {
    KillTimer(hwnd, STATUS_PANE_SIZE_CLICK_TIMER);
    timerIDPaneSizeClick = 0;
  }
  if (timerIDPaneSizeDblClick > 0)
  {
    KillTimer(hwnd, STATUS_PANE_SIZE_DBLCLICK_TIMER);
    timerIDPaneSizeDblClick = 0;
    if (singleClick)
    {
      return;
    }
  }

  if (singleClick)
  {
    if (runHandler)
    {
      ++modeExpressionValue;
      if (modeExpressionValue > EVM_MAX)
      {
        modeExpressionValue = EVM_MIN;
      }
      UpdateStatusbar();
    }
    else
    {
      // wait for possible double click
      timerIDPaneSizeClick = SetTimer(hwnd, STATUS_PANE_SIZE_CLICK_TIMER, GetDoubleClickTime() / 2, NULL);
    }
  }
  else
  {
    if (wcslen(arrwchExpressionValue) > 0)
    {
      SetClipboardText(hwnd, arrwchExpressionValue);
    }
    // skip useless single click
    timerIDPaneSizeDblClick = SetTimer(hwnd, STATUS_PANE_SIZE_DBLCLICK_TIMER, 100, NULL);
  }
}

LRESULT CALLBACK ShellProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode < 0)
  {
    return CallNextHookEx(hShellHook, nCode, wParam, lParam);
  }
  if (nCode == HSHELL_LANGUAGE)
  {
    PostMessage(hwndMain, WM_INPUTLANGCHANGE, 0, 0);
  }
  return 0;
}

