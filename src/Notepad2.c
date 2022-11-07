/******************************************************************************
*
*
* Notepad2
*
* Notepad2.c
*   Main application window functionality
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
#define _WIN32_WINNT 0x501
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "scintilla.h"
#include "scilexer.h"
#include "notepad2.h"
#include "edit.h"
#include "styles.h"
#include "dialogs.h"
#include "helpers.h"
#include "resource.h"
#include "SciCall.h"
#include "Extension/CommonUtils.h"
#include "Extension/DPIHelper.h"
#include "Extension/DPIHelperScintilla.h"
#include "Extension/EditHelper.h"
#include "Extension/EditHelperEx.h"
#include "Extension/ExtSelection.h"
#include "Extension/Lexers.h"
#include "Extension/LexerUtils.h"
#include "Extension/MainWndHelper.h"
#include "Extension/ProcessElevationUtils.h"
#include "Extension/InlineProgressBarCtrl.h"
#include "Extension/StringRecoding.h"
#include "Extension/Subclassing.h"
#include "Extension/Utils.h"
#include "Extension/User32Helper.h"
#include "Extension/VersionHelper.h"
#include "Extension/ViewHelper.h"



/******************************************************************************
*
* Local and global Variables for Notepad2.c
*
*/
HWND      hwndStatus;
HWND      hwndToolbar;
HWND      hwndReBar;
HWND      _hwndEdit;
HWND      hwndEditParent;
HWND      hwndEditFrame;
HWND      hwndMain;
HWND      hwndNextCBChain = NULL;
HWND      hDlgFindReplace = NULL;
HWND      hDlgGotoLine = NULL;
// [2e]: Save on deactivate #164
BOOL      bFileSaveInProgress = FALSE;
// [2e]: Open/Save dialogs - configurable filters #258
int       iOpenSaveFilterIndex = 1;
// [2e] : Use non-proportional font in search/replace dialog #381
HFONT     hMonospacedFont = NULL;

#define NUMTOOLBITMAPS  33
#define NUMINITIALTOOLS 29

