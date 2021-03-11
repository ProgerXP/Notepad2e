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
#include "Notepad2.h"

extern HWND hwndMain;
extern HWND _hwndEdit;
extern HWND hwndEditParent;
extern FILEVARS fvCurFile;

HWND hwndActiveEdit = NULL;

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
    return hwnd2;
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

void n2e_UpdateView(const HWND hwnd, const LRESULT pDoc)
{
  SendMessage(hwnd, SCI_SETDOCPOINTER, 0, pDoc);
  n2e_UpdateLexer(hwnd);
  InitScintillaHandle(hwnd);
  n2e_InitScintilla(hwnd);
  EditInit(hwnd);
  n2e_EditInit(hwnd);
  n2e_ScintillaDPIInit(hwnd);
  UpdateLineNumberWidth(hwnd);

  FileVars_Apply(hwnd, &fvCurFile);
}

void n2e_UpdateViews()
{
  const LRESULT pDoc = SendMessage(hwndEdit, SCI_GETDOCPOINTER, 0, 0);
  for (int i = 1; i < n2e_ScintillaWindowsCount(); ++i)
  {
    const HWND hwnd = n2e_ScintillaWindowByIndex(i);
    assert(hwnd != _hwndEdit);
    n2e_UpdateView(hwnd, pDoc);
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

void n2e_SplitView(const BOOL bHorizontally)
{
  const HWND hwndEditActive = n2e_GetActiveEditCheckFocus();
  hwndEditParent = GetParent(hwndEditActive);
  const HWND hwndEditNew = n2e_EditCreateImpl(hwndEditActive, hwndEditParent, bHorizontally, &hwndEditParent);
  if (PrivateIsAppThemed())
  {
    SetWindowLongPtr(hwndEditNew, GWL_EXSTYLE, GetWindowLongPtr(hwndEditNew, GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(hwndEditNew, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
  }
  SetWindowPos(hwndEditParent, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);
  n2e_UpdateMainWindow();
  n2e_UpdateView(hwndEditNew, SendMessage(hwndEdit, SCI_GETDOCPOINTER, 0, 0));
  n2e_SetActiveEdit(hwndEditNew);
  SetFocus(hwndEditNew);
}

void n2e_CloseView()
{
  const HWND hwnd = n2e_GetActiveEditCheckFocus();
  if (hwnd != _hwndEdit)
  {
    const int iViewCount = n2e_ScintillaWindowsCount();
    for (int i = 0; i < iViewCount; ++i)
    {
      if (n2e_ScintillaWindowByIndex(i) == hwnd)
      {
        n2e_SetActiveEdit((i + 1 >= iViewCount) ? n2e_ScintillaWindowByIndex(i - 1) : n2e_ScintillaWindowByIndex(i + 1));
        break;
      }
    }

    SendMessage(hwnd, SCI_SETDOCPOINTER, 0, 0);
    DeleteSplitterChild(hwnd, _hwndEdit, &hwndEditParent);

    if (hwndEditParent == _hwndEdit)
    {
      n2e_UpdateMainWindow();
    }
    SetFocus(n2e_GetActiveEdit());
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

BOOL n2e_IsScintillaWindow(const HWND hwnd)
{
  WCHAR szClassName[64] = { 0 };
  GetClassName(hwnd, szClassName, COUNTOF(szClassName));
  return (lstrcmpi(szClassName, L"Scintilla") == 0);
}

HWND n2e_GetActiveEditCheckFocus()
{
  const HWND hwndFocus = GetFocus();
  return n2e_IsScintillaWindow(hwndFocus)
    ? hwndFocus
    : hwndActiveEdit
    ? hwndActiveEdit
    : _hwndEdit;
}

HWND n2e_GetActiveEdit()
{
  return hwndActiveEdit
    ? hwndActiveEdit
    : _hwndEdit;
}

void n2e_SetActiveEdit(const HWND hwnd)
{
  assert(n2e_IsScintillaWindow(hwnd));
  hwndActiveEdit = hwnd;
}

void n2e_SaveActiveEdit()
{
  hwndActiveEdit = n2e_GetActiveEditCheckFocus();
}

void n2e_RestoreActiveEdit()
{
  SetFocus(hwndActiveEdit ? hwndActiveEdit : _hwndEdit);
}
