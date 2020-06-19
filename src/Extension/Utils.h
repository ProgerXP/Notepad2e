#pragma once
#include <stddef.h>
#include <wtypes.h>
#include <Richedit.h>

typedef enum
{
  CSS_SASSY = 1 << 0,
  CSS_LESS = 1 << 1,
  CSS_HSS = 1 << 2,
} ECSSSettingsMode;

typedef enum
{
  SSM_NO = 0,
  SSM_ALL = 1,
  SSM_RECENT = 2
} ESaveSettingsMode;

typedef enum
{
  UPO_DISABLED = 0,
  UPO_AUTO = 1,
  UPO_ENABLED = 2
} EUsePrefixInOpenDialog;

typedef enum
{
  SLF_DISABLED = 0,
  SLF_ENABLED = 1,
  SLF_ENABLED_UNTIL_NEW_FILE = 2,
} ESaveOnLoseFocus;

typedef enum
{
  PNM_FILENAMEONLY = 0,
  PNM_FILENAMEFIRST = 1,
  PNM_FULLPATH = 2
} EPathNameFormat;

typedef enum 
{
  EEF_IGNORE = 0,
  EEF_MINIMIZE = 1,
  EEF_EXIT = 2
} EEscFunction;

typedef enum
{
  LIT_HIDE = 0,
  LIT_SHOW = 1,
  LIT_SHOW_NON_US = 2
} ELanguageIndicatorMode;

typedef enum
{
  EEM_DISABLED = 0,
  EEM_LINE = 1,
  EEM_SELECTION = 2
} EExpressionEvaluationMode;

typedef enum
{
  WNM_STANDARD = 0,
  WNM_ACCELERATED = 1,
} EWordNavigationMode;

typedef enum
{
  UEM_LEGACY = 0,
  UEM_IMPROVED = 1,
} EUrlEncodeMode;

typedef enum
{
  SCP_LEGACY = 0,
  SCP_THIRD = 1,
  SCP_HALF = 2
} EScrollYCaretPolicy;

typedef enum
{
  HCS_DISABLED = 0,
  HCS_WORD = 1,
  HCS_SELECTION = 2,
  HCS_WORD_AND_SELECTION = 3,
  HCS_WORD_IF_NO_SELECTION = 4
} EHighlightCurrentSelectionMode;

typedef enum
{
  FCP_FIRST_LINE = 0,
  FCP_LAST_LINE = 1,
  FCP_CURRENT_LINE = 2,
  FCP_CURRENT_SELECTION = 3,
  FCP_FIRST_SUBSTRING = 4,
  FCP_LAST_SUBSTRING = 5
} EFavoritesCursorPosition;

typedef struct TAddToFavoritesParams
{
  WCHAR pszName[MAX_PATH];
  WCHAR pszTarget[MAX_PATH];
  WCHAR pszArguments[MAX_PATH];
  WCHAR pszCurrentSelection[MAX_PATH];
  EFavoritesCursorPosition cursorPosition;
} TADDFAVPARAMS, *PTADDFAVPARAMS;

#define N2E_INI_SECTION L"Notepad2e"

#define WM_N2E_RELOAD_SETTINGS (WM_USER + 0xFF)
#define N2E_MAX_PATH_N_CMD_LINE MAX_PATH + 40

extern ECSSSettingsMode iCSSSettings;
extern ELanguageIndicatorMode iShowLanguageInTitle;

void n2e_InitInstance();
void n2e_ExitInstance();