TBBUTTON  tbbMainWnd[] = { {0, IDT_FILE_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {1, IDT_FILE_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {2, IDT_FILE_BROWSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {3, IDT_FILE_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {0, 0, 0, TBSTYLE_SEP, 0, 0},
    {4, IDT_EDIT_UNDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {5, IDT_EDIT_REDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {0, 0, 0, TBSTYLE_SEP, 0, 0},
    {6, IDT_EDIT_CUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {7, IDT_EDIT_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {8, IDT_EDIT_PASTE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {0, 0, 0, TBSTYLE_SEP, 0, 0},
    {ICON_FIND_OK, IDT_EDIT_FIND, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {10, IDT_EDIT_REPLACE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {0, 0, 0, TBSTYLE_SEP, 0, 0},
    {11, IDT_VIEW_WORDWRAP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {32, IDT_VIEW_SELECTEX, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {0, 0, 0, TBSTYLE_SEP, 0, 0},
    {12, IDT_VIEW_ZOOMIN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {13, IDT_VIEW_ZOOMOUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {0, 0, 0, TBSTYLE_SEP, 0, 0},
    {28, IDT_SPLIT_VERTICALLY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {29, IDT_SPLIT_HORIZONTALLY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {30, IDT_CLOSE_SPLIT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {0, 0, 0, TBSTYLE_SEP, 0, 0},
    {14, IDT_VIEW_SCHEME, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {15, IDT_VIEW_SCHEMECONFIG, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {0, 0, 0, TBSTYLE_SEP, 0, 0},
    {31, IDM_VIEW_ALWAYSONTOP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {25, IDT_SETTINGS_SAVE_ON_EXIT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {16, IDT_FILE_EXIT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {17, IDT_FILE_SAVEAS, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {18, IDT_FILE_SAVECOPY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {27, IDT_BINARY_SAFE_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {19, IDT_EDIT_CLEAR, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {20, IDT_FILE_PRINT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {21, IDT_FILE_OPENFAV, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {22, IDT_FILE_ADDTOFAV, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {23, IDT_FILE_OPEN_PREV, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {24, IDT_FILE_OPEN_NEXT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
};

WCHAR      szIniFile[MAX_PATH] = L"";
WCHAR      szIniFile2[MAX_PATH] = L"";
// [2e]: Save on exit and History #101
enum ESaveSettingsMode nSaveSettingsMode = SSM_ALL;
BOOL      bSaveRecentFiles;
BOOL      bSaveFindReplace;
WCHAR      tchLastSaveCopyDir[MAX_PATH] = L"";
WCHAR      tchOpenWithDir[MAX_PATH];
WCHAR      tchFavoritesDir[MAX_PATH];
WCHAR      tchDefaultDir[MAX_PATH];
WCHAR      tchDefaultExtension[64];
WCHAR      tchFileDlgFilters[5 * 1024];
WCHAR      tchToolbarButtons[512];
WCHAR      tchToolbarBitmap[MAX_PATH];
WCHAR      tchToolbarBitmapHot[MAX_PATH];
WCHAR      tchToolbarBitmapDisabled[MAX_PATH];
// [2e]: Resize statusbar groups #304
WCHAR      tchDocPos[MAX_PATH] = { 0 };
enum EPathNameFormat iPathNameFormat = PNM_FILENAMEONLY;
BOOL      fWordWrap;
BOOL      fWordWrapG;
int       iWordWrapMode;
int       iWordWrapIndent;
int       iWordWrapSymbols;
BOOL      bShowWordWrapSymbols;
BOOL      fSelectEx = FALSE;
int       iSelectExMode = 0;
int       posSelectExStart = 0;
BOOL      bMatchBraces;
BOOL      bAutoIndent;
BOOL      bAutoCloseTags;
BOOL      bShowIndentGuides;
BOOL      bHighlightCurrentLine;
BOOL      bTabsAsSpaces;
BOOL      bTabsAsSpacesG;
BOOL      bTabIndents;
BOOL      bTabIndentsG;
BOOL      bBackspaceUnindents;
int       iTabWidth;
int       iTabWidthG;
int       iIndentWidth;
int       iIndentWidthG;
BOOL      bMarkLongLines;
int       iLongLinesLimit;
int       iLongLinesLimitG;
int       iLongLineMode;
int       iWrapCol = 0;
BOOL      bShowSelectionMargin;
BOOL      bShowFirstColumnMargin;
BOOL      bShowLineNumbers;
BOOL      bViewWhiteSpace;
BOOL      bViewEOLs;
int       iDefaultEncoding;
BOOL      bSkipUnicodeDetection;
BOOL      bLoadASCIIasUTF8;
BOOL      bNoEncodingTags;
int       iSrcEncoding = -1;
int       iWeakSrcEncoding = -1;
int       iDefaultEOLMode;
BOOL      bFixLineEndings;
BOOL      bAutoStripBlanks;
int       iPrintHeader;
int       iPrintFooter;
int       iPrintColor;
int       iPrintZoom;
RECT      pagesetupMargin;
// [2e]: Add INI setting to disable file save prompt in Launch > New Window #361
enum ESaveBeforeRunningToolsMode iSaveBeforeRunningTools = SBRT_DISABLED;
int       iFileWatchingMode;
BOOL      bResetFileWatching;
DWORD     dwFileCheckInterval;
DWORD     dwAutoReloadTimeout;
enum EEscFunction iEscFunction = EEF_IGNORE;
BOOL      bEscForCurrentSplitView = TRUE;
BOOL      bAlwaysOnTop;
BOOL      bMinimizeToTray;
BOOL      bTransparentMode;
BOOL      bTransparentModeAvailable;
BOOL      bShowToolbar;
BOOL      bShowStatusbar;

typedef struct _wi
{
  int x;
  int y;
  int cx;
  int cy;
  int max;
} WININFO;

WININFO wi;
BOOL    bStickyWinPos;

BOOL    bIsAppThemed;
int     cyReBar;
int     cyReBarFrame;
int     cxEditFrame;
int     cyEditFrame;

int     cxEncodingDlg;
int     cyEncodingDlg;
int     cxRecodeDlg;
int     cyRecodeDlg;
int     cxFileMRUDlg;
int     cyFileMRUDlg;
int     cxOpenWithDlg;
int     cyOpenWithDlg;
int     cxFavoritesDlg;
int     cyFavoritesDlg;
int     xFindReplaceDlg;
int     yFindReplaceDlg;

LPWSTR      lpFileList[32];
int         cFileList = 0;
int         cchiFileList = 0;
LPWSTR      lpFileArg = NULL;
LPWSTR      lpSchemeArg = NULL;
LPWSTR      lpMatchArg = NULL;
LPWSTR      lpEncodingArg = NULL;
LPMRULIST  pFileMRU;
LPMRULIST  mruFind;
LPMRULIST  mruReplace;

DWORD     dwLastIOError;
WCHAR      szCurFile[MAX_PATH + 40];
FILEVARS   fvCurFile;
BOOL      bModified;
BOOL      bReadOnly = FALSE;
int       iEncoding;
int       iOriginalEncoding;
int       iEOLMode;

int       iDefaultCodePage;
int       iDefaultCharSet;

int       iInitialLine;
int       iInitialColumn;
int       iInitialSelStart = -1;
int       iInitialSelEnd = -1;

int       iInitialLexer;

BOOL      bLastCopyFromMe = FALSE;
DWORD     dwLastCopyTime;

UINT      uidsAppTitle = IDS_APPTITLE;
WCHAR     szTitleExcerpt[128] = L"";
int       fKeepTitleExcerpt = 0;

UINT      msgTaskbarCreated = 0;

HMODULE   hModUxTheme = NULL;
HMODULE   hModRichEdit = NULL;  // [2e]: Attribution menu command #181

EDITFINDREPLACE efrData = { "", "", "", "", 0, 0, 0, 0, 0, 0, SIC_ALWAYS, NULL };
UINT cpLastFind = 0;
BOOL bReplaceInitialized = FALSE;

// [2e]: Find/Replace - add Go to Go To #259
GOTOPARAMS gotoData = { TRUE, &efrData };

extern NP2ENCODING mEncoding[];

int iLineEndings[3] = {
    SC_EOL_CRLF,
    SC_EOL_LF,
    SC_EOL_CR
};

WCHAR wchPrefixSelection[TEXT_BUFFER_LENGTH] = L"";
WCHAR wchAppendSelection[TEXT_BUFFER_LENGTH] = L"";

WCHAR wchPrefixLines[TEXT_BUFFER_LENGTH] = L"";
WCHAR wchAppendLines[TEXT_BUFFER_LENGTH] = L"";

int   iSortOptions = 0;
int   iAlignMode = 0;

BOOL      fIsElevated = FALSE;
WCHAR     wchWndClass[16] = WC_NOTEPAD2;
BOOL      fExpandEnvVariables = FALSE;

HINSTANCE g_hInstance;
UINT16    g_uWinVer;
WCHAR     g_wchAppUserModelID[32] = L"";
WCHAR     g_wchWorkingDirectory[MAX_PATH] = L"";



//=============================================================================
//
// Flags
//
int flagNoReuseWindow = 0;
int flagReuseWindow = 0;
int flagMultiFileArg = 0;
int flagSingleFileInstance = 0;
int flagStartAsTrayIcon = 0;
int flagAlwaysOnTop = 0;
int flagRelativeFileMRU = 0;
int flagPortableMyDocs = 0;
int flagNoFadeHidden = 0;
int flagToolbarLook = 0;
int flagSimpleIndentGuides = 0;
int fNoHTMLGuess = 0;
int fNoCGIGuess = 0;
int fNoFileVariables = 0;
int flagPosParam = 0;
int flagDefaultPos = 0;
int flagNewFromClipboard = 0;
int flagPasteBoard = 0;
int flagSetEncoding = 0;
int flagSetEOLMode = 0;
int flagJumpTo = 0;
int flagMatchText = 0;
int flagChangeNotify = 0;
int flagLexerSpecified = 0;
int flagQuietCreate = 0;
int flagUseSystemMRU = 0;
int flagRelaunchElevated = 0;
int flagDisplayHelp = 0;


//=============================================================================
//
//  WinMain()
//
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{

  MSG msg;
  HWND hwnd;
  HACCEL hAccMain;
  HACCEL hAccFindReplace;
  INITCOMMONCONTROLSEX icex;
  WCHAR wchWorkingDirectory[MAX_PATH];

  // Set global variable g_hInstance
  g_hInstance = hInstance;

  // Set the Windows version global variable
  g_uWinVer = LOWORD(GetVersion());
  g_uWinVer = MAKEWORD(HIBYTE(g_uWinVer), LOBYTE(g_uWinVer));

  // Don't keep working directory locked
  GetCurrentDirectory(COUNTOF(g_wchWorkingDirectory), g_wchWorkingDirectory);
  GetModuleFileName(NULL, wchWorkingDirectory, COUNTOF(wchWorkingDirectory));
  PathRemoveFileSpec(wchWorkingDirectory);
  SetCurrentDirectory(wchWorkingDirectory);
  
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

  // check if running at least on Windows 2000
  if (!Is2k())
  {
    LPVOID lpMsgBuf;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      ERROR_OLD_WIN_VERSION,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPWSTR)&lpMsgBuf,
      0,
      NULL);
    MessageBox(NULL, (LPCWSTR)lpMsgBuf, WC_NOTEPAD2, MB_OK | MB_ICONEXCLAMATION);
    LocalFree(lpMsgBuf);
    return (0);
  }

  // Check if running with elevated privileges
  fIsElevated = IsElevated();

  // Default Encodings (may already be used for command line parsing)
  Encoding_InitDefaults();

  // Command Line, Ini File and Flags
  if (!ParseCommandLine())
  {
    return 0;
  }
  FindIniFile();
  TestIniFile();
  CreateIniFile();
  LoadFlags();

  // set AppUserModelID
  PrivateSetCurrentProcessExplicitAppUserModelID(g_wchAppUserModelID);

  // Command Line Help Dialog
  if (flagDisplayHelp)
  {
    DisplayCmdLineHelp(NULL);
    return (0);
  }

  // Adapt window class name
  if (fIsElevated)
    StrCat(wchWndClass, L"U");
  if (flagPasteBoard)
    StrCat(wchWndClass, L"B");

  // Relaunch with elevated privileges
  if (RelaunchElevated())
    return (0);

  // Try to run multiple instances
  if (RelaunchMultiInst())
    return (0);

  // Try to activate another window
  if (ActivatePrevInst())
    return (0);

  // Init OLE and Common Controls
  OleInitialize(NULL);

  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES;
  InitCommonControlsEx(&icex);

  msgTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");

  hModUxTheme = LoadLibrary(L"uxtheme.dll");
  // [2e]: Attribution menu command #181
  hModRichEdit = LoadLibrary(L"RICHED20.DLL");

  Scintilla_RegisterClasses(hInstance);

  // Load Settings
  LoadSettings();

  if (!InitApplication(hInstance))
    return FALSE;

  // [2e]: DPI awareness #154
  DPIInitialize();

  if (!(hwnd = InitInstance(hInstance, lpCmdLine, nCmdShow)))
    return FALSE;

  hAccMain = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_MAINWND));
  hAccFindReplace = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCFINDREPLACE));
  // [2e]: Ctrl+H: Replace input behaviour #121
  const HACCEL hAccFindReplaceInline = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCFINDREPLACE_INLINE));

  while (GetMessage(&msg, NULL, 0, 0))
  {
    if (IsWindow(hDlgFindReplace) && (msg.hwnd == hDlgFindReplace || IsChild(hDlgFindReplace, msg.hwnd)))
    {
      // [2e]: Ctrl+H: Replace input behaviour #121
      if (n2e_IsSubclassedWindow(msg.hwnd) && TranslateAccelerator(msg.hwnd, hAccFindReplaceInline, &msg))
        continue;
      if (TranslateAccelerator(hDlgFindReplace, hAccFindReplace, &msg) || IsDialogMessage(hDlgFindReplace, &msg))
        continue;
    }

    else if (IsWindow(hDlgGotoLine) && (msg.hwnd == hDlgGotoLine || IsChild(hDlgGotoLine, msg.hwnd)))
    {
      if (TranslateAccelerator(hDlgGotoLine, hAccFindReplace, &msg) || IsDialogMessage(hDlgGotoLine, &msg))
        continue;
    }

    if (!TranslateAccelerator(hwnd, hAccMain, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  // [2e]: separate exit-handler function
  ExitInstance(hInstance);

  if (hModUxTheme)
    FreeLibrary(hModUxTheme);

  // [2e]: Attribution menu command #181
  if (hModRichEdit)
    FreeLibrary(hModRichEdit);

  OleUninitialize();

  return (int)(msg.wParam);

  hPrevInst;

}


//=============================================================================
//
//  InitApplication()
//
//
BOOL InitApplication(HINSTANCE hInstance)
{

  WNDCLASS   wc;

  wc.style         = CS_BYTEALIGNWINDOW | CS_DBLCLKS;
  wc.lpfnWndProc   = (WNDPROC)MainWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDR_MAINWND));
  wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
  wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINWND);
  wc.lpszClassName = wchWndClass;

  return RegisterClass(&wc);

}


//=============================================================================
//
//  InitInstance()
//
//
HWND InitInstance(HINSTANCE hInstance, LPSTR pszCmdLine, int nCmdShow)
{
  RECT rc = { wi.x, wi.y, wi.x + wi.cx, wi.y + wi.cy };
  RECT rc2;
  MONITORINFO mi;
  HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);
  if (flagDefaultPos == 1)
  {
    wi.x = wi.y = wi.cx = wi.cy = CW_USEDEFAULT;
    wi.max = 0;
  }
  else if (flagDefaultPos >= 4)
  {
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    if (flagDefaultPos & 8)
      wi.x = (rc.right - rc.left) / 2;
    else
      wi.x = rc.left;
    wi.cx = rc.right - rc.left;
    if (flagDefaultPos & (4 | 8))
      wi.cx /= 2;
    if (flagDefaultPos & 32)
      wi.y = (rc.bottom - rc.top) / 2;
    else
      wi.y = rc.top;
    wi.cy = rc.bottom - rc.top;
    if (flagDefaultPos & (16 | 32))
      wi.cy /= 2;
    if (flagDefaultPos & 64)
    {
      wi.x = rc.left;
      wi.y = rc.top;
      wi.cx = rc.right - rc.left;
      wi.cy = rc.bottom - rc.top;
    }
    if (flagDefaultPos & 128)
    {
      wi.x += (flagDefaultPos & 8) ? 4 : 8;
      wi.cx -= (flagDefaultPos & (4 | 8)) ? 12 : 16;
      wi.y += (flagDefaultPos & 32) ? 4 : 8;
      wi.cy -= (flagDefaultPos & (16 | 32)) ? 12 : 16;
    }
  }

  else if (flagDefaultPos == 2 || flagDefaultPos == 3 ||
           wi.x == CW_USEDEFAULT || wi.y == CW_USEDEFAULT ||
           wi.cx == CW_USEDEFAULT || wi.cy == CW_USEDEFAULT)
  {
    // default window position
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    wi.y = rc.top + 16;
    wi.cy = rc.bottom - rc.top - 32;
    wi.cx = min(rc.right - rc.left - 32, wi.cy);
    wi.x = (flagDefaultPos == 3) ? rc.left + 16 : rc.right - wi.cx - 16;
  }

  else
  {
    // fit window into working area of current monitor
    wi.x += (mi.rcWork.left - mi.rcMonitor.left);
    wi.y += (mi.rcWork.top - mi.rcMonitor.top);
    if (wi.x < mi.rcWork.left)
      wi.x = mi.rcWork.left;
    if (wi.y < mi.rcWork.top)
      wi.y = mi.rcWork.top;
    if (wi.x + wi.cx > mi.rcWork.right)
    {
      wi.x -= (wi.x + wi.cx - mi.rcWork.right);
      if (wi.x < mi.rcWork.left)
        wi.x = mi.rcWork.left;
      if (wi.x + wi.cx > mi.rcWork.right)
        wi.cx = mi.rcWork.right - wi.x;
    }
    if (wi.y + wi.cy > mi.rcWork.bottom)
    {
      wi.y -= (wi.y + wi.cy - mi.rcWork.bottom);
      if (wi.y < mi.rcWork.top)
        wi.y = mi.rcWork.top;
      if (wi.y + wi.cy > mi.rcWork.bottom)
        wi.cy = mi.rcWork.bottom - wi.y;
    }
    SetRect(&rc, wi.x, wi.y, wi.x + wi.cx, wi.y + wi.cy);
    if (!IntersectRect(&rc2, &rc, &mi.rcWork))
    {
      wi.y = mi.rcWork.top + 16;
      wi.cy = mi.rcWork.bottom - mi.rcWork.top - 32;
      wi.cx = min(mi.rcWork.right - mi.rcWork.left - 32, wi.cy);
      wi.x = mi.rcWork.right - wi.cx - 16;
    }
  }

  hwndMain = CreateWindowEx(
               0,
               wchWndClass,
               WC_NOTEPAD2,
               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
               wi.x,
               wi.y,
               wi.cx,
               wi.cy,
               NULL,
               NULL,
               hInstance,
               NULL);

  n2e_InitInstance();

  if (wi.max)
    nCmdShow = SW_SHOWMAXIMIZED;

  if ((bAlwaysOnTop || flagAlwaysOnTop == 2) && flagAlwaysOnTop != 1)
    SetWindowPos(hwndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  if (bTransparentMode)
    SetWindowTransparentMode(hwndMain, TRUE);

  // Current file information -- moved in front of ShowWindow()
  FileLoad(TRUE, TRUE, FALSE, FALSE, L"");

  if (!flagStartAsTrayIcon)
  {
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

#ifdef LPEG_LEXER
    if (!bLPegEnabled)
    {
      InfoBox(MBWARN, L"MsgLPegNotSet", IDS_ERR_LPEG_NOT_CONFIGURED);
    }
#endif
  }
  else
  {
    ShowWindow(hwndMain, SW_HIDE); // trick ShowWindow()
    ShowNotifyIcon(hwndMain, TRUE);
  }

  // Source Encoding
  if (lpEncodingArg)
    iSrcEncoding = Encoding_MatchW(lpEncodingArg);

  // Pathname parameter
  if (lpFileArg)
  {
    BOOL bOpened = FALSE;

    // Open from Directory
    if (PathIsDirectory(lpFileArg))
    {
      WCHAR tchFile[MAX_PATH];
      if (OpenFileDlg(hwndMain, tchFile, COUNTOF(tchFile), lpFileArg))
        bOpened = FileLoad(FALSE, FALSE, FALSE, FALSE, tchFile);
    }
    else
    {
      if (bOpened = FileLoad(FALSE, FALSE, FALSE, FALSE, lpFileArg))
      {
        if (flagJumpTo)
        { // Jump to position
          // [2e]: Add to Favorites - selection mode #249
          n2e_EditJumpTo(hwndEdit, iInitialLine, iInitialColumn, iInitialSelStart, iInitialSelEnd);
          EditEnsureSelectionVisible(hwndEdit);
        }
      }
    }
    GlobalFree(lpFileArg);

    if (bOpened)
    {
      if (flagChangeNotify == 1)
      {
        iFileWatchingMode = 0;
        bResetFileWatching = TRUE;
        InstallFileWatching(szCurFile);
      }
      else if (flagChangeNotify == 2)
      {
        iFileWatchingMode = 2;
        bResetFileWatching = TRUE;
        InstallFileWatching(szCurFile);
      }
    }
  }
  else
  {
    if (iSrcEncoding != -1)
    {
      iEncoding = iSrcEncoding;
      iOriginalEncoding = iSrcEncoding;
      SendMessage(hwndEdit, SCI_SETCODEPAGE, (iEncoding == CPI_DEFAULT) ? iDefaultCodePage : SC_CP_UTF8, 0);
    }
  }

  // reset
  iSrcEncoding = -1;
  flagQuietCreate = 0;
  fKeepTitleExcerpt = 0;

  // Check for /c [if no file is specified] -- even if a file is specified
  if (flagNewFromClipboard)
  {
    if (SendMessage(hwndEdit, SCI_CANPASTE, 0, 0))
    {
      BOOL bAutoIndent2 = bAutoIndent;
      bAutoIndent = 0;
      EditJumpTo(hwndEdit, -1, 0);
      SendMessage(hwndEdit, SCI_BEGINUNDOACTION, 0, 0);
      if (SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0) > 0)
        SendMessage(hwndEdit, SCI_NEWLINE, 0, 0);
      SendMessage(hwndEdit, SCI_PASTE, 0, 0);
      SendMessage(hwndEdit, SCI_NEWLINE, 0, 0);
      SendMessage(hwndEdit, SCI_ENDUNDOACTION, 0, 0);
      bAutoIndent = bAutoIndent2;
      if (flagJumpTo)
        // [2e]: Add to Favorites - selection mode #249
        n2e_EditJumpTo(hwndEdit, iInitialLine, iInitialColumn, iInitialSelStart, iInitialSelEnd);
      EditEnsureSelectionVisible(hwndEdit);
    }
  }

  // Encoding
  if (0 != flagSetEncoding)
  {
    SendMessage(
      hwndMain,
      WM_COMMAND,
      MAKELONG(IDM_ENCODING_ANSI + flagSetEncoding - 1, 1),
      0);
    flagSetEncoding = 0;
  }

  // EOL mode
  if (0 != flagSetEOLMode)
  {
    SendMessage(
      hwndMain,
      WM_COMMAND,
      MAKELONG(IDM_LINEENDINGS_CRLF + flagSetEOLMode - 1, 1),
      0);
    flagSetEOLMode = 0;
  }

  // Match Text
  if (flagMatchText && lpMatchArg)
  {
    if (lstrlen(lpMatchArg) && SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0))
    {
      UINT cp = (UINT)SendMessage(hwndEdit, SCI_GETCODEPAGE, 0, 0);
      WideCharToMultiByte(cp, 0, lpMatchArg, -1, efrData.szFind, COUNTOF(efrData.szFind), NULL, NULL);
      WideCharToMultiByte(CP_UTF8, 0, lpMatchArg, -1, efrData.szFindUTF8, COUNTOF(efrData.szFindUTF8), NULL, NULL);
      cpLastFind = cp;

      if (flagMatchText & 4)
        efrData.fuFlags |= SCFIND_REGEXP | SCFIND_POSIX;
      else if (flagMatchText & 8)
        efrData.bTransformBS = TRUE;

      if (flagMatchText & 2)
      {
        if (!flagJumpTo)
          EditJumpTo(hwndEdit, -1, 0);
        EditFindPrev(hwndEdit, &efrData, FALSE);
        EditEnsureSelectionVisible(hwndEdit);
      }
      else
      {
        if (!flagJumpTo)
          SendMessage(hwndEdit, SCI_DOCUMENTSTART, 0, 0);
        EditFindNext(hwndEdit, &efrData, FALSE);
        EditEnsureSelectionVisible(hwndEdit);
      }
    }
    GlobalFree(lpMatchArg);
  }

  // Check for Paste Board option -- after loading files
  if (flagPasteBoard)
  {
    bLastCopyFromMe = TRUE;
    hwndNextCBChain = SetClipboardViewer(hwndMain);
    n2e_UpdateWindowTitle(hwndMain);
    bLastCopyFromMe = FALSE;

    dwLastCopyTime = 0;
    SetTimer(hwndMain, ID_PASTEBOARDTIMER, 100, PasteBoardTimer);
  }

  // check if a lexer was specified from the command line
  if (flagLexerSpecified)
  {
    if (lpSchemeArg)
    {
      Style_SetLexerFromName(hwndEdit, szCurFile, lpSchemeArg);
      LocalFree(lpSchemeArg);
    }
    else if (iInitialLexer >= 0 && iInitialLexer < NUMLEXERS)
      Style_SetLexerFromID(hwndEdit, iInitialLexer);
    flagLexerSpecified = 0;
  }

  // If start as tray icon, set current filename as tooltip
  if (flagStartAsTrayIcon)
    SetNotifyIconTitle(hwndMain);

  UpdateToolbar();
  UpdateStatusbar();

  return (hwndMain);

}

// [2e]: separate exit-handler function
void ExitInstance(HINSTANCE hInstance)
{
  n2e_ExitInstance();
  Scintilla_ReleaseResources();
  UnregisterClass(wchWndClass, hInstance);
}
// [/2e]

//=============================================================================
//
//  MainWndProc()
//
//  Messages are distributed to the MsgXXX-handlers
//
//
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static BOOL bShutdownOK;

  switch (umsg)
  {
    // [2e]: DPI awareness #154
    case WM_DPICHANGED:
      n2e_UpdateViewsDPI(wParam);
      DPIChanged_WindowProcHandler(hwnd, wParam, lParam);
      MsgThemeChanged(hwnd, 0, 0);
      return 0;
    // [/2e]


    // Quickly handle painting and sizing messages, found in ScintillaWin.cxx
    // Cool idea, don't know if this has any effect... ;-)
    case WM_MOVE:
    case WM_NCHITTEST:
    case WM_NCCALCSIZE:
    case WM_NCPAINT:
    case WM_PAINT:
    case WM_ERASEBKGND:
    case WM_NCLBUTTONDOWN:
    case WM_WINDOWPOSCHANGING:
    case WM_WINDOWPOSCHANGED:
      return DefWindowProc(hwnd, umsg, wParam, lParam);

    // [2e]: Hide pointer while typing #230
    SET_CURSOR_HANDLER();

    // [2e]: DPI awareness #154
    case WM_NCCREATE:
      DPI_ENABLE_NC_SCALING();
      return (DefWindowProc(hwnd, umsg, wParam, lParam));
    // [/2e]


    // [2e]: Save on deactivate #164
    case WM_ACTIVATE:
      n2e_OnActivateMainWindow(wParam, lParam);
      break;
    // [/2e]


    // [2e]: Edit highlighted word #18
    case WM_ACTIVATEAPP:
      if (!wParam)
      {
        n2e_SaveActiveEdit();
        n2e_SelectionEditStop(hwndEdit, SES_APPLY);
        // [2e]: Save on deactivate #164
        if (!bReadOnly && n2e_IsDocumentModified() && (iSaveOnLoseFocus != SLF_DISABLED) && IsWindowVisible(hwnd)
            && lstrlen(szCurFile) && !bFileSaveInProgress && !n2e_IsModalDialogOnTop())
        {
          FileSave(TRUE, FALSE, FALSE, FALSE, FALSE);
        }
      }
      else
      {
        // [2e]: Split view #316
        n2e_RestoreActiveEdit(FALSE);
      }
      break;
    // [/2e]


    case WM_MOUSEACTIVATE:
      n2e_SelectionEditStop(hwndEdit, SES_APPLY);
      // [2e]: Broken X-Mouse #113
      return DefWindowProc(hwnd, umsg, wParam, lParam);
    // [/2e]


    case WM_CREATE:
      return MsgCreate(hwnd, wParam, lParam);


    case WM_DESTROY:
    case WM_ENDSESSION:
      if (!bShutdownOK)
      {
        WINDOWPLACEMENT wndpl;

        // Terminate file watching
        InstallFileWatching(NULL);

        // GetWindowPlacement
        wndpl.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd, &wndpl);

        wi.x = wndpl.rcNormalPosition.left;
        wi.y = wndpl.rcNormalPosition.top;
        wi.cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
        wi.cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
        wi.max = (IsZoomed(hwnd) || (wndpl.flags & WPF_RESTORETOMAXIMIZED));

        DragAcceptFiles(hwnd, FALSE);

        // Terminate clipboard watching
        if (flagPasteBoard)
        {
          KillTimer(hwnd, ID_PASTEBOARDTIMER);
          ChangeClipboardChain(hwnd, hwndNextCBChain);
        }

        // Destroy find / replace dialog
        if (IsWindow(hDlgFindReplace))
          DestroyWindow(hDlgFindReplace);

        if (IsWindow(hDlgGotoLine))
          DestroyWindow(hDlgGotoLine);

        // call SaveSettings() when hwndToolbar is still valid
        SaveSettings(FALSE);

        MRU_Destroy(pFileMRU);
        MRU_Destroy(mruFind);
        MRU_Destroy(mruReplace);

        // Remove tray icon if necessary
        ShowNotifyIcon(hwnd, FALSE);

        bShutdownOK = TRUE;
      }
      if (umsg == WM_DESTROY)
        PostQuitMessage(0);
      break;


    case WM_CLOSE:
      if (FileSave(FALSE, TRUE, FALSE, FALSE, FALSE))
        DestroyWindow(hwnd);
      break;


    case WM_QUERYENDSESSION:
      // [2e]: Customize Windows shutdown prevention message on modified file #422
      if (bModified && (lstrlen(szCurFile) > 0))
      {
        WCHAR tchs[MAX_STR_BLOCKREASON] = { 0 };
        FormatString(tchs, COUNTOF(tchs), IDS_UNSAVED_FILENAME, PathFindFileName(szCurFile));
        n2e_ShutdownBlockReasonCreate(hwndMain, tchs);
      }
      // [/2e]
      if (FileSave(FALSE, TRUE, FALSE, FALSE, FALSE))
        return TRUE;
      else
        return FALSE;


    // Reinitialize theme-dependent values and resize windows
    case WM_THEMECHANGED:
      MsgThemeChanged(hwnd, wParam, lParam);
      break;


    // [2e]: Language indication #86
    case WM_INPUTLANGCHANGE:
      n2e_UpdateWindowTitle(hwnd);
      break;
    // [/2e]


    // update Scintilla colors
    case WM_SYSCOLORCHANGE: {
        extern PEDITLEXER pLexCurrent;
        Style_SetLexer(hwndEdit, pLexCurrent);
        return DefWindowProc(hwnd, umsg, wParam, lParam);
      }


    case WM_SIZE:
      MsgSize(hwnd, wParam, lParam);
      break;


    case WM_SETFOCUS:
      n2e_RestoreActiveEdit(FALSE);
      UpdateToolbar();
      UpdateStatusbar();

      break;


    case WM_DROPFILES: {
        WCHAR szBuf[MAX_PATH + 40];
        HDROP hDrop = (HDROP)wParam;
        if (IsIconic(hwnd))
          ShowWindow(hwnd, SW_RESTORE);

        DragQueryFile(hDrop, 0, szBuf, COUNTOF(szBuf));

        if (PathIsDirectory(szBuf))
        {
          WCHAR tchFile[MAX_PATH];
          if (OpenFileDlg(hwndMain, tchFile, COUNTOF(tchFile), szBuf))
            FileLoad(FALSE, FALSE, FALSE, FALSE, tchFile);
        }

        else if (DragQueryFile(hDrop, (UINT)(-1), NULL, 0) == 1)
        {
          FileLoad(FALSE, FALSE, FALSE, FALSE, szBuf);
        }

        // [2e]: Multi-file drop to concatenate all #250
        else
        {
          LPSTR lpData = NULL;
          int iDataLength = 0;
          UINT iTotalFiles = DragQueryFile(hDrop, (UINT)(-1), NULL, 0);
          UINT iFile = 0;
          BOOL bError = FALSE;
          BOOL bUseUTF8 = FALSE;

          SciCall_SetSkipUIUpdate(1);
          for (iFile = 0; iFile < iTotalFiles; iFile++)
          {
            DragQueryFile(hDrop, iFile, szBuf, COUNTOF(szBuf));
            if (!FileLoad(FALSE, FALSE, FALSE, FALSE, szBuf))
            {
              bError = TRUE;
              break;
            }

            int iCurrentDataLength = SciCall_GetLength();
            lpData = n2e_Realloc(lpData, iDataLength + iCurrentDataLength + 1);
            if (!lpData)
            {
              MsgBox(MBWARN, IDS_ERR_MEMORY_ALLOCATION, iDataLength);
              bError = TRUE;
              break;
            }
            if (!bUseUTF8 && (n2e_IsUTF8EncodingMode() || n2e_IsUnicodeEncodingMode()))
            {
              bUseUTF8 = TRUE;
            }
            SciCall_GetText(iCurrentDataLength + 1, lpData + iDataLength);
            iDataLength += iCurrentDataLength;
          }
          if (!bError)
          {
            FileLoad(FALSE, TRUE, FALSE, FALSE, L"");
            if (bUseUTF8)
            {
              iEncoding = CPI_UTF8;
              SendMessage(hwndEdit, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
            }
            SciCall_AddText(iDataLength, lpData);
            SciCall_GotoPos(0);
            iEOLMode = EditDetectEOLMode(hwnd, lpData, iDataLength);
          }
          SciCall_SetSkipUIUpdate(0);
          if (lpData)
          {
            n2e_Free(lpData);
            lpData = NULL;
          }
        }
        // [/2e]

        DragFinish(hDrop);
      }
      break;


    case WM_COPYDATA: {
        PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;

        SetDlgItemInt(hwnd, IDC_REUSELOCK, GetTickCount(), FALSE);

        if (pcds->dwData == DATA_NOTEPAD2_PARAMS)
        {
          LPNP2PARAMS params = LocalAlloc(LPTR, pcds->cbData);
          CopyMemory(params, pcds->lpData, pcds->cbData);
          if (params->flagLexerSpecified)
            flagLexerSpecified = 1;

          if (params->flagQuietCreate)
            flagQuietCreate = 1;

          if (params->flagFileSpecified)
          {
            BOOL bOpened = FALSE;
            iSrcEncoding = params->iSrcEncoding;
            if (PathIsDirectory(&params->wchData))
            {
              WCHAR tchFile[MAX_PATH];
              if (OpenFileDlg(hwndMain, tchFile, COUNTOF(tchFile), &params->wchData))
                bOpened = FileLoad(FALSE, FALSE, FALSE, FALSE, tchFile);
            }

            else
              bOpened = FileLoad(FALSE, FALSE, FALSE, FALSE, &params->wchData);

            if (bOpened)
            {

              if (params->flagChangeNotify == 1)
              {
                iFileWatchingMode = 0;
                bResetFileWatching = TRUE;
                InstallFileWatching(szCurFile);
              }
              else if (params->flagChangeNotify == 2)
              {
                iFileWatchingMode = 2;
                bResetFileWatching = TRUE;
                InstallFileWatching(szCurFile);
              }
              if (0 != params->flagSetEncoding)
              {
                flagSetEncoding = params->flagSetEncoding;
                SendMessage(
                  hwnd,
                  WM_COMMAND,
                  MAKELONG(IDM_ENCODING_ANSI + flagSetEncoding - 1, 1),
                  0);
                flagSetEncoding = 0;
              }
              if (0 != params->flagSetEOLMode)
              {
                flagSetEOLMode = params->flagSetEOLMode;
                SendMessage(
                  hwndMain,
                  WM_COMMAND,
                  MAKELONG(IDM_LINEENDINGS_CRLF + flagSetEOLMode - 1, 1),
                  0);
                flagSetEOLMode = 0;
              }
              if (params->flagLexerSpecified)
              {
                if (params->iInitialLexer < 0)
                {
                  WCHAR wchExt[32] = L".";
                  lstrcpyn(CharNext(wchExt), StrEnd(&params->wchData) + 1, 30);
                  Style_SetLexerFromName(hwndEdit, &params->wchData, wchExt);
                }
                else if (params->iInitialLexer >= 0 && params->iInitialLexer < NUMLEXERS)
                {
                  Style_SetLexerFromID(hwndEdit, params->iInitialLexer);
                }
              }
              if (params->flagTitleExcerpt)
              {
                lstrcpyn(szTitleExcerpt, StrEnd(&params->wchData) + 1, COUNTOF(szTitleExcerpt));
                n2e_UpdateWindowTitle(hwnd);
              }
            }
            // reset
            iSrcEncoding = -1;
          }

          if (params->flagJumpTo)
          {
            // [2e]: Add to Favorites - selection mode #249
            if ((params->iInitialLine == 0) && (params->iInitialSelStart == -1))
              params->iInitialLine = 1;
            n2e_EditJumpTo(hwndEdit, params->iInitialLine, params->iInitialColumn, params->iInitialSelStart, params->iInitialSelEnd);
            EditEnsureSelectionVisible(hwndEdit);
          }

          flagLexerSpecified = 0;
          flagQuietCreate = 0;

          LocalFree(params);

          UpdateStatusbar();
        }
      }
      return TRUE;


    case WM_CONTEXTMENU: {
        HMENU hmenu;
        int imenu = 0;
        POINT pt;
        int nID = GetDlgCtrlID((HWND)wParam);
        if ((nID != IDC_EDIT) && (nID != IDC_STATUSBAR) &&
          (nID != IDC_REBAR) && (nID != IDC_TOOLBAR))
          return DefWindowProc(hwnd, umsg, wParam, lParam);

        pt.x = (int)(short)LOWORD(lParam);
        pt.y = (int)(short)HIWORD(lParam);
        // [2e]: right click for copy-to-clipboard evaluated result
        if (n2e_IsPaneSizePoint(hwnd, pt))
        {
          n2e_OnPaneSizeClick(hwnd, FALSE);
          break;
        }
        // [/2e]

        hmenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUPMENU));
        switch (nID)
        {
          case IDC_EDIT:
            if (bMoveCaretOnRightClick)
            {
              int iSelStart = (int)SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0);
              int iSelEnd = (int)SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0);
              if (iSelStart == iSelEnd && pt.x != -1 && pt.y != -1)
              {
                int iNewPos;
                POINT ptc = { pt.x, pt.y };
                ScreenToClient(hwndEdit, &ptc);
                iNewPos = (int)SendMessage(hwndEdit, SCI_POSITIONFROMPOINT, (WPARAM)ptc.x, (LPARAM)ptc.y);
                SendMessage(hwndEdit, SCI_GOTOPOS, (WPARAM)iNewPos, 0);
              }
              if (pt.x == -1 && pt.y == -1)
              {
                int iCurrentPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
                pt.x = (int)SendMessage(hwndEdit, SCI_POINTXFROMPOSITION, 0, (LPARAM)iCurrentPos);
                pt.y = (int)SendMessage(hwndEdit, SCI_POINTYFROMPOSITION, 0, (LPARAM)iCurrentPos);
                ClientToScreen(hwndEdit, &pt);
              }
              imenu = 0;
            }
            break;
          case IDC_TOOLBAR:
          case IDC_STATUSBAR:
          case IDC_REBAR:
            if (pt.x == -1 && pt.y == -1)
              GetCursorPos(&pt);
            imenu = 1;
            break;
        }

        TrackPopupMenuEx(GetSubMenu(hmenu, imenu),
                         TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x + 1, pt.y + 1, hwnd, NULL);

        DestroyMenu(hmenu);
      }
      break;


    case WM_INITMENU:
      MsgInitMenu(hwnd, wParam, lParam);
      break;

    
    // [2e]: Hide pointer while typing #230
    case WM_MENUSELECT:
      n2e_OnMouseVanishEvent(TRUE);
      break;
    // [/2e]


    case WM_NOTIFY:
      return MsgNotify(hwnd, wParam, lParam);


    case WM_COMMAND:
      return MsgCommand(hwnd, wParam, lParam);


    case WM_SYSCOMMAND:
      switch (wParam & 0xFFF0)
      {
        case SC_MINIMIZE:
          ShowOwnedPopups(hwnd, FALSE);
          if (bMinimizeToTray)
          {
            MinimizeWndToTray(hwnd);
            ShowNotifyIcon(hwnd, TRUE);
            SetNotifyIconTitle(hwnd);
            return (0);
          }
          else
            return DefWindowProc(hwnd, umsg, wParam, lParam);

        case SC_RESTORE: {
            LRESULT lrv = DefWindowProc(hwnd, umsg, wParam, lParam);
            ShowOwnedPopups(hwnd, TRUE);
            return (lrv);
          }

        // [2e]: Prevent Alt keypress from leaving the text area #399
        case SC_KEYMENU:
            return (0);

        // [2e]: Edit Mode hint not hidden on window resize/move #349
        case SC_MOVE:
        case SC_SIZE:
          if (n2e_IsSelectionEditModeOn())
          {
            n2e_SelectionEditHideToolTip();
          }
          break;
        // [/2e]
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);


    case WM_CHANGENOTIFY:
      if (iFileWatchingMode == 1 || n2e_IsDocumentModified())
        SetForegroundWindow(hwnd);

      if (PathFileExists(szCurFile))
      {
        if ((iFileWatchingMode == 2 && !n2e_IsDocumentModified()) ||
            MsgBox(MBYESNO, IDS_FILECHANGENOTIFY) == IDYES)
        {
          // [2e]: Retain split view state on document reload #405
          VIEW_COMMAND(n2e_SaveViewState);

          int iCurPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
          int iAnchorPos = (int)SendMessage(hwndEdit, SCI_GETANCHOR, 0, 0);
          BOOL bIsTail = (iCurPos == iAnchorPos) && (iCurPos == SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0));
          iWeakSrcEncoding = iEncoding;
          if (FileLoad(TRUE, FALSE, TRUE, FALSE, szCurFile))
          {
            if (bIsTail && iFileWatchingMode == 2)
            {
              EditJumpTo(hwndEdit, -1, 0);
              EditEnsureSelectionVisible(hwndEdit);
            }
            else if (SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0) >= 4)
            {
              char tch[5] = "";
              SendMessage(hwndEdit, SCI_GETTEXT, 5, (LPARAM)tch);
              if (lstrcmpiA(tch, ".LOG") != 0)
              {
                // [2e]: Retain split view state on document reload #405
                VIEW_COMMAND(n2e_LoadViewState);
              }
            }
          }
        }
      }
      else
      {
        if (MsgBox(MBYESNO, IDS_FILECHANGENOTIFY2) == IDYES)
          FileSave(TRUE, FALSE, FALSE, FALSE, FALSE);
      }
      // [2e]: Improve File Change Notification for slow medium #409
      if (!n2e_IsWatchThreadRunning())
        InstallFileWatching(szCurFile);
      break;


    case WM_DRAWCLIPBOARD:
      if (!bLastCopyFromMe)
        dwLastCopyTime = GetTickCount();
      else
        bLastCopyFromMe = FALSE;
      if (hwndNextCBChain)
        SendMessage(hwndNextCBChain, WM_DRAWCLIPBOARD, wParam, lParam);
      break;


    case WM_CHANGECBCHAIN:
      if ((HWND)wParam == hwndNextCBChain)
        hwndNextCBChain = (HWND)lParam;
      if (hwndNextCBChain)
        SendMessage(hwndNextCBChain, WM_CHANGECBCHAIN, lParam, wParam);
      break;


    case WM_TRAYMESSAGE:
      switch (lParam)
      {
        case WM_RBUTTONUP: {
            HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUPMENU));
            HMENU hMenuPopup = GetSubMenu(hMenu, 2);
            POINT pt;
            int iCmd;
            SetForegroundWindow(hwnd);
            GetCursorPos(&pt);
            SetMenuDefaultItem(hMenuPopup, IDM_TRAY_RESTORE, FALSE);
            iCmd = TrackPopupMenu(hMenuPopup,
                                  TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                                  pt.x, pt.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);
            DestroyMenu(hMenu);
            if (iCmd == IDM_TRAY_RESTORE)
            {
              ShowNotifyIcon(hwnd, FALSE);
              RestoreWndFromTray(hwnd);
              ShowOwnedPopups(hwnd, TRUE);
            }
            else if (iCmd == IDM_TRAY_EXIT)
            {
              SendMessage(hwnd, WM_CLOSE, 0, 0);
            }
          }
          return TRUE;

        case WM_LBUTTONUP:
          ShowNotifyIcon(hwnd, FALSE);
          RestoreWndFromTray(hwnd);
          ShowOwnedPopups(hwnd, TRUE);
          return TRUE;
      }
      break;

     
    // [2e]: Replace settings in all instances #5
    case WM_N2E_RELOAD_SETTINGS: {
        LoadSettings();
        MsgInitMenu(hwnd, 0, 0);
        VIEW_COMMAND(EditInit);
        n2e_Reset();
        n2e_UpdateWindowTitle(hwnd);
      }
      break;
    // [/2e]


    default:
      if (umsg == msgTaskbarCreated)
      {
        if (!IsWindowVisible(hwnd))
          ShowNotifyIcon(hwnd, TRUE);

        SetNotifyIconTitle(hwnd);
        return (0);
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);
  }
  return (0);
}

// [2e]: Edit initialization subroutine
void EditInit(HWND hwnd)
{
  // Tabs
  SendMessage(hwnd, SCI_SETUSETABS, !bTabsAsSpaces, 0);
  SendMessage(hwnd, SCI_SETTABINDENTS, bTabIndents, 0);
  SendMessage(hwnd, SCI_SETBACKSPACEUNINDENTS, bBackspaceUnindents, 0);
  SendMessage(hwnd, SCI_SETTABWIDTH, iTabWidth, 0);
  SendMessage(hwnd, SCI_SETINDENT, iIndentWidth, 0);
  // Indent Guides
  Style_SetIndentGuides(hwnd);
  // Word wrap
  if (!fWordWrap)
  {
    SendMessage(hwnd, SCI_SETWRAPMODE, SC_WRAP_NONE, 0);
  }
  else
  {
    SendMessage(hwnd, SCI_SETWRAPMODE, (iWordWrapMode == 0) ? SC_WRAP_WORD : SC_WRAP_CHAR, 0);
  }
  if (iWordWrapIndent == 5)
  {
    SendMessage(hwnd, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_SAME, 0);
  }
  else if (iWordWrapIndent == 6)
  {
    SendMessage(hwnd, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_INDENT, 0);
  }
  else
  {
    int i = 0;
    switch (iWordWrapIndent)
    {
      case 1:
        i = 1;
        break;
      case 2:
        i = 2;
        break;
      case 3:
        i = (iIndentWidth) ? 1 * iIndentWidth : 1 * iTabWidth;
        break;
      case 4:
        i = (iIndentWidth) ? 2 * iIndentWidth : 2 * iTabWidth;
        break;
    }
    SendMessage(hwnd, SCI_SETWRAPSTARTINDENT, i, 0);
    SendMessage(hwnd, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_FIXED, 0);
  }
  if (bShowWordWrapSymbols)
  {
    int wrapVisualFlags = 0;
    int wrapVisualFlagsLocation = 0;
    if (iWordWrapSymbols == 0)
    {
      iWordWrapSymbols = 22;
    }
    switch (iWordWrapSymbols % 10)
    {
      case 1:
        wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
        wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_END_BY_TEXT;
        break;
      case 2:
        wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
        break;
    }
    switch (((iWordWrapSymbols % 100) - (iWordWrapSymbols % 10)) / 10)
    {
      case 1:
        wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
        wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_START_BY_TEXT;
        break;
      case 2:
        wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
        break;
    }
    SendMessage(hwnd, SCI_SETWRAPVISUALFLAGSLOCATION, wrapVisualFlagsLocation, 0);
    SendMessage(hwnd, SCI_SETWRAPVISUALFLAGS, wrapVisualFlags, 0);
  }
  else
  {
    SendMessage(hwnd, SCI_SETWRAPVISUALFLAGS, 0, 0);
  }
  // Long Lines
  if (bMarkLongLines)
  {
    SendMessage(hwnd, SCI_SETEDGEMODE, (iLongLineMode == EDGE_LINE) ? EDGE_LINE : EDGE_BACKGROUND, 0);
  }
  else
  {
    SendMessage(hwnd, SCI_SETEDGEMODE, EDGE_NONE, 0);
  }
  SendMessage(hwnd, SCI_SETEDGECOLUMN, iLongLinesLimit, 0);
  // Margins
  SendMessage(hwnd, SCI_SETMARGINWIDTHN, 1, (bShowSelectionMargin) ? 16 : 0);
  // [2e]: Add View > First Column Margin #382
  SendMessage(hwnd, SCI_SETMARGINWIDTHN, 2, bShowFirstColumnMargin ? 16 : 0);
  SendMessage(hwnd, SCI_SETMARGINSENSITIVEN, 2, TRUE);
  SendMessage(hwnd, SCI_SETMARGINCURSORN, 2, 1);
  // [/2e]
  VIEW_COMMAND(UpdateLineNumberWidth);
  // Nonprinting characters
  SendMessage(hwnd, SCI_SETVIEWWS, bViewWhiteSpace ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE, 0);
  SendMessage(hwnd, SCI_SETVIEWEOL, bViewEOLs, 0);
  SendMessage(hwnd, SCI_MOVECARETONRCLICK, bMoveCaretOnRightClick, 0);
}
// [/2e]

//=============================================================================
//
//  MsgCreate() - Handles WM_CREATE
//
//
LRESULT MsgCreate(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  HINSTANCE hInstance;
  hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
  // Setup edit control
  _hwndEdit = n2e_EditCreate(hwnd, &hwndEditParent);
  EditInit(_hwndEdit);

  hwndEditFrame = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    WC_LISTVIEW,
    NULL,
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
    0, 0, 100, 100,
    hwnd,
    (HMENU)IDC_EDITFRAME,
    hInstance,
    NULL);
  if (PrivateIsAppThemed())
  {
    RECT rc, rc2;

    bIsAppThemed = TRUE;

    SetWindowLongPtr(hwndEditParent, GWL_EXSTYLE, GetWindowLongPtr(hwndEditParent, GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(hwndEditParent, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    if (IsVista())
    {
      cxEditFrame = 0;
      cyEditFrame = 0;
    }
    else
    {
      GetClientRect(hwndEditFrame, &rc);
      GetWindowRect(hwndEditFrame, &rc2);
      cxEditFrame = ((rc2.right - rc2.left) - (rc.right - rc.left)) / 2;
      cyEditFrame = ((rc2.bottom - rc2.top) - (rc.bottom - rc.top)) / 2;
    }
  }
  else
  {
    bIsAppThemed = FALSE;

    cxEditFrame = 0;
    cyEditFrame = 0;
  }

  // Create Toolbar and Statusbar
  CreateBars(hwnd, hInstance);

  // Window Initialization

  CreateWindow(
    WC_STATIC,
    NULL,
    WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
    0, 0, 10, 10,
    hwnd,
    (HMENU)IDC_FILENAME,
    hInstance,
    NULL);

  SetDlgItemText(hwnd, IDC_FILENAME, szCurFile);

  CreateWindow(
    WC_STATIC,
    NULL,
    WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
    10, 10, 10, 10,
    hwnd,
    (HMENU)IDC_REUSELOCK,
    hInstance,
    NULL);

  SetDlgItemInt(hwnd, IDC_REUSELOCK, GetTickCount(), FALSE);

  // Drag & Drop
  DragAcceptFiles(hwnd, TRUE);

  // File MRU
  pFileMRU = MRU_Create(L"Recent Files", MRU_NOCASE, 32);
  MRU_Load(pFileMRU);

  mruFind = MRU_Create(L"Recent Find", MRU_UTF8, 16);
  MRU_Load(mruFind);

  mruReplace = MRU_Create(L"Recent Replace", MRU_UTF8, 16);
  MRU_Load(mruReplace);

  if (hwndEdit == NULL || hwndEditFrame == NULL || hwndEditParent == NULL ||
      hwndStatus == NULL || hwndToolbar == NULL || hwndReBar == NULL)
    return (-1);

  n2e_UpdateViews();
  // [2e]: DPI awareness #154
  n2e_UpdateViewsDPI(GetDPIFromWindow(hwnd));
  return (0);
}


//=============================================================================
//
//  CreateBars() - Create Toolbar and Statusbar
//
//
void CreateBars(HWND hwnd, HINSTANCE hInstance)
{
  RECT rc;

  REBARINFO rbi;
  REBARBANDINFO rbBand;

  BITMAP bmp;
  HBITMAP hbmp, hbmpCopy = NULL;
  HIMAGELIST himl;
  WCHAR szTmp[MAX_PATH];
  BOOL bExternalBitmap = FALSE;

  DWORD dwToolbarStyle = WS_TOOLBAR;
  DWORD dwStatusbarStyle = WS_CHILD | WS_CLIPSIBLINGS;
  DWORD dwReBarStyle = WS_REBAR;

  BOOL bIsAppThemed = PrivateIsAppThemed();

  int i;
  WCHAR tchDesc[256];
  WCHAR tchIndex[256];

  WCHAR *pIniSection = NULL;
  int   cchIniSection = 0;

  if (bShowToolbar)
    dwReBarStyle |= WS_VISIBLE;

  hwndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, dwToolbarStyle,
                               0, 0, 0, 0, hwnd, (HMENU)IDC_TOOLBAR, hInstance, NULL);

  SendMessage(hwndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

  // Add normal Toolbar Bitmap
  hbmp = NULL;
  if (lstrlen(tchToolbarBitmap))
  {
    if (!SearchPath(NULL, tchToolbarBitmap, NULL, COUNTOF(szTmp), szTmp, NULL))
    {
      lstrcpy(szTmp, tchToolbarBitmap);
    }
    hbmp = LoadImage(NULL, szTmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
  }
  if (hbmp)
    bExternalBitmap = TRUE;
  else
  {
    // [2e] Enable toolbar scaling (DPI) #327
    hbmp = DPICreateToolbarBitmap(hwnd, hInstance);
    hbmpCopy = CopyImage(hbmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  }
  GetObject(hbmp, sizeof(BITMAP), &bmp);
  if (!IsXP())
    BitmapMergeAlpha(hbmp, GetSysColor(COLOR_3DFACE));
  himl = ImageList_Create(bmp.bmWidth / NUMTOOLBITMAPS, bmp.bmHeight, ILC_COLOR32 | ILC_MASK, 0, 0);
  ImageList_AddMasked(himl, hbmp, CLR_DEFAULT);
  DeleteObject(hbmp);
  SendMessage(hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)himl);

  // Optionally add hot Toolbar Bitmap
  hbmp = NULL;
  if (lstrlen(tchToolbarBitmapHot))
  {
    if (!SearchPath(NULL, tchToolbarBitmapHot, NULL, COUNTOF(szTmp), szTmp, NULL))
      lstrcpy(szTmp, tchToolbarBitmapHot);
    if (hbmp = LoadImage(NULL, szTmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE))
    {
      GetObject(hbmp, sizeof(BITMAP), &bmp);
      himl = ImageList_Create(bmp.bmWidth / NUMTOOLBITMAPS, bmp.bmHeight, ILC_COLOR32 | ILC_MASK, 0, 0);
      ImageList_AddMasked(himl, hbmp, CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(hwndToolbar, TB_SETHOTIMAGELIST, 0, (LPARAM)himl);
    }
  }

  // Optionally add disabled Toolbar Bitmap
  hbmp = NULL;
  if (lstrlen(tchToolbarBitmapDisabled))
  {
    if (!SearchPath(NULL, tchToolbarBitmapDisabled, NULL, COUNTOF(szTmp), szTmp, NULL))
      lstrcpy(szTmp, tchToolbarBitmapDisabled);
    if (hbmp = LoadImage(NULL, szTmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE))
    {
      GetObject(hbmp, sizeof(BITMAP), &bmp);
      himl = ImageList_Create(bmp.bmWidth / NUMTOOLBITMAPS, bmp.bmHeight, ILC_COLOR32 | ILC_MASK, 0, 0);
      ImageList_AddMasked(himl, hbmp, CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(hwndToolbar, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)himl);
      bExternalBitmap = TRUE;
    }
  }

  if (!bExternalBitmap)
  {
    BOOL fProcessed = FALSE;
    if (flagToolbarLook == 1)
      fProcessed = BitmapAlphaBlend(hbmpCopy, GetSysColor(COLOR_3DFACE), 0x60);
    else if (flagToolbarLook == 2 || (!IsXP() && flagToolbarLook == 0))
      fProcessed = BitmapGrayScale(hbmpCopy);
    if (fProcessed && !IsXP())
      BitmapMergeAlpha(hbmpCopy, GetSysColor(COLOR_3DFACE));
    if (fProcessed)
    {
      himl = ImageList_Create(bmp.bmWidth / NUMTOOLBITMAPS, bmp.bmHeight, ILC_COLOR32 | ILC_MASK, 0, 0);
      ImageList_AddMasked(himl, hbmpCopy, CLR_DEFAULT);
      SendMessage(hwndToolbar, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)himl);
    }
  }
  if (hbmpCopy)
    DeleteObject(hbmpCopy);

  // Load toolbar labels
  pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * 32 * 1024);
  cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);
  LoadIniSection(L"Toolbar Labels", pIniSection, cchIniSection);
  for (i = 0; i < COUNTOF(tbbMainWnd); i++)
  {
    if (tbbMainWnd[i].fsStyle == TBSTYLE_SEP)
      continue;

    wsprintf(tchIndex, L"%02i", tbbMainWnd[i].iBitmap + 1);

    if (IniSectionGetString(pIniSection, tchIndex, L"", tchDesc, COUNTOF(tchDesc)))
    {
      tbbMainWnd[i].iString = SendMessage(hwndToolbar, TB_ADDSTRING, 0, (LPARAM)tchDesc);
      tbbMainWnd[i].fsStyle |= BTNS_AUTOSIZE | BTNS_SHOWTEXT;
    }

    else
      tbbMainWnd[i].fsStyle &= ~(BTNS_AUTOSIZE | BTNS_SHOWTEXT);
  }
  LocalFree(pIniSection);

  SendMessage(hwndToolbar, TB_SETEXTENDEDSTYLE, 0,
              SendMessage(hwndToolbar, TB_GETEXTENDEDSTYLE, 0, 0) | TBSTYLE_EX_MIXEDBUTTONS);

  SendMessage(hwndToolbar, TB_ADDBUTTONS, NUMINITIALTOOLS, (LPARAM)tbbMainWnd);
  if (Toolbar_SetButtons(hwndToolbar, IDT_FILE_NEW, tchToolbarButtons, tbbMainWnd, COUNTOF(tbbMainWnd)) == 0)
    SendMessage(hwndToolbar, TB_ADDBUTTONS, NUMINITIALTOOLS, (LPARAM)tbbMainWnd);
  SendMessage(hwndToolbar, TB_GETITEMRECT, 0, (LPARAM)&rc);

  if (bShowStatusbar)
    dwStatusbarStyle |= WS_VISIBLE;

  hwndStatus = CreateStatusWindow(dwStatusbarStyle, NULL, hwnd, IDC_STATUSBAR);
  n2e_CreateProgressBarInStatusBar();

  // Create ReBar and add Toolbar
  hwndReBar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL, dwReBarStyle,
                             0, 0, 0, 0, hwnd, (HMENU)IDC_REBAR, hInstance, NULL);
  rbi.cbSize = sizeof(REBARINFO);
  rbi.fMask = 0;
  rbi.himl = (HIMAGELIST)NULL;
  SendMessage(hwndReBar, RB_SETBARINFO, 0, (LPARAM)&rbi);

  rbBand.cbSize = sizeof(REBARBANDINFO);
  rbBand.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE;
  rbBand.fStyle = RBBS_FIXEDSIZE;
  if (bIsAppThemed)
    rbBand.fStyle |= RBBS_CHILDEDGE;
  rbBand.hbmBack = NULL;
  rbBand.lpText = L"Toolbar";
  rbBand.hwndChild = hwndToolbar;
  rbBand.cxMinChild = (rc.right - rc.left) * COUNTOF(tbbMainWnd);
  rbBand.cyMinChild = (rc.bottom - rc.top) + 2 * rc.top;
  rbBand.cx = 0;
  SendMessage(hwndReBar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);

  SetWindowPos(hwndReBar, NULL, 0, 0, 0, 0, SWP_NOZORDER);
  GetWindowRect(hwndReBar, &rc);
  cyReBar = rc.bottom - rc.top;

  cyReBarFrame = bIsAppThemed ? 0 : 2;
}


//=============================================================================
//
//  MsgThemeChanged() - Handle WM_THEMECHANGED
//
//
void MsgThemeChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  RECT rc, rc2;
  HINSTANCE hInstance = (HINSTANCE)(INT_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
  // reinitialize edit frame
  if (PrivateIsAppThemed())
  {
    bIsAppThemed = TRUE;
    SetWindowLongPtr(hwndEditParent, GWL_EXSTYLE, GetWindowLongPtr(hwndEditParent, GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(hwndEditParent, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
    if (IsVista())
    {
      cxEditFrame = 0;
      cyEditFrame = 0;
    }
    else
    {
      SetWindowPos(hwndEditFrame, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
      GetClientRect(hwndEditFrame, &rc);
      GetWindowRect(hwndEditFrame, &rc2);

      cxEditFrame = ((rc2.right - rc2.left) - (rc.right - rc.left)) / 2;
      cyEditFrame = ((rc2.bottom - rc2.top) - (rc.bottom - rc.top)) / 2;
    }
  }
  else
  {
    bIsAppThemed = FALSE;

    SetWindowLongPtr(hwndEditParent, GWL_EXSTYLE, WS_EX_CLIENTEDGE | GetWindowLongPtr(hwndEditParent, GWL_EXSTYLE));
    SetWindowPos(hwndEditParent, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    cxEditFrame = 0;
    cyEditFrame = 0;
  }

  // recreate toolbar and statusbar
  Toolbar_GetButtons(hwndToolbar, IDT_FILE_NEW, tchToolbarButtons, COUNTOF(tchToolbarButtons));

  DestroyWindow(hwndToolbar);
  DestroyWindow(hwndReBar);
  DestroyWindow(hwndStatus);
  n2e_DestroyProgressBarInStatusBar();
  CreateBars(hwnd, hInstance);
  UpdateToolbar();

  GetClientRect(hwnd, &rc);
  SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.right, rc.bottom));
  UpdateStatusbar();
}

// Statusbar
int aWidth[6];

// [2e]: Resize statusbar groups #304
void UpdateStatusbarWidth(const int cx)
{
  aWidth[0] = max(cx / 3, StatusCalcPaneWidth(hwndStatus, tchDocPos));
  aWidth[1] = aWidth[0] + max(StatusCalcPaneWidth(hwndStatus, arrwchExpressionValue), StatusCalcPaneWidth(hwndStatus, L"9'999'999 Bytes"));
  aWidth[2] = aWidth[1] + StatusCalcPaneWidth(hwndStatus, L"Unicode BE BOM");
  aWidth[3] = aWidth[2] + StatusCalcPaneWidth(hwndStatus, L"CR+LF");
  aWidth[4] = aWidth[3] + StatusCalcPaneWidth(hwndStatus, L"OVR");
  aWidth[5] = -1;

  SendMessage(hwndStatus, SB_SETPARTS, COUNTOF(aWidth), (LPARAM)aWidth);
}
// [/2e]

//=============================================================================
//
//  MsgSize() - Handles WM_SIZE
//
//
void MsgSize(HWND hwnd, WPARAM wParam, LPARAM lParam)
{

  RECT rc;
  int x, y, cx, cy;
  HDWP hdwp;

  if (wParam == SIZE_MINIMIZED)
    return;

  x = 0;
  y = 0;

  cx = LOWORD(lParam);
  cy = HIWORD(lParam);

  if (bShowToolbar)
  {
    SetWindowPos(hwndReBar, NULL, 0, 0, LOWORD(lParam), cyReBar, SWP_NOZORDER);
    // the ReBar automatically sets the correct height
    // calling SetWindowPos() with the height of one toolbar button
    // causes the control not to temporarily use the whole client area
    // and prevents flickering
    y = cyReBar + cyReBarFrame;    // define
    cy -= cyReBar + cyReBarFrame;  // border
  }

  if (bShowStatusbar)
  {
    SendMessage(hwndStatus, WM_SIZE, 0, 0);
    GetWindowRect(hwndStatus, &rc);
    cy -= (rc.bottom - rc.top);
  }

  hdwp = BeginDeferWindowPos(2);

  DeferWindowPos(hdwp, hwndEditFrame, NULL, x, y, cx, cy,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  DeferWindowPos(hdwp, hwndEditParent, NULL, x + cxEditFrame, y + cyEditFrame,
                 cx - 2 * cxEditFrame, cy - 2 * cyEditFrame,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  EndDeferWindowPos(hdwp);

  UpdateStatusbarWidth(cx);

  // [2e]: Progress indication for Grep/Ungrep
  if (bShowProgressBar && hwndStatusProgressBar)
  {
    InlineProgressBarCtrl_Resize(hwndStatusProgressBar);
  }
  // [/2e]
}

//=============================================================================
//
//  MsgInitMenu() - Handles WM_INITMENU
//
//
void MsgInitMenu(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  int i, i2, i3;
  HMENU hmenu = (HMENU)wParam;

  i = lstrlen(szCurFile);
  EnableCmd(hmenu, IDM_FILE_REVERT, i);
  EnableCmd(hmenu, IDM_FILE_LAUNCH, i);
  EnableCmd(hmenu, IDM_FILE_OPENFOLDER, i);
  EnableCmd(hmenu, IDM_FILE_PROPERTIES, i);
  EnableCmd(hmenu, IDM_FILE_CREATELINK, i);
  // [2e]: Process elevation #166
  n2e_SetUACIcon(hmenu, IDM_FILE_ELEVATE);
  if (!IsWindowsVistaOrGreater() || fIsElevated)
  {
    EnableCmd(hmenu, IDM_FILE_ELEVATE, FALSE);
  }
  CheckCmd(hmenu, IDM_FILE_ELEVATE, fIsElevated || n2e_IsElevatedMode());
  // [2e]: Minor menu fixes #229
  EnableCmd(hmenu, ID_FILE_OPEN__NEXT, i);
  EnableCmd(hmenu, ID_FILE_OPEN__PREVIOUS, i);
  EnableCmd(hmenu, ID_FILE_INVOKESHELLMENU, i);
  EnableCmd(hmenu, IDM_FILE_ADDTOFAV, i);
  // [2e]: Disable Launch commands for Unnamed #238
  EnableCmd(hmenu, IDM_FILE_OPENWITH, i);
  // [2e]: File->RenameTo menu item
  EnableCmd(hmenu, ID_FILE_RENAMETO, i);
  EnableCmd(hmenu, IDM_FILE_READONLY, i);
  CheckCmd(hmenu, IDM_FILE_READONLY, bReadOnly);

  // [2e]: Save on deactivate #164
  CheckMenuRadioItem(hmenu, ID_SAVEONLOSEFOCUS_DISABLED, ID_SAVEONLOSEFOCUS_ENABLEDUNTILANEWFILE, n2e_GetCurrentSaveOnLoseFocusMenuID(), MF_BYCOMMAND);

  EnableCmd(hmenu, IDM_ENCODING_RECODE, i);
  if (mEncoding[iEncoding].uFlags & NCP_UNICODE_REVERSE)
    i = IDM_ENCODING_UNICODEREV;
  else if (mEncoding[iEncoding].uFlags & NCP_UNICODE)
    i = IDM_ENCODING_UNICODE;
  else if (mEncoding[iEncoding].uFlags & NCP_UTF8_SIGN)
    i = IDM_ENCODING_UTF8SIGN;
  else if (mEncoding[iEncoding].uFlags & NCP_UTF8)
    i = IDM_ENCODING_UTF8;
  else if (mEncoding[iEncoding].uFlags & NCP_DEFAULT)
    i = IDM_ENCODING_ANSI;
  else
    i = -1;
  CheckMenuRadioItem(hmenu, IDM_ENCODING_ANSI, IDM_ENCODING_UTF8SIGN, i, MF_BYCOMMAND);

  if (iEOLMode == SC_EOL_CRLF)
    i = IDM_LINEENDINGS_CRLF;
  else if (iEOLMode == SC_EOL_LF)
    i = IDM_LINEENDINGS_LF;
  else
    i = IDM_LINEENDINGS_CR;
  CheckMenuRadioItem(hmenu, IDM_LINEENDINGS_CRLF, IDM_LINEENDINGS_CR, i, MF_BYCOMMAND);
  const BOOL bEmptyRecentHistory = (MRU_Enum(pFileMRU, 0, NULL, 0) == 0);
  EnableCmd(hmenu, IDM_FILE_RECENT, !bEmptyRecentHistory);
  EnableCmd(hmenu, ID_FILE_OPENPREVIOUS, !bEmptyRecentHistory);
  const BOOL bCanUndo = SendMessage(hwndEdit, SCI_CANUNDO, 0, 0);
  const BOOL bCanRedo = SendMessage(hwndEdit, SCI_CANREDO, 0, 0);
  EnableCmd(hmenu, IDM_EDIT_UNDO, bCanUndo);
  EnableCmd(hmenu, IDM_EDIT_REDO, bCanRedo);
  EnableCmd(hmenu, ID_EDIT_UNDO_REDO, bCanUndo | bCanRedo);
  i = (int)SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0) - (int)SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0);
  i2 = (int)SendMessage(hwndEdit, SCI_CANPASTE, 0, 0);
  i3 = (int)SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0);
  EnableCmd(hmenu, IDM_EDIT_CUT, i3);
  EnableCmd(hmenu, IDM_EDIT_COPY, i3);
  EnableCmd(hmenu, IDM_EDIT_COPYALL, i3);
  // [2e]: "Copy Add (Ctrl+E)" not working when clipboard is empty
  EnableCmd(hmenu, IDM_EDIT_COPYADD, i3);
  EnableCmd(hmenu, IDM_EDIT_PASTE, i2);
  EnableCmd(hmenu, IDM_EDIT_SWAP, i || i2);
  EnableCmd(hmenu, IDM_EDIT_CLEAR, i);

  OpenClipboard(hwnd);
  EnableCmd(hmenu, IDM_EDIT_CLEARCLIPBOARD, CountClipboardFormats());
  CloseClipboard();

  EnableCmd(hmenu, IDM_EDIT_COLUMNWRAP, i);
  EnableCmd(hmenu, IDM_EDIT_SPLITLINES, i || bExtendedSplitLines);
  EnableCmd(hmenu, IDM_EDIT_JOINLINESEX, i);
  EnableCmd(hmenu, IDM_EDIT_SENTENCECASE, i);
  EnableCmd(hmenu, IDM_EDIT_CONVERTTABS, i);
  EnableCmd(hmenu, IDM_EDIT_CONVERTSPACES, i);
  EnableCmd(hmenu, IDM_EDIT_CONVERTTABS2, i);
  EnableCmd(hmenu, IDM_EDIT_CONVERTSPACES2, i);
  // [2e]: Url encode issue #189
  EnableCmd(hmenu, IDM_EDIT_URLENCODE, (iUrlEncodeMode == UEM_LEGACY) ? i : i3);
  EnableCmd(hmenu, IDM_EDIT_URLDECODE, (iUrlEncodeMode == UEM_LEGACY) ? i : i3);
  EnableCmd(hmenu, IDM_EDIT_ESCAPECCHARS, i);
  EnableCmd(hmenu, IDM_EDIT_UNESCAPECCHARS, i);
  EnableCmd(hmenu, IDM_EDIT_CHAR2HEX, i);
  EnableCmd(hmenu, IDM_EDIT_HEX2CHAR, i);
  EnableCmd(hmenu, IDM_EDIT_STRING2HEX, i3);
  EnableCmd(hmenu, IDM_EDIT_HEX2STRING, i3);
  EnableCmd(hmenu, IDM_EDIT_BASE64ENCODE, i3);
  EnableCmd(hmenu, IDM_EDIT_BASE64DECODE, i3);
  EnableCmd(hmenu, IDM_EDIT_QPENCODE, i3);
  EnableCmd(hmenu, IDM_EDIT_QPDECODE, i3);
  EnableCmd(hmenu, IDM_VIEW_SHOWEXCERPT, i);
  const COMMENTINFO* pCommentInfo = n2e_GetCommentInfo(SendMessage(hwndEdit, SCI_GETLEXER, 0, 0));
  EnableCmd(hmenu, IDM_EDIT_LINECOMMENT, wcslen(pCommentInfo->pszLineCommentW) > 0);
  EnableCmd(hmenu, IDM_EDIT_STREAMCOMMENT, wcslen(pCommentInfo->pszStreamStartW) > 0);
  EnableCmd(hmenu, IDM_EDIT_INSERT_ENCODING, *mEncoding[iEncoding].pszParseNames);
  i = (int)SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0);
  EnableCmd(hmenu, IDM_EDIT_FIND, i);
  EnableCmd(hmenu, IDM_EDIT_SAVEFIND, i);
  EnableCmd(hmenu, IDM_EDIT_FINDNEXT, i);
  EnableCmd(hmenu, IDM_EDIT_FINDPREV, i && lstrlenA(efrData.szFind));
  EnableCmd(hmenu, IDM_EDIT_REPLACE, i);
  EnableCmd(hmenu, IDM_EDIT_REPLACENEXT, i);
  EnableCmd(hmenu, IDM_EDIT_INSERTREPLACEMENT, i);
  EnableCmd(hmenu, IDM_EDIT_SELTONEXT, i && lstrlenA(efrData.szFind));
  EnableCmd(hmenu, IDM_EDIT_SELTOPREV, i && lstrlenA(efrData.szFind));
  EnableCmd(hmenu, IDM_EDIT_FINDMATCHINGBRACE, i);
  EnableCmd(hmenu, IDM_EDIT_SELTOMATCHINGBRACE, i);
  // [2e]: Go To dialog - make similar to Find/Replace #260
  EnableCmd(hmenu, IDM_EDIT_GOTOLINE, i);
  // [2e]: Disable more Edit commands on empty buffer #268
  EnableCmd(hmenu, IDM_EDIT_STRIP_HTML_TAGS, i);
  EnableCmd(hmenu, ID_SPECIAL_ESCAPEHTML, i);
  EnableCmd(hmenu, ID_EDIT_FINDNEXTWORD, i);
  EnableCmd(hmenu, ID_EDIT_FINDPREVIOUSWORD, i);
  EnableCmd(hmenu, ID_EDIT_EDITSELECTION, i);
  EnableCmd(hmenu, ID_EDIT_EDITSELECTION_LINE, i);
  EnableCmd(hmenu, ID_EDIT_EDITSELECTION_INVERSED, i);
  // [/2e]
  CheckCmd(hmenu, IDM_VIEW_USE2NDDEFAULT, Style_GetUse2ndDefault(hwndEdit));
  CheckCmd(hmenu, IDM_VIEW_WORDWRAP, fWordWrap);
  CheckCmd(hmenu, IDM_VIEW_LONGLINEMARKER, bMarkLongLines);
  CheckCmd(hmenu, IDM_VIEW_TABSASSPACES, bTabsAsSpaces);
  CheckCmd(hmenu, IDM_VIEW_SHOWINDENTGUIDES, bShowIndentGuides);
  CheckCmd(hmenu, IDM_VIEW_AUTOINDENTTEXT, bAutoIndent);
  CheckCmd(hmenu, IDM_VIEW_LINENUMBERS, bShowLineNumbers);
  CheckCmd(hmenu, IDM_VIEW_MARGIN, bShowSelectionMargin);
  CheckCmd(hmenu, IDM_VIEW_FIRST_COLUMN_MARGIN, bShowFirstColumnMargin);
  CheckCmd(hmenu, IDM_VIEW_SHOWWHITESPACE, bViewWhiteSpace);
  CheckCmd(hmenu, IDM_VIEW_SHOWEOLS, bViewEOLs);
  CheckCmd(hmenu, IDM_VIEW_WORDWRAPSYMBOLS, bShowWordWrapSymbols);
  CheckCmd(hmenu, IDM_VIEW_MATCHBRACES, bMatchBraces);
  CheckCmd(hmenu, IDM_VIEW_TOOLBAR, bShowToolbar);
  EnableCmd(hmenu, IDM_VIEW_CUSTOMIZETB, bShowToolbar);
  CheckCmd(hmenu, IDM_VIEW_STATUSBAR, bShowStatusbar);
  i = (int)SendMessage(hwndEdit, SCI_GETLEXER, 0, 0);
  CheckCmd(hmenu, IDM_VIEW_AUTOCLOSETAGS, bAutoCloseTags);
  CheckCmd(hmenu, IDM_VIEW_HIGHLIGHTCURRENTLINE, bHighlightCurrentLine);

  // [2e]: Improve selection/word highlighting #286
  CheckMenuRadioItem(hmenu, IDM_VIEW_HIGHLIGHTCURRENTSELECTION_DISABLED,
                      IDM_VIEW_HIGHLIGHTCURRENTSELECTION_WORDIFNOSELECTION,
                      n2e_GetCurrentHighlightCurrentSelectionMenuID(), MF_BYCOMMAND);
  CheckCmd(hmenu, IDM_VIEW_HIGHLIGHTCURRENTSELECTION_EDITWORD, bEditSelectionScope);
  // [/2e]

  i = IniGetInt(L"Settings2", L"ReuseWindow", 0);
  CheckCmd(hmenu, IDM_VIEW_REUSEWINDOW, i);
  i = IniGetInt(L"Settings2", L"SingleFileInstance", 0);
  CheckCmd(hmenu, IDM_VIEW_SINGLEFILEINSTANCE, i);
  bStickyWinPos = IniGetInt(L"Settings2", L"StickyWindowPosition", 0);
  CheckCmd(hmenu, IDM_VIEW_STICKYWINPOS, bStickyWinPos);
  CheckCmd(hmenu, IDM_VIEW_ALWAYSONTOP, ((bAlwaysOnTop || flagAlwaysOnTop == 2) && flagAlwaysOnTop != 1));
  CheckCmd(hmenu, IDM_VIEW_MINTOTRAY, bMinimizeToTray);
  CheckCmd(hmenu, IDM_VIEW_TRANSPARENT, bTransparentMode && bTransparentModeAvailable);
  EnableCmd(hmenu, IDM_VIEW_TRANSPARENT, bTransparentModeAvailable);
  CheckCmd(hmenu, IDM_VIEW_NOSAVERECENT, bSaveRecentFiles);
  CheckCmd(hmenu, IDM_VIEW_NOSAVEFINDREPL, bSaveFindReplace);
  CheckMenuRadioItem(hmenu, IDM_VIEW_SAVEBEFORERUNNINGTOOLS_DISABLED, IDM_VIEW_SAVEBEFORERUNNINGTOOLS_EXCEPT_NEW_WINDOW, n2e_GetSaveBeforeRunningToolsMenuID(), MF_BYCOMMAND);
  CheckCmd(hmenu, IDM_VIEW_CHANGENOTIFY, iFileWatchingMode);
  CheckMenuRadioItem(hmenu, IDM_VIEW_SHOWFILENAMEONLY, IDM_VIEW_SHOWEXCERPT, n2e_GetCurrentShowTitleMenuID(), MF_BYCOMMAND);
  // [2e]: Language indication #86
  CheckMenuRadioItem(hmenu, IDM_VIEW_NOLANGUAGEINDICATOR, IDM_VIEW_SHOWLANGUAGEINDICATORNONUS, n2e_GetCurrentLanguageIndicatorMenuID(), MF_BYCOMMAND);
  if (iEscFunction == EEF_MINIMIZE)
    i = IDM_VIEW_ESCMINIMIZE;
  else if (iEscFunction == EEF_EXIT)
    i = IDM_VIEW_ESCEXIT;
  else
    i = IDM_VIEW_NOESCFUNC;
  CheckMenuRadioItem(hmenu, IDM_VIEW_NOESCFUNC, IDM_VIEW_ESCEXIT, i, MF_BYCOMMAND);
  CheckCmd(hmenu, IDM_VIEW_ESCCLOSEVIEW, bEscForCurrentSplitView);

  i = lstrlen(szIniFile);
  // [2e]: Save on exit and History #101
  CheckMenuRadioItem(hmenu, IDM_VIEW_SAVESETTINGS_MODE_ALL, IDM_VIEW_SAVESETTINGS_MODE_NO, n2e_GetCurrentSaveSettingsMenuID(), MF_BYCOMMAND);

  EnableCmd(hmenu, IDM_VIEW_REUSEWINDOW, i);
  EnableCmd(hmenu, IDM_VIEW_STICKYWINPOS, i);
  EnableCmd(hmenu, IDM_VIEW_SINGLEFILEINSTANCE, i);
  EnableCmd(hmenu, IDM_VIEW_NOSAVERECENT, i);
  EnableCmd(hmenu, IDM_VIEW_NOSAVEFINDREPL, i);
  // [2e]: Save on exit and History #101
  EnableCmd(hmenu, IDM_VIEW_SAVESETTINGS_MODE_ALL, i);
  EnableCmd(hmenu, IDM_VIEW_SAVESETTINGS_MODE_RECENT, i);
  EnableCmd(hmenu, IDM_VIEW_SAVESETTINGS_MODE_NO, i);
  // [2e]: New command: Ope&n INI File #330
  EnableCmd(hmenu, CMD_OPENINIFILE, i);
  // [2e]: Save Settings submenu: disable commands if no INI used #353
  EnableCmd(hmenu, ID_SETTINGS_RELOADFROMDISK, i);
  EnableCmd(hmenu, ID_SETTINGS_REPLACESETTINGSINALLINSTANCES, i);
  // [/2e]

  i = (lstrlen(szIniFile) > 0 || lstrlen(szIniFile2) > 0);
  EnableCmd(hmenu, IDM_VIEW_SAVESETTINGSNOW, i);
  // [2e]: Ctrl+Wheel scroll feature
  CheckCmd(hmenu, ID_SETTINGS_CTRL_WHEEL_SCROLL, bCtrlWheelScroll);
  // [2e]: Implement Notepad's right click behavior #54
  CheckCmd(hmenu, ID_SETTINGS_MOVE_CARET_ON_RCLICK, bMoveCaretOnRightClick);
  // [2e]: MathEval INI setting #88
  CheckMenuRadioItem(hmenu, ID_SETTINGS_EVAL_DISABLED, ID_SETTINGS_EVAL_LINE, n2e_GetCurrentEvalMenuID(), MF_BYCOMMAND);
  // [2e]: ctrl + arrow behavior toggle #89
  CheckCmd(hmenu, ID_SETTINGS_WORD_NAVIGATION, iWordNavigationMode == WNM_ACCELERATED);
  // [/2e]
}


//=============================================================================
//
//  MsgCommand() - Handles WM_COMMAND
//
//
LRESULT MsgCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  const WORD wCommandID = LOWORD(wParam);
  switch (wCommandID)
  {

    case IDM_FILE_NEW:
      FileLoad(FALSE, TRUE, FALSE, FALSE, L"");
      break;


    case IDM_FILE_OPEN:
      FileLoad(FALSE, FALSE, FALSE, FALSE, L"");
      break;


    // [2e]: Open Next/Previous #43
    case ID_FILE_OPEN__NEXT:
    case IDT_FILE_OPEN_NEXT:
      n2e_OpenNextFile(hwnd, szCurFile, TRUE);
      break;


    case ID_FILE_OPEN__PREVIOUS:
    case IDT_FILE_OPEN_PREV:
      n2e_OpenNextFile(hwnd, szCurFile, FALSE);
      break;
    // [/2e]


    case IDM_FILE_REVERT: {
        if (lstrlen(szCurFile))
        {
          WCHAR tchCurFile2[MAX_PATH];

          if (n2e_IsDocumentModified() && MsgBox(MBOKCANCEL, IDS_ASK_REVERT) != IDOK)
            return (0);

          // [2e]: Retain split view state on document reload #405
          VIEW_COMMAND(n2e_SaveViewState);

          lstrcpy(tchCurFile2, szCurFile);
          iWeakSrcEncoding = iEncoding;
          if (FileLoad(TRUE, FALSE, TRUE, FALSE, tchCurFile2))
          {
            if (SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0) >= 4)
            {
              char tch[5] = "";
              SendMessage(hwndEdit, SCI_GETTEXT, 5, (LPARAM)tch);
              if (lstrcmpiA(tch, ".LOG") != 0)
              {
                // [2e]: Retain split view state on document reload #405
                VIEW_COMMAND(n2e_LoadViewState);
              }
            }
          }
          // [2e]: Match indicator
          n2e_ResetFindIcon();
        }
      }
      break;


    case IDM_FILE_SAVE:
      FileSave(TRUE, FALSE, FALSE, FALSE, FALSE);
      break;


    case IDM_FILE_SAVEAS:
      FileSave(TRUE, FALSE, TRUE, FALSE, FALSE);
      break;


    // [2e]: File->RenameTo menu item
    case ID_FILE_RENAMETO:
      FileSave(TRUE, FALSE, TRUE, FALSE, TRUE);
      break;
    // [/2e]


    case IDM_FILE_SAVECOPY:
      FileSave(TRUE, FALSE, TRUE, TRUE, FALSE);
      break;


    case IDM_FILE_READONLY:
      if (lstrlen(szCurFile))
      {
        DWORD dwFileAttributes = GetFileAttributes(szCurFile);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
        {
          if (bReadOnly)
            dwFileAttributes = (dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
          else
            dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
          if (!SetFileAttributes(szCurFile, dwFileAttributes))
            MsgBox(MBWARN, IDS_READONLY_MODIFY, szCurFile);
        }
        else
          MsgBox(MBWARN, IDS_READONLY_MODIFY, szCurFile);

        dwFileAttributes = GetFileAttributes(szCurFile);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
          bReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY);

        n2e_UpdateWindowTitle(hwnd);
      }
      break;


    // [2e]: Save on deactivate #164
    case ID_SAVEONLOSEFOCUS_DISABLED:
      iSaveOnLoseFocus = SLF_DISABLED;
      break;

    case ID_SAVEONLOSEFOCUS_ENABLED:
      iSaveOnLoseFocus = SLF_ENABLED;
      break;

    case ID_SAVEONLOSEFOCUS_ENABLEDUNTILANEWFILE:
      iSaveOnLoseFocus = SLF_ENABLED_UNTIL_NEW_FILE;
      break;
    // [/2e]


    case IDM_FILE_BROWSE: {
        SHELLEXECUTEINFO sei;
        WCHAR tchParam[MAX_PATH + 4] = L"";
        WCHAR tchExeFile[MAX_PATH + 4];
        WCHAR tchTemp[MAX_PATH + 4];

        if (!IniGetString(L"Settings2", L"filebrowser.exe", L"", tchTemp, COUNTOF(tchTemp)))
        {
          if (!SearchPath(NULL, L"metapath.exe", NULL, COUNTOF(tchExeFile), tchExeFile, NULL))
          {
            GetModuleFileName(NULL, tchExeFile, COUNTOF(tchExeFile));
            PathRemoveFileSpec(tchExeFile);
            PathAppend(tchExeFile, L"metapath.exe");
          }
        }
        else
        {
          ExtractFirstArgument(tchTemp, tchExeFile, tchParam);
          if (PathIsRelative(tchExeFile))
          {
            if (!SearchPath(NULL, tchExeFile, NULL, COUNTOF(tchTemp), tchTemp, NULL))
            {
              GetModuleFileName(NULL, tchTemp, COUNTOF(tchTemp));
              PathRemoveFileSpec(tchTemp);
              PathAppend(tchTemp, tchExeFile);
              lstrcpy(tchExeFile, tchTemp);
            }
          }
        }

        if (lstrlen(tchParam) && lstrlen(szCurFile))
          StrCatBuff(tchParam, L" ", COUNTOF(tchParam));

        if (lstrlen(szCurFile))
        {
          lstrcpy(tchTemp, szCurFile);
          PathQuoteSpaces(tchTemp);
          StrCatBuff(tchParam, tchTemp, COUNTOF(tchParam));
        }

        ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = SEE_MASK_FLAG_NO_UI | /*SEE_MASK_NOZONECHECKS*/0x00800000;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = tchExeFile;
        sei.lpParameters = tchParam;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;

        ShellExecuteEx(&sei);

        if ((INT_PTR)sei.hInstApp < 32)
          MsgBox(MBWARN, IDS_ERR_BROWSE);
      }
      break;


    case IDM_FILE_NEWWINDOW:
    case IDM_FILE_NEWWINDOW2: {
        SHELLEXECUTEINFO sei;
        WCHAR szModuleName[MAX_PATH];
        WCHAR szFileName[MAX_PATH];
        WCHAR szParameters[2 * MAX_PATH + 64];

        MONITORINFO mi;
        HMONITOR hMonitor;
        WINDOWPLACEMENT wndpl;
        int x, y, cx, cy, imax;
        WCHAR tch[64];

        // [2e]: Add INI setting to disable file save prompt in Launch > New Window #361
        // [2e]: Disable save prompt for some Launch commands #176
        if ((LOWORD(wParam) != IDM_FILE_NEWWINDOW2)
            && ((iSaveBeforeRunningTools == SBRT_ENABLED)
                || ((iSaveBeforeRunningTools == SBRT_EXCEPT_NEW_WINDOW) && (LOWORD(wParam) != IDM_FILE_NEWWINDOW)))
           )
        {
          if (!FileSave(FALSE, TRUE, FALSE, FALSE, FALSE))
            break;
        }

        GetModuleFileName(NULL, szModuleName, COUNTOF(szModuleName));

        wsprintf(tch, L"\"-appid=%s\"", g_wchAppUserModelID);
        lstrcpy(szParameters, tch);

        wsprintf(tch, L" \"-sysmru=%i\"", (flagUseSystemMRU == 2) ? 1 : 0);
        lstrcat(szParameters, tch);

        lstrcat(szParameters, L" -f");
        if (lstrlen(szIniFile))
        {
          lstrcat(szParameters, L" \"");
          lstrcat(szParameters, szIniFile);
          lstrcat(szParameters, L"\"");
        }
        else
          lstrcat(szParameters, L"0");

        lstrcat(szParameters, L" -n");
        wndpl.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd, &wndpl);

        hMonitor = MonitorFromRect(&wndpl.rcNormalPosition, MONITOR_DEFAULTTONEAREST);
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(hMonitor, &mi);

        // offset new window position +10/+10
        x = wndpl.rcNormalPosition.left + 10;
        y = wndpl.rcNormalPosition.top + 10;
        cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
        cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;

        // check if window fits monitor
        if ((x + cx) > mi.rcWork.right || (y + cy) > mi.rcWork.bottom)
        {
          x = mi.rcMonitor.left;
          y = mi.rcMonitor.top;
        }

        imax = IsZoomed(hwnd);

        wsprintf(tch, L" -pos %i,%i,%i,%i,%i", x, y, cx, cy, imax);
        lstrcat(szParameters, tch);

        if (LOWORD(wParam) != IDM_FILE_NEWWINDOW2 && lstrlen(szCurFile))
        {
          lstrcpy(szFileName, szCurFile);
          PathQuoteSpaces(szFileName);
          lstrcat(szParameters, L" ");
          lstrcat(szParameters, szFileName);
        }

        ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = /*SEE_MASK_NOZONECHECKS*/0x00800000;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = szModuleName;
        sei.lpParameters = szParameters;
        sei.lpDirectory = g_wchWorkingDirectory;
        sei.nShow = SW_SHOWNORMAL;

        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_LAUNCH: {
        SHELLEXECUTEINFO sei;
        WCHAR wchDirectory[MAX_PATH] = L"";

        if (!lstrlen(szCurFile))
          break;

        if ((iSaveBeforeRunningTools != SBRT_DISABLED) && !FileSave(FALSE, TRUE, FALSE, FALSE, FALSE))
          break;

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
        sei.lpFile = szCurFile;
        sei.lpParameters = NULL;
        sei.lpDirectory = wchDirectory;
        sei.nShow = SW_SHOWNORMAL;

        ShellExecuteEx(&sei);
      }
      break;

    // [2e]: Open Folder command #136
    case IDM_FILE_OPENFOLDER: {
        SHELLEXECUTEINFO sei = { 0 };
        WCHAR wchParams[MAX_PATH] = L"";
        int iParamsSize = _countof(wchParams) - 1;

        if (!lstrlen(szCurFile))
          break;

        wcscpy_s(wchParams, iParamsSize, L"/select, \"");
        wcscat_s(wchParams, iParamsSize, szCurFile);
        wcscat_s(wchParams, iParamsSize, L"\"");

        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = L"explorer.exe";
        sei.lpParameters = wchParams;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;

        ShellExecuteEx(&sei);
      }
      break;
    // [/2e]


    case IDM_FILE_RUN: {
        WCHAR tchCmdLine[MAX_PATH + 4];
        if ((iSaveBeforeRunningTools != SBRT_DISABLED) && !FileSave(FALSE, TRUE, FALSE, FALSE, FALSE))
          break;

        lstrcpy(tchCmdLine, szCurFile);
        PathQuoteSpaces(tchCmdLine);

        RunDlg(hwnd, tchCmdLine);
      }
      break;


    // [2e]: File context menu #12
    case ID_FILE_INVOKESHELLMENU: {
        if (lstrlen(szCurFile) > 0 &&
            PathFileExists(szCurFile)
            )
        {
          // [2e]: Hide pointer while typing #230
          n2e_OnMouseVanishEvent(TRUE);
          // [/2e]
          n2e_ExplorerCxtMenu(szCurFile, hwnd);
        }
      }
      break;
    // [/2e]


    case IDM_FILE_OPENWITH:
      if ((iSaveBeforeRunningTools != SBRT_DISABLED) && !FileSave(FALSE, TRUE, FALSE, FALSE, FALSE))
        break;
      OpenWithDlg(hwnd, szCurFile);
      break;


    case IDM_FILE_PAGESETUP:
      EditPrintSetup(hwndEdit);
      break;


    case IDM_FILE_PRINT: {
        SHFILEINFO shfi;
        WCHAR *pszTitle;
        WCHAR tchUntitled[32];
        WCHAR tchPageFmt[32];

        if (lstrlen(szCurFile))
        {
          SHGetFileInfo2(szCurFile, 0, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME);
          pszTitle = shfi.szDisplayName;
        }
        else
        {
          GetString(IDS_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszTitle = tchUntitled;
        }

        GetString(IDS_PRINT_PAGENUM, tchPageFmt, COUNTOF(tchPageFmt));

        if (!EditPrint(hwndEdit, pszTitle, tchPageFmt))
          MsgBox(MBWARN, IDS_PRINT_ERROR, pszTitle);
      }
      break;
    case IDM_FILE_PROPERTIES: {
        SHELLEXECUTEINFO sei;

        if (lstrlen(szCurFile) == 0)
          break;

        ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = SEE_MASK_INVOKEIDLIST;
        sei.hwnd = hwnd;
        sei.lpVerb = L"properties";
        sei.lpFile = szCurFile;
        sei.nShow = SW_SHOWNORMAL;

        ShellExecuteEx(&sei);
      }
      break;
    case IDM_FILE_CREATELINK: {
        if (!lstrlen(szCurFile))
        {
          break;
        }
        if (!PathCreateDeskLnk(szCurFile))
        {
          MsgBox(MBWARN, IDS_ERR_CREATELINK);
        }
      }
      break;

    // [2e]: Process elevation #166
    case IDM_FILE_ELEVATE:
      n2e_SwitchElevation();
      break;
    // [/2e]


    case IDM_FILE_OPENFAV:
      if (FileSave(FALSE, TRUE, FALSE, FALSE, FALSE))
      {
        WCHAR tchSelItem[MAX_PATH];

        if (FavoritesDlg(hwnd, tchSelItem))
        {
          if (PathIsLnkToDirectory(tchSelItem, NULL, 0))
            PathGetLnkPath(tchSelItem, tchSelItem, COUNTOF(tchSelItem));

          if (PathIsDirectory(tchSelItem))
          {
            WCHAR tchFile[MAX_PATH];

            if (OpenFileDlg(hwndMain, tchFile, COUNTOF(tchFile), tchSelItem))
              FileLoad(TRUE, FALSE, FALSE, FALSE, tchFile);
          }
          else
            FileLoad(TRUE, FALSE, FALSE, FALSE, tchSelItem);
        }
      }
      break;


    case IDM_FILE_ADDTOFAV:
      if (lstrlen(szCurFile))
      {
        SHFILEINFO shfi;
        SHGetFileInfo2(szCurFile, 0, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME);
        AddToFavDlg(hwnd, shfi.szDisplayName, szCurFile);
      }
      break;


    case IDM_FILE_MANAGEFAV: {
        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = tchFavoritesDir;
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;

        // Run favorites directory
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_RECENT:
      if (MRU_Enum(pFileMRU, 0, NULL, 0) > 0)
      {
        if (FileSave(FALSE, TRUE, FALSE, FALSE, FALSE))
        {
          WCHAR tchFile[MAX_PATH];
          if (FileMRUDlg(hwnd, tchFile))
          {
            FileLoad(TRUE, FALSE, FALSE, FALSE, tchFile);
            // [2e]: file history not updated when reloading recent files
            MRU_AddFile(pFileMRU, tchFile, flagRelativeFileMRU, flagPortableMyDocs);
            if (flagUseSystemMRU == 2)
            {
              SHAddToRecentDocs(SHARD_PATHW, tchFile);
            }
            // [/2e]
          }
        }
      }
      break;


    // [2e]: "Recall previous" command
    case ID_FILE_OPENPREVIOUS:
      if (MRU_Enum(pFileMRU, 0, NULL, 0) > 0)
      {
        if (FileSave(FALSE, TRUE, FALSE, FALSE, FALSE))
        {
          WCHAR tchFile[MAX_PATH];
          if (n2e_OpenMRULast(tchFile))
          {
            _FileLoad(TRUE, FALSE, FALSE, FALSE, tchFile, TRUE);
          }
        }
      }
      break;
    // [/2e]


    case IDM_FILE_EXIT:
      SendMessage(hwnd, WM_CLOSE, 0, 0);
      break;


    case IDM_ENCODING_ANSI:
    case IDM_ENCODING_UNICODE:
    case IDM_ENCODING_UNICODEREV:
    case IDM_ENCODING_UTF8:
    case IDM_ENCODING_UTF8SIGN:
    case IDM_ENCODING_SELECT:
      {
        int iNewEncoding = iEncoding;
        int pos, anch, fw;
        // [2e]: Retain caret position on File > Encoding #7
        pos = SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        anch = SendMessage(hwndEdit, SCI_GETANCHOR, 0, 0);
        fw = SendMessage(hwndEdit, SCI_GETFIRSTVISIBLELINE, 0, 0);
        fw = SendMessage(hwndEdit, SCI_DOCLINEFROMVISIBLE, fw, 0);
        // [/2e]
        if (LOWORD(wParam) == IDM_ENCODING_SELECT && !SelectEncodingDlg(hwnd, &iNewEncoding))
          break;
        else
        {
          switch (LOWORD(wParam))
          {
            case IDM_ENCODING_UNICODE:
              iNewEncoding = CPI_UNICODEBOM;
              break;
            case IDM_ENCODING_UNICODEREV:
              iNewEncoding = CPI_UNICODEBEBOM;
              break;
            case IDM_ENCODING_UTF8:
              iNewEncoding = CPI_UTF8;
              break;
            case IDM_ENCODING_UTF8SIGN:
              iNewEncoding = CPI_UTF8SIGN;
              break;
            case IDM_ENCODING_ANSI:
              iNewEncoding = CPI_DEFAULT;
              break;
          }
        }

        if (EditSetNewEncoding(hwndEdit,
                               iEncoding, iNewEncoding,
                               (flagSetEncoding), lstrlen(szCurFile) == 0))
        {
          if (SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0) == 0)
          {
            iEncoding = iNewEncoding;
            iOriginalEncoding = iNewEncoding;
          }
          else
          {
            if (iEncoding == CPI_DEFAULT || iNewEncoding == CPI_DEFAULT)
              iOriginalEncoding = -1;
            iEncoding = iNewEncoding;
          }

          UpdateToolbar();
          UpdateStatusbar();

          n2e_UpdateWindowTitle(hwnd);
        }
        // [2e]: Retain caret position on File > Encoding #7
        SendMessage(hwndEdit, SCI_SETANCHOR, anch, 0);
        SendMessage(hwndEdit, SCI_SETCURRENTPOS, pos, 0);        
        SendMessage(hwndEdit, SCI_SETFIRSTVISIBLELINE, SendMessage(hwndEdit, SCI_VISIBLEFROMDOCLINE, fw, 0), 0);
        // [/2e]
      }
      break;


    case IDM_ENCODING_RECODE: {
        if (lstrlen(szCurFile))
        {
          WCHAR tchCurFile2[MAX_PATH];

          int iNewEncoding = -1;
          if (iEncoding != CPI_DEFAULT)
            iNewEncoding = iEncoding;
          if (iEncoding == CPI_UTF8SIGN)
            iNewEncoding = CPI_UTF8;
          if (iEncoding == CPI_UNICODEBOM)
            iNewEncoding = CPI_UNICODE;
          if (iEncoding == CPI_UNICODEBEBOM)
            iNewEncoding = CPI_UNICODEBE;

          if (n2e_IsDocumentModified() && MsgBox(MBOKCANCEL, IDS_ASK_RECODE) != IDOK)
            return (0);

          if (RecodeDlg(hwnd, &iNewEncoding))
          {
            // [2e]: Retain caret position on File > Encoding #7
            int pos, anch, fw;
            pos = SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
            anch = SendMessage(hwndEdit, SCI_GETANCHOR, 0, 0);
            fw = SendMessage(hwndEdit, SCI_GETFIRSTVISIBLELINE, 0, 0);
            fw = SendMessage(hwndEdit, SCI_DOCLINEFROMVISIBLE, fw, 0);
            // [/2e]
            lstrcpy(tchCurFile2, szCurFile);
            iSrcEncoding = iNewEncoding;
            FileLoad(TRUE, FALSE, TRUE, FALSE, tchCurFile2);
            // [2e]: Retain caret position on File > Encoding #7
            SendMessage(hwndEdit, SCI_SETANCHOR, anch, 0);
            SendMessage(hwndEdit, SCI_SETCURRENTPOS, pos, 0);
            SendMessage(hwndEdit, SCI_SETFIRSTVISIBLELINE, SendMessage(hwndEdit, SCI_VISIBLEFROMDOCLINE, fw, 0), 0);
            // [/2e]
          }
        }
      }
      break;


    case IDM_ENCODING_SETDEFAULT:
      SelectDefEncodingDlg(hwnd, &iDefaultEncoding);
      break;


    case IDM_LINEENDINGS_CRLF:
    case IDM_LINEENDINGS_LF:
    case IDM_LINEENDINGS_CR: {
        int iNewEOLMode = iLineEndings[LOWORD(wParam) - IDM_LINEENDINGS_CRLF];
        iEOLMode = iNewEOLMode;
        SendMessage(hwndEdit, SCI_SETEOLMODE, iEOLMode, 0);
        SendMessage(hwndEdit, SCI_CONVERTEOLS, iEOLMode, 0);
        EditFixPositions(hwndEdit);
        UpdateToolbar();
        UpdateStatusbar();
        n2e_UpdateWindowTitle(hwnd);
      }
      break;


    case IDM_LINEENDINGS_SETDEFAULT:
      SelectDefLineEndingDlg(hwnd, &iDefaultEOLMode);
      break;


    case IDM_EDIT_UNDO:
      // [2e]: Edit highlighted word #18
      if (n2e_IsSelectionEditModeOn())
      {
        n2e_SelectionEditStop(hwndEdit, SES_REJECT);
      }
      // [/2e]
      else
      {
        SendMessage(hwndEdit, SCI_UNDO, 0, 0);
      }
      break;


    case IDM_EDIT_REDO:
      SendMessage(hwndEdit, SCI_REDO, 0, 0);
      break;


    // [2e]: "Go To Last Change" menu command (Undo+Redo)
    case ID_EDIT_UNDO_REDO:
      // [2e]: Go To Last Change - do not Redo if no Undo #306
      if (SendMessage(hwndEdit, SCI_CANUNDO, 0, 0))
      {
        SendMessage(hwndEdit, SCI_UNDO, 0, 0);
        SendMessage(hwndEdit, SCI_REDO, 0, 0);
        EditSelectEx(hwndEdit, SciCall_GetAnchor(), SciCall_GetCurrentPos());
      }
      break;
    // [/2e]


    case IDM_EDIT_CUT:
      if (flagPasteBoard)
        bLastCopyFromMe = TRUE;
      // [2e]: Copy/Cut to clipboard commands to work on empty selection (next word) #358
      SendMessage(hwndMain, WM_COMMAND, MAKELONG(IDM_EDIT_SELECTWORD, 1), SCI_CUT);
      break;


    case IDM_EDIT_COPY:
      if (flagPasteBoard)
        bLastCopyFromMe = TRUE;
      // [2e]: Copy/Cut to clipboard commands to work on empty selection (next word) #358
      SendMessage(hwndMain, WM_COMMAND, MAKELONG(IDM_EDIT_SELECTWORD, 1), SCI_COPY);
      UpdateToolbar();
      break;


    case IDM_EDIT_COPYALL:
      if (flagPasteBoard)
        bLastCopyFromMe = TRUE;
      SendMessage(hwndEdit, SCI_COPYRANGE, 0, SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0));
      UpdateToolbar();
      break;


    case IDM_EDIT_COPYADD:
      if (flagPasteBoard)
        bLastCopyFromMe = TRUE;
      EditCopyAppend(hwndEdit);
      UpdateToolbar();
      break;


    case IDM_EDIT_PASTE:
      SendMessage(hwndEdit, SCI_PASTE, 0, 0);
      break;


    case IDM_EDIT_SWAP:
      if (SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0) -
          SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0) == 0)
      {
        int iNewPos = -1;
        int iPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        SendMessage(hwndEdit, SCI_PASTE, 0, 0);
        iNewPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        SendMessage(hwndEdit, SCI_SETSEL, iPos, iNewPos);
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_CLEARCLIPBOARD, 1), 0);
      }
      else
      {
        int iPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        int iAnchor = (int)SendMessage(hwndEdit, SCI_GETANCHOR, 0, 0);
        char *pClip = EditGetClipboardText(hwndEdit);
        if (flagPasteBoard)
          bLastCopyFromMe = TRUE;
        SendMessage(hwndEdit, SCI_BEGINUNDOACTION, 0, 0);
        SendMessage(hwndEdit, SCI_CUT, 0, 0);
        SendMessage(hwndEdit, SCI_REPLACESEL, (WPARAM)0, (LPARAM)pClip);
        if (iPos > iAnchor)
          SendMessage(hwndEdit, SCI_SETSEL, iAnchor, iAnchor + lstrlenA(pClip));
        else
          SendMessage(hwndEdit, SCI_SETSEL, iPos + lstrlenA(pClip), iPos);
        SendMessage(hwndEdit, SCI_ENDUNDOACTION, 0, 0);
        LocalFree(pClip);
      }
      break;


    case IDM_EDIT_CLEAR:
      SendMessage(hwndEdit, SCI_CLEAR, 0, 0);
      break;


    case IDM_EDIT_CLEARCLIPBOARD:
      if (OpenClipboard(hwnd))
      {
        if (CountClipboardFormats() > 0)
        {
          EmptyClipboard();
          UpdateToolbar();
          UpdateStatusbar();
        }
        CloseClipboard();
      }
      break;


    case IDM_EDIT_SELECTALL:
      // [2e]: Implicit commit of Edit Mode on Shift+Tab and Ctrl+A #403
      if (n2e_IsSelectionEditModeOn())
      {
        n2e_SelectionEditStop(hwndEdit, SES_APPLY);
      }
      SendMessage(hwndEdit, SCI_SELECTALL, 0, 0);
      break;


    case IDM_EDIT_SELECTWORD: {
        int iWordStart = SciCall_GetSelStart();
        int iWordEnd = SciCall_GetSelEnd();
        // [2e]: Always select closest word #205
        if (iWordEnd - iWordStart <= 0)
        {
          const int iPos = SciCall_GetCurrentPos();
          const int iLine = SciCall_LineFromPosition(iPos);
          iWordStart = SciCall_GetWordStartPos(iPos, TRUE);
          iWordEnd = SciCall_GetWordEndPos(iPos, TRUE);
          if (iWordStart == iWordEnd)
          {
            // search forward
            const int iLineEndPos = SciCall_LineEndPosition(iLine);
            int i = iPos;
            while ((i < iLineEndPos) && (iWordStart == iWordEnd))
            {
              iWordStart = SciCall_GetWordStartPos(i, TRUE);
              iWordEnd = SciCall_GetWordEndPos(iWordStart, TRUE);
              i = SciCall_PositionAfter(iWordEnd);
            }
          }
          if (iWordStart == iWordEnd)
          {
            // search backward
            const int iLineStartPos = SciCall_PositionFromLine(iLine);
            int i = iPos;
            while ((i > iLineStartPos) && (iWordStart == iWordEnd))
            {
              iWordEnd = SciCall_GetWordEndPos(i, TRUE);
              iWordStart = SciCall_GetWordStartPos(iWordEnd, TRUE);
              i = SciCall_PositionBefore(iWordStart);
            }
          }
          if (iWordStart == iWordEnd)
          {
            // search forward, ignore EOLs
            const int iLastLine = SciCall_GetLineCount();
            for (int j = iLine + 1; j < iLastLine; ++j)
            {
              int i = SciCall_PositionFromLine(j);
              int iLineEndPos = SciCall_LineEndPosition(j);
              while ((i < iLineEndPos) && (iWordStart == iWordEnd))
              {
                iWordStart = SciCall_GetWordStartPos(i, TRUE);
                iWordEnd = SciCall_GetWordEndPos(iWordStart, TRUE);
                i = SciCall_PositionAfter(iWordEnd);
              }
              if (iWordStart != iWordEnd)
                break;
            }
          }
        }
        // [2e]: Copy/Cut to clipboard commands to work on empty selection (next word) #358
        switch (lParam)
        {
        case SCI_CUT:
          SciCall_SetSel(iWordStart, iWordEnd);
          SendMessage(hwndEdit, lParam, 0, 0);
          SciCall_SetSel(iWordStart, iWordStart);
          SciCall_SetAnchor(iWordStart);
          break;
        case SCI_COPY: {
          const LPSTR text = n2e_GetTextRange(iWordStart, iWordEnd);
          const LPWSTR wtext = n2e_MultiByteToWideString(text);
          n2e_SetClipboardText(hwndEdit, wtext);
          n2e_Free(wtext);
          n2e_Free(text);
          }
          break;
        default:
          SciCall_SetSel(iWordStart, iWordEnd);
          break;
        }
        // [/2e]
      }
      break;


    case IDM_EDIT_SELECTLINE: {
        int iSelStart = (int)SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0);
        int iSelEnd = (int)SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0);
        int iLineStart = (int)SendMessage(hwndEdit, SCI_LINEFROMPOSITION, iSelStart, 0);
        int iLineEnd = (int)SendMessage(hwndEdit, SCI_LINEFROMPOSITION, iSelEnd, 0);
        iSelStart = (int)SendMessage(hwndEdit, SCI_POSITIONFROMLINE, iLineStart, 0);
        iSelEnd = (int)SendMessage(hwndEdit, SCI_POSITIONFROMLINE, iLineEnd + 1, 0);
        SendMessage(hwndEdit, SCI_SETSEL, iSelStart, iSelEnd);
        SendMessage(hwndEdit, SCI_CHOOSECARETX, 0, 0);
      }
      break;


    case IDM_EDIT_MOVELINEUP:
      EditMoveUp(hwndEdit);
      break;


    case IDM_EDIT_MOVELINEDOWN:
      EditMoveDown(hwndEdit);
      break;


    // [2e]: Start new line operations
    case IDM_EDIT_NEWLINEABOVE:
      n2e_EditInsertNewLine(hwndEdit, TRUE);
      break;


    case IDM_EDIT_NEWLINEBELOW:
      n2e_EditInsertNewLine(hwndEdit, FALSE);
      break;
    // [/2e]


    case IDM_EDIT_DUPLICATELINE:
      // [2e]: Edit highlighted word #18
      n2e_SelectionEditStop(hwndEdit, SES_APPLY);
      SendMessage(hwndEdit, SCI_LINEDUPLICATE, 0, 0);
      break;


    case IDM_EDIT_CUTLINE:
      if (flagPasteBoard)
        bLastCopyFromMe = TRUE;
      SendMessage(hwndEdit, SCI_LINECUT, 0, 0);
      break;


    case IDM_EDIT_COPYLINE:
      if (flagPasteBoard)
        bLastCopyFromMe = TRUE;
      SendMessage(hwndEdit, SCI_LINECOPY, 0, 0);
      UpdateToolbar();
      break;


    case IDM_EDIT_DELETELINE:
      SendMessage(hwndEdit, SCI_LINEDELETE, 0, 0);
      break;


    case IDM_EDIT_DELETELINELEFT:
      SendMessage(hwndEdit, SCI_DELLINELEFT, 0, 0);
      break;


    case IDM_EDIT_DELETELINERIGHT:
      SendMessage(hwndEdit, SCI_DELLINERIGHT, 0, 0);
      break;


    case IDM_EDIT_INDENT:
      {
        int iLineSelStart = (int)SendMessage(hwndEdit, SCI_LINEFROMPOSITION,
                                             (int)SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0), 0);
        int iLineSelEnd = (int)SendMessage(hwndEdit, SCI_LINEFROMPOSITION,
                                           (int)SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0), 0);
        SendMessage(hwndEdit, SCI_SETTABINDENTS, TRUE, 0);
        if (iLineSelStart == iLineSelEnd)
        {
          SendMessage(hwndEdit, SCI_VCHOME, 0, 0);
          SendMessage(hwndEdit, SCI_TAB, 0, 0);
        }
        else
          SendMessage(hwndEdit, SCI_TAB, 0, 0);
        SendMessage(hwndEdit, SCI_SETTABINDENTS, bTabIndents, 0);
      }
      break;


    case IDM_EDIT_UNINDENT:
      // [2e]: Implicit commit of Edit Mode on Shift+Tab and Ctrl+A #403
      if (n2e_IsSelectionEditModeOn())
      {
        n2e_SelectionEditStop(hwndEdit, SES_APPLY);
      }
      // [2e]: Unindent and tabs #128
      SendMessage(hwndEdit, SCI_BACKTAB, 0, 0);
      break;


    case IDM_EDIT_ENCLOSESELECTION:
      if (EditEncloseSelectionDlg(hwnd, wchPrefixSelection, wchAppendSelection))
      {
        BeginWaitCursor();
        EditEncloseSelection(hwndEdit, wchPrefixSelection, wchAppendSelection);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_SELECTIONDUPLICATE:
      SendMessage(hwndEdit, SCI_BEGINUNDOACTION, 0, 0);
      SendMessage(hwndEdit, SCI_SELECTIONDUPLICATE, 0, 0);
      SendMessage(hwndEdit, SCI_ENDUNDOACTION, 0, 0);
      break;


    case IDM_EDIT_PADWITHSPACES:
      BeginWaitCursor();
      EditPadWithSpaces(hwndEdit, FALSE, FALSE);
      EndWaitCursor();
      break;


    case IDM_EDIT_STRIP1STCHAR:
      BeginWaitCursor();
      EditStripFirstCharacter(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_STRIPLASTCHAR:
      BeginWaitCursor();
      EditStripLastCharacter(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_TRIMLINES:
      BeginWaitCursor();
      EditStripTrailingBlanks(hwndEdit, FALSE);
      EndWaitCursor();
      break;


    case IDM_EDIT_COMPRESSWS:
      BeginWaitCursor();
      EditCompressSpaces(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_MERGEBLANKLINES:
      BeginWaitCursor();
      EditRemoveBlankLines(hwndEdit, TRUE);
      EndWaitCursor();
      break;


    case IDM_EDIT_REMOVEBLANKLINES:
      BeginWaitCursor();
      EditRemoveBlankLines(hwndEdit, FALSE);
      EndWaitCursor();
      break;


    case IDM_EDIT_MODIFYLINES:
      if (EditModifyLinesDlg(hwnd, wchPrefixLines, wchAppendLines))
      {
        BeginWaitCursor();
        EditModifyLines(hwndEdit, wchPrefixLines, wchAppendLines);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_ALIGN:
      if (EditAlignDlg(hwnd, &iAlignMode))
      {
        BeginWaitCursor();
        EditAlignText(hwndEdit, iAlignMode);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_SORTLINES:
      if (EditSortDlg(hwnd, &iSortOptions))
      {
        BeginWaitCursor();
        StatusSetText(hwndStatus, 255, L"...");
        StatusSetSimple(hwndStatus, TRUE);
        InvalidateRect(hwndStatus, NULL, TRUE);
        UpdateWindow(hwndStatus);
        EditSortLines(hwndEdit, iSortOptions);
        StatusSetSimple(hwndStatus, FALSE);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_COLUMNWRAP: {
        if (iWrapCol == 0)
          iWrapCol = iLongLinesLimit;

        if (ColumnWrapDlg(hwnd, IDD_COLUMNWRAP, &iWrapCol))
        {
          iWrapCol = max(min(iWrapCol, 512), 1);
          BeginWaitCursor();
          EditWrapToColumn(hwndEdit, iWrapCol);
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_SPLITLINES:
      BeginWaitCursor();
      // [2e]: Change Ctrl+I to perform comment-aware line wrapping #320
      if (bExtendedSplitLines)
      {
        n2e_SplitLines(hwndEdit);
      }
      else
      {
        SendMessage(hwndEdit, SCI_TARGETFROMSELECTION, 0, 0);
        SendMessage(hwndEdit, SCI_LINESSPLIT, 0, 0);
      }
      EndWaitCursor();
      break;


    case IDM_EDIT_JOINLINES:
      BeginWaitCursor();
      // [2e]: Join Lines/Paragraphs - ignore trailing break #135
      if (n2e_JoinLines_InitSelection())
      {
        SendMessage(hwndEdit, SCI_TARGETFROMSELECTION, 0, 0);
        SendMessage(hwndEdit, SCI_LINESJOIN, 0, 0);
        EditJoinLinesEx(hwndEdit);
      }
      EndWaitCursor();
      break;


    case IDM_EDIT_JOINLINESEX:
      BeginWaitCursor();
      EditJoinLinesEx(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_CONVERTUPPERCASE:
      BeginWaitCursor();
      // [2e]: Case commands to work on empty selection (next word) #319
      SendMessage(hwndMain, WM_COMMAND, MAKELONG(IDM_EDIT_SELECTWORD, 1), 0);
      SendMessage(hwndEdit, SCI_UPPERCASE, 0, 0);
      EndWaitCursor();
      break;


    case IDM_EDIT_CONVERTLOWERCASE:
      BeginWaitCursor();
      // [2e]: Case commands to work on empty selection (next word) #319
      SendMessage(hwndMain, WM_COMMAND, MAKELONG(IDM_EDIT_SELECTWORD, 1), 0);
      SendMessage(hwndEdit, SCI_LOWERCASE, 0, 0);
      EndWaitCursor();
      break;


    case IDM_EDIT_INVERTCASE:
      BeginWaitCursor();
      // [2e]: Case commands to work on empty selection (next word) #319
      SendMessage(hwndMain, WM_COMMAND, MAKELONG(IDM_EDIT_SELECTWORD, 1), 0);
      EditInvertCase(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_TITLECASE:
      BeginWaitCursor();
      // [2e]: Case commands to work on empty selection (next word) #319
      SendMessage(hwndMain, WM_COMMAND, MAKELONG(IDM_EDIT_SELECTWORD, 1), 0);
      EditTitleCase(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_SENTENCECASE:
      BeginWaitCursor();
      EditSentenceCase(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_CONVERTTABS:
      BeginWaitCursor();
      EditTabsToSpaces(hwndEdit, iTabWidth, FALSE);
      EndWaitCursor();
      break;


    case IDM_EDIT_CONVERTSPACES:
      BeginWaitCursor();
      EditSpacesToTabs(hwndEdit, iTabWidth, FALSE);
      EndWaitCursor();
      break;


    case IDM_EDIT_CONVERTTABS2:
      BeginWaitCursor();
      EditTabsToSpaces(hwndEdit, iTabWidth, TRUE);
      EndWaitCursor();
      break;


    case IDM_EDIT_CONVERTSPACES2:
      BeginWaitCursor();
      EditSpacesToTabs(hwndEdit, iTabWidth, TRUE);
      EndWaitCursor();
      break;


    case IDM_EDIT_INSERT_TAG: {
        WCHAR wszOpen[TEXT_BUFFER_LENGTH] = L"";
        WCHAR wszClose[TEXT_BUFFER_LENGTH] = L"";
        if (EditInsertTagDlg(hwnd, wszOpen, wszClose))
          EditEncloseSelection(hwndEdit, wszOpen, wszClose);
      }
      break;


    case IDM_EDIT_INSERT_ENCODING: {
        if (*mEncoding[iEncoding].pszParseNames)
        {
          char msz[32];
          char *p;
          lstrcpynA(msz, mEncoding[iEncoding].pszParseNames, COUNTOF(msz));
          if (p = StrChrA(msz, ','))
            *p = 0;
          SendMessage(hwndEdit, SCI_REPLACESEL, 0, (LPARAM)msz);
        }
      }
      break;


    case IDM_EDIT_INSERT_SHORTDATE:
    case IDM_EDIT_INSERT_LONGDATE: {
        WCHAR tchDate[128];
        WCHAR tchTime[128];
        WCHAR tchDateTime[256];
        WCHAR tchTemplate[256];
        SYSTEMTIME st;
        char  mszBuf[MAX_PATH * 3];
        UINT  uCP;
        GetLocalTime(&st);

        if (IniGetString(L"Settings2",
          (LOWORD(wParam) == IDM_EDIT_INSERT_SHORTDATE) ? L"DateTimeShort" : L"DateTimeLong",
                         L"", tchTemplate, COUNTOF(tchTemplate)))
        {
          struct tm sst;
          sst.tm_isdst = -1;
          sst.tm_sec = (int)st.wSecond;
          sst.tm_min = (int)st.wMinute;
          sst.tm_hour = (int)st.wHour;
          sst.tm_mday = (int)st.wDay;
          sst.tm_mon = (int)st.wMonth - 1;
          sst.tm_year = (int)st.wYear - 1900;
          sst.tm_wday = (int)st.wDayOfWeek;
          mktime(&sst);
          wcsftime(tchDateTime, COUNTOF(tchDateTime), tchTemplate, &sst);
        }
        else
        {
          GetDateFormat(LOCALE_USER_DEFAULT, (
            LOWORD(wParam) == IDM_EDIT_INSERT_SHORTDATE) ? DATE_SHORTDATE : DATE_LONGDATE,
            &st, NULL, tchDate, COUNTOF(tchDate));
          GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, tchTime, COUNTOF(tchTime));
          wsprintf(tchDateTime, L"%s %s", tchTime, tchDate);
        }
        uCP = (SendMessage(hwndEdit, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8) ? CP_UTF8 : CP_ACP;
        WideCharToMultiByte(uCP, 0, tchDateTime, -1, mszBuf, COUNTOF(mszBuf), NULL, NULL);
        SendMessage(hwndEdit, SCI_REPLACESEL, 0, (LPARAM)mszBuf);
      }
      break;


    case IDM_EDIT_INSERT_FILENAME:
    case IDM_EDIT_INSERT_PATHNAME: {
        SHFILEINFO shfi;
        WCHAR *pszInsert;
        WCHAR tchUntitled[32];
        char  mszBuf[MAX_PATH * 3];
        UINT  uCP;

        if (lstrlen(szCurFile))
        {
          if (LOWORD(wParam) == IDM_EDIT_INSERT_FILENAME)
          {
            SHGetFileInfo2(szCurFile, 0, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME);
            pszInsert = shfi.szDisplayName;
          }
          else
            pszInsert = szCurFile;
        }
        else
        {
          GetString(IDS_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszInsert = tchUntitled;
        }
        uCP = (SendMessage(hwndEdit, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8) ? CP_UTF8 : CP_ACP;
        WideCharToMultiByte(uCP, 0, pszInsert, -1, mszBuf, COUNTOF(mszBuf), NULL, NULL);
        SendMessage(hwndEdit, SCI_REPLACESEL, 0, (LPARAM)mszBuf);
      }
      break;


    // [2e]: Edit > Insert > Random #221
    case IDM_EDIT_INSERT_RANDOM:
      {
        char mszBuf[MAX_PATH] = { 0 };
        sprintf(mszBuf, "%05d", n2e_GenerateRandom());
        SciCall_ReplaceSel(0, mszBuf);
        SciCall_SetSel(SciCall_GetAnchor() - strlen(mszBuf), SciCall_GetSelStart());
      }
      break;
    // [/2e]


    case IDM_EDIT_LINECOMMENT:
      {
        const COMMENTINFO* pCommentInfo = n2e_GetCommentInfo(SendMessage(hwndEdit, SCI_GETLEXER, 0, 0));
        if (wcslen(pCommentInfo->pszLineCommentW) > 0)
        {
        case SCLEX_AHK:
          BeginWaitCursor();
          EditToggleLineComments(hwndEdit, pCommentInfo->pszLineCommentW, pCommentInfo->bInsertLineCommentAtLineStart);
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_STREAMCOMMENT:
      {
        const COMMENTINFO* pCommentInfo = n2e_GetCommentInfo(SendMessage(hwndEdit, SCI_GETLEXER, 0, 0));
        if (wcslen(pCommentInfo->pszStreamStartW) > 0)
        {
          BeginWaitCursor();
          EditEncloseSelection(hwndEdit, pCommentInfo->pszStreamStartW, pCommentInfo->pszStreamEndW);
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_URLENCODE:
      BeginWaitCursor();
      // [2e]: Url encode issue #189
      if (iUrlEncodeMode == UEM_LEGACY)
      {
        EditURLEncode(hwndEdit);
      }
      else
      {
        n2e_EditString2URL(hwndEdit);
      }
      EndWaitCursor();
      break;


    case IDM_EDIT_URLDECODE:
      BeginWaitCursor();
      // [2e]: Url encode issue #189
      if (iUrlEncodeMode == UEM_LEGACY)
      {
        EditURLDecode(hwndEdit);
      }
      else
      {
        n2e_EditURL2String(hwndEdit);
      }
      EndWaitCursor();
      break;


    case IDM_EDIT_ESCAPECCHARS:
      BeginWaitCursor();
      EditEscapeCChars(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_UNESCAPECCHARS:
      BeginWaitCursor();
      EditUnescapeCChars(hwndEdit);
      EndWaitCursor();
      break;


    // [2e]: Alt+Shift+X: strip HTML tags #40
    case IDM_EDIT_STRIP_HTML_TAGS:
      BeginWaitCursor();
      n2e_StripHTMLTags(hwndEdit);
      EndWaitCursor();
      break;

    // [2e]: Escape HTML #51
    case ID_SPECIAL_ESCAPEHTML:
      BeginWaitCursor();
      n2e_EscapeHTML(hwndEdit);
      EndWaitCursor();
      break;
    // [/2e]


    case IDM_EDIT_CHAR2HEX:
      BeginWaitCursor();
      EditChar2Hex(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_HEX2CHAR:
      BeginWaitCursor();
      EditHex2Char(hwndEdit);
      EndWaitCursor();
      break;


    // [2e]: Char/Hex #87
    case IDM_EDIT_STRING2HEX:
      BeginWaitCursor();
      n2e_EditString2Hex(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_HEX2STRING:
      BeginWaitCursor();
      n2e_EditHex2String(hwndEdit);
      EndWaitCursor();
      break;
    // [/2e]


    // [2e]: Char/Hex #87
    case IDM_EDIT_BASE64ENCODE:
      BeginWaitCursor();
      n2e_EditString2Base64(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_BASE64DECODE:
      BeginWaitCursor();
      n2e_EditBase642String(hwndEdit);
      EndWaitCursor();
      break;
    // [/2e]


    // [2e]: QP de/coding #124
    case IDM_EDIT_QPENCODE:
      BeginWaitCursor();
      n2e_EditString2QP(hwndEdit);
      EndWaitCursor();
      break;


    case IDM_EDIT_QPDECODE:
      BeginWaitCursor();
      n2e_EditQP2String(hwndEdit);
      EndWaitCursor();
      break;
    // [/2e]


    case IDM_EDIT_FINDMATCHINGBRACE: {
        int iBrace2 = -1;
        int iBraceAtPos = 1;
        int iPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        char c = (char)SendMessage(hwndEdit, SCI_GETCHARAT, iPos, 0);
        // [2e]: Treat quotes as braces #287
        if (!StrChrA(n2e_GetBracesList(), c))
        {
          iPos = (int)SendMessage(hwndEdit, SCI_POSITIONBEFORE, iPos, 0);
          c = (char)SendMessage(hwndEdit, SCI_GETCHARAT, iPos, 0);
          iBraceAtPos = 0;
        }
        // [2e]: Treat quotes as braces #287
        if (StrChrA(n2e_GetBracesList(), c))
          iBrace2 = (int)SendMessage(hwndEdit, SCI_BRACEMATCH, iPos, bTreatQuotesAsBraces);
        if (iBrace2 != -1)
        {
          // [2e]: Find/Select To Matching Brace - depend on caret location #293
          if (iBraceAtPos && (iFindSelectToMatchingBraceMode == FSM_IMPROVED_FIND_SELECT))
          {
            iBrace2 = SendMessage(hwndEdit, SCI_POSITIONAFTER, iBrace2, 0);
          }
          // [/2e]
          SendMessage(hwndEdit, SCI_GOTOPOS, (WPARAM)iBrace2, 0);
        }
      }
      break;


    case IDM_EDIT_SELTOMATCHINGBRACE: {
        int iBrace2 = -1;
        int iBraceAtPos = 1;
        int iPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        char c = (char)SendMessage(hwndEdit, SCI_GETCHARAT, iPos, 0);
        // [2e]: Treat quotes as braces #287
        if (!StrChrA(n2e_GetBracesList(), c))
        {
          iPos = SciCall_PositionBefore(iPos);
          c = SciCall_GetCharAt(iPos);
          iBraceAtPos = 0;
        }
        // [2e]: Treat quotes as braces #287
        if (StrChrA(n2e_GetBracesList(), c))
          iBrace2 = (int)SendMessage(hwndEdit, SCI_BRACEMATCH, iPos, bTreatQuotesAsBraces);
        if (iBrace2 != -1)
        {
          // [2e]: Find/Select To Matching Brace - depend on caret location #293
          if ((iBraceAtPos != (iBrace2 > iPos))
            && ((iFindSelectToMatchingBraceMode == FSM_IMPROVED_FIND_SELECT)
             || (iFindSelectToMatchingBraceMode == FSM_IMPROVED_SELECT)))
          {
            // [2e]: Select To Matching Brace to include brackets #367
            const int iSelEnd = SciCall_GetSelEnd();
            if (iBrace2 == iSelEnd)
            {
              // no additional setup required, do nothing
            }
            else if (iBrace2 > iPos)
            {
              iBrace2 = SciCall_PositionBefore(iBrace2);
              iPos = SciCall_PositionAfter(iPos);
            }
            else if (iSelEnd == SciCall_GetSelStart())
            {
              iBrace2 = SciCall_PositionAfter(iBrace2);
              iPos = SciCall_PositionBefore(iPos);
            }
          }
          // [2e]: Find/Select To Matching Brace - depend on caret location #293
          if (iBrace2 > iPos)
            SciCall_SetSel(iPos, SciCall_PositionAfter(iBrace2));
          else
            SciCall_SetSel(SciCall_PositionAfter(iPos), iBrace2);
          // [/2e]
        }
      }
      break;


    case IDM_EDIT_FIND:
      if (IsWindow(hDlgGotoLine))
      {
        SendMessage(hDlgGotoLine, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOFIND, 1), 0);
        DestroyWindow(hDlgGotoLine);
      }
      if (!IsWindow(hDlgFindReplace))
      {
        hDlgFindReplace = EditFindReplaceDlg(hwndEdit, &efrData, FALSE);
      }
      else
      {
        if (GetDlgItem(hDlgFindReplace, IDC_REPLACE))
        {
          SendMessage(hDlgFindReplace, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOFIND, 1), 0);
          DestroyWindow(hDlgFindReplace);
          hDlgFindReplace = EditFindReplaceDlg(hwndEdit, &efrData, FALSE);
        }
        else
        {
          SetForegroundWindow(hDlgFindReplace);
          PostMessage(hDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hDlgFindReplace, IDC_FINDTEXT)), 1);
        }
      }
      break;


    // [2e]: Ctrl+Shift+* - search for word #38
    case ID_EDIT_FINDNEXTWORD:
    case ID_EDIT_FINDPREVIOUSWORD:
      n2e_FindNextWord(hwndEdit, &efrData, ID_EDIT_FINDNEXTWORD == LOWORD(wParam));
      break;
    // [/2e]


    case IDM_EDIT_FINDNEXT:
    case IDM_EDIT_FINDPREV:
    case IDM_EDIT_REPLACENEXT:
    case IDM_EDIT_SELTONEXT:
    case IDM_EDIT_SELTOPREV:
    // [2e]: New command: Insert Replacement #387
    case IDM_EDIT_INSERTREPLACEMENT:

      if (SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0) == 0)
        break;

      if (!lstrlenA(efrData.szFind))
      {
        if ((LOWORD(wParam) != IDM_EDIT_REPLACENEXT) && (LOWORD(wParam) != IDM_EDIT_INSERTREPLACEMENT))
          SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_FIND, 1), 0);
        else
          SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_REPLACE, 1), 0);
      }
      else
      {
        UINT cp = (UINT)SendMessage(hwndEdit, SCI_GETCODEPAGE, 0, 0);
        if (cpLastFind != cp)
        {
          if (cp != SC_CP_UTF8)
          {
            WCHAR wch[TEXT_BUFFER_LENGTH];
            MultiByteToWideChar(CP_UTF8, 0, efrData.szFindUTF8, -1, wch, COUNTOF(wch));
            WideCharToMultiByte(cp, 0, wch, -1, efrData.szFind, COUNTOF(efrData.szFind), NULL, NULL);
            MultiByteToWideChar(CP_UTF8, 0, efrData.szReplaceUTF8, -1, wch, COUNTOF(wch));
            WideCharToMultiByte(cp, 0, wch, -1, efrData.szReplace, COUNTOF(efrData.szReplace), NULL, NULL);
          }
          else
          {
            lstrcpyA(efrData.szFind, efrData.szFindUTF8);
            lstrcpyA(efrData.szReplace, efrData.szReplaceUTF8);
          }
        }
        cpLastFind = cp;
        switch (LOWORD(wParam))
        {
          case IDM_EDIT_FINDNEXT:
            EditFindNext(n2e_GetActiveEditCheckFocus(), &efrData, FALSE);
            break;

          case IDM_EDIT_FINDPREV:
            EditFindPrev(n2e_GetActiveEditCheckFocus(), &efrData, FALSE);
            break;

          case IDM_EDIT_REPLACENEXT:
            if (bReplaceInitialized)
              EditReplace(n2e_GetActiveEditCheckFocus(), &efrData, TRUE);
            else
              SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_REPLACE, 1), 0);
            break;

          // [2e]: New command: Insert Replacement #387
          case IDM_EDIT_INSERTREPLACEMENT:
            if (bReplaceInitialized)
              EditReplace(n2e_GetActiveEditCheckFocus(), &efrData, FALSE);
            else
              SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_REPLACE, 1), 0);
            break;
          // [/2e]

          case IDM_EDIT_SELTONEXT:
            EditFindNext(n2e_GetActiveEditCheckFocus(), &efrData, TRUE);
            break;

          case IDM_EDIT_SELTOPREV:
            EditFindPrev(n2e_GetActiveEditCheckFocus(), &efrData, TRUE);
            break;
        }
      }
      break;


    case IDM_EDIT_REPLACE:
      if (IsWindow(hDlgGotoLine))
      {
        SendMessage(hDlgGotoLine, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOREPLACE, 1), 0);
        DestroyWindow(hDlgGotoLine);
      }
      if (!IsWindow(hDlgFindReplace))
      {
        hDlgFindReplace = EditFindReplaceDlg(hwndEdit, &efrData, TRUE);
      }
      else
      {
        if (!GetDlgItem(hDlgFindReplace, IDC_REPLACE))
        {
          SendMessage(hDlgFindReplace, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOREPLACE, 1), 0);
          DestroyWindow(hDlgFindReplace);
          hDlgFindReplace = EditFindReplaceDlg(hwndEdit, &efrData, TRUE);
        }
        else
        {
          SetForegroundWindow(hDlgFindReplace);
          PostMessage(hDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hDlgFindReplace, IDC_FINDTEXT)), 1);
        }
      }
      break;


    case IDM_EDIT_GOTOLINE:
      {
        gotoData.bForceDefaultInit = !IsWindow(hDlgFindReplace);
        if (IsWindow(hDlgFindReplace))
        {
          SendMessage(hDlgFindReplace, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOGOTO, 1), 0);
          DestroyWindow(hDlgFindReplace);
        }
        if (!IsWindow(hDlgGotoLine))
        {
          hDlgGotoLine = EditLinenumDlg(hwndEdit, &gotoData);
        }
        else
        {
          SetForegroundWindow(hDlgGotoLine);
        }
      }
      break;


    case IDM_VIEW_SCHEME:
      Style_SelectLexerDlg(hwndEdit);
      UpdateStatusbar();
      VIEW_COMMAND(UpdateLineNumberWidth);
      break;


    case IDM_VIEW_USE2NDDEFAULT:
      Style_ToggleUse2ndDefault(hwndEdit);
      UpdateStatusbar();
      VIEW_COMMAND(UpdateLineNumberWidth);
      break;


    case IDM_VIEW_SCHEMECONFIG:
      Style_ConfigDlg(hwndEdit);
      UpdateStatusbar();
      VIEW_COMMAND(UpdateLineNumberWidth);
      break;


    case IDM_VIEW_FONT:
      Style_SetDefaultFont(hwndEdit);
      UpdateStatusbar();
      VIEW_COMMAND(UpdateLineNumberWidth);
      break;


    case IDM_VIEW_WORDWRAP:
      fWordWrap = (fWordWrap) ? FALSE : TRUE;
      VIEW_COMMAND(SetWordWrap);
      fWordWrapG = fWordWrap;
      UpdateToolbar();
      break;


    case IDM_VIEW_WORDWRAPSETTINGS:
      if (WordWrapSettingsDlg(hwnd, IDD_WORDWRAP, &iWordWrapIndent))
      {
        if (fWordWrap)
          SendMessage(hwndEdit, SCI_SETWRAPMODE, (iWordWrapMode == 0) ? SC_WRAP_WORD : SC_WRAP_CHAR, 0);
        if (iWordWrapIndent == 5)
          SendMessage(hwndEdit, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_SAME, 0);
        else if (iWordWrapIndent == 6)
          SendMessage(hwndEdit, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_INDENT, 0);
        else
        {
          int i = 0;
          switch (iWordWrapIndent)
          {
            case 1:
              i = 1;
              break;
            case 2:
              i = 2;
              break;
            case 3:
              i = (iIndentWidth) ? 1 * iIndentWidth : 1 * iTabWidth;
              break;
            case 4:
              i = (iIndentWidth) ? 2 * iIndentWidth : 2 * iTabWidth;
              break;
          }
          SendMessage(hwndEdit, SCI_SETWRAPSTARTINDENT, i, 0);
          SendMessage(hwndEdit, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_FIXED, 0);
        }
        if (bShowWordWrapSymbols)
        {
          int wrapVisualFlags = 0;
          int wrapVisualFlagsLocation = 0;
          if (iWordWrapSymbols == 0)
            iWordWrapSymbols = 22;
          switch (iWordWrapSymbols % 10)
          {
            case 1:
              wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
              wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_END_BY_TEXT;
              break;
            case 2:
              wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
              break;
          }
          switch (((iWordWrapSymbols % 100) - (iWordWrapSymbols % 10)) / 10)
          {
            case 1:
              wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
              wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_START_BY_TEXT;
              break;
            case 2:
              wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
              break;
          }
          SendMessage(hwndEdit, SCI_SETWRAPVISUALFLAGSLOCATION, wrapVisualFlagsLocation, 0);
          SendMessage(hwndEdit, SCI_SETWRAPVISUALFLAGS, wrapVisualFlags, 0);
        }
        else
        {
          SendMessage(hwndEdit, SCI_SETWRAPVISUALFLAGS, 0, 0);
        }
      }
      break;


    // [2e]: New toolbar button: selection mode #339
    case IDM_VIEW_SELECTEX:
      fSelectEx = (fSelectEx) ? FALSE : TRUE;
      SetSelectEx(hwndEdit);
      UpdateToolbar();
      break;
    // [/2e]


    case IDM_VIEW_LONGLINEMARKER:
      bMarkLongLines = (bMarkLongLines) ? FALSE : TRUE;
      VIEW_COMMAND(SetLongLineSettings);
      UpdateStatusbar();
      break;


    case IDM_VIEW_LONGLINESETTINGS:
      if (LongLineSettingsDlg(hwnd, IDD_LONGLINES, &iLongLinesLimit))
      {
        bMarkLongLines = TRUE;
        iLongLinesLimit = max(min(iLongLinesLimit, 4096), 0);
        VIEW_COMMAND(SetLongLineSettings);
        UpdateStatusbar();
        iLongLinesLimitG = iLongLinesLimit;
      }
      break;


    case IDM_VIEW_TABSASSPACES:
      bTabsAsSpaces = (bTabsAsSpaces) ? FALSE : TRUE;
      SendMessage(hwndEdit, SCI_SETUSETABS, !bTabsAsSpaces, 0);
      bTabsAsSpacesG = bTabsAsSpaces;
      break;


    case IDM_VIEW_TABSETTINGS:
      if (TabSettingsDlg(hwnd, IDD_TABSETTINGS, NULL))
      {
        SendMessage(hwndEdit, SCI_SETUSETABS, !bTabsAsSpaces, 0);
        SendMessage(hwndEdit, SCI_SETTABINDENTS, bTabIndents, 0);
        SendMessage(hwndEdit, SCI_SETBACKSPACEUNINDENTS, bBackspaceUnindents, 0);
        iTabWidth = max(min(iTabWidth, 256), 1);
        iIndentWidth = max(min(iIndentWidth, 256), 0);
        SendMessage(hwndEdit, SCI_SETTABWIDTH, iTabWidth, 0);
        SendMessage(hwndEdit, SCI_SETINDENT, iIndentWidth, 0);
        bTabsAsSpacesG = bTabsAsSpaces;
        bTabIndentsG = bTabIndents;
        iTabWidthG = iTabWidth;
        iIndentWidthG = iIndentWidth;
        if (SendMessage(hwndEdit, SCI_GETWRAPINDENTMODE, 0, 0) == SC_WRAPINDENT_FIXED)
        {
          int i = 0;
          switch (iWordWrapIndent) {
            case 1: i = 1; break;
            case 2: i = 2; break;
            case 3: i = (iIndentWidth) ? 1 * iIndentWidth : 1 * iTabWidth; break;
            case 4: i = (iIndentWidth) ? 2 * iIndentWidth : 2 * iTabWidth; break;
          }
          SendMessage(hwndEdit, SCI_SETWRAPSTARTINDENT, i, 0);
        }
      }
      break;


    case IDM_VIEW_SHOWINDENTGUIDES:
      bShowIndentGuides = (bShowIndentGuides) ? FALSE : TRUE;
      VIEW_COMMAND(Style_SetIndentGuides);
      break;


    case IDM_VIEW_AUTOINDENTTEXT:
      bAutoIndent = (bAutoIndent) ? FALSE : TRUE;
      break;


    case IDM_VIEW_LINENUMBERS:
      bShowLineNumbers = (bShowLineNumbers) ? FALSE : TRUE;
      VIEW_COMMAND(UpdateLineNumberWidth);
      break;

    // [2e]: View > St&arting Line Number... #342
    case IDM_VIEW_STARTINGLINENUMBER:
      ThemedDialogBox(g_hInstance, MAKEINTRESOURCE(IDD_STARTINGLINENUMBER), hwnd, StartingLineNumberDlgProc);
      SendMessage(hwndEdit, SCI_SETSTARTINGLINENUMBER, iStartingLineNumber, 0);
      VIEW_COMMAND(UpdateLineNumberWidth);
      break;
    // [/2e]


    case IDM_VIEW_MARGIN:
      bShowSelectionMargin = (bShowSelectionMargin) ? FALSE : TRUE;
      VIEW_COMMAND(ShowSelectionMargin);
      break;


    // [2e]: Add View > First Column Margin #382
    case IDM_VIEW_FIRST_COLUMN_MARGIN:
      bShowFirstColumnMargin = (bShowFirstColumnMargin) ? FALSE : TRUE;
      VIEW_COMMAND(ShowFirstColumnMargin);
      break;
    // [/2e]


    case IDM_VIEW_SHOWWHITESPACE:
      bViewWhiteSpace = (bViewWhiteSpace) ? FALSE : TRUE;
      VIEW_COMMAND(ShowWhiteSpace);
      break;


    case IDM_VIEW_SHOWEOLS:
      bViewEOLs = (bViewEOLs) ? FALSE : TRUE;
      VIEW_COMMAND(SetViewEOL);
      break;


    case IDM_VIEW_WORDWRAPSYMBOLS:
      bShowWordWrapSymbols = (bShowWordWrapSymbols) ? 0 : 1;
      VIEW_COMMAND(SetShowWordWrapSymbols);
      break;


    case IDM_VIEW_MATCHBRACES:
      bMatchBraces = (bMatchBraces) ? FALSE : TRUE;
      if (bMatchBraces)
      {
        struct SCNotification scn;
        scn.nmhdr.hwndFrom = hwndEdit;
        scn.nmhdr.idFrom = IDC_EDIT;
        scn.nmhdr.code = SCN_UPDATEUI;
        scn.updated = SC_UPDATE_CONTENT;
        SendMessage(hwnd, WM_NOTIFY, IDC_EDIT, (LPARAM)&scn);
      }
      else
        VIEW_COMMAND(HideMatchBraces);
      break;


    case IDM_VIEW_AUTOCLOSETAGS:
      bAutoCloseTags = (bAutoCloseTags) ? FALSE : TRUE;
      break;


    case IDM_VIEW_HIGHLIGHTCURRENTLINE:
      bHighlightCurrentLine = (bHighlightCurrentLine) ? FALSE : TRUE;
      VIEW_COMMAND(Style_SetCurrentLineBackground);
      break;


    case IDM_VIEW_ZOOMIN:
      SendMessage(hwndEdit, SCI_ZOOMIN, 0, 0);
      break;


    case IDM_VIEW_ZOOMOUT:
      SendMessage(hwndEdit, SCI_ZOOMOUT, 0, 0);
      break;


    case IDM_VIEW_RESETZOOM:
      SendMessage(hwndEdit, SCI_SETZOOM, 0, 0);
      break;


    case IDM_VIEW_TOOLBAR:
      if (bShowToolbar)
      {
        bShowToolbar = 0;
        ShowWindow(hwndReBar, SW_HIDE);
      }
      else
      {
        bShowToolbar = 1;
        UpdateToolbar();
        ShowWindow(hwndReBar, SW_SHOW);
      }
      SendWMSize(hwnd);
      break;


    case IDM_VIEW_CUSTOMIZETB:
      SendMessage(hwndToolbar, TB_CUSTOMIZE, 0, 0);
      break;


    case IDM_VIEW_STATUSBAR:
      if (bShowStatusbar)
      {
        bShowStatusbar = 0;
        ShowWindow(hwndStatus, SW_HIDE);
      }
      else
      {
        bShowStatusbar = 1;
        UpdateStatusbar();
        ShowWindow(hwndStatus, SW_SHOW);
      }
      SendWMSize(hwnd);
      break;


    case IDM_VIEW_STICKYWINPOS:
      bStickyWinPos = IniGetInt(L"Settings2", L"StickyWindowPosition", bStickyWinPos);
      if (!bStickyWinPos)
      {
        WINDOWPLACEMENT wndpl;
        WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32];

        int ResX = GetSystemMetrics(SM_CXSCREEN);
        int ResY = GetSystemMetrics(SM_CYSCREEN);

        // GetWindowPlacement
        wndpl.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwndMain, &wndpl);
 
        wi.x = wndpl.rcNormalPosition.left;
        wi.y = wndpl.rcNormalPosition.top;
        wi.cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
        wi.cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
        wi.max = (IsZoomed(hwndMain) || (wndpl.flags & WPF_RESTORETOMAXIMIZED));
 
        wsprintf(tchPosX, L"%ix%i PosX", ResX, ResY);
        wsprintf(tchPosY, L"%ix%i PosY", ResX, ResY);
        wsprintf(tchSizeX, L"%ix%i SizeX", ResX, ResY);
        wsprintf(tchSizeY, L"%ix%i SizeY", ResX, ResY);
        wsprintf(tchMaximized, L"%ix%i Maximized", ResX, ResY);

        bStickyWinPos = 1;
        IniSetInt(L"Settings2", L"StickyWindowPosition", 1);

        IniSetInt(L"Window", tchPosX, wi.x);
        IniSetInt(L"Window", tchPosY, wi.y);
        IniSetInt(L"Window", tchSizeX, wi.cx);
        IniSetInt(L"Window", tchSizeY, wi.cy);
        IniSetInt(L"Window", tchMaximized, wi.max);

        InfoBox(0, L"MsgStickyWinPos", IDS_STICKYWINPOS);
      }
      else
      {
        bStickyWinPos = 0;
        IniSetInt(L"Settings2", L"StickyWindowPosition", 0);
      }
      break;


    case IDM_VIEW_REUSEWINDOW:
      if (IniGetInt(L"Settings2", L"ReuseWindow", 0))
        IniSetInt(L"Settings2", L"ReuseWindow", 0);
      else
        IniSetInt(L"Settings2", L"ReuseWindow", 1);
      break;


    case IDM_VIEW_SINGLEFILEINSTANCE:
      if (IniGetInt(L"Settings2", L"SingleFileInstance", 0))
        IniSetInt(L"Settings2", L"SingleFileInstance", 0);
      else
        IniSetInt(L"Settings2", L"SingleFileInstance", 1);
      break;


    case IDM_VIEW_ALWAYSONTOP:
      if ((bAlwaysOnTop || flagAlwaysOnTop == 2) && flagAlwaysOnTop != 1)
      {
        bAlwaysOnTop = 0;
        flagAlwaysOnTop = 0;
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      }
      else
      {
        bAlwaysOnTop = 1;
        flagAlwaysOnTop = 0;
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      }
      n2e_UpdateAlwaysOnTopButton();
      break;


    case IDM_VIEW_MINTOTRAY:
      bMinimizeToTray = (bMinimizeToTray) ? FALSE : TRUE;
      break;


    case IDM_VIEW_TRANSPARENT:
      bTransparentMode = (bTransparentMode) ? FALSE : TRUE;
      SetWindowTransparentMode(hwnd, bTransparentMode);
      break;


    case IDM_VIEW_SHOWFILENAMEONLY:
      iPathNameFormat = PNM_FILENAMEONLY;
      lstrcpy(szTitleExcerpt, L"");
      n2e_UpdateWindowTitle(hwnd);
      break;


    case IDM_VIEW_SHOWFILENAMEFIRST:
      iPathNameFormat = PNM_FILENAMEFIRST;
      lstrcpy(szTitleExcerpt, L"");
      n2e_UpdateWindowTitle(hwnd);
      break;


    case IDM_VIEW_SHOWFULLPATH:
      iPathNameFormat = PNM_FULLPATH;
      lstrcpy(szTitleExcerpt, L"");
      n2e_UpdateWindowTitle(hwnd);
      break;


    case IDM_VIEW_SHOWEXCERPT:
      EditGetExcerpt(hwndEdit, szTitleExcerpt, COUNTOF(szTitleExcerpt));
      n2e_UpdateWindowTitle(hwnd);
      break;


    // [2e]: Language indication #86
    case IDM_VIEW_NOLANGUAGEINDICATOR:
      iShowLanguageInTitle = LIT_HIDE;
      n2e_UpdateWindowTitle(hwnd);
      break;


    case IDM_VIEW_SHOWLANGUAGEINDICATOR:
      iShowLanguageInTitle = LIT_SHOW;
      n2e_UpdateWindowTitle(hwnd);
      break;


    case IDM_VIEW_SHOWLANGUAGEINDICATORNONUS:
      iShowLanguageInTitle = LIT_SHOW_NON_US;
      n2e_UpdateWindowTitle(hwnd);
      break;
    // [/2e]


    case IDM_VIEW_NOSAVERECENT:
      bSaveRecentFiles = (bSaveRecentFiles) ? FALSE : TRUE;
      break;


    case IDM_VIEW_NOSAVEFINDREPL:
      bSaveFindReplace = (bSaveFindReplace) ? FALSE : TRUE;
      break;


    // [2e]: Add INI setting to disable file save prompt in Launch > New Window #361
    case IDM_VIEW_SAVEBEFORERUNNINGTOOLS_DISABLED:
      iSaveBeforeRunningTools = SBRT_DISABLED;
      break;


    case IDM_VIEW_SAVEBEFORERUNNINGTOOLS_ENABLED:
      iSaveBeforeRunningTools = SBRT_ENABLED;
      break;

    case IDM_VIEW_SAVEBEFORERUNNINGTOOLS_EXCEPT_NEW_WINDOW:
      iSaveBeforeRunningTools = SBRT_EXCEPT_NEW_WINDOW;
      break;
    // [/2e]


    case IDM_VIEW_CHANGENOTIFY:
      if (ChangeNotifyDlg(hwnd))
        InstallFileWatching(szCurFile);
      break;


    case IDM_VIEW_NOESCFUNC:
      iEscFunction = EEF_IGNORE;
      break;


    case IDM_VIEW_ESCMINIMIZE:
      iEscFunction = EEF_MINIMIZE;
      break;


    case IDM_VIEW_ESCEXIT:
      iEscFunction = EEF_EXIT;
      break;

    // [2e]: Split view #316
    case IDM_VIEW_ESCCLOSEVIEW:
      bEscForCurrentSplitView = (bEscForCurrentSplitView) ? FALSE : TRUE;
      break;
    // [/2e]


    // [2e]: Save on exit and History #101
    case IDM_VIEW_SAVESETTINGS_MODE_ALL:
      nSaveSettingsMode = SSM_ALL;
      UpdateToolbar();
      break;


    case IDM_VIEW_SAVESETTINGS_MODE_RECENT:
      nSaveSettingsMode = SSM_RECENT;
      UpdateToolbar();
      break;


    case IDM_VIEW_SAVESETTINGS_MODE_NO:
      nSaveSettingsMode = SSM_NO;
      UpdateToolbar();
      break;
    // [/2e]


    case IDM_VIEW_SAVESETTINGSNOW: {
        BOOL bCreateFailure = FALSE;
        if (lstrlen(szIniFile) == 0)
        {
          if (lstrlen(szIniFile2) > 0)
          {
            if (CreateIniFileEx(szIniFile2))
            {
              lstrcpy(szIniFile, szIniFile2);
              lstrcpy(szIniFile2, L"");
            }
            else
              bCreateFailure = TRUE;
          }
          else
            break;
        }
        if (!bCreateFailure)
        {
          if (WritePrivateProfileString(L"Settings", L"WriteTest", L"ok", szIniFile))
          {
            BeginWaitCursor();
            StatusSetTextID(hwndStatus, STATUS_HELP, IDS_SAVINGSETTINGS);
            StatusSetSimple(hwndStatus, TRUE);
            InvalidateRect(hwndStatus, NULL, TRUE);
            UpdateWindow(hwndStatus);
            SaveSettings(TRUE);
            StatusSetSimple(hwndStatus, FALSE);
            EndWaitCursor();
            MsgBox(MBINFO, IDS_SAVEDSETTINGS);
          }
          else
          {
            dwLastIOError = GetLastError();
            MsgBox(MBWARN, IDS_WRITEINI_FAIL);
          }
        }
        else
          MsgBox(MBWARN, IDS_CREATEINI_FAIL);
      }
      break;


    // [2e]: Edit highlighted word #18
    case ID_EDIT_EDITSELECTION:
      n2e_SelectionEditStart(SM_ALL);
      return 1;


    case ID_EDIT_EDITSELECTION_LINE:
      n2e_SelectionEditStart(SM_LINE);
      return 1;


    // [2e]: Alt+Backtick: enter Edit Mode with inverse line/document-wise setting #366
    case ID_EDIT_EDITSELECTION_INVERSED:
      n2e_SelectionEditStart(SM_INVERSED_ALL);
      return 1;


    // [2e]: Replace settings in all instances #5
    case ID_SETTINGS_RELOADFROMDISK:
      PostMessage(hwnd, WM_N2E_RELOAD_SETTINGS, 0, 0);
      break;
    // [/2e]


    // [2e]: Replace settings in all instances #5
    case ID_SETTINGS_REPLACESETTINGSINALLINSTANCES: {
        MsgCommand(hwnd, MAKEWPARAM(IDM_VIEW_SAVESETTINGSNOW, 0), 0);
        n2e_Reload_Settings();
      }
      break;
    // [/2e]


    // [2e]: Ctrl+Wheel scroll feature
    case ID_SETTINGS_CTRL_WHEEL_SCROLL:
      bCtrlWheelScroll = (bCtrlWheelScroll) ? FALSE : TRUE;
      n2e_SetWheelScroll(bCtrlWheelScroll);
      break;
    // [/2e]


    // [2e]: Implement Notepad's right click behavior #54
    case ID_SETTINGS_MOVE_CARET_ON_RCLICK:
      bMoveCaretOnRightClick = !bMoveCaretOnRightClick;
      SendMessage(hwndEdit, SCI_MOVECARETONRCLICK, bMoveCaretOnRightClick, 0);
      break;
    // [/2e]


    // [2e]: MathEval INI setting #88
    case ID_SETTINGS_EVAL_DISABLED:
      iEvaluateMathExpression = EEM_DISABLED;
      UpdateStatusbar();
      break;


    case ID_SETTINGS_EVAL_SELECTION:
      iEvaluateMathExpression = EEM_SELECTION;
      UpdateStatusbar();
      break;


    case ID_SETTINGS_EVAL_LINE:
      iEvaluateMathExpression = EEM_LINE;
      UpdateStatusbar();
      break;

    // [2e]: Add command to copy result of math expression #414
    case ID_SETTINGS_EVAL_COPYRESULT:
      n2e_CopyEvaluatedExpressionToClipboard();
      break;
    // [/2e]


    // [2e]: ctrl + arrow behavior toggle #89
    case ID_SETTINGS_WORD_NAVIGATION:
      iWordNavigationMode = (iWordNavigationMode == WNM_STANDARD) ? WNM_ACCELERATED : WNM_STANDARD;
      SendMessage(hwndEdit, SCI_SETWORDNAVIGATIONMODE, iWordNavigationMode, 0);
      break;
    // [/2e]


    case IDM_HELP_COMMANDLINEARGUMENTS:
      DisplayCmdLineHelp(hwnd);
      break;


    case IDM_HELP_MANUAL:
      if ((int)ShellExecute(hwnd, L"open", L"https://github.com/ProgerXP/Notepad2e#readme", NULL, NULL, SW_SHOWNORMAL) <= 32)
      {
        SendMessage(hwndMain, WM_COMMAND, MAKELONG(IDM_HELP_ABOUT, 1), 0);
      }
      break;


    case IDM_HELP_ABOUT:
      ThemedDialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT),
                      hwnd, AboutDlgProc);
      break;


    // [2e]: Attribution menu command #181
    case IDM_HELP_ABOUT_3RD_PARTY:
      ThemedDialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT_3RD_PARTY),
                      hwnd, About3rdPartyDlgProc);
      break;
    // [/2e]


    case CMD_ESCAPE:
      // [2e]: Edit highlighted word #18
      if (n2e_IsSelectionEditModeOn())
      {
        n2e_SelectionEditStop(hwndEdit, SES_REJECT);
      }
      // [2e]: Make Escape in main window close visible dialog, if any #398
      else if (IsWindow(hDlgFindReplace) || IsWindow(hDlgGotoLine))
      {
        SendMessage(IsWindow(hDlgFindReplace) ? hDlgFindReplace : hDlgGotoLine, WM_SYSCOMMAND, SC_CLOSE, 0);
      }
      // [/2e]
      else
      {
        // [2e]: Split view #316
        const BOOL bEscUtilized = bEscForCurrentSplitView && n2e_CloseView();
        if (!bEscUtilized)
        {
          switch (iEscFunction)
          {
          case EEF_MINIMIZE:
            SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            break;
          case EEF_EXIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
          case EEF_IGNORE:
          default:
            break;
          }
        }
      }
      break;


    case CMD_SHIFTESC:
      if (FileSave(TRUE, FALSE, FALSE, FALSE, FALSE))
        SendMessage(hwnd, WM_CLOSE, 0, 0);
      break;


    // Newline with toggled auto indent setting
    case CMD_CTRLENTER:
      // [2e]: Edit highlighted word #18
      if (n2e_IsSelectionEditModeOn())
      {
        n2e_SelectionEditStop(hwndEdit, SES_APPLY);
      }
      // [/2e]
      else
      {
        bAutoIndent = (bAutoIndent) ? 0 : 1;
        SendMessage(hwndEdit, SCI_NEWLINE, 0, 0);
        bAutoIndent = (bAutoIndent) ? 0 : 1;
      }
      break;


    case CMD_CTRLBACK: {
        int iPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        int iAnchor = (int)SendMessage(hwndEdit, SCI_GETANCHOR, 0, 0);
        int iLine = (int)SendMessage(hwndEdit, SCI_LINEFROMPOSITION, (WPARAM)iPos, 0);
        int iStartPos = (int)SendMessage(hwndEdit, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
        int iIndentPos = (int)SendMessage(hwndEdit, SCI_GETLINEINDENTPOSITION, (WPARAM)iLine, 0);

        if (iPos != iAnchor)
          SendMessage(hwndEdit, SCI_REPLACESEL, 0, (LPARAM)"");
        else
        {
          if (iPos == iStartPos)
            SendMessage(hwndEdit, SCI_DELETEBACK, 0, 0);
          else if (iPos <= iIndentPos)
            SendMessage(hwndEdit, SCI_DELLINELEFT, 0, 0);
          else
            SendMessage(hwndEdit, SCI_DELWORDLEFT, 0, 0);
        }
      }
      break;


    case CMD_CTRLDEL: {
        int iPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        int iAnchor = (int)SendMessage(hwndEdit, SCI_GETANCHOR, 0, 0);
        int iLine = (int)SendMessage(hwndEdit, SCI_LINEFROMPOSITION, (WPARAM)iPos, 0);
        int iStartPos = (int)SendMessage(hwndEdit, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
        int iEndPos = (int)SendMessage(hwndEdit, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);
        if (iPos != iAnchor)
        {
          SendMessage(hwndEdit, SCI_REPLACESEL, 0, (LPARAM)"");
        }
        else
        {
          if (iStartPos != iEndPos)
            SendMessage(hwndEdit, SCI_DELWORDRIGHT, 0, 0);
          else
            SendMessage(hwndEdit, SCI_LINEDELETE, 0, 0);
        }
      }
      break;


    case CMD_CTRLTAB:
      SendMessage(hwndEdit, SCI_SETTABINDENTS, FALSE, 0);
      SendMessage(hwndEdit, SCI_SETUSETABS, TRUE, 0);
      SendMessage(hwndEdit, SCI_TAB, 0, 0);
      SendMessage(hwndEdit, SCI_SETUSETABS, !bTabsAsSpaces, 0);
      SendMessage(hwndEdit, SCI_SETTABINDENTS, bTabIndents, 0);
      break;


    case CMD_RECODEDEFAULT: {
        WCHAR tchCurFile2[MAX_PATH];
        if (lstrlen(szCurFile))
        {
          if (iDefaultEncoding == CPI_UNICODEBOM)
            iSrcEncoding = CPI_UNICODE;
          else if (iDefaultEncoding == CPI_UNICODEBEBOM)
            iSrcEncoding = CPI_UNICODEBE;
          else if (iDefaultEncoding == CPI_UTF8SIGN)
            iSrcEncoding = CPI_UTF8;
          else
            iSrcEncoding = iDefaultEncoding;
          lstrcpy(tchCurFile2, szCurFile);
          FileLoad(FALSE, FALSE, TRUE, FALSE, tchCurFile2);
        }
      }
      break;


    case CMD_RELOADANSI: {
        WCHAR tchCurFile2[MAX_PATH];
        if (lstrlen(szCurFile))
        {
          iSrcEncoding = CPI_DEFAULT;
          lstrcpy(tchCurFile2, szCurFile);
          FileLoad(FALSE, FALSE, TRUE, FALSE, tchCurFile2);
        }
      }
      break;


    case CMD_RELOADOEM: {
        WCHAR tchCurFile2[MAX_PATH];
        if (lstrlen(szCurFile))
        {
          iSrcEncoding = CPI_OEM;
          lstrcpy(tchCurFile2, szCurFile);
          FileLoad(FALSE, FALSE, TRUE, FALSE, tchCurFile2);
        }
      }
      break;


    case CMD_RELOADASCIIASUTF8: {
        WCHAR tchCurFile2[MAX_PATH];
        BOOL _bLoadASCIIasUTF8 = bLoadASCIIasUTF8;
        if (lstrlen(szCurFile))
        {
          bLoadASCIIasUTF8 = 1;
          lstrcpy(tchCurFile2, szCurFile);
          FileLoad(FALSE, FALSE, TRUE, FALSE, tchCurFile2);
          bLoadASCIIasUTF8 = _bLoadASCIIasUTF8;
        }
      }
      break;


    case CMD_RELOADNOFILEVARS: {
        WCHAR tchCurFile2[MAX_PATH];
        if (lstrlen(szCurFile))
        {
          int _fNoFileVariables = fNoFileVariables;
          BOOL _bNoEncodingTags = bNoEncodingTags;
          fNoFileVariables = 1;
          bNoEncodingTags = 1;
          lstrcpy(tchCurFile2, szCurFile);
          FileLoad(FALSE, FALSE, TRUE, FALSE, tchCurFile2);
          fNoFileVariables = _fNoFileVariables;
          bNoEncodingTags = _bNoEncodingTags;
        }
      }
      break;


    case CMD_LEXDEFAULT:
      Style_SetDefaultLexer(hwndEdit);
      UpdateStatusbar();
      VIEW_COMMAND(UpdateLineNumberWidth);
      break;


    case CMD_LEXHTML:
      Style_SetHTMLLexer(hwndEdit);
      UpdateStatusbar();
      VIEW_COMMAND(UpdateLineNumberWidth);
      break;


    case CMD_LEXXML:
      Style_SetXMLLexer(hwndEdit);
      UpdateStatusbar();
      VIEW_COMMAND(UpdateLineNumberWidth);
      break;


    case CMD_TIMESTAMPS: {
        WCHAR wchFind[256] = { 0 };
        WCHAR wchTemplate[256] = { 0 };
        WCHAR *pwchSep;
        WCHAR wchReplace[256];

        SYSTEMTIME st;
        struct tm sst;

        UINT cp;
        EDITFINDREPLACE efrTS = { "", "", "", "", SCFIND_REGEXP, 0, 0, 0, 0, 0, SIC_ALWAYS, hwndEdit };
        IniGetString(L"Settings2", L"TimeStamp", L"\\$Date:[^\\$]+\\$ | $Date: %Y/%m/%d %H:%M:%S $", wchFind, COUNTOF(wchFind));
        if (pwchSep = StrChr(wchFind, L'|'))
        {
          lstrcpy(wchTemplate, pwchSep + 1);
          *pwchSep = 0;
        }

        StrTrim(wchFind, L" ");
        StrTrim(wchTemplate, L" ");

        if (lstrlen(wchFind) == 0 || lstrlen(wchTemplate) == 0)
          break;

        GetLocalTime(&st);
        sst.tm_isdst = -1;
        sst.tm_sec   = (int)st.wSecond;
        sst.tm_min   = (int)st.wMinute;
        sst.tm_hour  = (int)st.wHour;
        sst.tm_mday  = (int)st.wDay;
        sst.tm_mon   = (int)st.wMonth - 1;
        sst.tm_year  = (int)st.wYear - 1900;
        sst.tm_wday  = (int)st.wDayOfWeek;
        wcsftime(wchReplace, COUNTOF(wchReplace), wchTemplate, &sst);

        cp = (UINT)SendMessage(hwndEdit, SCI_GETCODEPAGE, 0, 0);
        WideCharToMultiByte(cp, 0, wchFind, -1, efrTS.szFind, COUNTOF(efrTS.szFind), NULL, NULL);
        WideCharToMultiByte(cp, 0, wchReplace, -1, efrTS.szReplace, COUNTOF(efrTS.szReplace), NULL, NULL);

        if (SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0) !=
            SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0))
        {
          EditReplaceAllInSelection(hwndEdit, &efrTS, TRUE);
        }
        else
        {
          EditReplaceAll(hwndEdit, &efrTS, TRUE);
        }
      }
      break;


    case CMD_WEBACTION1:
    case CMD_WEBACTION2: {
        BOOL  bCmdEnabled = FALSE;
        LPWSTR lpszTemplateName;
        WCHAR  szCmdTemplate[256];
        char  mszSelection[512] = { 0 };
        DWORD cchSelection;
        char  *lpsz;
        LPWSTR lpszCommand;
        LPWSTR lpszArgs;
        SHELLEXECUTEINFO sei;
        WCHAR wchDirectory[MAX_PATH] = L"";

        lpszTemplateName = (LOWORD(wParam) == CMD_WEBACTION1) ? L"WebTemplate1" : L"WebTemplate2";

        bCmdEnabled = IniGetString(L"Settings2", lpszTemplateName, L"", szCmdTemplate, COUNTOF(szCmdTemplate));

        if (bCmdEnabled)
        {
          cchSelection = (int)SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0) -
                         (int)SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0);
          if (cchSelection > 0 && cchSelection <= 500 && SendMessage(hwndEdit, SCI_GETSELTEXT, 0, 0) < COUNTOF(mszSelection))
          {
            SendMessage(hwndEdit, SCI_GETSELTEXT, 0, (LPARAM)mszSelection);
            mszSelection[cchSelection] = 0; // zero terminate

            // Check lpszSelection and truncate bad WCHARs
            lpsz = StrChrA(mszSelection, 13);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(mszSelection, 10);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(mszSelection, 9);
            if (lpsz) *lpsz = '\0';

            if (lstrlenA(mszSelection))
            {
              WCHAR wszSelection[512];
              UINT uCP = (SendMessage(hwndEdit, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8) ? CP_UTF8 : CP_ACP;
              MultiByteToWideChar(uCP, 0, mszSelection, -1, wszSelection, COUNTOF(wszSelection));
              lpszCommand = GlobalAlloc(GPTR, sizeof(WCHAR) * (512 + COUNTOF(szCmdTemplate) + MAX_PATH + 32));
              wsprintf(lpszCommand, szCmdTemplate, wszSelection);
              ExpandEnvironmentStringsEx(lpszCommand, (DWORD)GlobalSize(lpszCommand) / sizeof(WCHAR));
              lpszArgs = GlobalAlloc(GPTR, GlobalSize(lpszCommand));
              ExtractFirstArgument(lpszCommand, lpszCommand, lpszArgs);
              if (lstrlen(szCurFile))
              {
                lstrcpy(wchDirectory, szCurFile);
                PathRemoveFileSpec(wchDirectory);
              }

              ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

              sei.cbSize = sizeof(SHELLEXECUTEINFO);
              sei.fMask = /*SEE_MASK_NOZONECHECKS*/0x00800000;
              sei.hwnd = NULL;
              sei.lpVerb = NULL;
              sei.lpFile = lpszCommand;
              sei.lpParameters = lpszArgs;
              sei.lpDirectory = wchDirectory;
              sei.nShow = SW_SHOWNORMAL;

              ShellExecuteEx(&sei);

              GlobalFree(lpszCommand);
              GlobalFree(lpszArgs);
            }
          }
        }
      }
      break;


    case CMD_FINDNEXTSEL:
    case CMD_FINDPREVSEL:
    case IDM_EDIT_SAVEFIND: {
        // [2e]: Save Find Text (Alt+F3) - remove selection #321
        const int iAnchor = SciCall_GetAnchor();
        int iSelectionStart = SciCall_GetSelStart();
        int iSelectionEnd = SciCall_GetSelEnd();
        // [/2e]
        int cchSelection = iSelectionEnd - iSelectionStart;
        if (cchSelection == 0)
        {
          // [2e]: Find Next/Previous must skip first match #424
          if (wCommandID != IDM_EDIT_SAVEFIND)
          {
            SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_SELECTWORD, 1), 0);
            iSelectionStart = SciCall_GetSelStart();
            iSelectionEnd = SciCall_GetSelEnd();
          }
          // [2e]: Save Find Text (Alt+F3) - remove selection #321
          else
          {
            const int iPos = SciCall_GetCurrentPos();
            iSelectionStart = SciCall_GetWordStartPos(iPos, TRUE);
            iSelectionEnd = SciCall_GetWordEndPos(iPos, TRUE);
          }
          cchSelection = iSelectionEnd - iSelectionStart;
          // [/2e]
        }
        if (cchSelection > 0 && cchSelection <= 500)
        {
          char  mszSelection[512];
          char  *lpsz;

          struct Sci_TextRange tr;
          tr.chrg.cpMin = iSelectionStart;
          tr.chrg.cpMax = iSelectionEnd;
          tr.lpstrText = mszSelection;

          SciCall_GetTextRange(0, &tr);
          mszSelection[cchSelection] = 0;

          // Check lpszSelection and truncate newlines
          lpsz = StrChrA(mszSelection, '\n');
          if (lpsz)
          {
            *lpsz = '\0';
          }
          lpsz = StrChrA(mszSelection, '\r');
          if (lpsz)
          {
            *lpsz = '\0';
          }
          cpLastFind = (UINT)SendMessage(hwndEdit, SCI_GETCODEPAGE, 0, 0);
          lstrcpyA(efrData.szFind, mszSelection);

          if (cpLastFind != SC_CP_UTF8)
          {
            WCHAR wszBuf[TEXT_BUFFER_LENGTH];

            MultiByteToWideChar(cpLastFind, 0, mszSelection, -1, wszBuf, COUNTOF(wszBuf));
            WideCharToMultiByte(CP_UTF8, 0, wszBuf, -1, efrData.szFindUTF8, COUNTOF(efrData.szFindUTF8), NULL, NULL);
          }
          else
            lstrcpyA(efrData.szFindUTF8, mszSelection);

          efrData.fuFlags &= (~(SCFIND_REGEXP | SCFIND_POSIX));
          efrData.bTransformBS = FALSE;

          switch (LOWORD(wParam))
          {
            case CMD_FINDNEXTSEL:
              EditFindNext(hwndEdit, &efrData, FALSE);
              break;

            case CMD_FINDPREVSEL:
              EditFindPrev(hwndEdit, &efrData, FALSE);
              break;
          }
        }
      }
      break;


    case CMD_INCLINELIMIT:
    case CMD_DECLINELIMIT:
      if (!bMarkLongLines)
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_LONGLINEMARKER, 1), 0);
      else
      {
        if (LOWORD(wParam) == CMD_INCLINELIMIT)
          iLongLinesLimit++;
        else
          iLongLinesLimit--;
        iLongLinesLimit = max(min(iLongLinesLimit, 4096), 0);
        SendMessage(hwndEdit, SCI_SETEDGECOLUMN, iLongLinesLimit, 0);
        UpdateStatusbar();
        iLongLinesLimitG = iLongLinesLimit;
      }
      break;


    case CMD_STRINGIFY:
      EditEncloseSelection(hwndEdit, L"'", L"'");
      break;


    case CMD_STRINGIFY2:
      EditEncloseSelection(hwndEdit, L"\"", L"\"");
      break;


    // [2e]: Ctrl+Shift+3 - unwrap brackets #39
    case ID_BLOCK_UNWRAPSELECTION:
      n2e_UnwrapSelection(hwndEdit, FALSE);
      break;


    case ID_BLOCK_UNWRAPQUOTES:
      n2e_UnwrapSelection(hwndEdit, TRUE);
      break;
    // [/2e]


    case CMD_EMBRACE:
      EditEncloseSelection(hwndEdit, L"(", L")");
      break;


    case CMD_EMBRACE2:
      EditEncloseSelection(hwndEdit, L"[", L"]");
      break;


    case CMD_EMBRACE3:
      EditEncloseSelection(hwndEdit, L"{", L"}");
      break;


    case CMD_EMBRACE4:
      EditEncloseSelection(hwndEdit, L"`", L"`");
      break;


    case CMD_INCREASENUM:
      EditModifyNumber(hwndEdit, TRUE);
      break;


    case CMD_DECREASENUM:
      EditModifyNumber(hwndEdit, FALSE);
      break;


    case CMD_TOGGLETITLE:
      EditGetExcerpt(hwndEdit, szTitleExcerpt, COUNTOF(szTitleExcerpt));
      n2e_UpdateWindowTitle(hwnd);
      break;


    case CMD_JUMP2SELSTART:
      if (SC_SEL_RECTANGLE != SendMessage(hwnd, SCI_GETSELECTIONMODE, 0, 0))
      {
        int iAnchorPos = (int)SendMessage(hwndEdit, SCI_GETANCHOR, 0, 0);
        int iCursorPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        if (iCursorPos > iAnchorPos)
        {
          SendMessage(hwndEdit, SCI_SETSEL, iCursorPos, iAnchorPos);
          SendMessage(hwndEdit, SCI_CHOOSECARETX, 0, 0);
        }
      }
      break;


    case CMD_JUMP2SELEND:
      if (SC_SEL_RECTANGLE != SendMessage(hwnd, SCI_GETSELECTIONMODE, 0, 0))
      {
        int iAnchorPos = (int)SendMessage(hwndEdit, SCI_GETANCHOR, 0, 0);
        int iCursorPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        if (iCursorPos < iAnchorPos)
        {
          SendMessage(hwndEdit, SCI_SETSEL, iCursorPos, iAnchorPos);
          SendMessage(hwndEdit, SCI_CHOOSECARETX, 0, 0);
        }
      }
      break;


    case CMD_COPYPATHNAME: {
        WCHAR *pszCopy;
        WCHAR tchUntitled[32];
        if (lstrlen(szCurFile))
          pszCopy = szCurFile;
        else
        {
          GetString(IDS_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszCopy = tchUntitled;
        }
        if (OpenClipboard(hwnd))
        {
          HANDLE hData;
          WCHAR *pData;
          EmptyClipboard();
          hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(WCHAR) * (lstrlen(pszCopy) + 1));
          pData = GlobalLock(hData);
          StrCpyN(pData, pszCopy, (int)GlobalSize(hData) / sizeof(WCHAR));
          GlobalUnlock(hData);
          SetClipboardData(CF_UNICODETEXT, hData);
          CloseClipboard();
        }
      }
      break;


    case CMD_COPYWINPOS: {
        WCHAR wszWinPos[256];
        WINDOWPLACEMENT wndpl;
        int x, y, cx, cy, max;

        wndpl.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwndMain, &wndpl);
        x = wndpl.rcNormalPosition.left;
        y = wndpl.rcNormalPosition.top;
        cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
        cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
        max = (IsZoomed(hwndMain) || (wndpl.flags & WPF_RESTORETOMAXIMIZED));
        wsprintf(wszWinPos, L"/pos %i,%i,%i,%i,%i", x, y, cx, cy, max);
        if (OpenClipboard(hwnd))
        {
          HANDLE hData;
          WCHAR *pData;
          EmptyClipboard();
          hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(WCHAR) * (lstrlen(wszWinPos) + 1));
          pData = GlobalLock(hData);
          StrCpyN(pData, wszWinPos, (int)GlobalSize(hData) / sizeof(WCHAR));
          GlobalUnlock(hData);
          SetClipboardData(CF_UNICODETEXT, hData);
          CloseClipboard();
        }

        UpdateToolbar();
      }
      break;


    case CMD_DEFAULTWINPOS:
      SnapToDefaultPos(hwnd);
      break;


    case CMD_OPENINIFILE:
      if (lstrlen(szIniFile))
      {
        CreateIniFile();
        FileLoad(FALSE, FALSE, FALSE, FALSE, szIniFile);
      }
      break;


    case IDT_FILE_NEW:
      if (IsCmdEnabled(hwnd, IDM_FILE_NEW))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_FILE_NEW, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_OPEN:
      if (IsCmdEnabled(hwnd, IDM_FILE_OPEN))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_FILE_OPEN, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_BROWSE:
      if (IsCmdEnabled(hwnd, IDM_FILE_BROWSE))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_FILE_BROWSE, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_SAVE:
      if (IsCmdEnabled(hwnd, IDM_FILE_SAVE))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_FILE_SAVE, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_UNDO:
      if (IsCmdEnabled(hwnd, IDM_EDIT_UNDO))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_UNDO, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_REDO:
      if (IsCmdEnabled(hwnd, IDM_EDIT_REDO))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_REDO, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_CUT:
      if (IsCmdEnabled(hwnd, IDM_EDIT_CUT))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_CUT, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_COPY:
      if (IsCmdEnabled(hwnd, IDM_EDIT_COPY))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_COPY, 1), 0);
      else
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_COPYALL, 1), 0);
      break;


    case IDT_EDIT_PASTE:
      if (IsCmdEnabled(hwnd, IDM_EDIT_PASTE))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_PASTE, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_FIND:
      if (IsCmdEnabled(hwnd, IDM_EDIT_FIND))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_FIND, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_REPLACE:
      if (IsCmdEnabled(hwnd, IDM_EDIT_REPLACE))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_REPLACE, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_WORDWRAP:
      if (IsCmdEnabled(hwnd, IDM_VIEW_WORDWRAP))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_WORDWRAP, 1), 0);
      else
        MessageBeep(0);
      break;


    // [2e]: New toolbar button: selection mode #339
    case IDT_VIEW_SELECTEX:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SELECTEX))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_SELECTEX, 1), 0);
      else
        MessageBeep(0);
      break;
    // [/2e]


    case IDT_VIEW_ZOOMIN:
      if (IsCmdEnabled(hwnd, IDM_VIEW_ZOOMIN))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_ZOOMIN, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_ZOOMOUT:
      if (IsCmdEnabled(hwnd, IDM_VIEW_ZOOMOUT))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_ZOOMOUT, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_SCHEME:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SCHEME))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_SCHEME, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_SCHEMECONFIG:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SCHEMECONFIG))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_SCHEMECONFIG, 1), 0);
      else
        MessageBeep(0);
      break;


    // [2e]: button to control Settings > Save Settings On Exit #95
    case IDT_SETTINGS_SAVE_ON_EXIT:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SAVESETTINGS_MODE_ALL))
      {
        UINT nNewSaveSettingsModeID = IDM_VIEW_SAVESETTINGS_MODE_ALL;
        switch (nSaveSettingsMode)
        {
        case SSM_NO:
        case SSM_RECENT:
          nNewSaveSettingsModeID = IDM_VIEW_SAVESETTINGS_MODE_ALL;
          break;
        case SSM_ALL:
          nNewSaveSettingsModeID = IDM_VIEW_SAVESETTINGS_MODE_RECENT;
          break;
        }
        SendMessage(hwnd, WM_COMMAND, MAKELONG(nNewSaveSettingsModeID, 1), 0);
      }
      else
        MessageBeep(0);
      break;
    // [/2e]


    case IDT_FILE_EXIT:
      SendMessage(hwnd, WM_CLOSE, 0, 0);
      break;


    case IDT_FILE_SAVEAS:
      if (IsCmdEnabled(hwnd, IDM_FILE_SAVEAS))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_FILE_SAVEAS, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_SAVECOPY:
      if (IsCmdEnabled(hwnd, IDM_FILE_SAVECOPY))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_FILE_SAVECOPY, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_CLEAR:
      if (IsCmdEnabled(hwnd, IDM_EDIT_CLEAR))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_CLEAR, 1), 0);
      else
        SendMessage(hwndEdit, SCI_CLEARALL, 0, 0);
      break;


    case IDT_FILE_PRINT:
      if (IsCmdEnabled(hwnd, IDM_FILE_PRINT))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_FILE_PRINT, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_OPENFAV:
      if (IsCmdEnabled(hwnd, IDM_FILE_OPENFAV))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_FILE_OPENFAV, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_ADDTOFAV:
      if (IsCmdEnabled(hwnd, IDM_FILE_ADDTOFAV))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_FILE_ADDTOFAV, 1), 0);
      else
        MessageBeep(0);
      break;


    // [2e]: Split view #316
    case IDT_SPLIT_VERTICALLY:
      n2e_SplitView(TRUE);
      UpdateToolbar();
      break;
    case IDT_SPLIT_HORIZONTALLY:
      n2e_SplitView(FALSE);
      UpdateToolbar();
      break;
    case IDT_CLOSE_SPLIT:
      n2e_CloseView();
      UpdateToolbar();
      break;
    // [/2e]


    // [2e]: Binary Save Options button #170
    case IDT_BINARY_SAFE_SAVE:
      if (bFixLineEndings || bAutoStripBlanks)
      {
        bFixLineEndings = FALSE;
        bAutoStripBlanks = FALSE;
      }
      else
      {
        bFixLineEndings = TRUE;
        bAutoStripBlanks = TRUE;
      }
      UpdateToolbar();
      break;
    // [/2e]

    // [2e]: Improve selection/word highlighting #286
    case IDM_VIEW_HIGHLIGHTCURRENTSELECTION_DISABLED:
      iHighlightSelection = HCS_DISABLED;
      n2e_SelectionUpdate(SUM_INIT);
      break;

    case IDM_VIEW_HIGHLIGHTCURRENTSELECTION_WORD:
      iHighlightSelection = HCS_WORD;
      n2e_SelectionUpdate(SUM_INIT);
      break;

    case IDM_VIEW_HIGHLIGHTCURRENTSELECTION_SELECTION:
      iHighlightSelection = HCS_SELECTION;
      n2e_SelectionUpdate(SUM_INIT);
      break;

    case IDM_VIEW_HIGHLIGHTCURRENTSELECTION_WORDANDSELECTION:
      iHighlightSelection = HCS_WORD_AND_SELECTION;
      n2e_SelectionUpdate(SUM_INIT);
      break;

    case IDM_VIEW_HIGHLIGHTCURRENTSELECTION_WORDIFNOSELECTION:
      iHighlightSelection = HCS_WORD_IF_NO_SELECTION;
      n2e_SelectionUpdate(SUM_INIT);
      break;

    case IDM_VIEW_HIGHLIGHTCURRENTSELECTION_EDITWORD:
      bEditSelectionScope = !bEditSelectionScope;
      break;
    // [/2e]

    // [2e]: Split view #316
    case CMD_GOTO_NEXT_VIEW:
    case CMD_GOTO_PREVIOUS_VIEW:
      {
        const HWND hwndActive = n2e_GetActiveEditCheckFocus();
        const int windowsCount = n2e_ScintillaWindowsCount();
        for (int i = 0; i < windowsCount; ++i)
        {
          const HWND hwnd = n2e_ScintillaWindowByIndex(i);
          if (hwnd == hwndActive)
          {
            n2e_SetActiveEdit(
              n2e_ScintillaWindowByIndex((wCommandID == CMD_GOTO_NEXT_VIEW)
                ? (i + 1 >= windowsCount) ? 0 : i + 1
                : (i - 1 < 0) ? windowsCount - 1 : i - 1));
            n2e_RestoreActiveEdit(TRUE);
            break;
          }
        }
      }
      break;

    case CMD_HSPLIT_VIEW:
    case CMD_VSPLIT_VIEW:
      n2e_SplitView(wCommandID == CMD_HSPLIT_VIEW);
      break;
    // [/2e]
  }
  return (0);
}


