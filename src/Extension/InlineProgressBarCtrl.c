#include "InlineProgressBarCtrl.h"

#define PROPERTY_PANE_ID  L"Pane Id"

void ModifyStyle(HWND hwnd, DWORD dwStyleRemove, DWORD dwStyleAdd)
{
  if (!IsWindow(hwnd))
    return;
  const DWORD dwStyleOriginal = GetWindowLong(hwnd, GWL_STYLE);
  DWORD dwStyleResult = dwStyleOriginal & ~dwStyleRemove;
  dwStyleResult |= dwStyleAdd;
  SetWindowLong(hwnd, GWL_STYLE, dwStyleResult);
}

RECT GetStatusBarPaneRect(const HWND hwndStatusBar, const int nPane)
{
  RECT rcPane;
  SendMessage(hwndStatusBar, SB_GETRECT, nPane, (LPARAM)&rcPane);
  return rcPane;
}

HWND InlineProgressBarCtrl_Create(HWND hwndStatusBar, const long nCurrentValue, const long MaxValue, const BOOL bSmooth, const int nPane)
{
	BOOL bSuccess = FALSE;

 	if (!hwndStatusBar)
 		return FALSE;

	DWORD dwStyle = WS_CHILD;
	if (bSmooth)
		dwStyle |= PBS_SMOOTH;

 	const RECT rcPane = GetStatusBarPaneRect(hwndStatusBar, nPane);

	// Create the progress bar
	HWND hwnd = CreateWindow(PROGRESS_CLASS,
                          NULL,
                          dwStyle,
                          rcPane.left,
                          rcPane.top,
                          rcPane.right-rcPane.left,
                          rcPane.bottom - rcPane.top,
                          hwndStatusBar,
                          NULL,
                          GetModuleHandle(NULL),
                          NULL);
	if (!hwnd)
		return FALSE;

  SetProp(hwnd, PROPERTY_PANE_ID, (HANDLE)nPane);

	// Set range and step
  InlineProgressBarCtrl_SetRange(hwnd, 0, MaxValue, 1);
  InlineProgressBarCtrl_SetStep(hwnd, 1);
  InlineProgressBarCtrl_StepIt(hwnd);
  InlineProgressBarCtrl_Resize(hwnd);

	return hwnd;
}

BOOL InlineProgressBarCtrl_SetRange(HWND hwnd, const long nLower, const long nUpper, const long nStep)
{	 
	if (!IsWindow(hwnd))
		return FALSE;

	SendMessage(hwnd, PBM_SETRANGE32, (WPARAM) nLower, (LPARAM) nUpper);
  InlineProgressBarCtrl_SetStep(hwnd, nStep);

	return TRUE;
}

int InlineProgressBarCtrl_SetStep(HWND hwnd, const long nStep)
{ 
	if (!IsWindow(hwnd))
		return 0;

  if (!IsWindowVisible(hwnd))
  {
    ModifyStyle(hwnd, 0, WS_VISIBLE);
  }
  return (int)SendMessage(hwnd, PBM_SETSTEP, nStep, 0);
}

void InlineProgressBarCtrl_StepIt(HWND hwnd)
{
  if (!IsWindow(hwnd))
    return;
  
  SendMessage(hwnd, PBM_STEPIT, 0, 0);
}

void InlineProgressBarCtrl_SetPos(HWND hwnd, const long nValue)
{
  if (!IsWindow(hwnd))
    return;

  SendMessage(hwnd, PBM_SETPOS, nValue, 0);
}

extern WCHAR tchProgressBarTaskName[MAX_PATH];

BOOL InlineProgressBarCtrl_Resize(HWND hwnd)
{
	if (!IsWindow(hwnd))
		return FALSE;

  HWND hwndStatusBar = GetParent(hwnd);
	if (!hwndStatusBar)
		return FALSE;

  const int nPane = (int)GetProp(hwnd, PROPERTY_PANE_ID);
  LPWSTR pPaneText = tchProgressBarTaskName;

	// Calculate text width
  int nTextMargin = 0;
  SIZE szText = { 0, 0 };
  if (tchProgressBarTaskName)
  {
    HDC hdc = GetDC(hwnd);
    HFONT hFont = (HFONT)SendMessage(hwndStatusBar, WM_GETFONT, 0, 0);
    HGDIOBJ hOldFont = SelectObject(hdc, hFont);
    GetTextExtentPoint32(hdc, pPaneText, wcslen(pPaneText), &szText);
    SIZE szMargin = { 0, 0 };
    GetTextExtentPoint32(hdc, L" ", 1, &szMargin);
    nTextMargin = szMargin.cx * 2;
    SelectObject(hdc, hOldFont);
    ReleaseDC(hwnd, hdc);
  }

	// Now calculate the rectangle in which we will draw the progress bar
  RECT rcPane;
  rcPane = GetStatusBarPaneRect(hwndStatusBar, nPane);

  if (pPaneText)
  {
    rcPane.left += (szText.cx + 2 * nTextMargin);
  }

  if (rcPane.right < rcPane.left)
  {
    rcPane.right = rcPane.left;
  }
	
	// Leave a litle vertical margin (10%) between the top and bottom of the bar
	int Height = rcPane.bottom - rcPane.top;
	rcPane.bottom -= Height/10;
	rcPane.top	  += Height/10;

	// If the window size has changed, resize the window
  RECT rcCurrent;
  GetWindowRect(hwnd, &rcCurrent);
	if ((rcPane.left != rcCurrent.left)
      || (rcPane.right != rcCurrent.right)
      || (rcPane.top != rcCurrent.top)
      || (rcPane.bottom != rcCurrent.bottom))
	{
		MoveWindow(hwnd, rcPane.left, rcPane.top, rcPane.right-rcPane.left, rcPane.bottom - rcPane.top, TRUE);
	}

	return TRUE;
}
