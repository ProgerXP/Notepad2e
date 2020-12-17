#include <Windows.h>
#include <windowsx.h>
#include "Edit.h"
#include "SplitterWnd.h"
#include "Scintilla.h"
#include "SciCall.h"
#include "DPIHelperScintilla.h"
#include "ExtSelection.h"
#include "Helpers.h"
#include "Styles.h"
#include "Utils.h"
#include "ViewHelper.h"

extern ESplitViewMode iSplitViewMode;
extern HWND _hwndEdit;
extern HWND hwndEditParent;
extern FILEVARS fvCurFile;

HWND EditCreate(HWND hwndParent);

HWND n2e_EditCreateImpl(HWND hwndParent, HWND* phwndEditParent, const BOOL bInitialCall)
{
  switch (iSplitViewMode)
  {
  case SVM_DISABLED:
  default:
    {
      *phwndEditParent = bInitialCall ? EditCreate(hwndParent) : hwndEditParent;
      return *phwndEditParent;
    }
  case SVM_SIDE_BY_SIDE:
    {
      HWND hwnd1 = bInitialCall ? EditCreate(hwndParent) : _hwndEdit;
      HWND hwnd2 = EditCreate(hwndParent);
      *phwndEditParent = CreateSplitterWnd(hwndParent, hwnd1, hwnd2, TRUE);
      return hwnd1;
    }
  case SVM_3_IN_1:
    {
      HWND hwnd1 = bInitialCall ? EditCreate(hwndParent) : _hwndEdit;
      HWND hwnd2 = EditCreate(hwndParent);
      HWND hwnd3 = EditCreate(hwndParent);
      HWND hwndSplitter2 = CreateSplitterWnd(hwndParent, hwnd2, hwnd3, TRUE);
      *phwndEditParent = CreateSplitterWnd(hwndParent, hwnd1, hwndSplitter2, FALSE);
      return hwnd1;
    }
  }
}

HWND n2e_EditCreate(HWND hwndParent, HWND* phwndEditParent)
{
  return n2e_EditCreateImpl(hwndParent, phwndEditParent, TRUE);
}

int n2e_ScintillaWindowsCount()
{
  switch (iSplitViewMode)
  {
  case SVM_DISABLED:
  default:
    return 1;
  case SVM_SIDE_BY_SIDE:
    return 2;
  case SVM_3_IN_1:
    return 3;
  }
}

HWND n2e_ScintillaWindowByIndex(const int index)
{
  switch (iSplitViewMode)
  {
  case SVM_DISABLED:
  default:
    return _hwndEdit;
  case SVM_SIDE_BY_SIDE:
    {
      HWND hwnd1 = GetFirstChild(hwndEditParent);
      if (index == 0)
        return hwnd1;
      return GetNextSibling(hwnd1);
    }
  case SVM_3_IN_1:
  {
    HWND hwnd1 = GetFirstChild(hwndEditParent);
    if (index == 0)
      return hwnd1;
    HWND hwndSplitter = GetNextSibling(hwnd1);
    HWND hwnd2 = GetFirstChild(hwndSplitter);
    if (index == 1)
      return hwnd2;
    return GetNextSibling(hwnd2);
  }
  }
}

void n2e_SwitchView(const ESplitViewMode modeNew)
{
  extern HWND hwndMain;
  void MsgSize(HWND, WPARAM, LPARAM);

  SetParent(_hwndEdit, hwndMain);
  if (iSplitViewMode != SVM_DISABLED)
  {
    // destroy top-level splitter
    DestroyWindow(hwndEditParent);
  }

  iSplitViewMode = modeNew;
  hwndEditParent = _hwndEdit;
  _hwndEdit = n2e_EditCreateImpl(hwndMain, &hwndEditParent, FALSE);
  SetWindowPos(hwndEditParent, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);

  RECT rc = { 0 };
  GetClientRect(hwndMain, &rc);
  MsgSize(hwndMain, SIZE_RESTORED, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));

  n2e_UpdateView();
}

void n2e_UpdateView()
{
  LRESULT pDoc = (LRESULT)SendMessage(hwndEdit, SCI_GETDOCPOINTER, 0, 0);
  for (int i = 1; i < n2e_ScintillaWindowsCount(); ++i)
  {
    const HWND hwnd = n2e_ScintillaWindowByIndex(i);
    SendMessage(hwnd, SCI_SETDOCPOINTER, 0, pDoc);
    n2e_UpdateLexer(hwnd);
    InitScintillaHandle(hwnd);
    n2e_InitScintilla(hwnd);
    n2e_EditInit(hwnd);
    n2e_ScintillaDPIInit(hwnd);
    UpdateLineNumberWidth(hwnd);
    
    FileVars_Apply(hwnd, &fvCurFile);
  }
}

void n2e_UpdateViewsDPI(const WPARAM dpi)
{
  for (int i = 0; i < n2e_ScintillaWindowsCount(); ++i)
  {
    const HWND hwnd = n2e_ScintillaWindowByIndex(i);
    n2e_ScintillaDPIUpdate(hwnd, dpi);
  }
}

PEDITLEXER pPreviousLexer = NULL;
void _Style_SetLexer(HWND, PEDITLEXER);

void n2e_UpdateLexer(HWND hwnd)
{
  _Style_SetLexer(hwnd, pPreviousLexer);
}

void Style_SetLexer(HWND hwnd, PEDITLEXER pLexNew)
{
  pPreviousLexer = pLexNew;
  for (int i = 0; i < n2e_ScintillaWindowsCount(); ++i)
  {
    _Style_SetLexer(n2e_ScintillaWindowByIndex(i), pLexNew);
  }
}

void n2e_ApplyViewCommand(ViewCommandHandler lpHandler)
{
  for (int i = 0; i < n2e_ScintillaWindowsCount(); ++i)
  {
    lpHandler(n2e_ScintillaWindowByIndex(i));
  }
}

HWND n2e_GetActiveEdit()
{
  const HWND hwndFocus = GetFocus();
  WCHAR szClassName[64] = { 0 };
  return  (GetClassName(hwndFocus, szClassName, COUNTOF(szClassName)) && (lstrcmpi(szClassName, L"Scintilla") == 0))
    ? hwndFocus
    : _hwndEdit;
}