//=============================================================================
//
//  MsgNotify() - Handles WM_NOTIFY
//
//
LRESULT MsgNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{

  LPNMHDR pnmh = (LPNMHDR)lParam;
  struct SCNotification *scn = (struct SCNotification *) lParam;
  const HWND hwndFrom = pnmh->hwndFrom;

  switch (pnmh->idFrom)
  {

    case IDC_EDIT:

      // [2e]: Split view #316
      switch (pnmh->code)
      {
      case SCN_SAVEPOINTREACHED:
      case SCN_SAVEPOINTLEFT:
        if (pnmh->hwndFrom != _hwndEdit)
          return 0; // process main edit only
      default:
        break;
      }
      // [/2e]

      switch (pnmh->code)
      {
        case SCN_UPDATEUI:

          if (scn->updated & ~(SC_UPDATE_V_SCROLL | SC_UPDATE_H_SCROLL))
          {
            UpdateToolbar();
            UpdateStatusbar();

            // Invalidate invalid selections
            if (SendMessage(hwndFrom, SCI_GETSELECTIONS, 0, 0) > 1 &&
                SendMessage(hwndFrom, SCI_GETSELECTIONMODE, 0, 0) != SC_SEL_RECTANGLE)
            {
              int iCurPos = (int)SendMessage(hwndFrom, SCI_GETCURRENTPOS, 0, 0);
              SendMessage(hwndFrom, WM_CANCELMODE, 0, 0);
              SendMessage(hwndFrom, SCI_CLEARSELECTIONS, 0, 0);
              SendMessage(hwndFrom, SCI_SETSELECTION, (WPARAM)iCurPos, (LPARAM)iCurPos);
            }

            // Brace Match
            if (bMatchBraces)
            {
              int iPos;
              char c;
              int iEndStyled = (int)SendMessage(hwndFrom, SCI_GETENDSTYLED, 0, 0);
              if (iEndStyled < (int)SendMessage(hwndFrom, SCI_GETLENGTH, 0, 0))
              {
                int iLine = (int)SendMessage(hwndFrom, SCI_LINEFROMPOSITION, iEndStyled, 0);
                int iEndStyled = (int)SendMessage(hwndFrom, SCI_POSITIONFROMLINE, iLine, 0);
                SendMessage(hwndFrom, SCI_COLOURISE, iEndStyled, -1);
              }
              iPos = (int)SendMessage(hwndFrom, SCI_GETCURRENTPOS, 0, 0);
              c = (char)SendMessage(hwndFrom, SCI_GETCHARAT, iPos, 0);
              // [2e]: Treat quotes as braces #287
              if (StrChrA(n2e_GetBracesList(), c))
              {
                int iBrace2 = (int)SendMessage(hwndFrom, SCI_BRACEMATCH, iPos, bTreatQuotesAsBraces);
                if (iBrace2 != -1)
                {
                  int col1 = (int)SendMessage(hwndFrom, SCI_GETCOLUMN, iPos, 0);
                  int col2 = (int)SendMessage(hwndFrom, SCI_GETCOLUMN, iBrace2, 0);
                  SendMessage(hwndFrom, SCI_BRACEHIGHLIGHT, iPos, iBrace2);
                  SendMessage(hwndFrom, SCI_SETHIGHLIGHTGUIDE, min(col1, col2), 0);
                }
                else
                {
                  SendMessage(hwndFrom, SCI_BRACEBADLIGHT, iPos, 0);
                  SendMessage(hwndFrom, SCI_SETHIGHLIGHTGUIDE, 0, 0);
                }
              }
              // Try one before
              else
              {
                iPos = (int)SendMessage(hwndFrom, SCI_POSITIONBEFORE, iPos, 0);
                c = (char)SendMessage(hwndFrom, SCI_GETCHARAT, iPos, 0);
                // [2e]: Treat quotes as braces #287
                if (StrChrA(n2e_GetBracesList(), c))
                {
                  int iBrace2 = (int)SendMessage(hwndFrom, SCI_BRACEMATCH, iPos, bTreatQuotesAsBraces);
                  if (iBrace2 != -1)
                  {
                    int col1 = (int)SendMessage(hwndFrom, SCI_GETCOLUMN, iPos, 0);
                    int col2 = (int)SendMessage(hwndFrom, SCI_GETCOLUMN, iBrace2, 0);
                    SendMessage(hwndFrom, SCI_BRACEHIGHLIGHT, iPos, iBrace2);
                    SendMessage(hwndFrom, SCI_SETHIGHLIGHTGUIDE, min(col1, col2), 0);
                  }
                  else
                  {
                    SendMessage(hwndFrom, SCI_BRACEBADLIGHT, iPos, 0);
                    SendMessage(hwndFrom, SCI_SETHIGHLIGHTGUIDE, 0, 0);
                  }
                }
                else
                {
                  SendMessage(hwndFrom, SCI_BRACEHIGHLIGHT, (WPARAM)-1, (LPARAM)-1);
                  SendMessage(hwndFrom, SCI_SETHIGHLIGHTGUIDE, 0, 0);
                }
              }
            }
          }
          break;

        case SCN_CHARADDED:
          // Auto indent
          if (bAutoIndent && (scn->ch == '\x0D' || scn->ch == '\x0A'))
          {
            // in CRLF mode handle LF only...
            if ((SC_EOL_CRLF == iEOLMode && scn->ch != '\x0A') || SC_EOL_CRLF != iEOLMode)
            {
              char *pLineBuf;
              char *pPos;
              int iCurPos = (int)SendMessage(hwndFrom, SCI_GETCURRENTPOS, 0, 0);
              int iCurLine = (int)SendMessage(hwndFrom, SCI_LINEFROMPOSITION, (WPARAM)iCurPos, 0);
              if (iCurLine > 0)
              {
                int iPrevLineLength = (int)SendMessage(hwndFrom, SCI_LINELENGTH, iCurLine - 1, 0);
                if (pLineBuf = GlobalAlloc(GPTR, iPrevLineLength + 1))
                {
                  SendMessage(hwndFrom, SCI_GETLINE, iCurLine - 1, (LPARAM)pLineBuf);
                  *(pLineBuf + iPrevLineLength) = '\0';
                  for (pPos = pLineBuf; *pPos; pPos++)
                  {
                    if (*pPos != ' ' && *pPos != '\t')
                    {
                      *pPos = '\0';
                    }
                  }
                  if (*pLineBuf)
                  {
                    SendMessage(hwndFrom, SCI_BEGINUNDOACTION, 0, 0);
                    SendMessage(hwndFrom, SCI_ADDTEXT, lstrlenA(pLineBuf), (LPARAM)pLineBuf);
                    SendMessage(hwndFrom, SCI_ENDUNDOACTION, 0, 0);
                  }
                  GlobalFree(pLineBuf);
                }
              }
            }
          }
          // Auto close tags
          else if (bAutoCloseTags && scn->ch == '>')
          {
            char tchBuf[512];
            wchar_t wchBuf[512];
            char tchIns[516] = "</";
            int  cchIns = 2;
            int  iCurPos = (int)SendMessage(hwndFrom, SCI_GETCURRENTPOS, 0, 0);
            int  iHelper = iCurPos - (COUNTOF(tchBuf) - 1);
            int  iStartPos = max(0, iHelper);
            int  iSize = iCurPos - iStartPos;
            if (iSize >= 3)
            {
              struct TextRange tr = { { iStartPos, iCurPos }, tchBuf };
              SendMessage(hwndFrom, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
              if (tchBuf[iSize - 2] != '/')
              {
                const char *pBegin = &tchBuf[0];
                const char *pCur = &tchBuf[iSize - 2];
                while (pCur > pBegin && *pCur != '<' && *pCur != '>')
                {
                  --pCur;
                }
                if (*pCur == '<')
                {
                  pCur++;
                  // [2e]: Autoclose XML corrupts UTF-8 #112
                  const BOOL bIsUTF8 = (SendMessage(hwndFrom, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8);
                  MultiByteToWideChar(bIsUTF8 ? CP_UTF8 : CP_ACP, 0, pCur, strlen(pCur), wchBuf, _countof(wchBuf));
                  const wchar_t *pCurW = &wchBuf[0];
                  while (StrChrA(":_-.", *pCur) || 
                         ((!bIsUTF8 || (n2e_GetUTF8CharLength(*pCur) == 1)) ? IsCharAlphaNumericA(*pCur) : IsCharAlphaNumericW(*pCurW)))
                  {
                    const int iCharLength = bIsUTF8 ? n2e_GetUTF8CharLength(*pCur) : 1;
                    for (int i = 0; i < iCharLength; ++i)
                    {
                      tchIns[cchIns++] = *pCur;
                      pCur++;
                    }
                    pCurW++;
                  }
                  // [/2e]
                }
                tchIns[cchIns++] = '>';
                tchIns[cchIns] = 0;
                if (cchIns > 3 && n2e_IsValidClosingTagA(tchIns))
                {
                  SendMessage(hwndFrom, SCI_BEGINUNDOACTION, 0, 0);
                  SendMessage(hwndFrom, SCI_REPLACESEL, 0, (LPARAM)tchIns);
                  SendMessage(hwndFrom, SCI_SETSEL, iCurPos, iCurPos);
                  SendMessage(hwndFrom, SCI_ENDUNDOACTION, 0, 0);
                }
              }
            }
          }
          // [2e]: ScrollYCaretPolicy affecting some input commands but not others #294
          EditSelectEx(hwndFrom, SciCall_GetAnchor(), SciCall_GetCurrentPos());
          break;

        case SCN_ZOOM:
          VIEW_COMMAND(UpdateLineNumberWidth);
          break;

        case SCN_SAVEPOINTREACHED:
          bModified = FALSE;
          n2e_UpdateWindowTitle(hwnd);
          break;

        case SCN_SAVEPOINTLEFT:
          bModified = TRUE;
          n2e_UpdateWindowTitle(hwnd);
          break;

        // [2e]: "Scroll margin"-feature
        case SCN_CARETMOVED:
          EditSelectEx(hwndFrom, SciCall_GetAnchor(), SciCall_GetCurrentPos());
          break;
        // [/2e]

        // [2e]: "Update gutter width"-feature
        case SCN_LINECOUNTCHANGED:
          VIEW_COMMAND(UpdateLineNumberWidth);
          break;

        // [2e]: Focus target split view after drag & drop between views #406
        case SCN_DROPCOMPLETED:
          SetFocus(hwndFrom);
          break;
        // [/2e]

        // [2e]: Add View > First Column Margin #382
        case SCN_MARGINCLICK:
          n2e_OnMarginClick(hwndFrom, scn->margin, scn->position, &scn->updated);
          break;
        // [/2e]
      }
      n2e_SelectionNotificationHandler(hwndFrom, pnmh->code, scn);
      break;


    case IDC_TOOLBAR:

      switch (pnmh->code)
      {
        case TBN_ENDADJUST:
          UpdateToolbar();
          break;

        case TBN_QUERYDELETE:
        case TBN_QUERYINSERT:
          return TRUE;

        case TBN_GETBUTTONINFO: {
            if (((LPTBNOTIFY)lParam)->iItem < COUNTOF(tbbMainWnd))
            {
              WCHAR tch[256];
              GetString(tbbMainWnd[((LPTBNOTIFY)lParam)->iItem].idCommand, tch, COUNTOF(tch));
              lstrcpyn(((LPTBNOTIFY)lParam)->pszText, tch, ((LPTBNOTIFY)lParam)->cchText);
              CopyMemory(&((LPTBNOTIFY)lParam)->tbButton, &tbbMainWnd[((LPTBNOTIFY)lParam)->iItem], sizeof(TBBUTTON));
              return TRUE;
            }
          }
          return FALSE;

        case TBN_RESET: {
            int i; int c = (int)SendMessage(hwndToolbar, TB_BUTTONCOUNT, 0, 0);
            for (i = 0; i < c; i++)
              SendMessage(hwndToolbar, TB_DELETEBUTTON, 0, 0);
            SendMessage(hwndToolbar, TB_ADDBUTTONS, NUMINITIALTOOLS, (LPARAM)tbbMainWnd);
            return (0);
          }
      }
      break;
    case IDC_STATUSBAR:
      switch (pnmh->code)
      {
        case NM_CLICK: {
            LPNMMOUSE pnmm = (LPNMMOUSE)lParam;
            switch (pnmm->dwItemSpec)
            {
              // [2e]: "Evaluate selection"-feature
              case STATUS_DOCSIZE:
                n2e_OnPaneSizeClick(hwnd, TRUE);
                return TRUE;
              // [/2e]

              case STATUS_EOLMODE:
                SendMessage(hwndEdit, SCI_CONVERTEOLS, SendMessage(hwndEdit, SCI_GETEOLMODE, 0, 0), 0);
                EditFixPositions(hwndEdit);
                return TRUE;

              default:
                return FALSE;
            }
          }

        case NM_DBLCLK: {
            int i;
            LPNMMOUSE pnmm = (LPNMMOUSE)lParam;
            switch (pnmm->dwItemSpec)
            {
              case STATUS_CODEPAGE:
                SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_ENCODING_SELECT, 1), 0);
                return TRUE;

              case STATUS_EOLMODE:
                if (iEOLMode == SC_EOL_CRLF)
                  i = IDM_LINEENDINGS_CRLF;
                else if (iEOLMode == SC_EOL_LF)
                  i = IDM_LINEENDINGS_LF;
                else
                  i = IDM_LINEENDINGS_CR;
                i++;
                if (i > IDM_LINEENDINGS_CR)
                  i = IDM_LINEENDINGS_CRLF;
                SendMessage(hwnd, WM_COMMAND, MAKELONG(i, 1), 0);
                return TRUE;

              case STATUS_LEXER:
                SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_SCHEME, 1), 0);
                return TRUE;

              case STATUS_OVRMODE:
                SendMessage(hwndEdit, SCI_EDITTOGGLEOVERTYPE, 0, 0);
                return TRUE;

              default:
                return FALSE;
            }
          }
          break;


      }
      break;


    default:
      switch (pnmh->code)
      {
        case TTN_NEEDTEXT: {
            WCHAR tch[256];

            if (((LPTOOLTIPTEXT)lParam)->uFlags & TTF_IDISHWND)
            {
              ;
            }
            else
            {
              GetString((UINT)pnmh->idFrom, tch, COUNTOF(tch));
              lstrcpyn(((LPTOOLTIPTEXT)lParam)->szText, tch, 80);
            }
          }
          break;
      }
      break;
  }


  return (0);
}


