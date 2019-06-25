#pragma once
#include "stdafx.h"

BOOL n2e_IsSubclassedWindow(const HWND hwnd);
BOOL n2e_SubclassWindow(const HWND hwnd, const WNDPROC lpWndProc);
LRESULT n2e_CallOriginalWindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam);
