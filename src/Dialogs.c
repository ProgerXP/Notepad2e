/******************************************************************************
*
*
* Notepad2
*
* Dialogs.c
*   Notepad2 dialog boxes implementation
*
* See Readme.txt for more information about this source code.
* Please send me your comments to this work.
*
* See License.txt for details about distribution and modification.
*
*                                              (c) Florian Balmer 1996-2011
*                                                  florian.balmer@gmail.com
*                                               http://www.flos-freeware.ch
*
*
******************************************************************************/
//"a| 'b' c"
//Ctrl+Shift+4 = [a 'b' c]
//
//"a b"
//Ctrl+Shift+B = "a[ b]" (правильно)
//Ctrl+Shift+B = ["a b"] (почему?)

//document  ctrl+b    shift+ctrl+b
//a|(bc)d   a(bc)|d   a[(bc)|]d
//a(bc)d   a(bc|)d   a([bc|])d
//a(bc|)d   a(|bc)d   a([|bc])d
//a(bc)|d   a|(bc)d   a[|(bc)]d

#define _WIN32_WINNT 0x501
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <string.h>
#include "SciCall.h"
#include "scintilla.h"
#include "notepad2.h"
#include "edit.h"
#include "dlapi.h"
#include "dialogs.h"
#include "helpers.h"
#include "resource.h"
#include "version.h"
#include "_version.h"
#include "Extension/Utils.h"
#include "Extension/CommonUtils.h"
#include "Extension/DPIHelper.h"
#include "Extension/EditHelper.h"
#include "Extension/EditHelperEx.h"
#include "Extension/MainWndHelper.h"

extern HWND  hwndMain;
extern HWND  hwndEdit;
extern HINSTANCE g_hInstance;
extern DWORD dwLastIOError;
extern BOOL bSkipUnicodeDetection;
extern BOOL bLoadASCIIasUTF8;
extern int fNoFileVariables;
extern BOOL bNoEncodingTags;
extern BOOL bFixLineEndings;
extern BOOL bAutoStripBlanks;
extern WCHAR szCurFile[MAX_PATH + 40];


//=============================================================================
//
//  MsgBox()
//
int MsgBox(int iType, UINT uIdMsg, ...)
{
  WCHAR szText[1024];
  WCHAR szBuf[1024];
  WCHAR szTitle[64];
  int iIcon = 0;
  HWND hwnd;
  if (!GetString(uIdMsg, szBuf, COUNTOF(szBuf)))
    return (0);

  wvsprintf(szText, szBuf, (LPVOID)((PUINT_PTR)&uIdMsg + 1));

  if (uIdMsg == IDS_ERR_LOADFILE || uIdMsg == IDS_ERR_SAVEFILE ||
      uIdMsg == IDS_CREATEINI_FAIL || uIdMsg == IDS_WRITEINI_FAIL ||
      uIdMsg == IDS_EXPORT_FAIL || uIdMsg == IDS_ERR_FAILED_CREATE)
  {
    LPVOID lpMsgBuf;
    WCHAR wcht;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      dwLastIOError,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR)&lpMsgBuf,
      0,
      NULL);
    StrTrim(lpMsgBuf, L" \a\b\f\n\r\t\v");
    StrCatBuff(szText, L"\n", COUNTOF(szText));
    StrCatBuff(szText, lpMsgBuf, COUNTOF(szText));
    LocalFree(lpMsgBuf);
    wcht = *CharPrev(szText, StrEnd(szText));
    if (IsCharAlphaNumeric(wcht) || wcht == '"' || wcht == '\'')
      StrCatBuff(szText, L".", COUNTOF(szText));
  }

  GetString(IDS_APPTITLE, szTitle, COUNTOF(szTitle));

  switch (iType)
  {
    case MBINFO:
      iIcon = MB_ICONINFORMATION;
      break;
    case MBWARN:
      iIcon = MB_ICONEXCLAMATION;
      break;
    case MBYESNO:
      iIcon = MB_YESNO;
      break;
    case MBYESNOCANCEL:
      iIcon = MB_YESNOCANCEL;
      break;
    case MBYESNOWARN:
      iIcon = MB_YESNO;
      break;
    case MBOKCANCEL:
      iIcon = MB_OKCANCEL;
      break;
  }

  // [2e]: File change notify MsgBox: use icon depending on unsaved state #350
  if ((iIcon & (MB_ICONEXCLAMATION | MB_ICONINFORMATION)) == 0)
  {
    iIcon |= n2e_IsDocumentModified() ? MB_ICONEXCLAMATION : MB_ICONINFORMATION;
  }
  // [/2e]

  if (!(hwnd = GetFocus()))
    hwnd = hwndMain;

  return MessageBoxEx(hwnd,
                      szText, szTitle,
                      MB_SETFOREGROUND | iIcon,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
}


//=============================================================================
//
//  DisplayCmdLineHelp()
//
void DisplayCmdLineHelp(const HWND hwnd)
{
  MSGBOXPARAMS mbp;

  WCHAR szTitle[32];
  WCHAR szText[2048];

  GetString(IDS_APPTITLE, szTitle, COUNTOF(szTitle));
  GetString(IDS_CMDLINEHELP, szText, COUNTOF(szText));

  mbp.cbSize = sizeof(MSGBOXPARAMS);
  mbp.hwndOwner = hwnd;
  mbp.hInstance = g_hInstance;
  mbp.lpszText = szText;
  mbp.lpszCaption = szTitle;
  mbp.dwStyle = MB_OK | MB_USERICON | MB_SETFOREGROUND;
  mbp.lpszIcon = MAKEINTRESOURCE(IDR_MAINWND);
  mbp.dwContextHelpId = 0;
  mbp.lpfnMsgBoxCallback = NULL;
  mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
  MessageBoxIndirect(&mbp);
}


//=============================================================================
//
//  BFFCallBack()
//
int CALLBACK BFFCallBack(HWND hwnd, UINT umsg, LPARAM lParam, LPARAM lpData)
{
  if (umsg == BFFM_INITIALIZED)
    SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);

  return (0);

  lParam;
}


//=============================================================================
//
//  GetDirectory()
//
BOOL GetDirectory(HWND hwndParent, int iTitle, LPWSTR pszFolder, LPCWSTR pszBase, BOOL bNewDialogStyle)
{

  BROWSEINFO bi;
  LPITEMIDLIST pidl;
  WCHAR szTitle[256];
  WCHAR szBase[MAX_PATH];
  BOOL fOk = FALSE;

  lstrcpy(szTitle, L"");
  GetString(iTitle, szTitle, COUNTOF(szTitle));

  if (!pszBase || !*pszBase)
    GetCurrentDirectory(MAX_PATH, szBase);
  else
    lstrcpy(szBase, pszBase);

  bi.hwndOwner = hwndParent;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = pszFolder;
  bi.lpszTitle = szTitle;
  bi.ulFlags = BIF_RETURNONLYFSDIRS;
  if (bNewDialogStyle)
    bi.ulFlags |= BIF_NEWDIALOGSTYLE;
  bi.lpfn = &BFFCallBack;
  bi.lParam = (LPARAM)szBase;
  bi.iImage = 0;

  pidl = SHBrowseForFolder(&bi);
  if (pidl)
  {
    SHGetPathFromIDList(pidl, pszFolder);
    CoTaskMemFree(pidl);
    fOk = TRUE;
  }

  return fOk;

}