//=============================================================================
//
//  LoadSettings()
//
//
void LoadSettings()
{
  WCHAR *pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * 32 * 1024);
  int   cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);
  LoadIniSection(L"Settings", pIniSection, cchIniSection);
  // [2e]: Save on exit and History #101
  const int nSaveSettings = IniSectionGetInt(pIniSection, L"SaveSettings", 1);
  switch (nSaveSettings)
  {
    case SSM_NO:
    case SSM_ALL:
    case SSM_RECENT:
      nSaveSettingsMode = nSaveSettings;
      break;
    default:
      nSaveSettingsMode = SSM_ALL;
      break;
  }
  // [/2e]

  bSaveRecentFiles = IniSectionGetInt(pIniSection, L"SaveRecentFiles", 0);
  if (bSaveRecentFiles) bSaveRecentFiles = 1;

  bSaveFindReplace = IniSectionGetInt(pIniSection, L"SaveFindReplace", 0);
  if (bSaveFindReplace) bSaveFindReplace = 1;

  efrData.bFindClose = IniSectionGetInt(pIniSection, L"CloseFind", 0);
  if (efrData.bFindClose) efrData.bReplaceClose = TRUE;

  efrData.bReplaceClose = IniSectionGetInt(pIniSection, L"CloseReplace", 0);
  if (efrData.bReplaceClose) efrData.bReplaceClose = TRUE;

  efrData.bNoFindWrap = IniSectionGetInt(pIniSection, L"NoFindWrap", 0);
  if (efrData.bNoFindWrap) efrData.bNoFindWrap = TRUE;

  if (!IniSectionGetString(pIniSection, L"OpenWithDir", L"",
                           tchOpenWithDir, COUNTOF(tchOpenWithDir)))
    SHGetSpecialFolderPath(NULL, tchOpenWithDir, CSIDL_DESKTOPDIRECTORY, TRUE);
  else
    PathAbsoluteFromApp(tchOpenWithDir, NULL, COUNTOF(tchOpenWithDir), TRUE);

  if (!IniSectionGetString(pIniSection, L"Favorites", L"",
                           tchFavoritesDir, COUNTOF(tchFavoritesDir)))
    SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, tchFavoritesDir);
  else
    PathAbsoluteFromApp(tchFavoritesDir, NULL, COUNTOF(tchFavoritesDir), TRUE);

  iPathNameFormat = IniSectionGetInt(pIniSection, L"PathNameFormat", 0);
  iPathNameFormat = max(min(iPathNameFormat, PNM_FULLPATH), 0);

  fWordWrap = IniSectionGetInt(pIniSection, L"WordWrap", 0);
  if (fWordWrap) fWordWrap = 1;
  fWordWrapG = fWordWrap;

  iWordWrapMode = IniSectionGetInt(pIniSection, L"WordWrapMode", 0);
  iWordWrapMode = max(min(iWordWrapMode, 1), 0);

  iWordWrapIndent = IniSectionGetInt(pIniSection, L"WordWrapIndent", 0);
  iWordWrapIndent = max(min(iWordWrapIndent, 6), 0);

  iWordWrapSymbols = IniSectionGetInt(pIniSection, L"WordWrapSymbols", 22);
  iWordWrapSymbols = max(min(iWordWrapSymbols % 10, 2), 0) + max(min((iWordWrapSymbols % 100 - iWordWrapSymbols % 10) / 10, 2), 0) * 10;

  bShowWordWrapSymbols = IniSectionGetInt(pIniSection, L"ShowWordWrapSymbols", 0);
  if (bShowWordWrapSymbols) bShowWordWrapSymbols = 1;

  bMatchBraces = IniSectionGetInt(pIniSection, L"MatchBraces", 1);
  if (bMatchBraces) bMatchBraces = 1;

  bAutoCloseTags = IniSectionGetInt(pIniSection, L"AutoCloseTags", 0);
  if (bAutoCloseTags) bAutoCloseTags = 1;

  bHighlightCurrentLine = IniSectionGetInt(pIniSection, L"HighlightCurrentLine", 0);
  if (bHighlightCurrentLine) bHighlightCurrentLine = 1;

  bAutoIndent = IniSectionGetInt(pIniSection, L"AutoIndent", 1);
  if (bAutoIndent) bAutoIndent = 1;

  bShowIndentGuides = IniSectionGetInt(pIniSection, L"ShowIndentGuides", 0);
  if (bShowIndentGuides) bShowIndentGuides = 1;

  bTabsAsSpaces = IniSectionGetInt(pIniSection, L"TabsAsSpaces", 1);
  if (bTabsAsSpaces) bTabsAsSpaces = 1;
  bTabsAsSpacesG = bTabsAsSpaces;

  bTabIndents = IniSectionGetInt(pIniSection, L"TabIndents", 1);
  if (bTabIndents) bTabIndents = 1;

  bTabIndentsG = bTabIndents;
  bBackspaceUnindents = IniSectionGetInt(pIniSection, L"BackspaceUnindents", 0);
  if (bBackspaceUnindents) bBackspaceUnindents = 1;

  iTabWidth = IniSectionGetInt(pIniSection, L"TabWidth", 2);
  iTabWidth = max(min(iTabWidth, 256), 1);
  iTabWidthG = iTabWidth;

  iIndentWidth = IniSectionGetInt(pIniSection, L"IndentWidth", 0);
  iIndentWidth = max(min(iIndentWidth, 256), 0);
  iIndentWidthG = iIndentWidth;

  bMarkLongLines = IniSectionGetInt(pIniSection, L"MarkLongLines", 0);
  if (bMarkLongLines) bMarkLongLines = 1;

  iLongLinesLimit = IniSectionGetInt(pIniSection, L"LongLinesLimit", 80);
  iLongLinesLimit = max(min(iLongLinesLimit, 4096), 0);
  iLongLinesLimitG = iLongLinesLimit;

  iLongLineMode = IniSectionGetInt(pIniSection, L"LongLineMode", EDGE_LINE);
  iLongLineMode = max(min(iLongLineMode, EDGE_BACKGROUND), EDGE_LINE);

  bShowSelectionMargin = IniSectionGetInt(pIniSection, L"ShowSelectionMargin", 0);
  if (bShowSelectionMargin) bShowSelectionMargin = 1;

  bShowFirstColumnMargin = IniSectionGetInt(pIniSection, L"ShowFirstColumnMargin", 0);
  if (bShowFirstColumnMargin) bShowFirstColumnMargin = 1;

  bShowLineNumbers = IniSectionGetInt(pIniSection, L"ShowLineNumbers", 1);
  if (bShowLineNumbers) bShowLineNumbers = 1;

  bViewWhiteSpace = IniSectionGetInt(pIniSection, L"ViewWhiteSpace", 0);
  if (bViewWhiteSpace) bViewWhiteSpace = 1;

  bViewEOLs = IniSectionGetInt(pIniSection, L"ViewEOLs", 0);
  if (bViewEOLs) bViewEOLs = 1;

  iDefaultEncoding = IniSectionGetInt(pIniSection, L"DefaultEncoding", 0);
  iDefaultEncoding = Encoding_MapIniSetting(TRUE, iDefaultEncoding);
  if (!Encoding_IsValid(iDefaultEncoding)) iDefaultEncoding = CPI_DEFAULT;

  bSkipUnicodeDetection = IniSectionGetInt(pIniSection, L"SkipUnicodeDetection", 0);
  if (bSkipUnicodeDetection) bSkipUnicodeDetection = 1;

  bLoadASCIIasUTF8 = IniSectionGetInt(pIniSection, L"LoadASCIIasUTF8", 0);
  if (bLoadASCIIasUTF8) bLoadASCIIasUTF8 = 1;

  bNoEncodingTags = IniSectionGetInt(pIniSection, L"NoEncodingTags", 0);
  if (bNoEncodingTags) bNoEncodingTags = 1;

  iDefaultEOLMode = IniSectionGetInt(pIniSection, L"DefaultEOLMode", 0);
  iDefaultEOLMode = max(min(iDefaultEOLMode, 2), 0);
  bFixLineEndings = IniSectionGetInt(pIniSection, L"FixLineEndings", 1);
  if (bFixLineEndings) bFixLineEndings = 1;

  bAutoStripBlanks = IniSectionGetInt(pIniSection, L"FixTrailingBlanks", 0);
  if (bAutoStripBlanks) bAutoStripBlanks = 1;

  iPrintHeader = IniSectionGetInt(pIniSection, L"PrintHeader", 1);
  iPrintHeader = max(min(iPrintHeader, 3), 0);
  iPrintFooter = IniSectionGetInt(pIniSection, L"PrintFooter", 0);
  iPrintFooter = max(min(iPrintFooter, 1), 0);
  iPrintColor = IniSectionGetInt(pIniSection, L"PrintColorMode", 3);
  iPrintColor = max(min(iPrintColor, 4), 0);
  iPrintZoom = IniSectionGetInt(pIniSection, L"PrintZoom", 10) - 10;
  iPrintZoom = max(min(iPrintZoom, 20), -10);
  pagesetupMargin.left = IniSectionGetInt(pIniSection, L"PrintMarginLeft", -1);
  pagesetupMargin.left = max(pagesetupMargin.left, -1);
  pagesetupMargin.top = IniSectionGetInt(pIniSection, L"PrintMarginTop", -1);
  pagesetupMargin.top = max(pagesetupMargin.top, -1);
  pagesetupMargin.right = IniSectionGetInt(pIniSection, L"PrintMarginRight", -1);
  pagesetupMargin.right = max(pagesetupMargin.right, -1);
  pagesetupMargin.bottom = IniSectionGetInt(pIniSection, L"PrintMarginBottom", -1);
  pagesetupMargin.bottom = max(pagesetupMargin.bottom, -1);

  iSaveBeforeRunningTools = IniSectionGetInt(pIniSection, L"SaveBeforeRunningTools", SBRT_DISABLED);
  iSaveBeforeRunningTools = max(min(iSaveBeforeRunningTools, SBRT_EXCEPT_NEW_WINDOW), SBRT_DISABLED);

  iFileWatchingMode = IniSectionGetInt(pIniSection, L"FileWatchingMode", 0);
  iFileWatchingMode = max(min(iFileWatchingMode, 2), 0);

  bResetFileWatching = IniSectionGetInt(pIniSection, L"ResetFileWatching", 1);
  if (bResetFileWatching) bResetFileWatching = 1;

  iEscFunction = IniSectionGetInt(pIniSection, L"EscFunction", EEF_EXIT);
  iEscFunction = max(min(iEscFunction, EEF_EXIT), 0);

  bEscForCurrentSplitView = IniSectionGetInt(pIniSection, L"EscForCurrentSplitView", 1);
  if (bEscForCurrentSplitView) bEscForCurrentSplitView = 1;

  bAlwaysOnTop = IniSectionGetInt(pIniSection, L"AlwaysOnTop", 0);
  if (bAlwaysOnTop) bAlwaysOnTop = 1;

  bMinimizeToTray = IniSectionGetInt(pIniSection, L"MinimizeToTray", 0);
  if (bMinimizeToTray) bMinimizeToTray = 1;

  bTransparentMode = IniSectionGetInt(pIniSection, L"TransparentMode", 0);
  if (bTransparentMode) bTransparentMode = 1;

  // Check if SetLayeredWindowAttributes() is available
  bTransparentModeAvailable =
    (GetProcAddress(GetModuleHandle(L"User32"), "SetLayeredWindowAttributes") != NULL);

  IniSectionGetString(pIniSection, L"ToolbarButtons", L"",
                      tchToolbarButtons, COUNTOF(tchToolbarButtons));
  bShowToolbar = IniSectionGetInt(pIniSection, L"ShowToolbar", 1);
  if (bShowToolbar) bShowToolbar = 1;

  bShowStatusbar = IniSectionGetInt(pIniSection, L"ShowStatusbar", 1);
  if (bShowStatusbar) bShowStatusbar = 1;

  cxEncodingDlg = IniSectionGetInt(pIniSection, L"EncodingDlgSizeX", 256);
  cxEncodingDlg = max(cxEncodingDlg, 0);

  cyEncodingDlg = IniSectionGetInt(pIniSection, L"EncodingDlgSizeY", 262);
  cyEncodingDlg = max(cyEncodingDlg, 0);

  cxRecodeDlg = IniSectionGetInt(pIniSection, L"RecodeDlgSizeX", 256);
  cxRecodeDlg = max(cxRecodeDlg, 0);

  cyRecodeDlg = IniSectionGetInt(pIniSection, L"RecodeDlgSizeY", 262);
  cyRecodeDlg = max(cyRecodeDlg, 0);

  cxFileMRUDlg = IniSectionGetInt(pIniSection, L"FileMRUDlgSizeX", 412);
  cxFileMRUDlg = max(cxFileMRUDlg, 0);

  cyFileMRUDlg = IniSectionGetInt(pIniSection, L"FileMRUDlgSizeY", 376);
  cyFileMRUDlg = max(cyFileMRUDlg, 0);

  cxOpenWithDlg = IniSectionGetInt(pIniSection, L"OpenWithDlgSizeX", 384);
  cxOpenWithDlg = max(cxOpenWithDlg, 0);

  cyOpenWithDlg = IniSectionGetInt(pIniSection, L"OpenWithDlgSizeY", 386);
  cyOpenWithDlg = max(cyOpenWithDlg, 0);

  cxFavoritesDlg = IniSectionGetInt(pIniSection, L"FavoritesDlgSizeX", 334);
  cxFavoritesDlg = max(cxFavoritesDlg, 0);

  cyFavoritesDlg = IniSectionGetInt(pIniSection, L"FavoritesDlgSizeY", 316);
  cyFavoritesDlg = max(cyFavoritesDlg, 0);

  xFindReplaceDlg = IniSectionGetInt(pIniSection, L"FindReplaceDlgPosX", 0);
  yFindReplaceDlg = IniSectionGetInt(pIniSection, L"FindReplaceDlgPosY", 0);
  LoadIniSection(L"Settings2", pIniSection, cchIniSection);
  bStickyWinPos = IniSectionGetInt(pIniSection, L"StickyWindowPosition", 0);
  if (bStickyWinPos) bStickyWinPos = 1;

  IniSectionGetString(pIniSection, L"DefaultExtension", L"txt",
                      tchDefaultExtension, COUNTOF(tchDefaultExtension));
  StrTrim(tchDefaultExtension, L" \t.\"");

  IniSectionGetString(pIniSection, L"DefaultDirectory", L"",
                      tchDefaultDir, COUNTOF(tchDefaultDir));

  ZeroMemory(tchFileDlgFilters, sizeof(WCHAR) *COUNTOF(tchFileDlgFilters));
  IniSectionGetString(pIniSection, L"FileDlgFilters", L"",
                      tchFileDlgFilters, COUNTOF(tchFileDlgFilters) - 2);

  dwFileCheckInterval = IniSectionGetInt(pIniSection, L"FileCheckInterval", 500);
  dwAutoReloadTimeout = IniSectionGetInt(pIniSection, L"AutoReloadTimeout", 100);

  LoadIniSection(L"Toolbar Images", pIniSection, cchIniSection);
  IniSectionGetString(pIniSection, L"BitmapDefault", L"",
                      tchToolbarBitmap, COUNTOF(tchToolbarBitmap));
  IniSectionGetString(pIniSection, L"BitmapHot", L"",
                      tchToolbarBitmapHot, COUNTOF(tchToolbarBitmap));
  IniSectionGetString(pIniSection, L"BitmapDisabled", L"",
                      tchToolbarBitmapDisabled, COUNTOF(tchToolbarBitmap));
  if (!flagPosParam)
  { // ignore window position if /p was specified
    WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32];

    int ResX = GetSystemMetrics(SM_CXSCREEN);
    int ResY = GetSystemMetrics(SM_CYSCREEN);
    wsprintf(tchPosX, L"%ix%i PosX", ResX, ResY);
    wsprintf(tchPosY, L"%ix%i PosY", ResX, ResY);
    wsprintf(tchSizeX, L"%ix%i SizeX", ResX, ResY);
    wsprintf(tchSizeY, L"%ix%i SizeY", ResX, ResY);
    wsprintf(tchMaximized, L"%ix%i Maximized", ResX, ResY);

    LoadIniSection(L"Window", pIniSection, cchIniSection);

    wi.x = IniSectionGetInt(pIniSection, tchPosX, CW_USEDEFAULT);
    wi.y = IniSectionGetInt(pIniSection, tchPosY, CW_USEDEFAULT);
    wi.cx = IniSectionGetInt(pIniSection, tchSizeX, CW_USEDEFAULT);
    wi.cy = IniSectionGetInt(pIniSection, tchSizeY, CW_USEDEFAULT);
    wi.max = IniSectionGetInt(pIniSection, tchMaximized, 0);
    if (wi.max) wi.max = 1;
  }

  LocalFree(pIniSection);

  iDefaultCodePage = 0;

  int acp = GetACP();
  if (acp == 932 || acp == 936 || acp == 949 || acp == 950 || acp == 1361)
    iDefaultCodePage = acp;

  CHARSETINFO ci = { 0 };
  if (TranslateCharsetInfo((DWORD *)(UINT_PTR)acp, &ci, TCI_SRCCODEPAGE))
    iDefaultCharSet = ci.ciCharset;
  else
    iDefaultCharSet = ANSI_CHARSET;

  // Scintilla Styles
  Style_Load();
  n2e_LoadINI();
}


