#pragma once
#include <wtypes.h>

extern WCHAR tchProgressBarTaskName[MAX_PATH];

HWND InlineProgressBarCtrl_Create(HWND hwndParent, const long nCurrentValue, const long MaxValue, const BOOL bSmooth, const int nPane);
BOOL InlineProgressBarCtrl_SetRange(HWND hwnd, const long nLower, const long nUpper, const long nStep);
int InlineProgressBarCtrl_SetStep(HWND hwnd, const long nStep);
void InlineProgressBarCtrl_StepIt(HWND hwnd);
void InlineProgressBarCtrl_SetPos(HWND hwnd, const long nValue);
BOOL InlineProgressBarCtrl_Resize(HWND hwnd);
