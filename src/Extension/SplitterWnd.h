#pragma once
#include <windows.h>
#include <winuser.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "ViewHelper.h"

  extern UINT WM_SPLITTER_CHILDREN_COUNT;
  extern UINT WM_SPLITTER_CHILD_BY_INDEX;

  BOOL IsSplitterWnd(const HWND hwnd);
  HWND GetTopSplitterWnd(const HWND hwnd);
  HWND CreateSplitterWnd(const HWND hwndParent, const HWND hwndChild1, const HWND hwndChild2, const BOOL bHorizontal);
  HWND AddSplitterChild(HWND hwndParent, const HWND hwndChildActive, const HWND hwndChild, const BOOL bHorizontal);
  void DeleteSplitterChild(HWND hwndChild, HWND hwndParentForLast, HWND* hwndEditParent);
#ifdef __cplusplus
}//end extern "C"
#endif