//=============================================================================
//
//  SaveSettings()
//
//
void SaveSettings(BOOL bSaveSettingsNow)
{
  WCHAR *pIniSection = NULL;
  int   cchIniSection = 0;
  WCHAR wchTmp[MAX_PATH];
  if (lstrlen(szIniFile) == 0)
    return;

  CreateIniFile();
  // [2e]: Save on exit and History #101
  if ((nSaveSettingsMode != SSM_ALL) && !bSaveSettingsNow)
  {
    IniSetInt(L"Settings", L"SaveSettings", nSaveSettingsMode);
    if (nSaveSettingsMode == SSM_NO)
      return;
  }
  const BOOL bCheckSaveSettingsMode = !bSaveSettingsNow;
  if (n2e_CanSaveINISection(bCheckSaveSettingsMode, SSM_ALL))
  {
    pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * 32 * 1024);
    cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);
    IniSectionSetInt(pIniSection, L"SaveSettings", nSaveSettingsMode);
    IniSectionSetInt(pIniSection, L"SaveRecentFiles", bSaveRecentFiles);
    IniSectionSetInt(pIniSection, L"SaveFindReplace", bSaveFindReplace);
    IniSectionSetInt(pIniSection, L"CloseFind", efrData.bFindClose);
    IniSectionSetInt(pIniSection, L"CloseReplace", efrData.bReplaceClose);
    IniSectionSetInt(pIniSection, L"NoFindWrap", efrData.bNoFindWrap);
    PathRelativeToApp(tchOpenWithDir, wchTmp, COUNTOF(wchTmp), FALSE, TRUE, flagPortableMyDocs);
    IniSectionSetString(pIniSection, L"OpenWithDir", wchTmp);
    PathRelativeToApp(tchFavoritesDir, wchTmp, COUNTOF(wchTmp), FALSE, TRUE, flagPortableMyDocs);
    IniSectionSetString(pIniSection, L"Favorites", wchTmp);
    IniSectionSetInt(pIniSection, L"PathNameFormat", iPathNameFormat);
    IniSectionSetInt(pIniSection, L"WordWrap", fWordWrapG);
    IniSectionSetInt(pIniSection, L"WordWrapMode", iWordWrapMode);
    IniSectionSetInt(pIniSection, L"WordWrapIndent", iWordWrapIndent);
    IniSectionSetInt(pIniSection, L"WordWrapSymbols", iWordWrapSymbols);
    IniSectionSetInt(pIniSection, L"ShowWordWrapSymbols", bShowWordWrapSymbols);
    IniSectionSetInt(pIniSection, L"MatchBraces", bMatchBraces);
    IniSectionSetInt(pIniSection, L"AutoCloseTags", bAutoCloseTags);
    IniSectionSetInt(pIniSection, L"HighlightCurrentLine", bHighlightCurrentLine);
    IniSectionSetInt(pIniSection, L"AutoIndent", bAutoIndent);
    IniSectionSetInt(pIniSection, L"ShowIndentGuides", bShowIndentGuides);
    IniSectionSetInt(pIniSection, L"TabsAsSpaces", bTabsAsSpacesG);
    IniSectionSetInt(pIniSection, L"TabIndents", bTabIndentsG);
    IniSectionSetInt(pIniSection, L"BackspaceUnindents", bBackspaceUnindents);
    IniSectionSetInt(pIniSection, L"TabWidth", iTabWidthG);
    IniSectionSetInt(pIniSection, L"IndentWidth", iIndentWidthG);
    IniSectionSetInt(pIniSection, L"MarkLongLines", bMarkLongLines);
    IniSectionSetInt(pIniSection, L"LongLinesLimit", iLongLinesLimitG);
    IniSectionSetInt(pIniSection, L"LongLineMode", iLongLineMode);
    IniSectionSetInt(pIniSection, L"ShowSelectionMargin", bShowSelectionMargin);
    IniSectionSetInt(pIniSection, L"ShowFirstColumnMargin", bShowFirstColumnMargin);
    IniSectionSetInt(pIniSection, L"ShowLineNumbers", bShowLineNumbers);
    IniSectionSetInt(pIniSection, L"ViewWhiteSpace", bViewWhiteSpace);
    IniSectionSetInt(pIniSection, L"ViewEOLs", bViewEOLs);
    IniSectionSetInt(pIniSection, L"DefaultEncoding", Encoding_MapIniSetting(FALSE, iDefaultEncoding));
    IniSectionSetInt(pIniSection, L"SkipUnicodeDetection", bSkipUnicodeDetection);
    IniSectionSetInt(pIniSection, L"LoadASCIIasUTF8", bLoadASCIIasUTF8);
    IniSectionSetInt(pIniSection, L"NoEncodingTags", bNoEncodingTags);
    IniSectionSetInt(pIniSection, L"DefaultEOLMode", iDefaultEOLMode);
    IniSectionSetInt(pIniSection, L"FixLineEndings", bFixLineEndings);
    IniSectionSetInt(pIniSection, L"FixTrailingBlanks", bAutoStripBlanks);
    IniSectionSetInt(pIniSection, L"PrintHeader", iPrintHeader);
    IniSectionSetInt(pIniSection, L"PrintFooter", iPrintFooter);
    IniSectionSetInt(pIniSection, L"PrintColorMode", iPrintColor);
    IniSectionSetInt(pIniSection, L"PrintZoom", iPrintZoom + 10);
    IniSectionSetInt(pIniSection, L"PrintMarginLeft", pagesetupMargin.left);
    IniSectionSetInt(pIniSection, L"PrintMarginTop", pagesetupMargin.top);
    IniSectionSetInt(pIniSection, L"PrintMarginRight", pagesetupMargin.right);
    IniSectionSetInt(pIniSection, L"PrintMarginBottom", pagesetupMargin.bottom);
    IniSectionSetInt(pIniSection, L"SaveBeforeRunningTools", iSaveBeforeRunningTools);
    IniSectionSetInt(pIniSection, L"FileWatchingMode", iFileWatchingMode);
    IniSectionSetInt(pIniSection, L"ResetFileWatching", bResetFileWatching);
    IniSectionSetInt(pIniSection, L"EscFunction", iEscFunction);
    IniSectionSetInt(pIniSection, L"EscForCurrentSplitView", bEscForCurrentSplitView);
    IniSectionSetInt(pIniSection, L"AlwaysOnTop", bAlwaysOnTop);
    IniSectionSetInt(pIniSection, L"MinimizeToTray", bMinimizeToTray);
    IniSectionSetInt(pIniSection, L"TransparentMode", bTransparentMode);
    Toolbar_GetButtons(hwndToolbar, IDT_FILE_NEW, tchToolbarButtons, COUNTOF(tchToolbarButtons));
    IniSectionSetString(pIniSection, L"ToolbarButtons", tchToolbarButtons);
    IniSectionSetInt(pIniSection, L"ShowToolbar", bShowToolbar);
    IniSectionSetInt(pIniSection, L"ShowStatusbar", bShowStatusbar);
    IniSectionSetInt(pIniSection, L"EncodingDlgSizeX", cxEncodingDlg);
    IniSectionSetInt(pIniSection, L"EncodingDlgSizeY", cyEncodingDlg);
    IniSectionSetInt(pIniSection, L"RecodeDlgSizeX", cxRecodeDlg);
    IniSectionSetInt(pIniSection, L"RecodeDlgSizeY", cyRecodeDlg);
    IniSectionSetInt(pIniSection, L"FileMRUDlgSizeX", cxFileMRUDlg);
    IniSectionSetInt(pIniSection, L"FileMRUDlgSizeY", cyFileMRUDlg);
    IniSectionSetInt(pIniSection, L"OpenWithDlgSizeX", cxOpenWithDlg);
    IniSectionSetInt(pIniSection, L"OpenWithDlgSizeY", cyOpenWithDlg);
    IniSectionSetInt(pIniSection, L"FavoritesDlgSizeX", cxFavoritesDlg);
    IniSectionSetInt(pIniSection, L"FavoritesDlgSizeY", cyFavoritesDlg);
    IniSectionSetInt(pIniSection, L"FindReplaceDlgPosX", xFindReplaceDlg);
    IniSectionSetInt(pIniSection, L"FindReplaceDlgPosY", yFindReplaceDlg);
    SaveIniSection(L"Settings", pIniSection);
    LocalFree(pIniSection);
  }
  // [/2e]
  /*
    SaveSettingsNow(): query Window Dimensions
  */
  if (n2e_CanSaveINISection(bCheckSaveSettingsMode, SSM_ALL) && bSaveSettingsNow)
  {
    WINDOWPLACEMENT wndpl;

    // GetWindowPlacement
    wndpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hwndMain, &wndpl);

    wi.x = wndpl.rcNormalPosition.left;
    wi.y = wndpl.rcNormalPosition.top;
    wi.cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
    wi.cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
    wi.max = (IsZoomed(hwndMain) || (wndpl.flags & WPF_RESTORETOMAXIMIZED));
  }
  // [2e]: Save on exit and History #101
  if (n2e_CanSaveINISection(bCheckSaveSettingsMode, SSM_ALL))
  {
    if (!IniGetInt(L"Settings2", L"StickyWindowPosition", 0))
    {
      WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32];
      int ResX = GetSystemMetrics(SM_CXSCREEN);
      int ResY = GetSystemMetrics(SM_CYSCREEN);
      wsprintf(tchPosX, L"%ix%i PosX", ResX, ResY);
      wsprintf(tchPosY, L"%ix%i PosY", ResX, ResY);
      wsprintf(tchSizeX, L"%ix%i SizeX", ResX, ResY);
      wsprintf(tchSizeY, L"%ix%i SizeY", ResX, ResY);
      wsprintf(tchMaximized, L"%ix%i Maximized", ResX, ResY);
      IniSetInt(L"Window", tchPosX, wi.x);
      IniSetInt(L"Window", tchPosY, wi.y);
      IniSetInt(L"Window", tchSizeX, wi.cx);
      IniSetInt(L"Window", tchSizeY, wi.cy);
      IniSetInt(L"Window", tchMaximized, wi.max);
    }
    // Scintilla Styles
    Style_Save();
    n2e_SaveINI();
  }
  
  if (n2e_CanSaveINISection(bCheckSaveSettingsMode, SSM_ALL)
      || (n2e_CanSaveINISection(bCheckSaveSettingsMode, SSM_RECENT) && bSaveRecentFiles))
  {
    // Cleanup unwanted MRU's
    if (!bSaveRecentFiles)
    {
      MRU_Empty(pFileMRU);
      MRU_Save(pFileMRU);
    }
    else
    {
      MRU_MergeSave(pFileMRU, TRUE, flagRelativeFileMRU, flagPortableMyDocs);
    }
  }

  if (n2e_CanSaveINISection(bCheckSaveSettingsMode, SSM_ALL)
      || (n2e_CanSaveINISection(bCheckSaveSettingsMode, SSM_RECENT) && bSaveFindReplace))
  {
    if (!bSaveFindReplace)
    {
      MRU_Empty(mruFind);
      MRU_Empty(mruReplace);
      MRU_Save(mruFind);
      MRU_Save(mruReplace);
    }
    else
    {
      MRU_MergeSave(mruFind, FALSE, FALSE, FALSE);
      MRU_MergeSave(mruReplace, FALSE, FALSE, FALSE);
    }
  }
  // [/2e]
}


