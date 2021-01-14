#include <Windows.h>
#include <windowsx.h>
#include <assert.h>
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

extern HWND hwndMain;
extern HWND _hwndEdit;
extern HWND hwndEditParent;
extern FILEVARS fvCurFile;

HWND EditCreate(HWND hwndParent);

HWND n2e_EditCreateImpl(HWND hwndEditActive, HWND hwndParent, const BOOL bHorizontally, HWND* phwndEditParent)
{
  if (!hwndEditActive)
  {
    *phwndEditParent = EditCreate(hwndParent);
    return *phwndEditParent;
  }
  else
  {
    HWND hwnd1 = hwndEditActive;
    HWND hwnd2 = EditCreate(hwndMain);
    *phwndEditParent = IsSplitterWnd(hwndParent)
      ? AddSplitterChild(hwndParent, hwnd1, hwnd2, bHorizontally)
      : CreateSplitterWnd(hwndParent, hwnd1, hwnd2, bHorizontally);
    return hwnd1;
  }
}

HWND n2e_EditCreate(HWND hwndParent, HWND* phwndEditParent)
{
  return n2e_EditCreateImpl(NULL, hwndParent, TRUE, phwndEditParent);
}

int n2e_ScintillaWindowsCount()
{
  const HWND hwndParent = GetTopSplitterWnd(_hwndEdit);
  return (hwndParent != _hwndEdit)
    ? SendMessage(hwndParent, WM_SPLITTER_CHILDREN_COUNT, 0, 0)
    : 1;
}

HWND n2e_ScintillaWindowByIndex(const int index)
{
  const HWND hwndParent = GetTopSplitterWnd(_hwndEdit);
  return (hwndParent != _hwndEdit)
    ? (HWND)SendMessage(hwndParent, WM_SPLITTER_CHILD_BY_INDEX, (WPARAM)index, 0)
    : _hwndEdit;
}

void n2e_UpdateMainWindow()
{
  extern HWND hwndMain;
  void MsgSize(HWND, WPARAM, LPARAM);
  
  RECT rc = { 0 };
  GetClientRect(hwndMain, &rc);
  MsgSize(hwndMain, SIZE_RESTORED, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
}

void n2e_SplitView(const BOOL bHorizontally)
{
  HWND hwndEditActive = hwndEdit;
  hwndEditParent = GetParent(hwndEditActive);
  n2e_EditCreateImpl(hwndEditActive, hwndEditParent, bHorizontally, &hwndEditParent);
  SetWindowPos(hwndEditParent, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);

  n2e_UpdateMainWindow();
  n2e_UpdateView();
}

void n2e_UpdateView()
{
  LRESULT pDoc = (LRESULT)SendMessage(hwndEdit, SCI_GETDOCPOINTER, 0, 0);
  for (int i = 1; i < n2e_ScintillaWindowsCount(); ++i)
  {
    const HWND hwnd = n2e_ScintillaWindowByIndex(i);
    assert(hwnd != _hwndEdit);
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

void n2e_CloseView()
{
  const HWND hwnd = hwndEdit;
  if (hwnd != _hwndEdit)
  {
    SendMessage(hwnd, SCI_SETDOCPOINTER, 0, 0);
    DeleteSplitterChild(hwnd, _hwndEdit, &hwndEditParent);
    if (hwndEditParent == _hwndEdit)
    {
      n2e_UpdateMainWindow();
    }
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
