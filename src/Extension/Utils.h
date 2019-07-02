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

#define N2E_INI_SECTION L"Notepad2e"

#define WM_N2E_RELOAD_SETTINGS (WM_USER + 0xFF)
#define N2E_MAX_PATH_N_CMD_LINE MAX_PATH + 40

extern ECSSSettingsMode iCSSSettings;
extern ELanguageIndicatorMode iShowLanguageInTitle;

void n2e_InitInstance();
void n2e_ExitInstance();

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
BOOL n2e_OpenMRULast(LPWSTR fn);
void n2e_GetLastDir(LPTSTR out);
UINT_PTR CALLBACK n2e_OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
BOOL n2e_GetGotoNumber(LPTSTR temp, int *out, const BOOL hex);
void n2e_InplaceRev(WCHAR * s);
int n2e_CompareFiles(LPCWSTR sz1, LPCWSTR sz2);
BOOL n2e_OpenFileByPrefix(LPCWSTR pref, LPWSTR dir, LPWSTR out);
void n2e_Grep(void* lpf, const BOOL grep);
void n2e_SetWheelScroll(const BOOL enable);
BOOL n2e_IsWordChar(const WCHAR ch);

#define N2E_IS_LITERAL(CH) n2e_IsWordChar(CH)

BOOL n2e_SetClipboardText(const HWND hwnd, const wchar_t* text);
void n2e_UpdateWindowTitle(const HWND hwnd);
int n2e_GetCurrentShowTitleMenuID();
int n2e_GetCurrentLanguageIndicatorMenuID();

extern int iScrollYCaretPolicy;
extern HWND hwndStatus;
extern HWND hwndStatusProgressBar;
extern BOOL bShowProgressBar;

void n2e_CreateProgressBarInStatusBar();
void n2e_DestroyProgressBarInStatusBar();
void n2e_ShowProgressBarInStatusBar(LPCWSTR pProgressText, const long nCurPos, const long nMaxPos);
void n2e_HideProgressBarInStatusBar();
void n2e_SetProgressBarPosInStatusBar(const long nCurPos);
void n2e_IncProgressBarPosInStatusBar(const long nOffset);
void n2e_ProcessPendingMessages();
int n2e_JoinParagraphs_GetSelEnd(const int iSelEnd);
int n2e_JoinLines_GetSelEnd(const int iSelStart, const int iSelEnd, BOOL *pbContinueProcessing);
void n2e_InitAbout3rdPartyText(const HWND hwndRichedit);
void n2e_ProcessAbout3rdPartyUrl(const HWND hwndRichedit, ENLINK* pENLink);
long n2e_GenerateRandom();
