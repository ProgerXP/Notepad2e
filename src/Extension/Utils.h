#pragma once
#include "stdafx.h"

extern HANDLE g_hScintilla;
extern WCHAR szCurFile[MAX_PATH + 40];

enum CSS_PROP
{
  css_prop_sassy = 1 << 0,
  css_prop_less = 1 << 1,
  css_prop_hss = 1 << 2,
};

extern UINT	_n2e_css_property;

typedef enum
{
  ELI_HIDE = 0,
  ELI_SHOW,
  ELI_SHOW_NON_US
} ELanguageIndicatorMode;

#define N2E_INI_SECTION L"Notepad2e"
#define INI_SETTING_HIGHLIGHT_LINE_IF_WINDOW_INACTIVE L"HighlightLineIfWindowInactive"
#define INI_SETTING_SCROLL_Y_CARET_POLICY L"ScrollYCaretPolicy"
#define INI_SETTING_FIND_WORD_MATCH_CASE  L"FindWordMatchCase"
#define INI_SETTING_FIND_WRAP_AROUND  L"FindWordWrapAround"
#define INI_SETTING_MOVE_CARET_ON_RIGHT_CLICK L"MoveCaretOnRightClick"
#define INI_SETTING_MATH_EVAL L"MathEval"
#define INI_SETTING_LANGUAGE_INDICATOR L"TitleLanguage"
#define INI_SETTING_WORD_NAVIGATION_MODE  L"WordNavigationMode"

#define HWM_RELOAD_SETTINGS	(WM_USER + 0xee)

VOID N2E_Trace(const char *fmt, ...);
VOID N2E_WTrace(const char *fmt, LPCWSTR word);
VOID N2E_WTrace2(const char *fmt, LPCWSTR word1, LPCWSTR word2);

#ifdef _DEBUG
#define __FILE_LOC (1+strrchr(__FILE__,'\\')/*?strrchr(__FILE__,'\\')+1:__FILE__*/)
#define N2E_TRACE(FMT,...)	N2E_Trace ( "[%s: %d] - "#FMT , __FILE_LOC , __LINE__ , __VA_ARGS__ );
#define N2E_TRACE_S(OBJ)		N2E_Trace ( "[%s: %d] [%s]=%s " , __FILE_LOC , __LINE__ , #OBJ , OBJ );
#define N2E_TRACE_I(OBJ)		N2E_Trace ( "[%s: %d] [%s]=%d (0x%04xd) " , __FILE_LOC , __LINE__ , #OBJ , OBJ , OBJ );
#define N2E_TRACE_TR(OBJ)		N2E_Trace ( "[%s: %d] [%s]= TEXTRANGE %d:%d(%s) " , __FILE_LOC , __LINE__ , #OBJ , OBJ.chrg.cpMin ,OBJ.chrg.cpMax ,OBJ.lpstrText );
#else
#define N2E_TRACE(FMT,...)	 (void)(FMT);
#define N2E_TRACE_S(OBJ)		(void)(OBJ);
#define N2E_TRACE_I(OBJ)		(void)(OBJ);
#define N2E_TRACE_TR(OBJ)		(void)(OBJ);
#endif

#define	N2E_MAX_PATH_N_CMD_LINE	MAX_PATH + 40
WCHAR	_n2e_last_run[N2E_MAX_PATH_N_CMD_LINE];
extern ELanguageIndicatorMode iShowLanguageInTitle;

void n2e_InitInstance();
void n2e_ExitInstance();

void* n2e_Alloc(size_t size);
void n2e_Free(void* ptr);
void* n2e_Realloc(void* ptr, size_t len);
VOID n2e_Init(HWND hWnd);
VOID n2e_LoadINI();
VOID n2e_SaveINI();
VOID n2e_Release();
VOID n2e_Reload_Settings();
BOOL n2e_Is_Empty(LPCWSTR txt);
BOOL n2e_OpenMRULast(LPWSTR fn);
VOID n2e_GetLastDir(LPTSTR out);
UINT_PTR CALLBACK n2e_OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
BOOL n2e_GetGotoNumber(LPTSTR temp, int *out, BOOL hex);
void n2e_InplaceRev(WCHAR * s);
int n2e_CompareFiles(LPCWSTR sz1, LPCWSTR sz2);
VOID	n2e_Grep(VOID* lpf, BOOL grep);
VOID n2e_SetWheelScroll(BOOL on);
BOOL n2e_IsWordChar(WCHAR ch);
BOOL n2e_IsSpace(WCHAR ch);
BOOL n2e_IsKeyDown(int key);

#define _N2E_COMPARE_FILES( F1 , F2 )  (n2e_CompareFiles(F1,F2))
#define N2E_IS_LITERAL(CH) n2e_IsWordChar(CH)
#define N2E_IS_SPACE(CH) n2e_IsSpace(CH)

BOOL n2e_SetClipboardText(const HWND hwnd, const wchar_t* text);
void n2e_SaveWindowTitleParams(UINT uIDAppName, BOOL bIsElevated, UINT uIDUntitled,
                           LPCWSTR lpszFile, int iFormat, BOOL bModified,
                           UINT uIDReadOnly, BOOL bReadOnly, LPCWSTR lpszExcerpt);
void n2e_UpdateWindowTitle(HWND hwnd);

extern int iScrollYCaretPolicy;
extern HWND hwndStatus;
extern HWND hwndStatusProgressBar;
extern BOOL bShowProgressBar;
WCHAR tchProgressBarTaskName[MAX_PATH];

void n2e_CreateProgressBarInStatusBar();
void n2e_DestroyProgressBarInStatusBar();
void n2e_ShowProgressBarInStatusBar(LPCWSTR pProgressText, const long nCurPos, const long nMaxPos);
void n2e_HideProgressBarInStatusBar();
void n2e_UpdateProgressBarInStatusBar(const long nCurPos);
void n2e_AdjustProgressBarInStatusBar(const long nCurPos, const long nMaxPos);
