#pragma once
#include "stdafx.h"

#define MAX_EXPRESSION_LENGTH 4096

typedef enum
{
  EVM_DEC,
  EVM_HEX,
  EVM_BIN,
  EVM_OCT,
  EVM_MAX = EVM_OCT,
  EVM_MIN = EVM_DEC
} EExpressionValueMode;

extern HHOOK hShellHook;
extern EExpressionValueMode modePrevExpressionValue;
extern char arrchExpressionText[MAX_EXPRESSION_LENGTH];
extern EExpressionValueMode modeExpressionValue;
extern WCHAR arrwchExpressionValue[MAX_PATH];
extern int flagPasteBoard;
extern BOOL bLastCopyFromMe;

BOOL n2e_ScreenToClientRect(const HWND hwnd, LPRECT pRect);
BOOL n2e_IsPaneSizePoint(const HWND hwnd, POINT pt);
void n2e_OnPaneSizeClick(const HWND hwnd, const BOOL bLeftClick);
LRESULT CALLBACK n2e_ShellProc(int nCode, WPARAM wParam, LPARAM lParam);
BOOL n2e_FormatEvaluatedExpression(const HWND hwnd, char* expressionText, const int expressionTextLength, WCHAR* expressionValue, const int expressionValueLength, const BOOL bApplyLocaleForDecimalResult);
BOOL n2e_IsModalDialogOnTop();
void n2e_OnActivateMainWindow(const WPARAM wParam, const LPARAM lParam);
void n2e_SetUACIcon(const HMENU hMenu, const UINT nItem);