BOOL n2e_InitLPegHomeDir();
#ifdef LPEG_LEXER
BOOL n2e_UseLuaLexer(LPCWSTR lpszExt, LPBOOL pbLexerFileExists);
LPSTR n2e_GetLuaLexerName();
#endif
void n2e_Init();
LPCWSTR n2e_GetLastRun(LPCWSTR lpstrDefault);
void n2e_SetLastRun(LPCWSTR arg);
void n2e_ResetLastRun();
void n2e_ResetSaveOnLoseFocus();
void n2e_LoadINI();
void n2e_SaveINI();
void n2e_Release();
void n2e_Reset();
void n2e_Reload_Settings();
BOOL n2e_CanSaveINISection(const BOOL bCheckSaveSettingsMode, const ESaveSettingsMode modeRequired);
BOOL n2e_IsTextEmpty(LPCWSTR txt);
BOOL n2e_IsRectangularSelection();
BOOL n2e_GetCurrentSelection(LPWSTR buf, const int iCount);
BOOL n2e_OpenMRULast(LPWSTR fn);
void n2e_GetLastDir(LPTSTR out);
LPCWSTR n2e_GetExePath();
UINT_PTR CALLBACK n2e_OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
BOOL n2e_GetGotoNumber(LPTSTR temp, int *out, const BOOL hex);
void n2e_InplaceRev(WCHAR * s);
int n2e_CompareFiles(LPCWSTR sz1, LPCWSTR sz2);
BOOL n2e_OpenFileByPrefix(LPCWSTR pref, LPWSTR dir, LPWSTR out);
BOOL n2e_Grep(void* lpf, const BOOL grep);
void n2e_SetWheelScroll(const BOOL enable);
BOOL n2e_IsWordChar(const WCHAR ch);

#define N2E_IS_LITERAL(CH) n2e_IsWordChar(CH)

BOOL n2e_SetClipboardText(const HWND hwnd, const wchar_t* text);
void n2e_UpdateWindowTitle(const HWND hwnd);
int n2e_GetCurrentShowTitleMenuID();
int n2e_GetCurrentLanguageIndicatorMenuID();
int n2e_GetCurrentSaveSettingsMenuID();
int n2e_GetCurrentSaveOnLoseFocusMenuID();
int n2e_GetCurrentHighlightCurrentSelectionMenuID();
int n2e_GetCurrentEvalMenuID();

extern int iScrollYCaretPolicy;
extern HWND hwndStatus;
extern HWND hwndStatusProgressBar;
extern BOOL bShowProgressBar;
extern WCHAR g_wchWorkingDirectory[MAX_PATH];
extern BOOL bLPegEnabled;
extern WCHAR g_wchLPegHome[MAX_PATH];

void n2e_CreateProgressBarInStatusBar();
void n2e_DestroyProgressBarInStatusBar();
void n2e_ShowProgressBarInStatusBar(LPCWSTR pProgressText, const long nCurPos, const long nMaxPos);
void n2e_HideProgressBarInStatusBar();
void n2e_SetProgressBarPosInStatusBar(const long nCurPos);
void n2e_IncProgressBarPosInStatusBar(const long nOffset);
int n2e_JoinParagraphs_GetSelEnd(const int iSelEnd);
int n2e_JoinLines_GetSelEnd(const int iSelStart, const int iSelEnd, BOOL *pbContinueProcessing);
void n2e_InitAbout3rdPartyText(const HWND hwndRichedit);
void n2e_ProcessAbout3rdPartyUrl(const HWND hwndRichedit, ENLINK* pENLink);
long n2e_GenerateRandom();

void n2e_SetCheckedRadioButton(const HWND hwnd, const int idFirst, const int idLast, const int selectedIndex);
int n2e_GetCheckedRadioButton(const HWND hwnd, const int idFirst, const int idLast);

void n2e_UpdateFavLnkParams(TADDFAVPARAMS* lpParams);
void n2e_EditJumpTo(const HWND hwnd, const int iNewLine, const int iNewCol, const int iNewSelStart, const int iNewSelEnd);

HWND n2e_ToolTipCreate(const HWND hwndParent);
BOOL n2e_ToolTipAddControl(const HWND hwndToolTip, const HWND hwndControl, LPTSTR pszText);
BOOL n2e_ToolTipAddToolInfo(const HWND hwndToolTip, LPVOID lpToolInfo);
BOOL n2e_ToolTipSetToolInfo(const HWND hwndToolTip, LPVOID lpToolInfo);
void n2e_ToolTipTrackPosition(const HWND hwndToolTip, const POINT pt);
void n2e_ToolTipTrackActivate(const HWND hwndToolTip, const BOOL bActivate, LPVOID lpToolInfo);