//=============================================================================
//
//  AboutDlgProc()
//
INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  static HFONT hFontTitle;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    // [2e]: Updated with Notepad 2e info
    case WM_INITDIALOG: {
        // [2e]: Boost regex and Cyrillic #162 (ICU build)
        wchar_t szText[MAX_PATH] = { 0 };
        if (GetString(IDS_APPTITLE, szText, COUNTOF(szText)-1))
        {
          StrCatBuff(szText, L" (R", COUNTOF(szText));
          StrCatBuff(szText, VERSION_FILEVERSION_BUILD_STRING, COUNTOF(szText));
          StrCatBuff(szText, L", ", COUNTOF(szText));
          StrCatBuff(szText, VERSION_COMMIT, COUNTOF(szText));
          StrCatBuff(szText, L")", COUNTOF(szText));
          SendDlgItemMessage(hwnd, IDC_VERSION, WM_SETTEXT, 0, (LPARAM)szText);

          char szVersion[MAX_PATH] = { 0 };
          WCharToMBCS(CP_UTF8, szText, szVersion, COUNTOF(szVersion));

          char szBuildDate[MAX_PATH] = { 0 };
          lstrcpy(szText, L"Built on ");
          lstrcat(szText, H_TIMESTAMP);
          WCharToMBCS(CP_UTF8, szText, szBuildDate, COUNTOF(szBuildDate));

          n2e_InitAboutText(GetDlgItem(hwnd, IDC_RICHEDIT), szVersion, BUILD_YEAR_STR, szBuildDate);
        }

        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;

    case WM_NOTIFY: {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        if (pnmhdr->idFrom == IDC_RICHEDIT)
        {
          if (pnmhdr->code == EN_REQUESTRESIZE)
          {
            RECT rc, rcRichedit, rcOK;
            GetWindowRect(hwnd, &rc);
            GetWindowRect(pnmhdr->hwndFrom, &rcRichedit);
            const HWND hwndOK = GetDlgItem(hwnd, IDOK);
            GetWindowRect(hwndOK, &rcOK);

            REQRESIZE* prr = (REQRESIZE*)pnmhdr;
            const int cx = prr->rc.right - prr->rc.left;
            const int cy = prr->rc.bottom - prr->rc.top;
            const int heightDiff = (rcRichedit.bottom - rcRichedit.top) - cy;
            SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top - heightDiff, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
            SetWindowPos(pnmhdr->hwndFrom, NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
            n2e_ScreenToClientRect(hwnd, &rcOK);
            SetWindowPos(hwndOK, NULL, rcOK.left, rcOK.top - heightDiff, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
          }
          else
          {
            n2e_ProcessAbout3rdPartyUrl(GetDlgItem(hwnd, IDC_RICHEDIT), (ENLINK*)pnmhdr);
          }
          break;
        }
      }
      break;

    case WM_COMMAND:

      switch (LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
          EndDialog(hwnd, IDOK);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


// [2e]: Attribution menu command #181
INT_PTR CALLBACK About3rdPartyDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
      n2e_InitAbout3rdPartyText(GetDlgItem(hwnd, IDC_RICHEDIT));
      DPI_INIT();
      CenterDlgInParent(hwnd);
    }
    return TRUE;

    case WM_NOTIFY: {
      LPNMHDR pnmhdr = (LPNMHDR)lParam;
      switch (pnmhdr->code)
      {
      case EN_LINK:
        if (pnmhdr->idFrom == IDC_RICHEDIT) {
          n2e_ProcessAbout3rdPartyUrl(GetDlgItem(hwnd, IDC_RICHEDIT), (ENLINK*)pnmhdr);
        }
        break;
      }
     }
     break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
    case IDCANCEL:
      EndDialog(hwnd, IDOK);
      break;
    }
    return TRUE;
  }
  return FALSE;
}
// [/2e]


//=============================================================================
//
//  RunDlgProc()
//
INT_PTR CALLBACK RunDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        MakeBitmapButton(hwnd, IDC_SEARCHEXE, g_hInstance, IDB_OPEN);
        SendDlgItemMessage(hwnd, IDC_COMMANDLINE, EM_LIMITTEXT, MAX_PATH - 1, 0);
        SetDlgItemText(hwnd, IDC_COMMANDLINE, (LPCWSTR)lParam);
        SHAutoComplete(GetDlgItem(hwnd, IDC_COMMANDLINE), SHACF_FILESYSTEM);
        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      DeleteBitmapButton(hwnd, IDC_SEARCHEXE);
      return FALSE;


    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_SEARCHEXE: {
            WCHAR szArgs[MAX_PATH];
            WCHAR szArg2[MAX_PATH];
            WCHAR szFile[MAX_PATH * 2];
            WCHAR szFilter[256];
            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(OPENFILENAME));

            GetDlgItemText(hwnd, IDC_COMMANDLINE, szArgs, COUNTOF(szArgs));
            ExpandEnvironmentStringsEx(szArgs, COUNTOF(szArgs));
            ExtractFirstArgument(szArgs, szFile, szArg2);

            GetString(IDS_FILTER_EXE, szFilter, COUNTOF(szFilter));
            PrepareFilterStr(szFilter);

            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = szFilter;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = COUNTOF(szFile);
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
              | OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_NODEREFERENCELINKS;

            if (GetOpenFileName(&ofn))
            {
              PathQuoteSpaces(szFile);
              if (lstrlen(szArg2))
              {
                lstrcat(szFile, L" ");
                lstrcat(szFile, szArg2);
              }
              SetDlgItemText(hwnd, IDC_COMMANDLINE, szFile);
            }

            PostMessage(hwnd, WM_NEXTDLGCTL, 1, 0);
          }
          break;


        case IDC_COMMANDLINE: {
            BOOL bEnableOK = FALSE;
            WCHAR args[MAX_PATH];
            if (GetDlgItemText(hwnd, IDC_COMMANDLINE, args, MAX_PATH))
              if (ExtractFirstArgument(args, args, NULL))
                if (lstrlen(args))
                  bEnableOK = TRUE;

            EnableWindow(GetDlgItem(hwnd, IDOK), bEnableOK);
          }
          break;


        case IDOK: {
            WCHAR arg1[MAX_PATH];
            WCHAR arg2[MAX_PATH];
            SHELLEXECUTEINFO sei;
            WCHAR wchDirectory[MAX_PATH] = L"";

            if (GetDlgItemText(hwnd, IDC_COMMANDLINE, arg1, MAX_PATH))
            {
              BOOL bQuickExit = FALSE;
              n2e_SetLastRun(arg1);
              ExpandEnvironmentStringsEx(arg1, COUNTOF(arg1));
              ExtractFirstArgument(arg1, arg1, arg2);
              if (lstrcmpi(arg1, L"notepad2") == 0 ||
                  lstrcmpi(arg1, L"notepad2.exe") == 0)
              {
                GetModuleFileName(NULL, arg1, COUNTOF(arg1));
                bQuickExit = TRUE;
              }
              if (lstrlen(szCurFile))
              {
                lstrcpy(wchDirectory, szCurFile);
                PathRemoveFileSpec(wchDirectory);
              }

              ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

              sei.cbSize = sizeof(SHELLEXECUTEINFO);
              sei.fMask = 0;
              sei.hwnd = hwnd;
              sei.lpVerb = NULL;
              sei.lpFile = arg1;
              sei.lpParameters = arg2;
              sei.lpDirectory = wchDirectory;
              sei.nShow = SW_SHOWNORMAL;

              if (bQuickExit)
              {
                sei.fMask |= /*SEE_MASK_NOZONECHECKS*/0x00800000;
                EndDialog(hwnd, IDOK);
                ShellExecuteEx(&sei);
              }

              else
              {
                if (ShellExecuteEx(&sei))
                  EndDialog(hwnd, IDOK);
                else
                  PostMessage(hwnd, WM_NEXTDLGCTL,
                  (WPARAM)(GetDlgItem(hwnd, IDC_COMMANDLINE)), 1);
              }
            }
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;
  }
  return FALSE;
}

//=============================================================================
//
//  RunDlg()
//
void RunDlg(HWND hwnd, LPCWSTR lpstrDefault)
{
  ThemedDialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_RUN),
                       hwnd, RunDlgProc, (LPARAM)n2e_GetLastRun(lpstrDefault));
}


//=============================================================================
//
//  OpenWithDlgProc()
//
extern WCHAR tchOpenWithDir[MAX_PATH];
extern int  flagNoFadeHidden;

extern int cxOpenWithDlg;
extern int cyOpenWithDlg;

