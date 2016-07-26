#pragma once
#include <windows.h>
#include <commctrl.h>

HWND InlineProgressBarCtrl_Create(HWND hwndParent, const int nCurrentValue, const int MaxValue, const BOOL bSmooth, const int nPane);
BOOL InlineProgressBarCtrl_SetRange(HWND hwnd, const int nLower, const int nUpper, const int nStep);
int InlineProgressBarCtrl_SetStep(HWND hwnd, const int nStep);
void InlineProgressBarCtrl_StepIt(HWND hwnd);
void InlineProgressBarCtrl_SetPos(HWND hwnd, const int nValue);
BOOL InlineProgressBarCtrl_Resize(HWND hwnd);
