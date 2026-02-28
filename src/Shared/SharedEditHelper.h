#pragma once

#include "Extension/Externals.h"

#ifdef __cplusplus
#define C_PREFIX extern "C"
#else
#define C_PREFIX
#endif

C_PREFIX void EditSelectEx(HWND hwnd, int iAnchorPos, int iCurrentPos);
C_PREFIX BOOL n2e_IsRectangularSelection();
C_PREFIX BOOL n2e_ShowPromptIfSelectionModeIsRectangle(const HWND hwnd);
C_PREFIX BOOL isEndedWithEOL(char *psz);
C_PREFIX BOOL EditReplaceAllInSelection(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bShowInfo);
C_PREFIX BOOL EditReplaceAll(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bShowInfo);