INT_PTR CALLBACK OpenWithDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        ResizeDlg_Init(hwnd, cxOpenWithDlg, cyOpenWithDlg, IDC_RESIZEGRIP3);

        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);

        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd, IDC_OPENWITHDIR), LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd, IDC_OPENWITHDIR), 0, &lvc);
        DirList_Init(GetDlgItem(hwnd, IDC_OPENWITHDIR), NULL);
        DirList_Fill(GetDlgItem(hwnd, IDC_OPENWITHDIR), tchOpenWithDir, DL_ALLOBJECTS, L"", FALSE, flagNoFadeHidden, DS_NAME, FALSE);
        DirList_StartIconThread(GetDlgItem(hwnd, IDC_OPENWITHDIR));
        ListView_SetItemState(GetDlgItem(hwnd, IDC_OPENWITHDIR), 0, LVIS_FOCUSED, LVIS_FOCUSED);
        MakeBitmapButton(hwnd, IDC_GETOPENWITHDIR, g_hInstance, IDB_OPEN);

        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      DirList_Destroy(GetDlgItem(hwnd, IDC_OPENWITHDIR));
      DeleteBitmapButton(hwnd, IDC_GETOPENWITHDIR);

      ResizeDlg_Destroy(hwnd, &cxOpenWithDlg, &cyOpenWithDlg);
      return FALSE;


    case WM_SIZE: {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd, lParam, &dx, &dy);

        hdwp = BeginDeferWindowPos(6);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_RESIZEGRIP3, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDOK, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDCANCEL, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_OPENWITHDIR, dx, dy, SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_GETOPENWITHDIR, 0, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_OPENWITHDESCR, 0, dy, SWP_NOSIZE);
        EndDeferWindowPos(hdwp);

        ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_OPENWITHDIR), 0, LVSCW_AUTOSIZE_USEHEADER);
        DPI_RESIZE();
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd, lParam);
      DPI_GETMINMAXINFO();
      return TRUE;


    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_OPENWITHDIR)
        {
          switch (pnmh->code)
          {
            case LVN_GETDISPINFO:
              DirList_GetDispInfo(GetDlgItem(hwnd, IDC_OPENWITHDIR), lParam, flagNoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(GetDlgItem(hwnd, IDC_OPENWITHDIR), lParam);
              break;

            case LVN_ITEMCHANGED: {
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW *)lParam;
                EnableWindow(GetDlgItem(hwnd, IDOK), (pnmlv->uNewState & LVIS_SELECTED));
              }
              break;

            case NM_DBLCLK:
              if (ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_OPENWITHDIR)))
              {
                SendMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, 1), 0);
              }
              break;
          }
        }
      }
      return TRUE;


    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_GETOPENWITHDIR: {
            if (GetDirectory(hwnd, IDS_OPENWITH, tchOpenWithDir, tchOpenWithDir, TRUE))
            {
              DirList_Fill(GetDlgItem(hwnd, IDC_OPENWITHDIR), tchOpenWithDir, DL_ALLOBJECTS, L"", FALSE, flagNoFadeHidden, DS_NAME, FALSE);
              DirList_StartIconThread(GetDlgItem(hwnd, IDC_OPENWITHDIR));
              ListView_EnsureVisible(GetDlgItem(hwnd, IDC_OPENWITHDIR), 0, FALSE);
              ListView_SetItemState(GetDlgItem(hwnd, IDC_OPENWITHDIR), 0, LVIS_FOCUSED, LVIS_FOCUSED);
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_OPENWITHDIR)), 1);
          }
          break;


        case IDOK: {
            LPDLITEM lpdli = (LPDLITEM)GetWindowLongPtr(hwnd, DWLP_USER);
            lpdli->mask = DLI_FILENAME | DLI_TYPE;
            lpdli->ntype = DLE_NONE;
            DirList_GetItem(GetDlgItem(hwnd, IDC_OPENWITHDIR), (-1), lpdli);

            if (lpdli->ntype != DLE_NONE)
              EndDialog(hwnd, IDOK);
            else
              MessageBeep(0);
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  OpenWithDlg()
//
BOOL OpenWithDlg(HWND hwnd, LPCWSTR lpstrFile)
{

  DLITEM dliOpenWith;
  dliOpenWith.mask = DLI_FILENAME;
  if (IDOK == ThemedDialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_OPENWITH),
                                   hwnd, OpenWithDlgProc, (LPARAM)&dliOpenWith))
  {
    SHELLEXECUTEINFO sei;
    WCHAR szParam[MAX_PATH];
    WCHAR wchDirectory[MAX_PATH] = L"";

    if (lstrlen(szCurFile))
    {
      lstrcpy(wchDirectory, szCurFile);
      PathRemoveFileSpec(wchDirectory);
    }

    ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = 0;
    sei.hwnd = hwnd;
    sei.lpVerb = NULL;
    sei.lpFile = dliOpenWith.szFileName;
    sei.lpParameters = szParam;
    sei.lpDirectory = wchDirectory;
    sei.nShow = SW_SHOWNORMAL;

    // resolve links and get short path name
    if (!(PathIsLnkFile(lpstrFile) && PathGetLnkPath(lpstrFile, szParam, COUNTOF(szParam))))
      lstrcpy(szParam, lpstrFile);
    PathQuoteSpaces(szParam);

    ShellExecuteEx(&sei);

    return (TRUE);
  }
  return (FALSE);
}


//=============================================================================
//
//  FavoritesDlgProc()
//
extern WCHAR tchFavoritesDir[MAX_PATH];
extern int cxFavoritesDlg;
extern int cyFavoritesDlg;

INT_PTR CALLBACK FavoritesDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static HWND hwndLV;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        ResizeDlg_Init(hwnd, cxFavoritesDlg, cyFavoritesDlg, IDC_RESIZEGRIP3);

        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);

        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd, IDC_FAVORITESDIR), LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd, IDC_FAVORITESDIR), 0, &lvc);
        DirList_Init(GetDlgItem(hwnd, IDC_FAVORITESDIR), NULL);
        DirList_Fill(GetDlgItem(hwnd, IDC_FAVORITESDIR), tchFavoritesDir, DL_ALLOBJECTS, L"", FALSE, flagNoFadeHidden, DS_NAME, FALSE);
        DirList_StartIconThread(GetDlgItem(hwnd, IDC_FAVORITESDIR));

        MakeBitmapButton(hwnd, IDC_GETFAVORITESDIR, g_hInstance, IDB_OPEN);
        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      DirList_Destroy(GetDlgItem(hwnd, IDC_FAVORITESDIR));
      DeleteBitmapButton(hwnd, IDC_GETFAVORITESDIR);

      ResizeDlg_Destroy(hwnd, &cxFavoritesDlg, &cyFavoritesDlg);
      return FALSE;


    case WM_SIZE: {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd, lParam, &dx, &dy);

        hdwp = BeginDeferWindowPos(6);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_RESIZEGRIP3, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDOK, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDCANCEL, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_FAVORITESDIR, dx, dy, SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_GETFAVORITESDIR, 0, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_FAVORITESDESCR, 0, dy, SWP_NOSIZE);
        EndDeferWindowPos(hdwp);
        ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_FAVORITESDIR), 0, LVSCW_AUTOSIZE_USEHEADER);
        DPI_RESIZE();
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd, lParam);
      DPI_GETMINMAXINFO();
      return TRUE;


    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_FAVORITESDIR)
        {
          switch (pnmh->code)
          {
            case LVN_GETDISPINFO:
              DirList_GetDispInfo(GetDlgItem(hwnd, IDC_OPENWITHDIR), lParam, flagNoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(GetDlgItem(hwnd, IDC_FAVORITESDIR), lParam);
              break;

            case LVN_ITEMCHANGED: {
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW *)lParam;
                EnableWindow(GetDlgItem(hwnd, IDOK), (pnmlv->uNewState & LVIS_SELECTED));
              }
              break;

            case NM_DBLCLK:
              if (ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_FAVORITESDIR)))
                SendMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, 1), 0);
              break;
          }
        }
      }
      return TRUE;


    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_GETFAVORITESDIR: {
            if (GetDirectory(hwnd, IDS_FAVORITES, tchFavoritesDir, tchFavoritesDir, TRUE))
            {
              DirList_Fill(GetDlgItem(hwnd, IDC_FAVORITESDIR), tchFavoritesDir, DL_ALLOBJECTS, L"", FALSE, flagNoFadeHidden, DS_NAME, FALSE);
              DirList_StartIconThread(GetDlgItem(hwnd, IDC_FAVORITESDIR));
              ListView_EnsureVisible(GetDlgItem(hwnd, IDC_FAVORITESDIR), 0, FALSE);
              ListView_SetItemState(GetDlgItem(hwnd, IDC_FAVORITESDIR), 0, LVIS_FOCUSED, LVIS_FOCUSED);
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_FAVORITESDIR)), 1);
          }
          break;


        case IDOK: {
            LPDLITEM lpdli = (LPDLITEM)GetWindowLongPtr(hwnd, DWLP_USER);
            lpdli->mask = DLI_FILENAME | DLI_TYPE;
            lpdli->ntype = DLE_NONE;
            DirList_GetItem(GetDlgItem(hwnd, IDC_FAVORITESDIR), (-1), lpdli);

            if (lpdli->ntype != DLE_NONE)
              EndDialog(hwnd, IDOK);
            else
              MessageBeep(0);
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  FavoritesDlg()
//
BOOL FavoritesDlg(HWND hwnd, LPWSTR lpstrFile)
{

  DLITEM dliFavorite;
  dliFavorite.mask = DLI_FILENAME;
  if (IDOK == ThemedDialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_FAVORITES),
                                   hwnd, FavoritesDlgProc, (LPARAM)&dliFavorite))
  {
    lstrcpyn(lpstrFile, dliFavorite.szFileName, MAX_PATH);
    return (TRUE);
  }

  return (FALSE);
}


