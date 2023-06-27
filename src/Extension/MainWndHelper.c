#include <math.h>

#include "MainWndHelper.h"
#include "CommonUtils.h"
#include "EditHelper.h"
#include "EditHelperEx.h"
#include "Notepad2.h"
#include "resource.h"
#include "Scintilla.h"
#include "SciCall.h"
#include "Shell32Helper.h"
#include "Helpers.h"
#include "tinyexpr/tinyexpr.h"
#include "Utils.h"
#include "VersionHelper.h"

HHOOK hShellHook = NULL;

EExpressionValueMode modePrevExpressionValue = EVM_DEC;
char arrchExpressionText[MAX_EXPRESSION_LENGTH] = { 0 };
EExpressionValueMode modeExpressionValue = EVM_DEC;
WCHAR arrwchExpressionValue[MAX_PATH] = { 0 };

extern HWND hwndMain;
extern int aWidth[6];

BOOL n2e_ScreenToClientRect(const HWND hwnd, LPRECT pRect)
{
  if (!pRect)
  {
    return FALSE;
  }
  POINT ptLeftTop = { pRect->left, pRect->top };
  POINT ptRightBottom = { pRect->right, pRect->bottom };
  ScreenToClient(hwnd, &ptLeftTop);
  ScreenToClient(hwnd, &ptRightBottom);
  pRect->left = ptLeftTop.x;
  pRect->top = ptLeftTop.y;
  pRect->right = ptRightBottom.x;
  pRect->bottom = ptRightBottom.y;
  return TRUE;
}

BOOL n2e_IsPaneSizePoint(const HWND hwnd, POINT pt)
{
  RECT rectStatus;
  ScreenToClient(hwndStatus, &pt);
  if (!GetWindowRect(hwndStatus, &rectStatus)
      || !n2e_ScreenToClientRect(hwndStatus, &rectStatus)
      || !PtInRect(&rectStatus, pt))
  {
    return FALSE;
  }
  return (pt.x > aWidth[0]) && (pt.x < aWidth[1]);
}

void n2e_OnPaneSizeClick(const HWND hwnd, const BOOL bLeftClick)
{
  if (!bLeftClick)
  {
    ++modeExpressionValue;
    if (modeExpressionValue > EVM_MAX)
    {
      modeExpressionValue = EVM_MIN;
    }
    UpdateStatusbar();
  }
  else
  {
    if (!n2e_CopyEvaluatedExpressionToClipboard())
    {
      WCHAR wchValue[MAX_EXPRESSION_LENGTH] = { 0 };
      _itow_s(SciCall_GetLength(), wchValue, COUNTOF(wchValue), 10);
      if (flagPasteBoard)
        bLastCopyFromMe = TRUE;
      n2e_SetClipboardText(hwndMain, wchValue);
    }
  }
}

LRESULT CALLBACK n2e_ShellProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode < 0)
  {
    return CallNextHookEx(hShellHook, nCode, wParam, lParam);
  }
  if (nCode == HSHELL_LANGUAGE)
  {
    SendMessage(hwndMain, WM_INPUTLANGCHANGE, 0, 0);
  }
  return 0;
}

