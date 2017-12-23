#pragma once
#include <wtypes.h>

extern WCHAR tchProgressBarTaskName[MAX_PATH];

HWND InlineProgressBarCtrl_Create(const HWND hwndParent, const long nCurrentValue, const long MaxValue, const BOOL bSmooth, const int nPane);
BOOL InlineProgressBarCtrl_SetRange(const HWND hwnd, const long nLower, const long nUpper, const long nStep);
int InlineProgressBarCtrl_SetStep(const HWND hwnd, const long nStep);
void InlineProgressBarCtrl_StepIt(const HWND hwnd);
void InlineProgressBarCtrl_SetPos(const HWND hwnd, const long nValue);
BOOL InlineProgressBarCtrl_Resize(const HWND hwnd);