//=============================================================================
//
//  AddToFavDlgProc()
//
//  Controls: 100 Edit
//
INT_PTR CALLBACK AddToFavDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    // [2e]: Add to Favorites - selection mode #249
    PTADDFAVPARAMS lpParams = NULL;

    case WM_INITDIALOG:
      lpParams = (PTADDFAVPARAMS)lParam;
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lpParams);

      SendDlgItemMessage(hwnd, 100, EM_LIMITTEXT, MAX_PATH - 1, 0);
      SetDlgItemText(hwnd, 100, lpParams->pszName);
      if (n2e_IsRectangularSelection())
      {
        EnableWindow(GetDlgItem(hwnd, IDC_RADIO4), FALSE);
      }

      if (n2e_IsRectangularSelection()
        || !n2e_GetCurrentSelection(lpParams->pszCurrentSelection, COUNTOF(lpParams->pszCurrentSelection))
        || (lstrlen(lpParams->pszCurrentSelection) == 0)
        || (wcspbrk(lpParams->pszCurrentSelection, L"\t\r\n\"") != NULL))
      {
        EnableWindow(GetDlgItem(hwnd, IDC_RADIO5), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_RADIO6), FALSE);
      }
      n2e_SetCheckedRadioButton(hwnd, IDC_RADIO1, IDC_RADIO6, lpParams->cursorPosition);

      DPI_INIT();
      CenterDlgInParent(hwnd);
      return TRUE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        case 100:
          EnableWindow(GetDlgItem(hwnd, IDOK),
                       GetWindowTextLength(GetDlgItem(hwnd, 100)));
          break;


        case IDOK:
          lpParams = (PTADDFAVPARAMS)GetWindowLongPtr(hwnd, DWLP_USER);
          GetDlgItemText(hwnd, 100, lpParams->pszName,
                         MAX_PATH - 1);
          // [2e]: Add to Favorites - selection mode #249
          lpParams->cursorPosition = (EFavoritesCursorPosition)n2e_GetCheckedRadioButton(hwnd, IDC_RADIO1, IDC_RADIO6);
          n2e_UpdateFavLnkParams(lpParams);
          // [/2e]
          EndDialog(hwnd, IDOK);
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  AddToFavDlg()
//
BOOL AddToFavDlg(HWND hwnd, LPCWSTR lpszName, LPCWSTR lpszTarget)
{

  INT_PTR iResult;

  // [2e]: Add to Favorites - selection mode #249
  TADDFAVPARAMS params = { 0, 0, 0, FCP_FIRST_LINE };
  lstrcpy(params.pszName, lpszName);
  lstrcpy(params.pszTarget, lpszTarget);

  // [2e]: Add to favourites - append unique suffix #290
  WCHAR tchLnkFileName[MAX_PATH] = { 0 };
  BOOL bMustCopy = FALSE;
  if (SHGetNewLinkInfo(lpszTarget, tchFavoritesDir, tchLnkFileName, &bMustCopy, 0))
  {
    LPWSTR ptrLnkExtension = PathFindExtension(tchLnkFileName);
    if (ptrLnkExtension && (StrCmpI(ptrLnkExtension, L".LNK") == 0))
    {
      ptrLnkExtension[0] = 0;
    }
    lstrcpy(params.pszName, PathFindFileName(tchLnkFileName));
  }
  // [/2e]

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(IDD_ADDTOFAV),
    hwnd,
    AddToFavDlgProc, (LPARAM)&params);

  if (iResult == IDOK)
  {
    // [2e]: Add to Favorites - selection mode #249
    if (!PathCreateFavLnk(params.pszName, params.pszTarget, params.pszArguments, tchFavoritesDir))
    {
      MsgBox(MBWARN, IDS_FAV_FAILURE);
      return FALSE;
    }

    else
    {
      MsgBox(MBINFO, IDS_FAV_SUCCESS);
      return TRUE;
    }
  }

  else
    return FALSE;

}


//=============================================================================
//
//  FileMRUDlgProc()
//
//
extern LPMRULIST pFileMRU;
extern BOOL bSaveRecentFiles;
extern int  cxFileMRUDlg;
extern int  cyFileMRUDlg;
extern int  flagNoFadeHidden;

typedef struct tagIconThreadInfo
{
  HWND hwnd;                 // HWND of ListView Control
  HANDLE hExitThread;        // Flag is set when Icon Thread should terminate
  HANDLE hTerminatedThread;  // Flag is set when Icon Thread has terminated

} ICONTHREADINFO, *LPICONTHREADINFO;