BOOL n2e_FormatEvaluatedExpression(const HWND hwnd,
  char* expressionText, const int expressionTextLength,
  WCHAR* expressionValue, const int expressionValueLength, const BOOL bApplyLocaleForDecimalResult)
{
  WCHAR tchBuffer[MAX_PATH] = { 0 };
  const int bufferLength = COUNTOF(tchBuffer);

  int iCount = 0;
  BOOL bValidExpression = FALSE;
  if (n2e_IsExpressionEvaluationEnabled())
  {
    char *pszText = NULL;
    if (n2e_IsRectangularSelection())
    {
      pszText = n2e_Alloc(MAX_EXPRESSION_LENGTH + 1);
      ZeroMemory(pszText, MAX_EXPRESSION_LENGTH + 1);
      const int iSelections = SciCall_GetSelections();
      for (int i = 0; i < iSelections; ++i)
      {
        const Sci_Position posStart = SciCall_GetSelectionNStart(i);
        const Sci_Position posEnd = SciCall_GetSelectionNEnd(i);
        const int iCountOnLine = posEnd - posStart;
        if (iCountOnLine == 0)
        {
          continue;
        }
        if ((iCountOnLine > MAX_EXPRESSION_LENGTH) || (iCount + iCountOnLine + 2 > MAX_EXPRESSION_LENGTH))
        {
          break;
        }
        char *pszTextOnLine = n2e_GetTextRange(posStart, posEnd);
        if (pszTextOnLine)
        {
          lstrcatA(pszText, pszTextOnLine);
          lstrcatA(pszText, "\n");
          iCount += iCountOnLine + 1;
        }
        n2e_Free(pszTextOnLine);
      }
    }
    else
    {
      int iPosStart = 0;
      int iPosEnd = 0;
      if (((iCount = n2e_GetExpressionTextRange(&iPosStart, &iPosEnd)) > 0) &&
           (iCount <= MAX_EXPRESSION_LENGTH))
      {
        pszText = n2e_GetTextRange(iPosStart, iPosEnd);
      }
    }

    bValidExpression = (strlen(expressionText) > 0) && pszText && (strcmp(pszText, expressionText) == 0);
    if ((iCount > 0) && (iCount <= MAX_EXPRESSION_LENGTH) && 
      ((strcmp(pszText, expressionText) != 0) || (modePrevExpressionValue != modeExpressionValue)))
    {
      double exprValue = 0.0;
      bValidExpression = is_valid_expression(pszText, 1, &exprValue);
      if (bValidExpression && 
        ((strcmp(pszText, expressionText) != 0) || (modePrevExpressionValue != modeExpressionValue)))
      {
        UINT idExpressionFormatString = IDS_EXPRESSION_VALUE_INTEGER;
        switch (modeExpressionValue)
        {
          case EVM_DEC:
            idExpressionFormatString = (floor(exprValue) == exprValue) ? IDS_EXPRESSION_VALUE_INTEGER : IDS_EXPRESSION_VALUE_FLOAT;
            break;
          case EVM_HEX:
            idExpressionFormatString = IDS_EXPRESSION_VALUE_HEX;
            break;
          case EVM_BIN:
            {
              n2e_int2bin((unsigned int)floor(exprValue), expressionValue);
              idExpressionFormatString = IDS_EXPRESSION_VALUE_BINARY_STRING;
            }
            break;
          case EVM_OCT:
            idExpressionFormatString = IDS_EXPRESSION_VALUE_OCT;
            break;
          default:
            break;
        }
        switch (modeExpressionValue)
        {
          case EVM_BIN:
            FormatString(tchBuffer, bufferLength - 1, idExpressionFormatString, expressionValue);
            break;
          case EVM_DEC:
            FormatString(tchBuffer, bufferLength - 1, idExpressionFormatString, exprValue);
            break;
          case EVM_HEX:
          case EVM_OCT:
            FormatString(tchBuffer, bufferLength - 1, idExpressionFormatString, (int)exprValue);
            break;
        }
        modePrevExpressionValue = modeExpressionValue;
        strncpy_s(expressionText, expressionTextLength, pszText, expressionTextLength - 1);
        wcsncpy_s(expressionValue, expressionValueLength, tchBuffer, expressionValueLength - 1);
        if (bApplyLocaleForDecimalResult && (modeExpressionValue == EVM_DEC))
        {
          LPNUMBERFMT lpFormat = NULL;
          NUMBERFMT format = { 0 };
          if (idExpressionFormatString == IDS_EXPRESSION_VALUE_INTEGER)
          {
            n2e_GetNumberFormat(&format);
            format.NumDigits = 0;
            lpFormat = &format;
          }
          GetNumberFormat(LOCALE_USER_DEFAULT, 0, expressionValue, lpFormat, tchBuffer, bufferLength - 1);
          if (!n2e_CheckStringContainsAnyOf(tchBuffer, L"123456789"))
          {
            n2e_GetNumberFormat(&format);
            format.NumDigits = 6;
            lpFormat = &format;
            GetNumberFormat(LOCALE_USER_DEFAULT, 0, expressionValue, lpFormat, tchBuffer, bufferLength - 1);
          }
          wcsncpy_s(expressionValue, expressionValueLength, tchBuffer, expressionValueLength - 1);
        }
        n2e_Free(pszText);
        return TRUE;
      }
    }
    
    n2e_Free(pszText);
  }
  if (!bValidExpression && (strlen(expressionText) > 0))
  {
    expressionText[0] = 0;
    expressionValue[0] = 0;
  }
  return bValidExpression;
}

BOOL bIsModalDialogOnTop = FALSE;

BOOL n2e_IsModalDialogOnTop()
{
  return bIsModalDialogOnTop;
}

HWND n2e_GetTopLevelWindow(const HWND hwnd)
{
  return GetAncestor(hwnd, GA_ROOT);
}

BOOL n2e_IsMainWindowActive()
{
  return n2e_GetTopLevelWindow(GetForegroundWindow()) == hwndMain;
}

BOOL n2e_IsTopLevelWindow(const HWND hwnd)
{
  return hwnd == n2e_GetTopLevelWindow(hwnd);
}

BOOL n2e_IsModalDialog(const HWND hwnd)
{
  const HWND hwndOwner = GetWindow(hwnd, GW_OWNER);
  return hwnd
    && n2e_IsTopLevelWindow(hwnd)
    && n2e_IsTopLevelWindow(hwndOwner)
    && !IsWindowEnabled(hwndOwner);
}

void n2e_OnActivateMainWindow(const WPARAM wParam, const LPARAM lParam)
{
  bIsModalDialogOnTop = (wParam == WA_INACTIVE) ? n2e_IsModalDialog((HWND)lParam) : FALSE;
  if (wParam != WA_INACTIVE)
  {
    n2e_RestoreActiveEdit(FALSE);
    UpdateToolbar();
    UpdateStatusbar();
  }
}