//=============================================================================
//
//  ParseCommandLine()
//
//
BOOL ParseCommandLine()
{

  LPWSTR lp1, lp2, lp3;
  BOOL bContinue = TRUE;
  BOOL bIsFileArg = FALSE;
  BOOL bIsNotepadReplacement = FALSE;

  LPWSTR lpCmdLine = GetCommandLine();

  if (lstrlen(lpCmdLine) == 0)
    return TRUE;

  // Good old console can also send args separated by Tabs
  StrTab2Space(lpCmdLine);

  lp1 = LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(lpCmdLine) + 1));
  lp2 = LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(lpCmdLine) + 1));
  lp3 = LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(lpCmdLine) + 1));

  // Start with 2nd argument
  ExtractFirstArgument(lpCmdLine, lp1, lp3);

  while (bContinue && ExtractFirstArgument(lp3, lp1, lp2))
  {

    // options
    if (!bIsFileArg && lstrcmp(lp1, L"+") == 0)
    {
      flagMultiFileArg = 2;
      bIsFileArg = TRUE;
    }

    else if (!bIsFileArg && lstrcmp(lp1, L"-") == 0)
    {
      flagMultiFileArg = 1;
      bIsFileArg = TRUE;
    }
    else if (!bIsFileArg && ((*lp1 == L'/') || (*lp1 == L'-')))
    {

      // LTrim
      StrLTrim(lp1, L"-/");

      // Encoding
      if (lstrcmpi(lp1, L"ANSI") == 0 || lstrcmpi(lp1, L"A") == 0 || lstrcmpi(lp1, L"MBCS") == 0)
        flagSetEncoding = IDM_ENCODING_ANSI - IDM_ENCODING_ANSI + 1;
      else if (lstrcmpi(lp1, L"UNICODE") == 0 || lstrcmpi(lp1, L"W") == 0)
        flagSetEncoding = IDM_ENCODING_UNICODE - IDM_ENCODING_ANSI + 1;
      else if (lstrcmpi(lp1, L"UNICODEBE") == 0 || lstrcmpi(lp1, L"UNICODE-BE") == 0)
        flagSetEncoding = IDM_ENCODING_UNICODEREV - IDM_ENCODING_ANSI + 1;
      else if (lstrcmpi(lp1, L"UTF8") == 0 || lstrcmpi(lp1, L"UTF-8") == 0)
        flagSetEncoding = IDM_ENCODING_UTF8 - IDM_ENCODING_ANSI + 1;
      else if (lstrcmpi(lp1, L"UTF8SIG") == 0 || lstrcmpi(lp1, L"UTF-8SIG") == 0 ||
               lstrcmpi(lp1, L"UTF8SIGNATURE") == 0 || lstrcmpi(lp1, L"UTF-8SIGNATURE") == 0 ||
               lstrcmpi(lp1, L"UTF8-SIGNATURE") == 0 || lstrcmpi(lp1, L"UTF-8-SIGNATURE") == 0)
        flagSetEncoding = IDM_ENCODING_UTF8SIGN - IDM_ENCODING_ANSI + 1;

      // EOL Mode
      else if (lstrcmpi(lp1, L"CRLF") == 0 || lstrcmpi(lp1, L"CR+LF") == 0)
        flagSetEOLMode = IDM_LINEENDINGS_CRLF - IDM_LINEENDINGS_CRLF + 1;
      else if (lstrcmpi(lp1, L"LF") == 0)
        flagSetEOLMode = IDM_LINEENDINGS_LF - IDM_LINEENDINGS_CRLF + 1;
      else if (lstrcmpi(lp1, L"CR") == 0)
        flagSetEOLMode = IDM_LINEENDINGS_CR - IDM_LINEENDINGS_CRLF + 1;

      // Shell integration
      else if (StrCmpNI(lp1, L"appid=", CSTRLEN(L"appid=")) == 0)
      {
        StrCpyN(g_wchAppUserModelID, lp1 + CSTRLEN(L"appid="), COUNTOF(g_wchAppUserModelID));
        StrTrim(g_wchAppUserModelID, L" ");
        if (lstrlen(g_wchAppUserModelID) == 0)
          lstrcpy(g_wchAppUserModelID, L"(default)");
      }
      else if (StrCmpNI(lp1, L"sysmru=", CSTRLEN(L"sysmru=")) == 0)
      {
        WCHAR wch[8];
        StrCpyN(wch, lp1 + CSTRLEN(L"sysmru="), COUNTOF(wch));
        StrTrim(wch, L" ");
        if (*wch == L'1')
          flagUseSystemMRU = 2;
        else
          flagUseSystemMRU = 1;
      }
      // [2e]: Don't interpret %envvars% in pathname when opening file #193
      else if (StrCmpNI(lp1, L"expandenv=", CSTRLEN(L"expandenv=")) == 0)
      {
        WCHAR wch[8];
        StrCpyN(wch, lp1 + CSTRLEN(L"expandenv="), COUNTOF(wch));
        StrTrim(wch, L" ");
        fExpandEnvVariables = (_wtoi(wch) != 0);
      }
      // [2e]: Process elevation #166
      else if (n2e_IsIPCIDParam(lp1))
      {
        DWORD idIPC = 0;
        if ((swscanf_s(lp1 + CSTRLEN(IPCID_PARAM), L"%d", &idIPC) == 0)
            || !n2e_InitializeIPC(idIPC, FALSE))
        {
          return FALSE;
        }
        n2e_ChildProcess_FileIOHandler(idIPC);
        return FALSE;
      }
      // [2e]: Add to Favorites - selection mode #249
      else if (StrCmpNI(lp1, L"gs", CSTRLEN(L"gs")) == 0)
      {
        if (ExtractFirstArgument(lp2, lp1, lp2))
        {
          const int itok = swscanf(lp1, L"%i:%i", &iInitialSelStart, &iInitialSelEnd);
          if ((itok == 2) && (iInitialSelStart >= 0) && (iInitialSelEnd >= -1))
          {
            flagJumpTo = 1;
          }
          else
          {
            iInitialSelStart = -1;
            iInitialSelEnd = -1;
          }
        }
      }
      // [/2e]

      else switch (*CharUpper(lp1))
      {

        case L'N':
          flagReuseWindow = 0;
          flagNoReuseWindow = 1;
          if (*CharUpper(lp1 + 1) == L'S')
            flagSingleFileInstance = 1;
          else
            flagSingleFileInstance = 0;
          break;

        case L'R':
          flagReuseWindow = 1;
          flagNoReuseWindow = 0;
          if (*CharUpper(lp1 + 1) == L'S')
            flagSingleFileInstance = 1;
          else
            flagSingleFileInstance = 0;
          break;

        case L'F':
          if (*(lp1 + 1) == L'0' || *CharUpper(lp1 + 1) == L'O')
            lstrcpy(szIniFile, L"*?");
          else if (ExtractFirstArgument(lp2, lp1, lp2))
          {
            StrCpyN(szIniFile, lp1, COUNTOF(szIniFile));
            TrimString(szIniFile);
            PathUnquoteSpaces(szIniFile);
          }
          break;

        case L'I':
          flagStartAsTrayIcon = 1;
          break;

        case L'O':
          if (*(lp1 + 1) == L'0' || *(lp1 + 1) == L'-' || *CharUpper(lp1 + 1) == L'O')
            flagAlwaysOnTop = 1;
          else
            flagAlwaysOnTop = 2;
          break;

        case L'P': {
            WCHAR *lp = lp1;
            if (StrCmpNI(lp1, L"POS:", CSTRLEN(L"POS:")) == 0)
            {
              lp += CSTRLEN(L"POS:") - 1;
            }
            else if (StrCmpNI(lp1, L"POS", CSTRLEN(L"POS")) == 0)
            {
              lp += CSTRLEN(L"POS") - 1;
            }
            else if (*(lp1 + 1) == L':')
            {
              lp += 1;
            }
            else if (bIsNotepadReplacement)
            {
              if (*(lp1 + 1) == L'T')
              {
                ExtractFirstArgument(lp2, lp1, lp2);
              }
              break;
            }
            if (*(lp + 1) == L'0' || *CharUpper(lp + 1) == L'O')
            {
              flagPosParam = 1;
              flagDefaultPos = 1;
            }
            else if (*CharUpper(lp + 1) == L'D' || *CharUpper(lp + 1) == L'S')
            {
              flagPosParam = 1;
              flagDefaultPos = (StrChrI((lp + 1), L'L')) ? 3 : 2;
            }
            else if (StrChrI(L"FLTRBM", *(lp + 1)))
            {
              WCHAR *p = (lp + 1);
              flagPosParam = 1;
              flagDefaultPos = 0;
              while (*p)
              {
                switch (*CharUpper(p))
                {
                  case L'F':
                    flagDefaultPos &= ~(4 | 8 | 16 | 32);
                    flagDefaultPos |= 64;
                    break;
                  case L'L':
                    flagDefaultPos &= ~(8 | 64);
                    flagDefaultPos |= 4;
                    break;
                  case  L'R':
                    flagDefaultPos &= ~(4 | 64);
                    flagDefaultPos |= 8;
                    break;
                  case L'T':
                    flagDefaultPos &= ~(32 | 64);
                    flagDefaultPos |= 16;
                    break;
                  case L'B':
                    flagDefaultPos &= ~(16 | 64);
                    flagDefaultPos |= 32;
                    break;
                  case L'M':
                    if (flagDefaultPos == 0)
                      flagDefaultPos |= 64;
                    flagDefaultPos |= 128;
                    break;
                }
                p = CharNext(p);
              }
            }
            else if (ExtractFirstArgument(lp2, lp1, lp2))
            {
              int itok =
                swscanf(lp1, L"%i,%i,%i,%i,%i", &wi.x, &wi.y, &wi.cx, &wi.cy, &wi.max);
              if (itok == 4 || itok == 5)
              { // scan successful
                flagPosParam = 1;
                flagDefaultPos = 0;
                if (wi.cx < 1) wi.cx = CW_USEDEFAULT;
                if (wi.cy < 1) wi.cy = CW_USEDEFAULT;
                if (wi.max) wi.max = 1;
                if (itok == 4) wi.max = 0;
              }
            }
          }
          break;
        case L'T':
          if (ExtractFirstArgument(lp2, lp1, lp2))
          {
            StrCpyN(szTitleExcerpt, lp1, COUNTOF(szTitleExcerpt));
            fKeepTitleExcerpt = 1;
          }
          break;

        case L'C':
          flagNewFromClipboard = 1;
          break;

        case L'B':
          flagPasteBoard = 1;
          break;

        case L'E':
          if (ExtractFirstArgument(lp2, lp1, lp2))
          {
            if (lpEncodingArg)
            {
              LocalFree(lpEncodingArg);
            }
            lpEncodingArg = StrDup(lp1);
          }
          break;

        case L'G':
          if (ExtractFirstArgument(lp2, lp1, lp2))
          {
            int itok =
              swscanf(lp1, L"%i,%i", &iInitialLine, &iInitialColumn);
            if (itok == 1 || itok == 2)
            { // scan successful
              flagJumpTo = 1;
            }
          }
          break;

        case L'M': {
            BOOL bFindUp = FALSE;
            BOOL bRegex = FALSE;
            BOOL bTransBS = FALSE;

            if (StrChr(lp1, L'-'))
              bFindUp = TRUE;
            if (StrChr(lp1, L'R'))
              bRegex = TRUE;
            if (StrChr(lp1, L'B'))
              bTransBS = TRUE;

            if (ExtractFirstArgument(lp2, lp1, lp2))
            {
              if (lpMatchArg)
                GlobalFree(lpMatchArg);
              lpMatchArg = StrDup(lp1);
              flagMatchText = 1;
              if (bFindUp)
                flagMatchText |= 2;

              if (bRegex)
              {
                flagMatchText &= ~8;
                flagMatchText |= 4;
              }

              if (bTransBS)
              {
                flagMatchText &= ~4;
                flagMatchText |= 8;
              }
            }
          }
          break;

        case L'L':
          if (*(lp1 + 1) == L'0' || *(lp1 + 1) == L'-' || *CharUpper(lp1 + 1) == L'O')
            flagChangeNotify = 1;
          else
            flagChangeNotify = 2;
          break;

        case L'Q':
          flagQuietCreate = 1;
          break;

        case L'S':
          if (ExtractFirstArgument(lp2, lp1, lp2))
          {
            if (lpSchemeArg)
              LocalFree(lpSchemeArg);
            lpSchemeArg = StrDup(lp1);
            flagLexerSpecified = 1;
          }
          break;

        case L'D':
          if (lpSchemeArg)
          {
            LocalFree(lpSchemeArg);
            lpSchemeArg = NULL;
          }
          iInitialLexer = 0;
          flagLexerSpecified = 1;
          break;

        case L'H':
          if (lpSchemeArg)
          {
            LocalFree(lpSchemeArg);
            lpSchemeArg = NULL;
          }
          iInitialLexer = 1;
          flagLexerSpecified = 1;
          break;

        case L'X':
          if (lpSchemeArg)
          {
            LocalFree(lpSchemeArg);
            lpSchemeArg = NULL;
          }
          iInitialLexer = 2;
          flagLexerSpecified = 1;
          break;

        case L'U':
          flagRelaunchElevated = 1;
          break;
        case L'Z':
          ExtractFirstArgument(lp2, lp1, lp2);
          flagMultiFileArg = 1;
          bIsNotepadReplacement = TRUE;
          break;

        case L'?':
          flagDisplayHelp = 1;
          break;

        default:
          break;

      }

    }

    // pathname
    else
    {
      LPWSTR lpFileBuf = LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(lpCmdLine) + 1));

      cchiFileList = lstrlen(lpCmdLine) - lstrlen(lp3);

      if (lpFileArg)
        GlobalFree(lpFileArg);

      lpFileArg = GlobalAlloc(GPTR, sizeof(WCHAR) * (MAX_PATH + 2)); // changed for ActivatePrevInst() needs
      StrCpyN(lpFileArg, lp3, MAX_PATH);

      PathFixBackslashes(lpFileArg);

      if (!PathIsRelative(lpFileArg) && !PathIsUNC(lpFileArg) &&
          PathGetDriveNumber(lpFileArg) == -1)
      {
        WCHAR wchPath[MAX_PATH];
        lstrcpy(wchPath, g_wchWorkingDirectory);
        PathStripToRoot(wchPath);
        PathAppend(wchPath, lpFileArg);
        lstrcpy(lpFileArg, wchPath);
      }

      StrTrim(lpFileArg, L" \"");

      while (cFileList < 32 && ExtractFirstArgument(lp3, lpFileBuf, lp3))
      {
        PathQuoteSpaces(lpFileBuf);
        lpFileList[cFileList++] = StrDup(lpFileBuf);
      }

      bContinue = FALSE;
      LocalFree(lpFileBuf);
    }

    // Continue with next argument
    if (bContinue)
    {
      lstrcpy(lp3, lp2);
    }
  }

  LocalFree(lp1);
  LocalFree(lp2);
  LocalFree(lp3);

  return TRUE;
}