DWORD WINAPI FileMRUIconThread(LPVOID lpParam)
{
  HWND hwnd;
  LPICONTHREADINFO lpit;
  LV_ITEM lvi;
  WCHAR tch[MAX_PATH];
  SHFILEINFO shfi;
  DWORD dwFlags = SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED;
  DWORD dwAttr = 0;
  int iItem = 0;
  int iMaxItem;

  lpit = (LPICONTHREADINFO)lpParam;
  ResetEvent(lpit->hTerminatedThread);

  hwnd = lpit->hwnd;
  iMaxItem = ListView_GetItemCount(hwnd);

  CoInitialize(NULL);

  ZeroMemory(&lvi, sizeof(LV_ITEM));

  while (iItem < iMaxItem && WaitForSingleObject(lpit->hExitThread, 0) != WAIT_OBJECT_0)
  {
    lvi.mask = LVIF_TEXT;
    lvi.pszText = tch;
    lvi.cchTextMax = COUNTOF(tch);
    lvi.iItem = iItem;
    if (ListView_GetItem(hwnd, &lvi))
    {
      if (PathIsUNC(tch) || !PathFileExists(tch))
      {
        dwFlags |= SHGFI_USEFILEATTRIBUTES;
        dwAttr = FILE_ATTRIBUTE_NORMAL;
        shfi.dwAttributes = 0;
        SHGetFileInfo(PathFindFileName(tch), dwAttr, &shfi, sizeof(SHFILEINFO), dwFlags);
      }
      else
      {
        shfi.dwAttributes = SFGAO_LINK | SFGAO_SHARE;
        SHGetFileInfo(tch, dwAttr, &shfi, sizeof(SHFILEINFO), dwFlags);
      }

      lvi.mask = LVIF_IMAGE;
      lvi.iImage = shfi.iIcon;
      lvi.stateMask = 0;
      lvi.state = 0;
      if (shfi.dwAttributes & SFGAO_LINK)
      {
        lvi.mask |= LVIF_STATE;
        lvi.stateMask |= LVIS_OVERLAYMASK;
        lvi.state |= INDEXTOOVERLAYMASK(2);
      }

      if (shfi.dwAttributes & SFGAO_SHARE)
      {
        lvi.mask |= LVIF_STATE;
        lvi.stateMask |= LVIS_OVERLAYMASK;
        lvi.state |= INDEXTOOVERLAYMASK(1);
      }

      if (PathIsUNC(tch))
        dwAttr = FILE_ATTRIBUTE_NORMAL;
      else
        dwAttr = GetFileAttributes(tch);

      if (!flagNoFadeHidden &&
          dwAttr != INVALID_FILE_ATTRIBUTES &&
          dwAttr & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
      {
        lvi.mask |= LVIF_STATE;
        lvi.stateMask |= LVIS_CUT;
        lvi.state |= LVIS_CUT;
      }

      lvi.iSubItem = 0;
      ListView_SetItem(hwnd, &lvi);
    }
    iItem++;
  }

  CoUninitialize();

  SetEvent(lpit->hTerminatedThread);
  ExitThread(0);
  return (0);
}

INT_PTR CALLBACK FileMRUDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        SHFILEINFO shfi;
        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        LPICONTHREADINFO lpit = (LPVOID)GlobalAlloc(GPTR, sizeof(ICONTHREADINFO));
        SetProp(hwnd, L"it", (HANDLE)lpit);
        lpit->hwnd = GetDlgItem(hwnd, IDC_FILEMRU);
        lpit->hExitThread = CreateEvent(NULL, TRUE, FALSE, NULL);
        lpit->hTerminatedThread = CreateEvent(NULL, TRUE, TRUE, NULL);

        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);

        ResizeDlg_Init(hwnd, cxFileMRUDlg, cyFileMRUDlg, IDC_RESIZEGRIP);

        ListView_SetImageList(GetDlgItem(hwnd, IDC_FILEMRU),
          (HIMAGELIST)SHGetFileInfo(L"C:\\", 0, &shfi, sizeof(SHFILEINFO), SHGFI_SMALLICON | SHGFI_SYSICONINDEX),
                              LVSIL_SMALL);

        ListView_SetImageList(GetDlgItem(hwnd, IDC_FILEMRU),
          (HIMAGELIST)SHGetFileInfo(L"C:\\", 0, &shfi, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_SYSICONINDEX),
                              LVSIL_NORMAL);

        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd, IDC_FILEMRU), LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd, IDC_FILEMRU), 0, &lvc);

        // Update view
        SendMessage(hwnd, WM_COMMAND, MAKELONG(0x00A0, 1), 0);

        if (bSaveRecentFiles)
          CheckDlgButton(hwnd, IDC_SAVEMRU, BST_CHECKED);

        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY: {
        LPICONTHREADINFO lpit = (LPVOID)GetProp(hwnd, L"it");
        SetEvent(lpit->hExitThread);
        while (WaitForSingleObject(lpit->hTerminatedThread, 0) != WAIT_OBJECT_0)
        {
          MSG msg;
          if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
          {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
        }
        CloseHandle(lpit->hExitThread);
        CloseHandle(lpit->hTerminatedThread);
        RemoveProp(hwnd, L"it");
        GlobalFree(lpit);

        bSaveRecentFiles = (IsDlgButtonChecked(hwnd, IDC_SAVEMRU)) ? 1 : 0;

        ResizeDlg_Destroy(hwnd, &cxFileMRUDlg, &cyFileMRUDlg);
      }
      return FALSE;


    case WM_SIZE: {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd, lParam, &dx, &dy);

        hdwp = BeginDeferWindowPos(5);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_RESIZEGRIP, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDOK, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDCANCEL, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_FILEMRU, dx, dy, SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_SAVEMRU, 0, dy, SWP_NOSIZE);
        EndDeferWindowPos(hdwp);
        ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_FILEMRU), 0, LVSCW_AUTOSIZE_USEHEADER);
        DPI_RESIZE();
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd, lParam);
      DPI_GETMINMAXINFO();
      return TRUE;


    case WM_NOTIFY: {
        if (((LPNMHDR)(lParam))->idFrom == IDC_FILEMRU)
        {
          switch (((LPNMHDR)(lParam))->code)
          {
            case NM_DBLCLK:
              SendMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, 1), 0);
              break;
            case LVN_GETDISPINFO:
              break;
            case LVN_ITEMCHANGED:
            case LVN_DELETEITEM:
              EnableWindow(GetDlgItem(hwnd, IDOK), ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_FILEMRU)));
              break;
          }
        }
      }

      return TRUE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {
        case 0x00A0: {
            int i;
            WCHAR tch[MAX_PATH];
            LV_ITEM lvi;
            SHFILEINFO shfi;

            DWORD dwtid;
            LPICONTHREADINFO lpit = (LPVOID)GetProp(hwnd, L"it");

            SetEvent(lpit->hExitThread);
            while (WaitForSingleObject(lpit->hTerminatedThread, 0) != WAIT_OBJECT_0)
            {
              MSG msg;
              if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
              {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
              }
            }
            ResetEvent(lpit->hExitThread);
            SetEvent(lpit->hTerminatedThread);

            ListView_DeleteAllItems(GetDlgItem(hwnd, IDC_FILEMRU));

            ZeroMemory(&lvi, sizeof(LV_ITEM));
            lvi.mask = LVIF_TEXT | LVIF_IMAGE;

            SHGetFileInfo(L"Icon", FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO),
                          SHGFI_USEFILEATTRIBUTES | SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
            lvi.iImage = shfi.iIcon;

            // [2e]: History dialog: focus third (second) item on show #209
            const int iCount = MRU_Enum(pFileMRU, 0, NULL, 0);
            int iOpenedItem = -1;
            for (i = 0; i < iCount; i++)
            {
              MRU_Enum(pFileMRU, i, tch, COUNTOF(tch));
              PathAbsoluteFromApp(tch, NULL, 0, TRUE);
              lvi.iItem = i;
              lvi.pszText = tch;
              ListView_InsertItem(GetDlgItem(hwnd, IDC_FILEMRU), &lvi);
              if ((iOpenedItem < 0) && (lstrcmp(tch, szCurFile) == 0))
              {
                iOpenedItem = i;
              }
            }
            const int iSelectedItem = ((iCount > 1) && (iOpenedItem < iCount - 1))
                                      ? min(iOpenedItem + 2, iCount - 1)
                                      : 0;
            ListView_SetItemState(GetDlgItem(hwnd, IDC_FILEMRU), iSelectedItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
            // [/2e]
            ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_FILEMRU), 0, LVSCW_AUTOSIZE_USEHEADER);

            CreateThread(NULL, 0, FileMRUIconThread, (LPVOID)lpit, 0, &dwtid);
          }
          break;

        case IDC_FILEMRU:
          break;


        case IDOK: {
            WCHAR tch[MAX_PATH];

            if (ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_FILEMRU)))
            {
              LV_ITEM lvi;
              ZeroMemory(&lvi, sizeof(LV_ITEM));

              lvi.mask = LVIF_TEXT;
              lvi.pszText = tch;
              lvi.cchTextMax = COUNTOF(tch);
              lvi.iItem = ListView_GetNextItem(GetDlgItem(hwnd, IDC_FILEMRU), -1, LVNI_ALL | LVNI_SELECTED);
 
              ListView_GetItem(GetDlgItem(hwnd, IDC_FILEMRU), &lvi);

              PathUnquoteSpaces(tch);

              if (!PathFileExists(tch))
              {
                if (IDYES == MsgBox(MBYESNO, IDS_ERR_MRUDLG))
                {
                  MRU_Delete(pFileMRU, lvi.iItem);
                  MRU_DeleteFileFromStore(pFileMRU, tch);
                  // must use IDM_VIEW_REFRESH, index might change...
                  SendMessage(hwnd, WM_COMMAND, MAKELONG(0x00A0, 1), 0);
                  EnableWindow(GetDlgItem(hwnd, IDOK),
                               ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_FILEMRU)));
                }
              }
              else
              {
                lstrcpy((LPWSTR)GetWindowLongPtr(hwnd, DWLP_USER), tch);
                EndDialog(hwnd, IDOK);
              }
            }
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  FileMRUDlg()
//
//
BOOL FileMRUDlg(HWND hwnd, LPWSTR lpstrFile)
{
  if (IDOK == ThemedDialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_FILEMRU),
                                   hwnd, FileMRUDlgProc, (LPARAM)lpstrFile))
    return TRUE;
  else
    return FALSE;
}


//=============================================================================
//
//  ChangeNotifyDlgProc()
//
//  Controls: 100 Radio Button
//            101 Radio Button
//            102 Radio Button
//            103 Check Box
//
extern int iFileWatchingMode;
extern BOOL bResetFileWatching;

INT_PTR CALLBACK ChangeNotifyDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG:
      CheckRadioButton(hwnd, 100, 102, 100 + iFileWatchingMode);
      if (bResetFileWatching)
      {
        CheckDlgButton(hwnd, 103, BST_CHECKED);
      }
      DPI_INIT();
      CenterDlgInParent(hwnd);
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
          if (IsDlgButtonChecked(hwnd, 100) == BST_CHECKED)
            iFileWatchingMode = 0;
          else if (IsDlgButtonChecked(hwnd, 101) == BST_CHECKED)
            iFileWatchingMode = 1;
          else
            iFileWatchingMode = 2;
          bResetFileWatching = (IsDlgButtonChecked(hwnd, 103) == BST_CHECKED) ? TRUE : FALSE;
          EndDialog(hwnd, IDOK);
          break;

        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  ChangeNotifyDlg()
