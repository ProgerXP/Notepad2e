#pragma once
#include "Utils.h"

typedef void (*ViewCommandHandler)(HWND hwnd);

HWND n2e_EditCreate(HWND hwndParent, HWND* phwndEditParent);
int n2e_ScintillaWindowsCount();
HWND n2e_ScintillaWindowByIndex(const int index);

void n2e_SwitchView(const ESplitViewMode modeNew);
void n2e_UpdateView();
void n2e_UpdateViewsDPI(const WPARAM dpi);
void n2e_UpdateLexer(HWND hwnd);
void n2e_ApplyViewCommand(ViewCommandHandler lpHandler);

HWND n2e_GetActiveEdit();

#ifndef N2E_TESTING
#define hwndEdit n2e_GetActiveEdit()
#else
#define hwndEdit NULL
#endif
