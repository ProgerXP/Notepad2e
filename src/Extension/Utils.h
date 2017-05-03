#pragma once
#include "stdafx.h"

extern HANDLE g_hScintilla;

enum CSS_PROP
{
  css_prop_sassy = 1 << 0,
  css_prop_less = 1 << 1,
  css_prop_hss = 1 << 2,
};

extern UINT	_hl_css_property;

typedef enum
{
  ELI_HIDE = 0,
  ELI_SHOW,
  ELI_SHOW_NON_US
} ELanguageIndicatorMode;

#define HL_INI_SECTION L"Notepad2e"
#define INI_SETTING_HIGHLIGHT_LINE_IF_WINDOW_INACTIVE L"HighlightLineIfWindowInactive"
#define INI_SETTING_SCROLL_Y_CARET_POLICY L"ScrollYCaretPolicy"
#define INI_SETTING_FIND_WORD_MATCH_CASE  L"FindWordMatchCase"
#define INI_SETTING_FIND_WRAP_AROUND  L"FindWordWrapAround"
#define INI_SETTING_MOVE_CARET_ON_RIGHT_CLICK L"MoveCaretOnRightClick"
#define INI_SETTING_MATH_EVAL L"MathEval"
#define INI_SETTING_LANGUAGE_INDICATOR L"TitleLanguage"
#define INI_SETTING_WORD_NAVIGATION_MODE  L"WordNavigationMode"

#define HWM_RELOAD_SETTINGS	(WM_USER + 0xee)

VOID HL_Trace(const char *fmt, ...);
VOID HL_WTrace(const char *fmt, LPCWSTR word);
VOID HL_WTrace2(const char *fmt, LPCWSTR word1, LPCWSTR word2);

#ifdef _DEBUG
#define __FILE_LOC (1+strrchr(__FILE__,'\\')/*?strrchr(__FILE__,'\\')+1:__FILE__*/)
#define HL_TRACE(FMT,...)	HL_Trace ( "[%s: %d] - "#FMT , __FILE_LOC , __LINE__ , __VA_ARGS__ );
#define HL_TRACE_S(OBJ)		HL_Trace ( "[%s: %d] [%s]=%s " , __FILE_LOC , __LINE__ , #OBJ , OBJ );
#define HL_TRACE_I(OBJ)		HL_Trace ( "[%s: %d] [%s]=%d (0x%04xd) " , __FILE_LOC , __LINE__ , #OBJ , OBJ , OBJ );
#define HL_TRACE_TR(OBJ)		HL_Trace ( "[%s: %d] [%s]= TEXTRANGE %d:%d(%s) " , __FILE_LOC , __LINE__ , #OBJ , OBJ.chrg.cpMin ,OBJ.chrg.cpMax ,OBJ.lpstrText );
#else
#define HL_TRACE(FMT,...)	 (void)(FMT);
#define HL_TRACE_S(OBJ)		(void)(OBJ);
#define HL_TRACE_I(OBJ)		(void)(OBJ);
#define HL_TRACE_TR(OBJ)		(void)(OBJ);
#endif

#define	HL_MAX_PATH_N_CMD_LINE	MAX_PATH + 40
WCHAR	_hl_last_run[HL_MAX_PATH_N_CMD_LINE];
extern ELanguageIndicatorMode iShowLanguageInTitle;

void n2e_InitInstance();
void n2e_ExitInstance();

void* HL_Alloc(size_t size);
void HL_Free(void* ptr);
void* HL_Realloc(void* ptr, size_t len);
VOID HL_Init(HWND hWnd);
VOID HL_LoadINI();
VOID HL_SaveINI();
VOID HL_Release();
VOID HL_Reload_Settings();
BOOL HL_Is_Empty(LPCWSTR txt);
VOID HL_Get_last_dir(LPTSTR out);
UINT_PTR CALLBACK HL_OFN__hook_proc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
BOOL HL_Get_goto_number(LPTSTR temp, int *out, BOOL hex);
void HL_inplace_rev(WCHAR * s);
int HL_Compare_files(LPCWSTR sz1, LPCWSTR sz2);
VOID	HL_Grep(VOID* lpf, BOOL grep);
VOID HL_Set_wheel_scroll(BOOL on);
BOOL hl_iswordchar(WCHAR ch);
BOOL hl_isspace(WCHAR ch);

#define _HL_COMPARE_FILES( F1 , F2 )  (HL_Compare_files(F1,F2))
#define HL_IS_LITERAL(CH) hl_iswordchar(CH)
#define HL_IS_SPACE(CH) hl_isspace(CH)

BOOL SetClipboardText(const HWND hwnd, const wchar_t* text);
void SaveWindowTitleParams(UINT uIDAppName, BOOL bIsElevated, UINT uIDUntitled,
                           LPCWSTR lpszFile, int iFormat, BOOL bModified,
                           UINT uIDReadOnly, BOOL bReadOnly, LPCWSTR lpszExcerpt);
void UpdateWindowTitle(HWND hwnd);

extern int iScrollYCaretPolicy;
extern HWND hwndStatus;
extern HWND hwndStatusProgressBar;
extern BOOL bShowProgressBar;
WCHAR tchProgressBarTaskName[MAX_PATH];

void CreateProgressBarInStatusBar();
void DestroyProgressBarInStatusBar();
void ShowProgressBarInStatusBar(LPCWSTR pProgressText, const long nCurPos, const long nMaxPos);
void HideProgressBarInStatusBar();
void UpdateProgressBarInStatusBar(const long nCurPos);
void AdjustProgressBarInStatusBar(const long nCurPos, const long nMaxPos);