//
BOOL ChangeNotifyDlg(HWND hwnd)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCEW(IDD_CHANGENOTIFY),
    hwnd,
    ChangeNotifyDlgProc,
    0);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  ColumnWrapDlgProc()
//
//  Controls: 100 Edit
//
INT_PTR CALLBACK ColumnWrapDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  static int *piNumber;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {

        piNumber = (int *)lParam;

        SetDlgItemInt(hwnd, 100, *piNumber, FALSE);
        SendDlgItemMessage(hwnd, 100, EM_LIMITTEXT, 15, 0);

        DPI_INIT();
        CenterDlgInParent(hwnd);

      }
      return TRUE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        case IDOK: {

            BOOL fTranslated;

            int iNewNumber = GetDlgItemInt(hwnd, 100, &fTranslated, FALSE);

            if (fTranslated)
            {
              *piNumber = iNewNumber;

              EndDialog(hwnd, IDOK);
            }

            else
              PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, 100)), 1);
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  ColumnWrapDlg()
//
BOOL ColumnWrapDlg(HWND hwnd, UINT uidDlg, int *iNumber)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(uidDlg),
    hwnd,
    ColumnWrapDlgProc, (LPARAM)iNumber);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  WordWrapSettingsDlgProc()
//
//  Controls: 100 Combo
//            101 Combo
//            102 Combo
//            103 Combo
//            200 Text
//            201 Text
//            202 Text
//            203 Text
//
extern int  iWordWrapMode;
extern int  iWordWrapIndent;
extern int  iWordWrapSymbols;
extern BOOL bShowWordWrapSymbols;

INT_PTR CALLBACK WordWrapSettingsDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {

        WCHAR tch[512];
        WCHAR *p1, *p2;
        int i;

        for (i = 0; i < 4; i++)
        {
          GetDlgItemText(hwnd, 200 + i, tch, COUNTOF(tch));
          lstrcat(tch, L"|");
          p1 = tch;
          while (p2 = StrChr(p1, L'|'))
          {
            *p2++ = L'\0';
            if (*p1)
              SendDlgItemMessage(hwnd, 100 + i, CB_ADDSTRING, 0, (LPARAM)p1);
            p1 = p2;
          }
          SendDlgItemMessage(hwnd, 100 + i, CB_SETEXTENDEDUI, TRUE, 0);
        }

        SendDlgItemMessage(hwnd, 100, CB_SETCURSEL, (WPARAM)iWordWrapIndent, 0);
        SendDlgItemMessage(hwnd, 101, CB_SETCURSEL, (WPARAM)(bShowWordWrapSymbols) ? iWordWrapSymbols % 10 : 0, 0);
        SendDlgItemMessage(hwnd, 102, CB_SETCURSEL,
          (WPARAM)(bShowWordWrapSymbols) ? ((iWordWrapSymbols % 100) - (iWordWrapSymbols % 10)) / 10 : 0, 0);
        SendDlgItemMessage(hwnd, 103, CB_SETCURSEL, (WPARAM)iWordWrapMode, 0);

        DPI_INIT();
        CenterDlgInParent(hwnd);

      }
      return TRUE;


    case WM_COMMAND:
      switch (LOWORD(wParam))
      {

        case IDOK: {

            int iSel, iSel2;

            iSel = (int)SendDlgItemMessage(hwnd, 100, CB_GETCURSEL, 0, 0);
            iWordWrapIndent = iSel;

            bShowWordWrapSymbols = FALSE;
            iSel = (int)SendDlgItemMessage(hwnd, 101, CB_GETCURSEL, 0, 0);
            iSel2 = (int)SendDlgItemMessage(hwnd, 102, CB_GETCURSEL, 0, 0);
            if (iSel > 0 || iSel2 > 0)
            {
              bShowWordWrapSymbols = TRUE;
              iWordWrapSymbols = iSel + iSel2 * 10;
            }

            iSel = (int)SendDlgItemMessage(hwnd, 103, CB_GETCURSEL, 0, 0);
            iWordWrapMode = iSel;

            EndDialog(hwnd, IDOK);
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  WordWrapSettingsDlg()
//
BOOL WordWrapSettingsDlg(HWND hwnd, UINT uidDlg, int *iNumber)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(uidDlg),
    hwnd,
    WordWrapSettingsDlgProc, (LPARAM)iNumber);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  LongLineSettingsDlgProc()
//
//  Controls: 100 Edit
//            101 Radio1
//            102 Radio2
//
extern int iLongLineMode;

INT_PTR CALLBACK LongLineSettingsDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  static int *piNumber;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {

        piNumber = (int *)lParam;

        SetDlgItemInt(hwnd, 100, *piNumber, FALSE);
        SendDlgItemMessage(hwnd, 100, EM_LIMITTEXT, 15, 0);

        if (iLongLineMode == EDGE_LINE)
          CheckRadioButton(hwnd, 101, 102, 101);
        else
          CheckRadioButton(hwnd, 101, 102, 102);

        DPI_INIT();
        CenterDlgInParent(hwnd);

      }
      return TRUE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        case IDOK: {

            BOOL fTranslated;

            int iNewNumber = GetDlgItemInt(hwnd, 100, &fTranslated, FALSE);

            if (fTranslated)
            {
              *piNumber = iNewNumber;

              iLongLineMode = (IsDlgButtonChecked(hwnd, 101)) ? EDGE_LINE : EDGE_BACKGROUND;

              EndDialog(hwnd, IDOK);
            }

            else
              PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, 100)), 1);

          }
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  LongLineSettingsDlg()
//
BOOL LongLineSettingsDlg(HWND hwnd, UINT uidDlg, int *iNumber)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(uidDlg),
    hwnd,
    LongLineSettingsDlgProc, (LPARAM)iNumber);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  TabSettingsDlgProc()
//
//  Controls: 100 Edit
//            101 Edit
//            102 Check
//            103 Check
//            104 Check
//
extern int iTabWidth;
extern int iIndentWidth;
extern BOOL bTabsAsSpaces;
extern BOOL bTabIndents;
extern BOOL bBackspaceUnindents;

INT_PTR CALLBACK TabSettingsDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {

        SetDlgItemInt(hwnd, 100, iTabWidth, FALSE);
        SendDlgItemMessage(hwnd, 100, EM_LIMITTEXT, 15, 0);

        SetDlgItemInt(hwnd, 101, iIndentWidth, FALSE);
        SendDlgItemMessage(hwnd, 101, EM_LIMITTEXT, 15, 0);

        if (bTabsAsSpaces)
          CheckDlgButton(hwnd, 102, BST_CHECKED);

        if (bTabIndents)
          CheckDlgButton(hwnd, 103, BST_CHECKED);

        if (bBackspaceUnindents)
          CheckDlgButton(hwnd, 104, BST_CHECKED);

        DPI_INIT();
        CenterDlgInParent(hwnd);

      }
      return TRUE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        case IDOK: {

            BOOL fTranslated1, fTranslated2;

            int iNewTabWidth = GetDlgItemInt(hwnd, 100, &fTranslated1, FALSE);
            int iNewIndentWidth = GetDlgItemInt(hwnd, 101, &fTranslated2, FALSE);

            if (fTranslated1 && fTranslated2)
            {
              iTabWidth = iNewTabWidth;
              iIndentWidth = iNewIndentWidth;

              bTabsAsSpaces = (IsDlgButtonChecked(hwnd, 102)) ? TRUE : FALSE;

              bTabIndents = (IsDlgButtonChecked(hwnd, 103)) ? TRUE : FALSE;

              bBackspaceUnindents = (IsDlgButtonChecked(hwnd, 104)) ? TRUE : FALSE;

              EndDialog(hwnd, IDOK);
            }

            else
              PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, (fTranslated1) ? 101 : 100)), 1);
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  TabSettingsDlg()
//
BOOL TabSettingsDlg(HWND hwnd, UINT uidDlg, int *iNumber)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(uidDlg),
    hwnd,
    TabSettingsDlgProc, (LPARAM)iNumber);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  SelectDefEncodingDlgProc()
//
//
typedef struct encodedlg
{
  BOOL bRecodeOnly;
  int  idEncoding;
  int  cxDlg;
  int  cyDlg;
} ENCODEDLG, *PENCODEDLG;

