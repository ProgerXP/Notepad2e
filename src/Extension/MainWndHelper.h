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
} ExpressionValueMode;

extern HHOOK hShellHook;
extern UINT_PTR timerIDPaneSizeClick;
extern ExpressionValueMode modePrevExpressionValue;
extern char arrchPrevExpressionText[MAX_EXPRESSION_LENGTH];
extern ExpressionValueMode modeExpressionValue;
extern WCHAR arrwchExpressionValue[MAX_PATH];

LRESULT CALLBACK ShellProc(int nCode, WPARAM wParam, LPARAM lParam);
void OnPaneSizeClick(const HWND hwnd, const BOOL singleClick, const BOOL runHandler);
