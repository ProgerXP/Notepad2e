#pragma once
#include "Utils.h"

typedef void (*ViewCommandHandler)(HWND hwnd);

#define VIEW_COMMAND(c) n2e_ApplyViewCommand(c)

HWND n2e_EditCreate(HWND hwndParent, HWND* phwndEditParent);
int n2e_ScintillaWindowsCount();
HWND n2e_ScintillaWindowByIndex(const int index);

void n2e_SplitView(const BOOL bHorizontally);
void n2e_UpdateViews();
void n2e_UpdateViewsDPI(const WPARAM dpi);
void n2e_CloseView();
void n2e_UpdateLexer(HWND hwnd);
void n2e_ApplyViewCommand(ViewCommandHandler lpHandler);

BOOL n2e_IsScintillaWindow(const HWND hwnd);
HWND n2e_GetActiveEditCheckFocus();
HWND n2e_GetActiveEdit();
void n2e_SetActiveEdit(const HWND hwnd);
void n2e_SaveActiveEdit();
void n2e_RestoreActiveEdit();

#ifndef N2E_TESTING
#define hwndEdit n2e_GetActiveEdit()
#else
#define hwndEdit NULL
#endif
