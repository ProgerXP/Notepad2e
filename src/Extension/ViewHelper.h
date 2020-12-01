#pragma once
#include "Utils.h"

HWND n2e_EditCreate(HWND hwndParent, HWND* phwndEditParent);
int n2e_ScintillaWindowsCount();
HWND n2e_ScintillaWindowByIndex(const int index);

void n2e_SwitchView(const ESplitViewMode modeOld, const ESplitViewMode modeNew);
void n2e_UpdateView();
void n2e_UpdateLexer(HWND hwnd);
HWND n2e_GetActiveEdit();

#define hwndEdit n2e_GetActiveEdit()
