#include "MainWndHelper.h"
#include <math.h>
#include "EditHelper.h"
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
char arrchPrevExpressionText[MAX_EXPRESSION_LENGTH] = { 0 };
EExpressionValueMode modeExpressionValue = EVM_DEC;
WCHAR arrwchExpressionValue[MAX_PATH] = { 0 };

extern HWND hwndMain;
extern int aWidth[6];

BOOL ScreenToClientRect(const HWND hwnd, LPRECT pRect)
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
      || !ScreenToClientRect(hwndStatus, &rectStatus)
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
    if (wcslen(arrwchExpressionValue) > 0)
    {
      n2e_SetClipboardText(hwnd, arrwchExpressionValue);
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

BOOL n2e_FormatEvaluatedExpression(const HWND hwnd, WCHAR* tchBuffer, const int bufferSize)
{
  int iCount = 0;
  if (n2e_IsExpressionEvaluationEnabled())
  {
    char *pszText = NULL;
    if (n2e_IsRectangularSelection())
    {
      pszText = LocalAlloc(LPTR, MAX_EXPRESSION_LENGTH + 1);
      const int iSelections = SciCall_GetSelections();
      for (int i = 0; i < iSelections; ++i)
      {
        const Sci_Position posStart = SciCall_GetSelectionNStart(i);
        const Sci_Position posEnd = SciCall_GetSelectionNEnd(i);
        const int iCountOnLine = posEnd - posStart;
        if (((iCountOnLine <= 0)
          || (iCountOnLine > MAX_EXPRESSION_LENGTH))
          || (iCount + iCountOnLine + 2 > MAX_EXPRESSION_LENGTH))
        {
          break;
        }
        char *pszTextOnLine = LocalAlloc(LPTR, iCountOnLine + 1);
        struct TextRange tr = { { posStart, posEnd }, pszTextOnLine };
        if (SciCall_GetTextRange(0, &tr) > 0)
        {
          lstrcatA(pszText, pszTextOnLine);
          lstrcatA(pszText, "\n");
          iCount += iCountOnLine + 1;
        }
        LocalFree(pszTextOnLine);
      }
    }
    else
    {
      int iPosStart = 0;
      int iPosEnd = 0;
      if (((iCount = n2e_GetExpressionTextRange(&iPosStart, &iPosEnd)) > 0) &&
           (iCount <= MAX_EXPRESSION_LENGTH))
      {
        pszText = LocalAlloc(LPTR, iCount + 1);
        struct TextRange tr = { { iPosStart, iPosEnd }, pszText };
        SciCall_GetTextRange(0, &tr);
      }
    }

    if ((iCount > 0) && (iCount <= MAX_EXPRESSION_LENGTH) && 
        ((strcmp(pszText, arrchPrevExpressionText) != 0) || (modePrevExpressionValue != modeExpressionValue)))
    {
      double exprValue = 0.0;
      if (is_valid_expression(pszText, 1, &exprValue))
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
              n2e_int2bin((unsigned int)floor(exprValue), arrwchExpressionValue);
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
            FormatString(tchBuffer, bufferSize - 1, idExpressionFormatString, arrwchExpressionValue);
            break;
          case EVM_DEC:
            FormatString(tchBuffer, bufferSize - 1, idExpressionFormatString, exprValue);
            break;
          case EVM_HEX:
          case EVM_OCT:
            FormatString(tchBuffer, bufferSize - 1, idExpressionFormatString, (int)exprValue);
            break;
        }
        modePrevExpressionValue = modeExpressionValue;
        strncpy_s(arrchPrevExpressionText, COUNTOF(arrchPrevExpressionText) - 1, pszText, strlen(pszText));
        wcsncpy_s(arrwchExpressionValue, COUNTOF(arrwchExpressionValue) - 1, tchBuffer, bufferSize - 1);
        return TRUE;
      }
    }
    else if (lstrlen(pszText) > 0)
    {
      wcsncpy_s(tchBuffer, bufferSize - 1, arrwchExpressionValue, wcslen(arrwchExpressionValue));
      return TRUE;
    }
    LocalFree(pszText);
  }
  else
  {
    memset(arrchPrevExpressionText, 0, sizeof(arrchPrevExpressionText));
    memset(arrwchExpressionValue, 0, sizeof(arrwchExpressionValue));
  }
  return FALSE;
}

BOOL bIsModalDialogOnTop = FALSE;

BOOL n2e_IsModalDialogOnTop()
{
  return bIsModalDialogOnTop;
}

BOOL n2e_IsTopLevelWindow(const HWND hwnd)
{
  return hwnd == GetAncestor(hwnd, GA_ROOT);
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