INT_PTR CALLBACK SelectDefEncodingDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static PENCODEDLG pdd;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        HBITMAP hbmp;
        HIMAGELIST himl;

        pdd = (PENCODEDLG)lParam;

        hbmp = LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_ENCODING), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        himl = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
        ImageList_AddMasked(himl, hbmp, CLR_DEFAULT);
        DeleteObject(hbmp);
        SendDlgItemMessage(hwnd, IDC_ENCODINGLIST, CBEM_SETIMAGELIST, 0, (LPARAM)himl);
        SendDlgItemMessage(hwnd, IDC_ENCODINGLIST, CB_SETEXTENDEDUI, TRUE, 0);
        Encoding_AddToComboboxEx(GetDlgItem(hwnd, IDC_ENCODINGLIST), pdd->idEncoding, 0);

        if (bSkipUnicodeDetection)
          CheckDlgButton(hwnd, IDC_NOUNICODEDETECTION, BST_CHECKED);

        if (bLoadASCIIasUTF8)
          CheckDlgButton(hwnd, IDC_ASCIIASUTF8, BST_CHECKED);

        if (bNoEncodingTags)
          CheckDlgButton(hwnd, IDC_ENCODINGFROMFILEVARS, BST_CHECKED);

        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK: {
            if (Encoding_GetFromComboboxEx(GetDlgItem(hwnd, IDC_ENCODINGLIST), &pdd->idEncoding))
            {
              bSkipUnicodeDetection = (IsDlgButtonChecked(hwnd, IDC_NOUNICODEDETECTION) == BST_CHECKED) ? 1 : 0;
              bLoadASCIIasUTF8 = (IsDlgButtonChecked(hwnd, IDC_ASCIIASUTF8) == BST_CHECKED) ? 1 : 0;
              bNoEncodingTags = (IsDlgButtonChecked(hwnd, IDC_ENCODINGFROMFILEVARS) == BST_CHECKED) ? 1 : 0;
              EndDialog(hwnd, IDOK);
            }
            else
              PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_ENCODINGLIST)), 1);
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  SelectDefEncodingDlg()
//
BOOL SelectDefEncodingDlg(HWND hwnd, int *pidREncoding)
{
  INT_PTR iResult;
  ENCODEDLG dd;

  dd.bRecodeOnly = FALSE;
  dd.idEncoding = *pidREncoding;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(IDD_DEFENCODING),
    hwnd,
    SelectDefEncodingDlgProc,
    (LPARAM)&dd);

  if (iResult == IDOK)
  {
    *pidREncoding = dd.idEncoding;
    return (TRUE);
  }
  else
    return (FALSE);
}


//=============================================================================
//
//  SelectEncodingDlgProc()
//
//
INT_PTR CALLBACK SelectEncodingDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  static PENCODEDLG pdd;
  static HWND hwndLV;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };
        HBITMAP hbmp;
        HIMAGELIST himl;

        pdd = (PENCODEDLG)lParam;

        ResizeDlg_Init(hwnd, pdd->cxDlg, pdd->cyDlg, IDC_RESIZEGRIP4);

        hwndLV = GetDlgItem(hwnd, IDC_ENCODINGLIST);

        hbmp = LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_ENCODING), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        himl = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
        ImageList_AddMasked(himl, hbmp, CLR_DEFAULT);
        DeleteObject(hbmp);
        ListView_SetImageList(GetDlgItem(hwnd, IDC_ENCODINGLIST), himl, LVSIL_SMALL);

        ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV, 0, &lvc);

        Encoding_AddToListView(hwndLV, pdd->idEncoding, pdd->bRecodeOnly);

        ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE_USEHEADER);

        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      ResizeDlg_Destroy(hwnd, &pdd->cxDlg, &pdd->cyDlg);
      return FALSE;
    case WM_SIZE: {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd, lParam, &dx, &dy);

        hdwp = BeginDeferWindowPos(4);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_RESIZEGRIP4, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDOK, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDCANCEL, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_ENCODINGLIST, dx, dy, SWP_NOMOVE);
        EndDeferWindowPos(hdwp);
        ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_ENCODINGLIST), 0, LVSCW_AUTOSIZE_USEHEADER);
        DPI_RESIZE();
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd, lParam);
      DPI_GETMINMAXINFO();
      return TRUE;


    case WM_NOTIFY: {
        if (((LPNMHDR)(lParam))->idFrom == IDC_ENCODINGLIST)
        {

          switch (((LPNMHDR)(lParam))->code)
          {

            case NM_DBLCLK:
              SendMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, 1), 0);
              break;

            case LVN_ITEMCHANGED:
            case LVN_DELETEITEM: {
                int i = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
                EnableWindow(GetDlgItem(hwnd, IDOK), i != -1);
              }
              break;
          }
        }
      }
      return TRUE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        case IDOK:
          if (Encoding_GetFromListView(hwndLV, &pdd->idEncoding))
            EndDialog(hwnd, IDOK);
          else
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_ENCODINGLIST)), 1);
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  SelectEncodingDlg()
//
extern int cxEncodingDlg;
extern int cyEncodingDlg;

BOOL SelectEncodingDlg(HWND hwnd, int *pidREncoding)
{

  INT_PTR iResult;
  ENCODEDLG dd;

  dd.bRecodeOnly = FALSE;
  dd.idEncoding = *pidREncoding;
  dd.cxDlg = cxEncodingDlg;
  dd.cyDlg = cyEncodingDlg;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(IDD_ENCODING),
    hwnd,
    SelectEncodingDlgProc,
    (LPARAM)&dd);

  cxEncodingDlg = dd.cxDlg;
  cyEncodingDlg = dd.cyDlg;

  if (iResult == IDOK)
  {
    *pidREncoding = dd.idEncoding;
    return (TRUE);
  }
  else
    return (FALSE);
}


//=============================================================================
//
//  RecodeDlg()
//
extern int cxRecodeDlg;
extern int cyRecodeDlg;

BOOL RecodeDlg(HWND hwnd, int *pidREncoding)
{

  INT_PTR iResult;
  ENCODEDLG dd;

  dd.bRecodeOnly = TRUE;
  dd.idEncoding = *pidREncoding;
  dd.cxDlg = cxRecodeDlg;
  dd.cyDlg = cyRecodeDlg;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(IDD_RECODE),
    hwnd,
    SelectEncodingDlgProc,
    (LPARAM)&dd);

  cxRecodeDlg = dd.cxDlg;
  cyRecodeDlg = dd.cyDlg;

  if (iResult == IDOK)
  {
    *pidREncoding = dd.idEncoding;
    return (TRUE);
  }
  else
    return (FALSE);
}


//=============================================================================
//
//  SelectDefLineEndingDlgProc()
//
//  Controls: 100 Combo
//            IDC_CONSISTENTEOLS
//            IDC_AUTOSTRIPBLANKS
//
INT_PTR CALLBACK SelectDefLineEndingDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static int *piOption;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        int i;
        WCHAR wch[128];

        piOption = (int *)lParam;

        // Load options
        for (i = 0; i < 3; i++)
        {
          GetString(IDS_EOLMODENAME0 + i, wch, COUNTOF(wch));
          SendDlgItemMessage(hwnd, 100, CB_ADDSTRING, 0, (LPARAM)wch);
        }

        SendDlgItemMessage(hwnd, 100, CB_SETCURSEL, (WPARAM)*piOption, 0);
        SendDlgItemMessage(hwnd, 100, CB_SETEXTENDEDUI, TRUE, 0);

        if (bFixLineEndings)
          CheckDlgButton(hwnd, IDC_CONSISTENTEOLS, BST_CHECKED);

        if (bAutoStripBlanks)
          CheckDlgButton(hwnd, IDC_AUTOSTRIPBLANKS, BST_CHECKED);

        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK: {
            *piOption = (int)SendDlgItemMessage(hwnd, 100, CB_GETCURSEL, 0, 0);
            bFixLineEndings = (IsDlgButtonChecked(hwnd, IDC_CONSISTENTEOLS) == BST_CHECKED) ? 1 : 0;
            bAutoStripBlanks = (IsDlgButtonChecked(hwnd, IDC_AUTOSTRIPBLANKS) == BST_CHECKED) ? 1 : 0;
            EndDialog(hwnd, IDOK);
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  SelectDefLineEndingDlg()
//
BOOL SelectDefLineEndingDlg(HWND hwnd, int *iOption)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(IDD_DEFEOLMODE),
    hwnd,
    SelectDefLineEndingDlgProc,
    (LPARAM)iOption);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  InfoBoxDlgProc()
