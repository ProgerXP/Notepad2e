#include "MainWndHelper.h"
#include "Notepad2.h"
#include "Utils.h"

HHOOK hShellHook = NULL;

ExpressionValueMode modePrevExpressionValue = EVM_DEC;
char arrchPrevExpressionText[MAX_EXPRESSION_LENGTH] = { 0 };
ExpressionValueMode modeExpressionValue = EVM_DEC;
WCHAR arrwchExpressionValue[MAX_PATH] = { 0 };

extern HWND hwndMain;
extern int aWidth[6];

BOOL n2e_IsPaneSizePoint(const HWND hwnd, POINT pt)
{
  ScreenToClient(hwnd, &pt);
  return (pt.x > aWidth[0]) && (pt.x < aWidth[1]);
}

void n2e_OnPaneSizeClick(const HWND hwnd, const BOOL bLeftClick)
{
  if (bLeftClick)
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
    if (wcslen(arrwchExpressionValue) > 0)
    {
      n2e_SetClipboardText(hwnd, arrwchExpressionValue);
    }
  }
}

LRESULT CALLBACK n2e_ShellProc(int nCode, WPARAM wParam, LPARAM lParam)
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