//=============================================================================
//
//  LoadFlags()
//
//
void LoadFlags()
{
  WCHAR *pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * 32 * 1024);
  int   cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);

  LoadIniSection(L"Settings2", pIniSection, cchIniSection);

  if (!flagReuseWindow && !flagNoReuseWindow)
  {
    if (!IniSectionGetInt(pIniSection, L"ReuseWindow", 0))
      flagNoReuseWindow = 1;

    if (IniSectionGetInt(pIniSection, L"SingleFileInstance", 0))
      flagSingleFileInstance = 1;
  }

  if (flagMultiFileArg == 0)
  {
    if (IniSectionGetInt(pIniSection, L"MultiFileArg", 0))
      flagMultiFileArg = 2;
  }

  if (IniSectionGetInt(pIniSection, L"RelativeFileMRU", 1))
    flagRelativeFileMRU = 1;

  if (IniSectionGetInt(pIniSection, L"PortableMyDocs", flagRelativeFileMRU))
    flagPortableMyDocs = 1;

  if (IniSectionGetInt(pIniSection, L"NoFadeHidden", 0))
    flagNoFadeHidden = 1;

  flagToolbarLook = IniSectionGetInt(pIniSection, L"ToolbarLook", IsXP() ? 1 : 2);
  flagToolbarLook = max(min(flagToolbarLook, 2), 0);

  if (IniSectionGetInt(pIniSection, L"SimpleIndentGuides", 0))
    flagSimpleIndentGuides = 1;

  if (IniSectionGetInt(pIniSection, L"NoHTMLGuess", 0))
    fNoHTMLGuess = 1;

  if (IniSectionGetInt(pIniSection, L"NoCGIGuess", 0))
    fNoCGIGuess = 1;

  if (IniSectionGetInt(pIniSection, L"NoFileVariables", 0))
    fNoFileVariables = 1;

  if (lstrlen(g_wchAppUserModelID) == 0)
  {
    IniSectionGetString(pIniSection, L"ShellAppUserModelID", L"(default)",
                        g_wchAppUserModelID, COUNTOF(g_wchAppUserModelID));
  }

  if (flagUseSystemMRU == 0)
  {
    if (IniSectionGetInt(pIniSection, L"ShellUseSystemMRU", 0))
      flagUseSystemMRU = 2;
  }

  LocalFree(pIniSection);
}


//=============================================================================
//
//  FindIniFile()
//
//
int CheckIniFile(LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tchFileExpanded[MAX_PATH];
  WCHAR tchBuild[MAX_PATH];
  ExpandEnvironmentStrings(lpszFile, tchFileExpanded, COUNTOF(tchFileExpanded));

  if (PathIsRelative(tchFileExpanded))
  {
    // program directory
    lstrcpy(tchBuild, lpszModule);
    lstrcpy(PathFindFileName(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild))
    {
      lstrcpy(lpszFile, tchBuild);
      return (1);
    }
    // %appdata%
    if (S_OK == SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, tchBuild))
    {
      PathAppend(tchBuild, tchFileExpanded);
      if (PathFileExists(tchBuild))
      {
        lstrcpy(lpszFile, tchBuild);
        return (1);
      }
    }
    // general
    if (SearchPath(NULL, tchFileExpanded, NULL, COUNTOF(tchBuild), tchBuild, NULL))
    {
      lstrcpy(lpszFile, tchBuild);
      return (1);
    }
  }

  else if (PathFileExists(tchFileExpanded))
  {
    lstrcpy(lpszFile, tchFileExpanded);
    return (1);
  }

  return (0);
}

int CheckIniFileRedirect(LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH];
  if (GetPrivateProfileString(L"Notepad2", L"Notepad2.ini", L"", tch, COUNTOF(tch), lpszFile))
  {
    if (CheckIniFile(tch, lpszModule))
    {
      lstrcpy(lpszFile, tch);
      return (1);
    }
    else
    {
      WCHAR tchFileExpanded[MAX_PATH];
      ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
      if (PathIsRelative(tchFileExpanded))
      {
        lstrcpy(lpszFile, lpszModule);
        lstrcpy(PathFindFileName(lpszFile), tchFileExpanded);
        return (1);
      }
      else
      {
        lstrcpy(lpszFile, tchFileExpanded);
        return (1);
      }
    }
  }
  return (0);
}

int FindIniFile()
{
  int bFound = 0;
  WCHAR tchTest[MAX_PATH];
  WCHAR tchModule[MAX_PATH];
  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));
  if (lstrlen(szIniFile))
  {
    if (lstrcmpi(szIniFile, L"*?") == 0)
      return (0);
    else
    {
      if (!CheckIniFile(szIniFile, tchModule))
      {
        ExpandEnvironmentStringsEx(szIniFile, COUNTOF(szIniFile));
        if (PathIsRelative(szIniFile))
        {
          lstrcpy(tchTest, tchModule);
          PathRemoveFileSpec(tchTest);
          PathAppend(tchTest, szIniFile);
          lstrcpy(szIniFile, tchTest);
        }
      }
    }
    return (1);
  }

  lstrcpy(tchTest, PathFindFileName(tchModule));
  PathRenameExtension(tchTest, L".ini");
  bFound = CheckIniFile(tchTest, tchModule);

  if (!bFound)
  {
    lstrcpy(tchTest, L"Notepad2.ini");
    bFound = CheckIniFile(tchTest, tchModule);
  }

  if (bFound)
  {
    // allow two redirections: administrator -> user -> custom
    if (CheckIniFileRedirect(tchTest, tchModule))
      CheckIniFileRedirect(tchTest, tchModule);
    lstrcpy(szIniFile, tchTest);
  }
  else
  {
    lstrcpy(szIniFile, tchModule);
    PathRenameExtension(szIniFile, L".ini");
  }

  return (1);
}


int TestIniFile()
{
  if (lstrcmpi(szIniFile, L"*?") == 0)
  {
    lstrcpy(szIniFile2, L"");
    lstrcpy(szIniFile, L"");
    return (0);
  }

  if (PathIsDirectory(szIniFile) || *CharPrev(szIniFile, StrEnd(szIniFile)) == L'\\')
  {
    WCHAR wchModule[MAX_PATH];
    GetModuleFileName(NULL, wchModule, COUNTOF(wchModule));
    PathAppend(szIniFile, PathFindFileName(wchModule));
    PathRenameExtension(szIniFile, L".ini");
    if (!PathFileExists(szIniFile))
    {
      lstrcpy(PathFindFileName(szIniFile), L"Notepad2.ini");
      if (!PathFileExists(szIniFile))
      {
        lstrcpy(PathFindFileName(szIniFile), PathFindFileName(wchModule));
        PathRenameExtension(szIniFile, L".ini");
      }
    }
  }

  if (!PathFileExists(szIniFile) || PathIsDirectory(szIniFile))
  {
    lstrcpy(szIniFile2, szIniFile);
    lstrcpy(szIniFile, L"");
    return (0);
  }
  else
    return (1);
}


int CreateIniFile()
{
  return (CreateIniFileEx(szIniFile));
}


int CreateIniFileEx(LPCWSTR lpszIniFile)
{
  if (*lpszIniFile)
  {
    HANDLE hFile;
    WCHAR *pwchTail;
    if (pwchTail = StrRChrW(lpszIniFile, NULL, L'\\'))
    {
      *pwchTail = 0;
      SHCreateDirectoryEx(NULL, lpszIniFile, NULL);
      *pwchTail = L'\\';
    }

    hFile = CreateFile(lpszIniFile,
                       GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    dwLastIOError = GetLastError();
    if (hFile != INVALID_HANDLE_VALUE)
    {
      if (GetFileSize(hFile, NULL) == 0)
      {
        DWORD dw;
        WriteFile(hFile, (LPCVOID)L"\xFEFF[Notepad2]\r\n", 26, &dw, NULL);
      }
      CloseHandle(hFile);
      return (1);
    }
    else
      return (0);
  }
  else
    return (0);
}


//=============================================================================
//
//  UpdateToolbar()
//
//
#define EnableTool(id,b) SendMessage(hwndToolbar,TB_ENABLEBUTTON,id, \
                                     MAKELONG(((b) ? 1 : 0), 0))
#define CheckTool(id,b)  SendMessage(hwndToolbar,TB_CHECKBUTTON,id, \
                                     MAKELONG(b,0))
void UpdateToolbar()
{
  int i;
  if (!bShowToolbar)
    return;

  EnableTool(IDT_FILE_ADDTOFAV, lstrlen(szCurFile));

  EnableTool(IDT_EDIT_UNDO, SendMessage(hwndEdit, SCI_CANUNDO, 0, 0));
  EnableTool(IDT_EDIT_REDO, SendMessage(hwndEdit, SCI_CANREDO, 0, 0));

  i = (int)SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0);
  EnableTool(IDT_EDIT_CUT, i);
  EnableTool(IDT_EDIT_COPY, i);
  EnableTool(IDT_EDIT_PASTE, SendMessage(hwndEdit, SCI_CANPASTE, 0, 0));
  EnableTool(IDT_EDIT_FIND, i);
  EnableTool(IDT_EDIT_REPLACE, i);
  EnableTool(IDT_EDIT_CLEAR, i);
  CheckTool(IDT_VIEW_WORDWRAP, fWordWrap);
  // [2e]: New toolbar button: selection mode #339
  CheckTool(IDT_VIEW_SELECTEX, fSelectEx);
  // [2e]: Function requirements #333
  n2e_UpdateAlwaysOnTopButton();
  // [2e]: Save on exit and History #101
  const BOOL bCommandEnabled = IsCmdEnabled(hwndMain, IDM_VIEW_SAVESETTINGS_MODE_ALL);
  EnableTool(IDT_SETTINGS_SAVE_ON_EXIT, bCommandEnabled);
  if (bCommandEnabled)
  {
    CheckTool(IDT_SETTINGS_SAVE_ON_EXIT, nSaveSettingsMode == SSM_ALL);
  }
  // [2e]: Binary Save Options button #170
  CheckTool(IDT_BINARY_SAFE_SAVE, !bFixLineEndings && !bAutoStripBlanks);
  // [2e]: Split view #316
  EnableTool(IDT_CLOSE_SPLIT, n2e_ScintillaWindowsCount() > 1);
}


//=============================================================================
//
//  UpdateStatusbar()
//
//
void UpdateStatusbar()
{

  int iPos;
  int iLn;
  int iLines;
  int iCol;
  int iSel;
  int iSelLines;
  WCHAR tchLn[32];
  WCHAR tchLines[32];
  WCHAR tchCol[32];
  WCHAR tchCols[32];
  WCHAR tchSel[32];
  WCHAR tchSelLines[32];
  WCHAR tchPos[32];
  
  int iBytes;
  WCHAR tchBytes[64];
  WCHAR tchDocSize[MAX_EXPRESSION_LENGTH];

  WCHAR tchEOLMode[32];
  WCHAR tchOvrMode[32];
  WCHAR tchLexerName[128];

  if (!bShowStatusbar)
    return;

  iPos = (int)SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);

  // [2e]: View > St&arting Line Number... #342
  iLn = n2e_GetVisibleLineNumber((int)SendMessage(hwndEdit, SCI_LINEFROMPOSITION, iPos, 0));
  wsprintf(tchLn, L"%i", iLn);
  FormatNumberStr(tchLn);

  // [2e]: View > St&arting Line Number... #342
  iLines = n2e_GetVisibleLineNumber((int)SendMessage(hwndEdit, SCI_GETLINECOUNT, 0, 0) - 1);
  wsprintf(tchLines, L"%i", iLines);
  FormatNumberStr(tchLines);

  // [2e]: Increasingly slow to hex/base64/qp #142
  iCol = (iLines > 1) ? (int)SendMessage(hwndEdit, SCI_GETCOLUMN, iPos, 0) + 1 : iPos;
  wsprintf(tchCol, L"%i", iCol);
  FormatNumberStr(tchCol);

  if (bMarkLongLines)
  {
    wsprintf(tchCols, L"%i", iLongLinesLimit);
    FormatNumberStr(tchCols);
  }

  // [2e]: Show line count for selection #204
  iSelLines = SciCall_LineFromPosition(SciCall_GetSelEnd())
              - SciCall_LineFromPosition(SciCall_GetSelStart())
              + 1;
  wsprintf(tchSelLines, L"%i", iSelLines);
  FormatNumberStr(tchSelLines);
  lstrcpy(tchSel, tchSelLines);

  if (!n2e_IsRectangularSelection())
  {
    iSel = SciCall_GetSelEnd() - SciCall_GetSelStart();
    wsprintf(tchSelLines, L"%i", iSel);
    FormatNumberStr(tchSelLines);
    lstrcat(tchSel, L" L : ");
    lstrcat(tchSel, tchSelLines);
    lstrcat(tchSel, L" B");
  }
  else  
  {
    // [2e]: Update status bar selection texts #262
    lstrcat(tchSel, L" L");
  }
  // [/2e]

  // [2e]: Display the data offset in the Statusbar #233
  wsprintf(tchPos, L"%i", iPos);
  FormatNumberStr(tchPos);
  // [/2e]
  
  if (!bMarkLongLines)
    FormatString(tchDocPos, COUNTOF(tchDocPos), IDS_DOCPOS, tchLn, tchLines, tchCol, tchSel, tchPos);
  else
    FormatString(tchDocPos, COUNTOF(tchDocPos), IDS_DOCPOS2, tchLn, tchLines, tchCol, tchCols, tchSel, tchPos);
  
  // [2e]: "Evaluate selection"-feature
  if (n2e_FormatEvaluatedExpression(hwndEdit,
        arrchExpressionText, COUNTOF(arrchExpressionText),
        arrwchExpressionValue, COUNTOF(arrwchExpressionValue), TRUE))
  {
    wcsncpy_s(tchDocSize, COUNTOF(tchDocSize), arrwchExpressionValue, COUNTOF(tchDocSize) - 1);
  }
  else
  {
    iBytes = (int)SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0);
    StrFormatByteSize(iBytes, tchBytes, COUNTOF(tchBytes));
    FormatString(tchDocSize, COUNTOF(tchDocSize), IDS_DOCSIZE, tchBytes);
  }

  Encoding_GetLabel(iEncoding);

  if (iEOLMode == SC_EOL_CR)
    lstrcpy(tchEOLMode, L"CR");
  else if (iEOLMode == SC_EOL_LF)
    lstrcpy(tchEOLMode, L"LF");
  else
    lstrcpy(tchEOLMode, L"CR+LF");

  if (SendMessage(hwndEdit, SCI_GETOVERTYPE, 0, 0))
    lstrcpy(tchOvrMode, L"OVR");
  else
    lstrcpy(tchOvrMode, L"INS");

  Style_GetCurrentLexerName(tchLexerName, COUNTOF(tchLexerName));

  StatusSetText(hwndStatus, STATUS_DOCPOS, tchDocPos);
  StatusSetText(hwndStatus, STATUS_DOCSIZE, tchDocSize);
  StatusSetText(hwndStatus, STATUS_CODEPAGE, mEncoding[iEncoding].wchLabel);
  StatusSetText(hwndStatus, STATUS_EOLMODE, tchEOLMode);
  StatusSetText(hwndStatus, STATUS_OVRMODE, tchOvrMode);
  StatusSetText(hwndStatus, STATUS_LEXER, bShowProgressBar ? tchProgressBarTaskName : tchLexerName);

  RECT rcMain = { 0 };
  GetWindowRect(hwndMain, &rcMain);
  UpdateStatusbarWidth(rcMain.right - rcMain.left);
}


//=============================================================================
//
//  UpdateLineNumberWidth()
//
//
void UpdateLineNumberWidth(HWND hwnd)
{
  char tchFirstLineIndex[32], tchLastLineIndex[32];
  int  iLineMarginWidthNow;
  int  iLineMarginWidthFit;

  if (bShowLineNumbers)
  {
    // [2e]: View > St&arting Line Number... #342
    wsprintfA(tchFirstLineIndex, "_%i_", n2e_GetVisibleLineNumber(0));
    wsprintfA(tchLastLineIndex, "_%i_", n2e_GetVisibleLineNumber(SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0) - 1));

    iLineMarginWidthNow = (int)SendMessage(hwnd, SCI_GETMARGINWIDTHN, 0, 0);

    // View > St&arting Line Number... #342
    iLineMarginWidthFit = max(
        SendMessage(hwnd, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)tchFirstLineIndex),
        SendMessage(hwnd, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)tchLastLineIndex)
      );

    if (iLineMarginWidthNow != iLineMarginWidthFit)
    {
      SendMessage(hwnd, SCI_SETMARGINWIDTHN, 0, iLineMarginWidthFit);
    }
  }

  else
    SendMessage(hwnd, SCI_SETMARGINWIDTHN, 0, 0);
}


void SetWordWrap(HWND hwnd)
{
  if (!fWordWrap)
    SendMessage(hwnd, SCI_SETWRAPMODE, SC_WRAP_NONE, 0);
  else
    SendMessage(hwnd, SCI_SETWRAPMODE, (iWordWrapMode == 0) ? SC_WRAP_WORD : SC_WRAP_CHAR, 0);
}


// [2e]: New toolbar button: selection mode #339
void SetSelectEx(HWND hwnd)
{
  if (fSelectEx)
  {
    iSelectExMode = SendMessage(hwnd, SCI_GETSELECTIONMODE, 0, 0);
    posSelectExStart = SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  }
}
// [/2e]


void SetLongLineSettings(HWND hwnd)
{
  if (bMarkLongLines)
  {
    SendMessage(hwnd, SCI_SETEDGEMODE, (iLongLineMode == EDGE_LINE) ? EDGE_LINE : EDGE_BACKGROUND, 0);
    Style_SetLongLineColors(hwnd);
  }
  else
  {
    SendMessage(hwnd, SCI_SETEDGEMODE, EDGE_NONE, 0);
  }  
  SendMessage(hwnd, SCI_SETEDGECOLUMN, iLongLinesLimit, 0);
}

void ShowSelectionMargin(HWND hwnd)
{
  SendMessage(hwnd, SCI_SETMARGINWIDTHN, 1, (bShowSelectionMargin) ? 16 : 0);
}

void ShowFirstColumnMargin(HWND hwnd)
{
  SendMessage(hwnd, SCI_SETMARGINWIDTHN, 2, (bShowFirstColumnMargin) ? 16 : 0);
}

void ShowWhiteSpace(HWND hwnd)
{
  SendMessage(hwnd, SCI_SETVIEWWS, (bViewWhiteSpace) ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE, 0);
}

void SetViewEOL(HWND hwnd)
{
  SendMessage(hwnd, SCI_SETVIEWEOL, bViewEOLs, 0);
}

void SetShowWordWrapSymbols(HWND hwnd)
{
  if (bShowWordWrapSymbols)
  {
    int wrapVisualFlags = 0;
    int wrapVisualFlagsLocation = 0;
    if (iWordWrapSymbols == 0)
      iWordWrapSymbols = 22;
    switch (iWordWrapSymbols % 10)
    {
    case 1:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
      wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_END_BY_TEXT;
      break;
    case 2:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
      break;
    }
    switch (((iWordWrapSymbols % 100) - (iWordWrapSymbols % 10)) / 10)
    {
    case 1:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
      wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_START_BY_TEXT;
      break;
    case 2:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
      break;
    }
    SendMessage(hwnd, SCI_SETWRAPVISUALFLAGSLOCATION, wrapVisualFlagsLocation, 0);
    SendMessage(hwnd, SCI_SETWRAPVISUALFLAGS, wrapVisualFlags, 0);
  }
  else
  {
    SendMessage(hwnd, SCI_SETWRAPVISUALFLAGS, 0, 0);
  }

}

void HideMatchBraces(HWND hwnd)
{
  SendMessage(hwnd, SCI_BRACEHIGHLIGHT, (WPARAM)-1, (LPARAM)-1);
}