HBITMAP ConvertIconToBitmap(const HICON hIcon, const int cx, const int cy)
{
  const HDC hScreenDC = GetDC(NULL);
  const HBITMAP hbmpTmp = CreateCompatibleBitmap(hScreenDC, cx, cy);
  const HDC hMemDC = CreateCompatibleDC(hScreenDC);
  const HBITMAP hOldBmp = SelectObject(hMemDC, hbmpTmp);
  DrawIconEx(hMemDC, 0, 0, hIcon, cx, cy, 0, NULL, DI_NORMAL);
  SelectObject(hMemDC, hOldBmp);

  const HBITMAP hDibBmp = (HBITMAP)CopyImage((HANDLE)hbmpTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);

  DeleteObject(hbmpTmp);
  DeleteDC(hMemDC);
  ReleaseDC(NULL, hScreenDC);

  return hDibBmp;
}

void n2e_SetUACIcon(const HMENU hMenu, const UINT nItem)
{
  static BOOL bInitialized = FALSE;
  if (bInitialized)
  {
    return;
  }

#define IDI_SHIELD          32518

  if (IsWindowsVistaOrGreater())
  {
    const int cx = GetSystemMetrics(SM_CYMENU) - 4;
    const int cy = cx;

    HICON hIconShield = NULL;
    SHSTOCKICONINFO sii = { 0 };
    sii.cbSize = sizeof(sii);
    if (SUCCEEDED(n2e_SHGetStockIconInfo(SIID_SHIELD, SHGFI_ICON | SHGFI_SMALLICON, &sii)))
    {
      hIconShield = sii.hIcon;
    }
    if (!hIconShield)
    {
      hIconShield = LoadImage(NULL, (LPCWSTR)IDI_SHIELD, IMAGE_ICON, cx, cy, LR_SHARED);
    }
    if (hIconShield)
    {
      MENUITEMINFO mii = { 0 };
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_BITMAP;
      mii.hbmpItem = ConvertIconToBitmap(hIconShield, cx, cy);
      SetMenuItemInfo(hMenu, nItem, FALSE, &mii);
    }
  }
  bInitialized = TRUE;
}

void n2e_RunTool(const ETool tool)
{
  extern EDITFINDREPLACE efrData;
  extern HWND hDlgGotoLine;
  extern HWND hDlgFindReplace;
  extern BOOL bSwitchedFindReplace;

  HWND* hwndTools[2] = { &hDlgGotoLine, &hDlgFindReplace };

  BOOL isToolRunning = FALSE;
  HWND* phwndTool = NULL;
  UINT uiSwitchToId = 0;
  switch (tool)
  {
  case Find:
    phwndTool = &hDlgFindReplace;
    isToolRunning = IsWindow(*phwndTool) && !GetDlgItem(*phwndTool, IDC_REPLACE);
    uiSwitchToId = IDMSG_SWITCHTOFIND;
    break;
  case Replace:
    phwndTool = &hDlgFindReplace;
    isToolRunning = IsWindow(*phwndTool) && GetDlgItem(*phwndTool, IDC_REPLACE);
    uiSwitchToId = IDMSG_SWITCHTOREPLACE;
    break;
  case GoTo:
    phwndTool = &hDlgGotoLine;
    isToolRunning = IsWindow(*phwndTool);
    uiSwitchToId = IDMSG_SWITCHTOGOTO;
    break;
  }

  const BOOL isMainWindowActive = n2e_IsMainWindowActive();
  if (isToolRunning)
  {
    const HWND hwndTool = *phwndTool;
    SetForegroundWindow(hwndTool);
    if (isMainWindowActive)
      SendMessage(hwndTool, WM_COMMAND, MAKELONG(IDC_INITIALIZE_SEARCH_STRING, 1), 0);

    UINT idControl = 0;
    switch (tool)
    {
    case Find:
    case Replace:
      idControl = IDC_FINDTEXT;
      break;
    case GoTo:
      idControl = IDC_LINENUM;
      break;
    }
    if (idControl)
      PostMessage(hwndTool, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwndTool, IDC_FINDTEXT)), 1);

    return;
  }

  for (int i = 0; i < COUNTOF(hwndTools); ++i)
  {
    HWND* lpHwndTool = hwndTools[i];
    if (lpHwndTool && IsWindow(*lpHwndTool))
    {
      SendMessage(*lpHwndTool, WM_COMMAND, MAKELONG(uiSwitchToId, 1), 0);
      DestroyWindow(*lpHwndTool);
      break;
    }
  }

  bSwitchedFindReplace = !isMainWindowActive;
  switch (tool)
  {
  case Find:
    *phwndTool = EditFindReplaceDlg(hwndEdit, &efrData, FALSE);
    break;
  case Replace:
    *phwndTool = EditFindReplaceDlg(hwndEdit, &efrData, TRUE);
    break;
  case GoTo:
    *phwndTool = EditLinenumDlg(hwndEdit, &efrData);
    break;
  }
}