//
//
typedef struct _infobox
{
  LPWSTR lpstrMessage;
  LPWSTR lpstrSetting;
  BOOL   bDisableCheckBox;
} INFOBOX, *LPINFOBOX;

INT_PTR CALLBACK InfoBoxDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  LPINFOBOX lpib;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG:
      lpib = (LPINFOBOX)lParam;
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      SendDlgItemMessage(hwnd, IDC_INFOBOXICON, STM_SETICON,
        (WPARAM)LoadIcon(NULL, IDI_EXCLAMATION), 0);
      SetDlgItemText(hwnd, IDC_INFOBOXTEXT, lpib->lpstrMessage);
      if (lpib->bDisableCheckBox)
        EnableWindow(GetDlgItem(hwnd, IDC_INFOBOXCHECK), FALSE);
      LocalFree(lpib->lpstrMessage);
      DPI_INIT();
      CenterDlgInParent(hwnd);
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
        case IDYES:
        case IDNO:
          lpib = (LPINFOBOX)GetWindowLongPtr(hwnd, DWLP_USER);
          if (IsDlgButtonChecked(hwnd, IDC_INFOBOXCHECK))
            IniSetInt(L"Suppressed Messages", lpib->lpstrSetting, 1);
          EndDialog(hwnd, LOWORD(wParam));
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  InfoBox()
//
//
extern WCHAR szIniFile[MAX_PATH];

INT_PTR InfoBox(int iType, LPCWSTR lpstrSetting, int uidMessage, ...)
{

  HWND hwnd;
  int idDlg = IDD_INFOBOX;
  INFOBOX ib;
  WCHAR wchFormat[512];
  int iMode;

  iMode = IniGetInt(L"Suppressed Messages", lpstrSetting, 0);

  if (lstrlen(lpstrSetting) > 0 && iMode == 1)
    return (iType == MBYESNO) ? IDYES : IDOK;

  if (!GetString(uidMessage, wchFormat, COUNTOF(wchFormat)))
    return (-1);

  ib.lpstrMessage = LocalAlloc(LPTR, 1024 * sizeof(WCHAR));
  wvsprintf(ib.lpstrMessage, wchFormat, (LPVOID)((PUINT_PTR)&uidMessage + 1));
  ib.lpstrSetting = (LPWSTR)lpstrSetting;
  ib.bDisableCheckBox = (lstrlen(szIniFile) == 0 || lstrlen(lpstrSetting) == 0 || iMode == 2) ? TRUE : FALSE;

  if (iType == MBYESNO)
    idDlg = IDD_INFOBOX2;
  else if (iType == MBOKCANCEL)
    idDlg = IDD_INFOBOX3;

  if (!(hwnd = GetFocus()))
    hwnd = hwndMain;

  MessageBeep(MB_ICONEXCLAMATION);

  return ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(idDlg),
    hwnd,
    InfoBoxDlgProc,
    (LPARAM)&ib);
}

// [2e]: View > St&arting Line Number... #342
INT_PTR CALLBACK StartingLineNumberDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        DPI_INIT();
        SendDlgItemMessage(hwnd, IDC_LINENUM, EM_LIMITTEXT, 7, 0);
        SetDlgItemInt(hwnd, IDC_LINENUM, iStartingLineNumber, TRUE);
        n2e_EnforceSignedIntegerEdit(GetDlgItem(hwnd, 100));
        CenterDlgInParent(hwnd);
      }
      return TRUE;

    case WM_DESTROY:
      return FALSE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
          {
            BOOL fTranslated;
            iStartingLineNumber = GetDlgItemInt(hwnd, IDC_LINENUM, &fTranslated, TRUE);
            if (fTranslated)
            {
              EndDialog(hwnd, IDOK);
            }
            else
            {
              PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, 100)), 1);
            }
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
      }
      return TRUE;
  }
  return FALSE;
}

// [2e]: New command: Show Outline #432
typedef struct outlinedlg
{
  BOOL bForceGoOutline;
  int  iLine;
  int  cxDlg;
  int  cyDlg;

} OUTLINEDLG, *POUTLINEDLG;

void Outline_AddToListView(HWND hwndListView, int iLineFrom)
{
  n2e_GetPreviousFoldLevels(hwndListView, iLineFrom);
  n2e_SelectListViewItem(hwndListView, ListView_GetItemCount(hwndListView) - 1);
}

BOOL Outline_GetFromListView(HWND hwnd, int *piLine)
{
  LVITEM lvi;

  lvi.iItem = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
  lvi.iSubItem = 0;
  lvi.mask = LVIF_PARAM;
  if (ListView_GetItem(hwnd, &lvi))
  {
    *piLine = (int)lvi.lParam;
    return (TRUE);
  }
  return (FALSE);
}

INT_PTR CALLBACK ShowOutlineDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static POUTLINEDLG pod;
  static HWND hwndLV;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        pod = (POUTLINEDLG)lParam;

        ResizeDlg_Init(hwnd, pod->cxDlg, pod->cyDlg, IDC_RESIZEGRIP4);

        hwndLV = GetDlgItem(hwnd, IDC_OUTLINELIST);

        ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV, 0, &lvc);

        Outline_AddToListView(hwndLV, pod->iLine);

        ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE_USEHEADER);

        DPI_INIT();
        CenterDlgInParent(hwnd);

        if (pod->bForceGoOutline)
        {
          PostMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, 1), 0);
        }
      }
      return TRUE;


    case WM_DESTROY:
      ResizeDlg_Destroy(hwnd, &pod->cxDlg, &pod->cyDlg);
      return FALSE;

    case WM_SIZE: {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd, lParam, &dx, &dy);

        hdwp = BeginDeferWindowPos(4);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_RESIZEGRIP4, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDOK, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDCANCEL, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_OUTLINELIST, dx, dy, SWP_NOMOVE);
        EndDeferWindowPos(hdwp);
        ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_OUTLINELIST), 0, LVSCW_AUTOSIZE_USEHEADER);
        DPI_RESIZE();
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd, lParam);
      DPI_GETMINMAXINFO();
      return TRUE;


    case WM_NOTIFY: {
        if (((LPNMHDR)(lParam))->idFrom == IDC_OUTLINELIST)
        {

          switch (((LPNMHDR)(lParam))->code)
          {

            case NM_DBLCLK:
              SendMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, 1), 0);
              break;

            case LVN_ITEMCHANGED:
            case LVN_DELETEITEM: {
                int i = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
                EnableWindow(GetDlgItem(hwnd, IDOK), i != -1);
              }
              break;
          }
        }
      }
      return TRUE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        case IDOK:
          if (Outline_GetFromListView(hwndLV, &pod->iLine))
            EndDialog(hwnd, IDOK);
          else
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_OUTLINELIST)), 1);
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return TRUE;

    case WM_SYSCOMMAND:
      if (wParam == SC_KEYMENU) // [n2e]: handle F10 key
      {
        const int iItem = ListView_GetSelectionMark(hwndLV);
        if (iItem > 0)
        {
          SetFocus(hwndLV);
          n2e_SelectListViewItem(hwndLV, iItem - 1);
        }
        return TRUE;
      }
      break;
  }

  return FALSE;

}

extern int cxOutlineDlg;
extern int cyOutlineDlg;

BOOL ShowOutlineDlg(HWND hwnd, int* piLine, const BOOL bForceGoOutline)
{
  RECT rcMain;
  GetWindowRect(hwnd, &rcMain);

  INT_PTR iResult;
  OUTLINEDLG od;

  od.bForceGoOutline = bForceGoOutline;
  od.iLine = *piLine;
  od.cxDlg = max(cyOutlineDlg, max(rcMain.right - rcMain.left - 64, 300));
  od.cyDlg = cyOutlineDlg;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCE(IDD_OUTLINE),
    hwnd,
    ShowOutlineDlgProc,
    (LPARAM)&od);

  cxOutlineDlg = od.cxDlg;
  cyOutlineDlg = od.cyDlg;

  if (iResult == IDOK)
  {
    *piLine = od.iLine;
    return (TRUE);
  }
  else
    return (FALSE);
}
// [/2e]