//=============================================================================
//
//  FileIO()
//
//
BOOL FileIO(BOOL fLoad, LPCWSTR psz, BOOL bNoEncDetect, int *ienc, int *ieol,
            BOOL *pbUnicodeErr, BOOL *pbFileTooBig,
            BOOL *pbCancelDataLoss, BOOL bSaveCopy)
{
  WCHAR tch[MAX_PATH + 40];
  BOOL fSuccess;
  DWORD dwFileAttributes;

  BeginWaitCursor();

  FormatString(tch, COUNTOF(tch), (fLoad) ? IDS_LOADFILE : IDS_SAVEFILE, PathFindFileName(psz));

  StatusSetText(hwndStatus, STATUS_HELP, tch);
  StatusSetSimple(hwndStatus, TRUE);

  InvalidateRect(hwndStatus, NULL, TRUE);
  UpdateWindow(hwndStatus);

  if (fLoad)
    fSuccess = EditLoadFile(hwndEdit, psz, bNoEncDetect, ienc, ieol, pbUnicodeErr, pbFileTooBig);
  else
    fSuccess = EditSaveFile(hwndEdit, psz, *ienc, pbCancelDataLoss, bSaveCopy);

  dwFileAttributes = GetFileAttributes(psz);
  bReadOnly = (dwFileAttributes != INVALID_FILE_ATTRIBUTES && dwFileAttributes & FILE_ATTRIBUTE_READONLY);

  StatusSetSimple(hwndStatus, FALSE);

  EndWaitCursor();

  return (fSuccess);
}


//=============================================================================
//
//  FileLoad()
//
//
BOOL FileLoad(BOOL bDontSave, BOOL bNew, BOOL bReload, BOOL bNoEncDetect, LPCWSTR lpszFile)
{
  return _FileLoad(bDontSave, bNew, bReload, bNoEncDetect, lpszFile, FALSE);
}

BOOL _FileLoad(BOOL bDontSave, BOOL bNew, BOOL bReload, BOOL bNoEncDetect, LPCWSTR lpszFile, BOOL no_mru)
{
  WCHAR tch[MAX_PATH] = L"";
  WCHAR szFileName[MAX_PATH] = L"";
  BOOL fSuccess;
  BOOL bUnicodeErr = FALSE;
  BOOL bFileTooBig = FALSE;

  if (!bDontSave)
  {
    if (!FileSave(FALSE, TRUE, FALSE, FALSE, FALSE))
      return FALSE;
  }

  if (bNew)
  {
    lstrcpy(szCurFile, L"");
    SetDlgItemText(hwndMain, IDC_FILENAME, szCurFile);
    SetDlgItemInt(hwndMain, IDC_REUSELOCK, GetTickCount(), FALSE);
    if (!fKeepTitleExcerpt)
      lstrcpy(szTitleExcerpt, L"");
    FileVars_Init(NULL, 0, &fvCurFile);
    EditSetNewText(hwndEdit, "", 0);
    Style_SetLexer(hwndEdit, NULL);
    VIEW_COMMAND(UpdateLineNumberWidth);
    bModified = FALSE;
    bReadOnly = FALSE;
    iEOLMode = iLineEndings[iDefaultEOLMode];
    SendMessage(hwndEdit, SCI_SETEOLMODE, iLineEndings[iDefaultEOLMode], 0);
    iEncoding = iDefaultEncoding;
    iOriginalEncoding = iDefaultEncoding;
    SendMessage(hwndEdit, SCI_SETCODEPAGE, (iDefaultEncoding == CPI_DEFAULT) ? iDefaultCodePage : SC_CP_UTF8, 0);
    EditSetNewText(hwndEdit, "", 0);
    n2e_UpdateWindowTitle(hwndMain);

    // Terminate file watching
    if (bResetFileWatching)
      iFileWatchingMode = 0;
    InstallFileWatching(NULL);
    // [2e]: Match indicator
    n2e_ResetFindIcon();
    n2e_ResetLastRun();
    // [2e]: Save on deactivate #164
    n2e_ResetSaveOnLoseFocus();

    return TRUE;
  }
  if (!lpszFile || lstrlen(lpszFile) == 0)
  {
    if (!OpenFileDlg(hwndMain, tch, COUNTOF(tch), NULL))
      return FALSE;
  }
  else
    lstrcpy(tch, lpszFile);

  ExpandEnvironmentStringsEx(tch, COUNTOF(tch));

  if (PathIsRelative(tch))
  {
    StrCpyN(szFileName, g_wchWorkingDirectory, COUNTOF(szFileName));
    PathAppend(szFileName, tch);
    if (!PathFileExists(szFileName))
    {
      WCHAR wchFullPath[MAX_PATH];
      if (SearchPath(NULL, tch, NULL, COUNTOF(wchFullPath), wchFullPath, NULL))
        lstrcpy(szFileName, wchFullPath);
    }
  }
  else
    lstrcpy(szFileName, tch);

  PathCanonicalizeEx(szFileName);
  GetLongPathNameEx(szFileName, COUNTOF(szFileName));

  if (PathIsLnkFile(szFileName))
    PathGetLnkPath(szFileName, szFileName, COUNTOF(szFileName));

  // [2e]: File lock not released on open #100
  // change current directory to prevent directory lock on another path
  WCHAR szFolder[MAX_PATH];
  if (lstrcpyn(szFolder, szFileName, _countof(szFolder) - 1) && PathRemoveFileSpec(szFolder))
  {
    SetCurrentDirectory(szFolder);
  }
  // [/2e]

  // Ask to create a new file...
  if (!bReload && !PathFileExists(szFileName))
  {
    if (flagQuietCreate || MsgBox(MBYESNO, IDS_ASK_CREATE, szFileName) == IDYES)
    {
      HANDLE hFile = CreateFile(szFileName,
                                GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
      dwLastIOError = GetLastError();
      if (fSuccess = (hFile != INVALID_HANDLE_VALUE))
      {
        CloseHandle(hFile);
        FileVars_Init(NULL, 0, &fvCurFile);
        EditSetNewText(hwndEdit, "", 0);
        Style_SetLexer(hwndEdit, NULL);
        iEOLMode = iLineEndings[iDefaultEOLMode];
        SendMessage(hwndEdit, SCI_SETEOLMODE, iLineEndings[iDefaultEOLMode], 0);
        if (iSrcEncoding != -1)
        {
          iEncoding = iSrcEncoding;
          iOriginalEncoding = iSrcEncoding;
        }
        else
        {
          iEncoding = iDefaultEncoding;
          iOriginalEncoding = iDefaultEncoding;
        }
        SendMessage(hwndEdit, SCI_SETCODEPAGE, (iEncoding == CPI_DEFAULT) ? iDefaultCodePage : SC_CP_UTF8, 0);
        bReadOnly = FALSE;
        EditSetNewText(hwndEdit, "", 0);
      }
    }
    else
      return FALSE;
  }
  else
  {
    fSuccess = FileIO(TRUE, szFileName, bNoEncDetect, &iEncoding, &iEOLMode, &bUnicodeErr, &bFileTooBig, NULL, FALSE);
  }

  if (fSuccess)
  {
    lstrcpy(szCurFile, szFileName);
    SetDlgItemText(hwndMain, IDC_FILENAME, szCurFile);
    SetDlgItemInt(hwndMain, IDC_REUSELOCK, GetTickCount(), FALSE);
    if (!fKeepTitleExcerpt)
      lstrcpy(szTitleExcerpt, L"");
    if (!flagLexerSpecified) // flag will be cleared
      Style_SetLexerFromFile(hwndEdit, szCurFile);
    VIEW_COMMAND(UpdateLineNumberWidth);
    iOriginalEncoding = iEncoding;
    bModified = FALSE;
    SendMessage(hwndEdit, SCI_SETEOLMODE, iEOLMode, 0);
    if (!no_mru)
    {
      MRU_AddFile(pFileMRU, szFileName, flagRelativeFileMRU, flagPortableMyDocs);
      if (flagUseSystemMRU == 2)
        SHAddToRecentDocs(SHARD_PATHW, szFileName);
    }
    n2e_UpdateWindowTitle(hwndMain);
    
    // Install watching of the current file
    if (!bReload && bResetFileWatching)
      iFileWatchingMode = 0;
    InstallFileWatching(szCurFile);

    // the .LOG feature ...
    if (SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0) >= 4)
    {
      char tchLog[5] = "";
      SendMessage(hwndEdit, SCI_GETTEXT, 5, (LPARAM)tchLog);
      if (lstrcmpiA(tchLog, ".LOG") == 0)
      {
        EditJumpTo(hwndEdit, -1, 0);
        SendMessage(hwndEdit, SCI_BEGINUNDOACTION, 0, 0);
        SendMessage(hwndEdit, SCI_NEWLINE, 0, 0);
        SendMessage(hwndMain, WM_COMMAND, MAKELONG(IDM_EDIT_INSERT_SHORTDATE, 1), 0);
        EditJumpTo(hwndEdit, -1, 0);
        SendMessage(hwndEdit, SCI_NEWLINE, 0, 0);
        SendMessage(hwndEdit, SCI_ENDUNDOACTION, 0, 0);
        EditJumpTo(hwndEdit, -1, 0);
        EditEnsureSelectionVisible(hwndEdit);
      }
    }

    // Show warning: Unicode file loaded as ANSI
    if (bUnicodeErr)
    {
      MsgBox(MBWARN, IDS_ERR_UNICODE);
    }
  }
  else if (!bFileTooBig)
    MsgBox(MBWARN, IDS_ERR_LOADFILE, szFileName);

  if (fSuccess)
  {
    // [2e]: Match indicator
    n2e_ResetFindIcon();
    n2e_ResetLastRun();
    // [2e]: Save on deactivate #164
    n2e_ResetSaveOnLoseFocus();
  }

  return (fSuccess);
}


//=============================================================================
//
//  FileSave()
//
//
BOOL FileSaveImpl(BOOL bSaveAlways, BOOL bAsk, BOOL bSaveAs, BOOL bSaveCopy, BOOL bDeleteOld)
{
  WCHAR tchFile[MAX_PATH];
  BOOL fSuccess = FALSE;
  BOOL bCancelDataLoss = FALSE;

  BOOL bIsEmptyNewFile = FALSE;
  if (lstrlen(szCurFile) == 0)
  {
    int cchText = (int)SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0);
    if (cchText == 0)
      bIsEmptyNewFile = TRUE;
    else if (cchText < 1023)
    {
      char tchText[2048];
      SendMessage(hwndEdit, SCI_GETTEXT, (WPARAM)2047, (LPARAM)tchText);
      StrTrimA(tchText, " \t\n\r");
      if (lstrlenA(tchText) == 0)
        bIsEmptyNewFile = TRUE;
    }
  }

  if (!bSaveAlways && (!n2e_IsDocumentModified() || bIsEmptyNewFile) && !bSaveAs)
    return TRUE;

  if (bAsk)
  {
    // File or "Untitled" ...
    WCHAR tch[MAX_PATH];
    if (lstrlen(szCurFile))
      lstrcpy(tch, szCurFile);
    else
      GetString(IDS_UNTITLED, tch, COUNTOF(tch));

    switch (MsgBox(MBYESNOCANCEL, IDS_ASK_SAVE, tch))
    {
      case IDCANCEL:
        return FALSE;
      case IDNO:
        return TRUE;
    }
  }

  // Read only...
  if (!bSaveAs && !bSaveCopy && lstrlen(szCurFile))
  {
    DWORD dwFileAttributes = GetFileAttributes(szCurFile);
    if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
      bReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY);
    if (bReadOnly)
    {
      n2e_UpdateWindowTitle(hwndMain);
      if (MsgBox(MBYESNOWARN, IDS_READONLY_SAVE, szCurFile) == IDYES)
        bSaveAs = TRUE;
      else
        return FALSE;
    }
  }

  // Save As...
  if (bSaveAs || bSaveCopy || lstrlen(szCurFile) == 0)
  {
    WCHAR tchInitialDir[MAX_PATH] = L"";
    if (bSaveCopy && lstrlen(tchLastSaveCopyDir))
    {
      lstrcpy(tchInitialDir, tchLastSaveCopyDir);
      lstrcpy(tchFile, tchLastSaveCopyDir);
      PathAppend(tchFile, PathFindFileName(szCurFile));
    }
    else
      lstrcpy(tchFile, szCurFile);

    if (SaveFileDlg(hwndMain, tchFile, COUNTOF(tchFile), tchInitialDir))
    {
      // [2e]: Rename To fails if new name only differs in char case #140
      if (lstrcmp(szCurFile, tchFile) == 0)
      {
        WCHAR tchDescription[MAX_PATH] = { 0 };
        if (bDeleteOld)
        {
          GetString(IDS_ERR_SAVEAS_RENAME_DETAILS, tchDescription, COUNTOF(tchDescription) - 1);
        }
        MsgBox(MBWARN, IDS_ERR_SAVEAS, tchDescription);
        return FALSE;
      }
      // [/2e]
      else if (fSuccess = FileIO(FALSE, tchFile, FALSE, &iEncoding, &iEOLMode, NULL, NULL, &bCancelDataLoss, bSaveCopy)
                          // [2e]: Process elevation #166
                          || n2e_ParentProcess_ElevatedFileIO(tchFile))
      {
        n2e_ResetLastRun();
        // [2e]: File->RenameTo menu item
        if (bDeleteOld && lstrlen(szCurFile))
        {
          DeleteFile(szCurFile);
        }
        // [/2e]
        if (!bSaveCopy)
        {
          lstrcpy(szCurFile, tchFile);
          SetDlgItemText(hwndMain, IDC_FILENAME, szCurFile);
          SetDlgItemInt(hwndMain, IDC_REUSELOCK, GetTickCount(), FALSE);
          if (!fKeepTitleExcerpt)
            lstrcpy(szTitleExcerpt, L"");
          Style_SetLexerFromFile(hwndEdit, szCurFile);
          UpdateStatusbar();
          VIEW_COMMAND(UpdateLineNumberWidth);
        }
        else
        {
          lstrcpy(tchLastSaveCopyDir, tchFile);
          PathRemoveFileSpec(tchLastSaveCopyDir);
        }
      }
    }
    else
      return FALSE;
  }

  else
  {
    // [2e]: Junk file name output in the Access Denied message box #412
    lstrcpy(tchFile, szCurFile);

    fSuccess = FileIO(FALSE, szCurFile, FALSE, &iEncoding, &iEOLMode, NULL, NULL, &bCancelDataLoss, FALSE)
               // [2e]: Process elevation #166
               || n2e_ParentProcess_ElevatedFileIO(szCurFile);
  }

  if (fSuccess)
  {
    if (!bSaveCopy)
    {
      bModified = FALSE;
      iOriginalEncoding = iEncoding;
      MRU_AddFile(pFileMRU, szCurFile, flagRelativeFileMRU, flagPortableMyDocs);
      if (flagUseSystemMRU == 2)
        SHAddToRecentDocs(SHARD_PATHW, szCurFile);
      n2e_UpdateWindowTitle(hwndMain);
      // Install watching of the current file
      if (bSaveAs && bResetFileWatching)
        iFileWatchingMode = 0;
      InstallFileWatching(szCurFile);
    }
  }

  else if (!bCancelDataLoss)
  {
    n2e_UpdateWindowTitle(hwndMain);
    MsgBox(MBWARN, IDS_ERR_SAVEFILE, tchFile);
  }

  return (fSuccess);
}


BOOL FileSave(BOOL bSaveAlways, BOOL bAsk, BOOL bSaveAs, BOOL bSaveCopy, BOOL bDeleteOld)
{
  // [2e]: Save on deactivate #164
  BOOL res = FALSE;
  bFileSaveInProgress = TRUE;
  res = FileSaveImpl(bSaveAlways, bAsk, bSaveAs, bSaveCopy, bDeleteOld);
  bFileSaveInProgress = FALSE;
  return res;
  // [/2e]
}

//=============================================================================
//
//  OpenFileDlg()
//
//
BOOL OpenFileDlg(HWND hwnd, LPWSTR lpstrFile, int cchFile, LPCWSTR lpstrInitialDir)
{
  OPENFILENAME ofn;
  WCHAR szFile[MAX_PATH];
  WCHAR szFilter[NUMLEXERS * 1024];
  WCHAR tchInitialDir[MAX_PATH] = L"";

  lstrcpy(szFile, L"");
  Style_GetOpenDlgFilterStr(szFilter, COUNTOF(szFilter));

  if (!lpstrInitialDir)
  {
    if (lstrlen(szCurFile))
    {
      lstrcpy(tchInitialDir, szCurFile);
      PathRemoveFileSpec(tchInitialDir);
    }
    else if (lstrlen(tchDefaultDir))
    {
      ExpandEnvironmentStrings(tchDefaultDir, tchInitialDir, COUNTOF(tchInitialDir));
      if (PathIsRelative(tchInitialDir))
      {
        WCHAR tchModule[MAX_PATH];
        GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));
        PathRemoveFileSpec(tchModule);
        PathAppend(tchModule, tchInitialDir);
        PathCanonicalize(tchInitialDir, tchModule);
      }
    }
    else
    {
      n2e_GetLastDir(tchInitialDir);
    }
  }

  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  // [2e]: Open/Save dialogs - configurable filters #258
  ofn.nFilterIndex = iOpenSaveFilterIndex;
  ofn.lpstrFile = szFile;
  ofn.lpstrInitialDir = (lpstrInitialDir) ? lpstrInitialDir : tchInitialDir;
  ofn.nMaxFile = COUNTOF(szFile);
  // [2e]: Open dialog - open by name prefix #19
  ofn.lpfnHook = n2e_OFNHookProc;
  ofn.Flags =
    OFN_HIDEREADONLY |
    OFN_DONTADDTORECENT |
    OFN_SHAREAWARE;
  if (bUsePrefixInOpenDialog)
    ofn.Flags |= (OFN_ENABLEHOOK | OFN_EXPLORER);
  else
    ofn.Flags |= OFN_FILEMUSTEXIST;
  // [/2e]
  ofn.lpstrDefExt = (lstrlen(tchDefaultExtension)) ? tchDefaultExtension : NULL;

  if (GetOpenFileName(&ofn))
  {
    // [2e]: Open/Save dialogs - configurable filters #258
    iOpenSaveFilterIndex = ofn.nFilterIndex;
    lstrcpyn(lpstrFile, szFile, cchFile);
    return TRUE;
  }
  else
    return FALSE;
}


//=============================================================================
//
//  SaveFileDlg()
//
//
BOOL SaveFileDlg(HWND hwnd, LPWSTR lpstrFile, int cchFile, LPCWSTR lpstrInitialDir)
{
  OPENFILENAME ofn;
  WCHAR szNewFile[MAX_PATH] = { 0 };
  WCHAR szFilter[NUMLEXERS * 1024];
  WCHAR tchInitialDir[MAX_PATH] = L"";

  // [2e]: Save Copy doesn't work second time #148
  if (PathFindFileName(lpstrFile) != lpstrFile)
  {
    lstrcpy(szNewFile, lpstrFile);
  }
  // [/2e]
  Style_GetOpenDlgFilterStr(szFilter, COUNTOF(szFilter));

  if (lstrlen(lpstrInitialDir))
    lstrcpy(tchInitialDir, lpstrInitialDir);
  else if (lstrlen(szCurFile))
  {
    lstrcpy(tchInitialDir, szCurFile);
    PathRemoveFileSpec(tchInitialDir);
  }
  else if (lstrlen(tchDefaultDir))
  {
    ExpandEnvironmentStrings(tchDefaultDir, tchInitialDir, COUNTOF(tchInitialDir));
    if (PathIsRelative(tchInitialDir))
    {
      WCHAR tchModule[MAX_PATH];
      GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));
      PathRemoveFileSpec(tchModule);
      PathAppend(tchModule, tchInitialDir);
      PathCanonicalize(tchInitialDir, tchModule);
    }
  }
  else
  {
    n2e_GetLastDir(tchInitialDir);
  }

  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  // [2e]: Open/Save dialogs - configurable filters #258
  ofn.nFilterIndex = iOpenSaveFilterIndex;
  ofn.lpstrFile = szNewFile;
  ofn.lpstrInitialDir = tchInitialDir;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST;
  // [2e]: Save/Save To - file extension #17
  LPWSTR oext = PathFindExtensionW(szCurFile);
  LPWSTR oname = PathFindFileNameW(szCurFile);
  if (0 == lstrlen(szCurFile))
  {
    ofn.lpstrDefExt = (lstrlen(tchDefaultExtension)) ? tchDefaultExtension : NULL;
  }
  else  if (NULL == oext || lstrlen(oext) < 2 || 0 == lstrcmp(oext, oname))
  {
    ofn.lpstrDefExt = NULL;
  }
  else
  {
    ofn.lpstrDefExt = (oext + 1);
  }
  // [/2e]
  if (GetSaveFileName(&ofn))
  {
    // [2e]: Open/Save dialogs - configurable filters #258
    iOpenSaveFilterIndex = ofn.nFilterIndex;
    lstrcpyn(lpstrFile, szNewFile, cchFile);
    return TRUE;
  }
  else
    return FALSE;
}


/******************************************************************************
*
* ActivatePrevInst()
*
* Tries to find and activate an already open Notepad2 Window
*
*
******************************************************************************/
BOOL CALLBACK EnumWndProc(HWND hwnd, LPARAM lParam)
{
  BOOL bContinue = TRUE;
  WCHAR szClassName[64];

  if (GetClassName(hwnd, szClassName, COUNTOF(szClassName)))
    if (lstrcmpi(szClassName, wchWndClass) == 0)
    {
      DWORD dwReuseLock = GetDlgItemInt(hwnd, IDC_REUSELOCK, NULL, FALSE);
      if (GetTickCount() - dwReuseLock >= REUSEWINDOWLOCKTIMEOUT)
      {
        *(HWND *)lParam = hwnd;

        if (IsWindowEnabled(hwnd))
          bContinue = FALSE;
      }
    }
  return (bContinue);
}

BOOL CALLBACK EnumWndProc2(HWND hwnd, LPARAM lParam)
{
  BOOL bContinue = TRUE;
  WCHAR szClassName[64];

  if (GetClassName(hwnd, szClassName, COUNTOF(szClassName)))
    if (lstrcmpi(szClassName, wchWndClass) == 0)
    {
      DWORD dwReuseLock = GetDlgItemInt(hwnd, IDC_REUSELOCK, NULL, FALSE);
      if (GetTickCount() - dwReuseLock >= REUSEWINDOWLOCKTIMEOUT)
      {
        WCHAR tchFileName[MAX_PATH] = L"";

        if (IsWindowEnabled(hwnd))
          bContinue = FALSE;

        GetDlgItemText(hwnd, IDC_FILENAME, tchFileName, COUNTOF(tchFileName));
        if (lstrcmpi(tchFileName, lpFileArg) == 0)
          *(HWND *)lParam = hwnd;
        else
          bContinue = TRUE;
      }
    }
  return (bContinue);
}

BOOL ActivatePrevInst()
{
  HWND hwnd = NULL;
  COPYDATASTRUCT cds;

  if ((flagNoReuseWindow && !flagSingleFileInstance) || flagStartAsTrayIcon || flagNewFromClipboard || flagPasteBoard)
    return (FALSE);

  if (flagSingleFileInstance && lpFileArg)
  {
    // Search working directory from second instance, first!
    // lpFileArg is at least MAX_PATH+2 bytes
    WCHAR tchTmp[MAX_PATH];

    ExpandEnvironmentStringsEx(lpFileArg, (DWORD)GlobalSize(lpFileArg) / sizeof(WCHAR));

    if (PathIsRelative(lpFileArg))
    {
      StrCpyN(tchTmp, g_wchWorkingDirectory, COUNTOF(tchTmp));
      PathAppend(tchTmp, lpFileArg);
      if (PathFileExists(tchTmp))
        lstrcpy(lpFileArg, tchTmp);
      else
      {
        if (SearchPath(NULL, lpFileArg, NULL, COUNTOF(tchTmp), tchTmp, NULL))
          lstrcpy(lpFileArg, tchTmp);
        else
        {
          StrCpyN(tchTmp, g_wchWorkingDirectory, COUNTOF(tchTmp));
          PathAppend(tchTmp, lpFileArg);
          lstrcpy(lpFileArg, tchTmp);
        }
      }
    }
    else if (SearchPath(NULL, lpFileArg, NULL, COUNTOF(tchTmp), tchTmp, NULL))
      lstrcpy(lpFileArg, tchTmp);

    GetLongPathNameEx(lpFileArg, MAX_PATH);

    EnumWindows(EnumWndProc2, (LPARAM)&hwnd);

    if (hwnd != NULL)
    {
      // Enabled
      if (IsWindowEnabled(hwnd))
      {
        LPNP2PARAMS params;
        DWORD cb = sizeof(NP2PARAMS);

        if (IsIconic(hwnd))
          ShowWindowAsync(hwnd, SW_RESTORE);

        if (!IsWindowVisible(hwnd))
        {
          SendMessage(hwnd, WM_TRAYMESSAGE, 0, WM_LBUTTONDBLCLK);
          SendMessage(hwnd, WM_TRAYMESSAGE, 0, WM_LBUTTONUP);
        }

        SetForegroundWindow(hwnd);

        if (lpSchemeArg)
          cb += (lstrlen(lpSchemeArg) + 1) * sizeof(WCHAR);

        params = GlobalAlloc(GPTR, cb);
        params->flagFileSpecified = FALSE;
        params->flagChangeNotify = 0;
        params->flagQuietCreate = FALSE;
        params->flagLexerSpecified = flagLexerSpecified;
        if (flagLexerSpecified && lpSchemeArg)
        {
          lstrcpy(StrEnd(&params->wchData) + 1, lpSchemeArg);
          params->iInitialLexer = -1;
        }
        else
          params->iInitialLexer = iInitialLexer;
        params->flagJumpTo = flagJumpTo;
        params->iInitialLine = iInitialLine;
        params->iInitialColumn = iInitialColumn;
        params->iInitialSelStart = iInitialSelStart;
        params->iInitialSelEnd = iInitialSelEnd;

        params->iSrcEncoding = (lpEncodingArg) ? Encoding_MatchW(lpEncodingArg) : -1;
        params->flagSetEncoding = flagSetEncoding;
        params->flagSetEOLMode = flagSetEOLMode;
        params->flagTitleExcerpt = 0;

        cds.dwData = DATA_NOTEPAD2_PARAMS;
        cds.cbData = (DWORD)GlobalSize(params);
        cds.lpData = params;

        SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
        GlobalFree(params);

        return (TRUE);
      }

      else // IsWindowEnabled()
      {
        if (IDYES == MsgBox(MBYESNO, IDS_ERR_PREVWINDISABLED))
          return (FALSE);
        else
          return (TRUE);
      }
    }
  }

  if (flagNoReuseWindow)
    return (FALSE);

  hwnd = NULL;
  EnumWindows(EnumWndProc, (LPARAM)&hwnd);

  // Found a window
  if (hwnd != NULL)
  {
    // Enabled
    if (IsWindowEnabled(hwnd))
    {
      if (IsIconic(hwnd))
        ShowWindowAsync(hwnd, SW_RESTORE);

      if (!IsWindowVisible(hwnd))
      {
        SendMessage(hwnd, WM_TRAYMESSAGE, 0, WM_LBUTTONDBLCLK);
        SendMessage(hwnd, WM_TRAYMESSAGE, 0, WM_LBUTTONUP);
      }

      SetForegroundWindow(hwnd);

      if (lpFileArg)
      {
        // Search working directory from second instance, first!
        // lpFileArg is at least MAX_PATH+2 bytes
        WCHAR tchTmp[MAX_PATH];
        LPNP2PARAMS params;
        DWORD cb = sizeof(NP2PARAMS);
        int cchTitleExcerpt;

        ExpandEnvironmentStringsEx(lpFileArg, (DWORD)GlobalSize(lpFileArg) / sizeof(WCHAR));

        if (PathIsRelative(lpFileArg))
        {
          StrCpyN(tchTmp, g_wchWorkingDirectory, COUNTOF(tchTmp));
          PathAppend(tchTmp, lpFileArg);
          if (PathFileExists(tchTmp))
            lstrcpy(lpFileArg, tchTmp);
          else
          {
            if (SearchPath(NULL, lpFileArg, NULL, COUNTOF(tchTmp), tchTmp, NULL))
              lstrcpy(lpFileArg, tchTmp);
          }
        }
        else if (SearchPath(NULL, lpFileArg, NULL, COUNTOF(tchTmp), tchTmp, NULL))
          lstrcpy(lpFileArg, tchTmp);

        cb += (lstrlen(lpFileArg) + 1) * sizeof(WCHAR);

        if (lpSchemeArg)
          cb += (lstrlen(lpSchemeArg) + 1) * sizeof(WCHAR);

        cchTitleExcerpt = lstrlen(szTitleExcerpt);
        if (cchTitleExcerpt)
          cb += (cchTitleExcerpt + 1) * sizeof(WCHAR);

        params = GlobalAlloc(GPTR, cb);
        params->flagFileSpecified = TRUE;
        lstrcpy(&params->wchData, lpFileArg);
        params->flagChangeNotify = flagChangeNotify;
        params->flagQuietCreate = flagQuietCreate;
        params->flagLexerSpecified = flagLexerSpecified;
        if (flagLexerSpecified && lpSchemeArg)
        {
          lstrcpy(StrEnd(&params->wchData) + 1, lpSchemeArg);
          params->iInitialLexer = -1;
        }
        else
          params->iInitialLexer = iInitialLexer;
        params->flagJumpTo = flagJumpTo;
        params->iInitialLine = iInitialLine;
        params->iInitialColumn = iInitialColumn;
        params->iInitialSelStart = iInitialSelStart;
        params->iInitialSelEnd = iInitialSelEnd;

        params->iSrcEncoding = (lpEncodingArg) ? Encoding_MatchW(lpEncodingArg) : -1;
        params->flagSetEncoding = flagSetEncoding;
        params->flagSetEOLMode = flagSetEOLMode;
        if (cchTitleExcerpt)
        {
          lstrcpy(StrEnd(&params->wchData) + 1, szTitleExcerpt);
          params->flagTitleExcerpt = 1;
        }
        else
          params->flagTitleExcerpt = 0;

        cds.dwData = DATA_NOTEPAD2_PARAMS;
        cds.cbData = (DWORD)GlobalSize(params);
        cds.lpData = params;

        SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
        GlobalFree(params);
        GlobalFree(lpFileArg);
      }
      return (TRUE);
    }
    else
    {
      if (IDYES == MsgBox(MBYESNO, IDS_ERR_PREVWINDISABLED))
        return (FALSE);
      else
        return (TRUE);
    }
  }
  else
    return (FALSE);
}


//=============================================================================
//
//  RelaunchMultiInst()
//
//
BOOL RelaunchMultiInst()
{
  if (flagMultiFileArg == 2 && cFileList > 1)
  {
    WCHAR *pwch;
    int i = 0;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    LPWSTR lpCmdLineNew = StrDup(GetCommandLine());
    LPWSTR lp1 = LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(lpCmdLineNew) + 1));
    LPWSTR lp2 = LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(lpCmdLineNew) + 1));

    StrTab2Space(lpCmdLineNew);
    lstrcpy(lpCmdLineNew + cchiFileList, L"");

    pwch = CharPrev(lpCmdLineNew, StrEnd(lpCmdLineNew));
    while (*pwch == L' ' || *pwch == L'-' || *pwch == L'+')
    {
      *pwch = L' ';
      pwch = CharPrev(lpCmdLineNew, pwch);
      if (i++ > 1)
        cchiFileList--;
    }
    for (i = 0; i < cFileList; i++)
    {
      lstrcpy(lpCmdLineNew + cchiFileList, L" /n - ");
      lstrcat(lpCmdLineNew, lpFileList[i]);
      LocalFree(lpFileList[i]);

      ZeroMemory(&si, sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);
      ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

      CreateProcess(NULL, lpCmdLineNew, NULL, NULL, FALSE, 0, NULL, g_wchWorkingDirectory, &si, &pi);
    }
    LocalFree(lpCmdLineNew);
    LocalFree(lp1);
    LocalFree(lp2);
    GlobalFree(lpFileArg);

    return TRUE;
  }
  else
  {
    int i;
    for (i = 0; i < cFileList; i++)
      LocalFree(lpFileList[i]);
    return FALSE;
  }
}


//=============================================================================
//
//  RelaunchElevated()
//
//
BOOL RelaunchElevated()
{
  if (!IsVista() || fIsElevated || !flagRelaunchElevated || flagDisplayHelp)
    return (FALSE);

  else
  {

    LPWSTR lpCmdLine;
    LPWSTR lpArg1, lpArg2;
    STARTUPINFO si;
    SHELLEXECUTEINFO sei;

    si.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&si);

    lpCmdLine = GetCommandLine();
    lpArg1 = LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(lpCmdLine) + 1));
    lpArg2 = LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(lpCmdLine) + 1));
    ExtractFirstArgument(lpCmdLine, lpArg1, lpArg2);

    if (lstrlen(lpArg1))
    {

      ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
      sei.cbSize = sizeof(SHELLEXECUTEINFO);
      sei.fMask = SEE_MASK_FLAG_NO_UI | /*SEE_MASK_NOASYNC*/0x00000100 | /*SEE_MASK_NOZONECHECKS*/0x00800000;
      sei.hwnd = GetForegroundWindow();
      sei.lpVerb = L"runas";
      sei.lpFile = lpArg1;
      sei.lpParameters = lpArg2;
      sei.lpDirectory = g_wchWorkingDirectory;
      sei.nShow = si.wShowWindow;

      ShellExecuteEx(&sei);
    }

    LocalFree(lpArg1);
    LocalFree(lpArg2);

    return (TRUE);
  }
}


//=============================================================================
//
//  SnapToDefaultPos()
//
//  Aligns Notepad2 to the default window position on the current screen
//
//
void SnapToDefaultPos(HWND hwnd)
{
  WINDOWPLACEMENT wndpl;
  HMONITOR hMonitor;
  MONITORINFO mi;
  int x, y, cx, cy;
  RECT rcOld;
  GetWindowRect(hwnd, &rcOld);
  hMonitor = MonitorFromRect(&rcOld, MONITOR_DEFAULTTONEAREST);
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);

  y = mi.rcWork.top + 16;
  cy = mi.rcWork.bottom - mi.rcWork.top - 32;
  cx = min(mi.rcWork.right - mi.rcWork.left - 32, cy);
  x = mi.rcWork.right - cx - 16;

  wndpl.length = sizeof(WINDOWPLACEMENT);
  wndpl.flags = WPF_ASYNCWINDOWPLACEMENT;
  wndpl.showCmd = SW_RESTORE;

  wndpl.rcNormalPosition.left = x;
  wndpl.rcNormalPosition.top = y;
  wndpl.rcNormalPosition.right = x + cx;
  wndpl.rcNormalPosition.bottom = y + cy;

  if (EqualRect(&rcOld, &wndpl.rcNormalPosition))
  {
    x = mi.rcWork.left + 16;
    wndpl.rcNormalPosition.left = x;
    wndpl.rcNormalPosition.right = x + cx;
  }

  if (GetDoAnimateMinimize())
  {
    DrawAnimatedRects(hwnd, IDANI_CAPTION, &rcOld, &wndpl.rcNormalPosition);
    OffsetRect(&wndpl.rcNormalPosition, mi.rcMonitor.left - mi.rcWork.left, mi.rcMonitor.top - mi.rcWork.top);
  }
  SetWindowPlacement(hwnd, &wndpl);
}


//=============================================================================
//
//  ShowNotifyIcon()
//
//
void ShowNotifyIcon(HWND hwnd, BOOL bAdd)
{

  static HICON hIcon;
  NOTIFYICONDATA nid;

  if (!hIcon)
    hIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDR_MAINWND),
                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

  ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 0;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nid.uCallbackMessage = WM_TRAYMESSAGE;
  nid.hIcon = hIcon;
  lstrcpy(nid.szTip, WC_NOTEPAD2);

  if (bAdd)
    Shell_NotifyIcon(NIM_ADD, &nid);
  else
    Shell_NotifyIcon(NIM_DELETE, &nid);
}


//=============================================================================
//
//  SetNotifyIconTitle()
//
//
void SetNotifyIconTitle(HWND hwnd)
{

  NOTIFYICONDATA nid;
  SHFILEINFO shfi;
  WCHAR tchTitle[128];
  WCHAR tchFormat[32];

  ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 0;
  nid.uFlags = NIF_TIP;
  if (lstrlen(szTitleExcerpt))
  {
    GetString(IDS_TITLEEXCERPT, tchFormat, COUNTOF(tchFormat));
    wsprintf(tchTitle, tchFormat, szTitleExcerpt);
  }
  else if (lstrlen(szCurFile))
  {
    SHGetFileInfo2(szCurFile, 0, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME);
    PathCompactPathEx(tchTitle, shfi.szDisplayName, COUNTOF(tchTitle) - 4, 0);
  }
  else
    GetString(IDS_UNTITLED, tchTitle, COUNTOF(tchTitle) - 4);

  if (n2e_IsDocumentModified())
    lstrcpy(nid.szTip, L"* ");
  else
    lstrcpy(nid.szTip, L"");
  lstrcat(nid.szTip, tchTitle);

  Shell_NotifyIcon(NIM_MODIFY, &nid);
}


//=============================================================================
//
//  InstallFileWatching()
//
//
void InstallFileWatching(LPCWSTR lpszFile)
{
  // Terminate
  if (!iFileWatchingMode || !lpszFile || lstrlen(lpszFile) == 0)
  {
    // [2e]: Improve File Change Notification for slow medium #409
    n2e_StopWatchThread();
    return;
  }

  // Install
  else
  {
    // [2e]: Improve File Change Notification for slow medium #409
    n2e_RunWatchThread(lpszFile);
  }
}


//=============================================================================
//
//  PasteBoardTimer()
//
//
void CALLBACK PasteBoardTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  if (dwLastCopyTime > 0 && GetTickCount() - dwLastCopyTime > 200)
  {
    if (SendMessage(hwndEdit, SCI_CANPASTE, 0, 0))
    {
      BOOL bAutoIndent2 = bAutoIndent;
      bAutoIndent = 0;
      EditJumpTo(hwndEdit, -1, 0);
      SendMessage(hwndEdit, SCI_BEGINUNDOACTION, 0, 0);
      if (SendMessage(hwndEdit, SCI_GETLENGTH, 0, 0) > 0)
        SendMessage(hwndEdit, SCI_NEWLINE, 0, 0);
      SendMessage(hwndEdit, SCI_PASTE, 0, 0);
      SendMessage(hwndEdit, SCI_NEWLINE, 0, 0);
      SendMessage(hwndEdit, SCI_ENDUNDOACTION, 0, 0);

      EditEnsureSelectionVisible(hwndEdit);
      bAutoIndent = bAutoIndent2;
    }

    dwLastCopyTime = 0;
  }
}
