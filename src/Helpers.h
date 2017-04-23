#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************
*
*
* Notepad2
*
* Helpers.h
*   Definitions for general helper functions and macros
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

extern HINSTANCE g_hInstance;
extern UINT16 g_uWinVer;

#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))
#define CSTRLEN(s)  (COUNTOF(s)-1)
#define STRING2(x) #x
#define STRING(x) STRING2(x)
#define CT_WARNING(M) message("\n\t** " M " => [ "__FILE__ ":" STRING(__LINE__) " ]  **\n")
enum CSS_PROP
{
  css_prop_sassy = 1 << 0,
  css_prop_less = 1 << 1,
  css_prop_hss = 1 << 2,
};

extern WCHAR szIniFile[MAX_PATH];
#define IniGetString(lpSection,lpName,lpDefault,lpReturnedStr,nSize) \
    GetPrivateProfileString(lpSection,lpName,lpDefault,lpReturnedStr,nSize,szIniFile)
#define IniGetInt(lpSection,lpName,nDefault) \
    GetPrivateProfileInt(lpSection,lpName,nDefault,szIniFile)
#define IniSetString(lpSection,lpName,lpString) \
    WritePrivateProfileString(lpSection,lpName,lpString,szIniFile)
#define IniDeleteSection(lpSection) \
    WritePrivateProfileSection(lpSection,NULL,szIniFile)
__inline BOOL IniSetInt(LPCWSTR lpSection, LPCWSTR lpName, int i)
{
  WCHAR tch[32];
  wsprintf(tch, L"%i", i);
  return IniSetString(lpSection, lpName, tch);
}
#define LoadIniSection(lpSection,lpBuf,cchBuf) \
    GetPrivateProfileSection(lpSection,lpBuf,cchBuf,szIniFile);
#define SaveIniSection(lpSection,lpBuf) \
    WritePrivateProfileSection(lpSection,lpBuf,szIniFile)
int IniSectionGetString(LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, int);
int IniSectionGetInt(LPCWSTR, LPCWSTR, int);
BOOL IniSectionSetString(LPWSTR, LPCWSTR, LPCWSTR);
__inline BOOL IniSectionSetInt(LPWSTR lpCachedIniSection, LPCWSTR lpName, int i)
{
  WCHAR tch[32];
  wsprintf(tch, L"%i", i);
  return IniSectionSetString(lpCachedIniSection, lpName, tch);
}

extern HWND hwndEdit;

__inline void BeginWaitCursor()
{
  SendMessage(hwndEdit, SCI_SETCURSOR, (WPARAM)SC_CURSORWAIT, 0);
}
__inline void EndWaitCursor()
{
  POINT pt;
  SendMessage(hwndEdit, SCI_SETCURSOR, (WPARAM)SC_CURSORNORMAL, 0);
  GetCursorPos(&pt);
  SetCursorPos(pt.x, pt.y);
}

#define Is2k()    (g_uWinVer >= 0x0500)
#define IsXP()    (g_uWinVer >= 0x0501)
#define IsVista() (g_uWinVer >= 0x0600)
#define IsW7()    (g_uWinVer >= 0x0601)

#define	HL_MAX_PATH_N_CMD_LINE	MAX_PATH + 40

BOOL PrivateIsAppThemed();
HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR);
BOOL IsElevated();
BOOL BitmapMergeAlpha(HBITMAP, COLORREF);
BOOL BitmapAlphaBlend(HBITMAP, COLORREF, BYTE);
BOOL BitmapGrayScale(HBITMAP);
BOOL VerifyContrast(COLORREF, COLORREF);
BOOL IsFontAvailable(LPCWSTR);

BOOL SetWindowTitle(HWND, UINT, BOOL, UINT, LPCWSTR, int, BOOL, UINT, BOOL, LPCWSTR);
void SetWindowTransparentMode(HWND, BOOL);

void CenterDlgInParent(HWND);
void GetDlgPos(HWND, LPINT, LPINT);
void SetDlgPos(HWND, int, int);
void ResizeDlg_Init(HWND, int, int, int);
void ResizeDlg_Destroy(HWND, int *, int *);
void ResizeDlg_Size(HWND, LPARAM, int *, int *);
void ResizeDlg_GetMinMaxInfo(HWND, LPARAM);
HDWP DeferCtlPos(HDWP, HWND, int, int, int, UINT);
void MakeBitmapButton(HWND, int, HINSTANCE, UINT);
void MakeColorPickButton(HWND, int, HINSTANCE, COLORREF);
void DeleteBitmapButton(HWND, int);

#define StatusSetSimple(hwnd,b) SendMessage(hwnd,SB_SIMPLE,(WPARAM)b,0)
BOOL StatusSetText(HWND, UINT, LPCWSTR);
BOOL StatusSetTextID(HWND, UINT, UINT);
int  StatusCalcPaneWidth(HWND, LPCWSTR);

int Toolbar_GetButtons(HWND, int, LPWSTR, int);
int Toolbar_SetButtons(HWND, int, LPCWSTR, void *, int);

LRESULT SendWMSize(HWND);

#define EnableCmd(hmenu,id,b) EnableMenuItem(hmenu,id,(b)\
        ?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED)

#define CheckCmd(hmenu,id,b)  CheckMenuItem(hmenu,id,(b)\
        ?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED)

BOOL IsCmdEnabled(HWND, UINT);

#define GetString(id,pb,cb) LoadString(g_hInstance,id,pb,cb)

#define StrEnd(pStart) (pStart + lstrlen(pStart))

int FormatString(LPWSTR, int, UINT, ...);

void PathRelativeToApp(LPWSTR, LPWSTR, int, BOOL, BOOL, BOOL);
void PathAbsoluteFromApp(LPWSTR, LPWSTR, int, BOOL);

BOOL PathIsLnkFile(LPCWSTR);
BOOL PathGetLnkPath(LPCWSTR, LPWSTR, int);
BOOL PathIsLnkToDirectory(LPCWSTR, LPWSTR, int);
BOOL PathCreateDeskLnk(LPCWSTR);
BOOL PathCreateFavLnk(LPCWSTR, LPCWSTR, LPCWSTR);

BOOL StrLTrim(LPWSTR, LPCWSTR);
BOOL TrimString(LPWSTR);
BOOL ExtractFirstArgument(LPCWSTR, LPWSTR, LPWSTR);

void PrepareFilterStr(LPWSTR);

void StrTab2Space(LPWSTR);
void PathFixBackslashes(LPWSTR);

void  ExpandEnvironmentStringsEx(LPWSTR, DWORD);
void  PathCanonicalizeEx(LPWSTR);
DWORD GetLongPathNameEx(LPWSTR, DWORD);
DWORD_PTR SHGetFileInfo2(LPCWSTR, DWORD, SHFILEINFO *, UINT, UINT);

int  FormatNumberStr(LPWSTR);
BOOL SetDlgItemIntEx(HWND, int, UINT);

#define MBCSToWChar(c,a,w,i) MultiByteToWideChar(c,0,a,-1,w,i)
#define WCharToMBCS(c,w,a,i) WideCharToMultiByte(c,0,w,-1,a,i,NULL,NULL)

UINT    GetDlgItemTextA2W(UINT, HWND, int, LPSTR, int);
UINT    SetDlgItemTextA2W(UINT, HWND, int, LPSTR);
LRESULT ComboBox_AddStringA2W(UINT, HWND, LPCSTR);

UINT CodePageFromCharSet(UINT);

//==== MRU Functions ==========================================================
#define MRU_MAXITEMS 24
#define MRU_NOCASE    1
#define MRU_UTF8      2

typedef struct _mrulist
{
  WCHAR  szRegKey[256];
  int   iFlags;
  int   iSize;
  LPWSTR pszItems[MRU_MAXITEMS];
} MRULIST, *PMRULIST, *LPMRULIST;

LPMRULIST MRU_Create(LPCWSTR, int, int);
BOOL      MRU_Destroy(LPMRULIST);
BOOL      MRU_Add(LPMRULIST, LPCWSTR);
BOOL      MRU_AddFile(LPMRULIST, LPCWSTR, BOOL, BOOL);
BOOL      MRU_Delete(LPMRULIST, int);
BOOL      MRU_DeleteFileFromStore(LPMRULIST, LPCWSTR);
BOOL      MRU_Empty(LPMRULIST);
int       MRU_Enum(LPMRULIST, int, LPWSTR, int);
BOOL      MRU_Load(LPMRULIST);
BOOL      MRU_Save(LPMRULIST);
BOOL      MRU_MergeSave(LPMRULIST, BOOL, BOOL, BOOL);

//==== Themed Dialogs =========================================================
#ifndef DLGTEMPLATEEX
#pragma pack(push, 1)
typedef struct
{
  WORD      dlgVer;
  WORD      signature;
  DWORD     helpID;
  DWORD     exStyle;
  DWORD     style;
  WORD      cDlgItems;
  short     x;
  short     y;
  short     cx;
  short     cy;
} DLGTEMPLATEEX;
#pragma pack(pop)
#endif

BOOL GetThemedDialogFont(LPWSTR, WORD *);
DLGTEMPLATE *LoadThemedDialogTemplate(LPCTSTR, HINSTANCE);
#define ThemedDialogBox(hInstance,lpTemplate,hWndParent,lpDialogFunc) \
    ThemedDialogBoxParam(hInstance,lpTemplate,hWndParent,lpDialogFunc,0)
INT_PTR ThemedDialogBoxParam(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);
HWND    CreateThemedDialogParam(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);

//==== UnSlash Functions ======================================================
void TransformBackslashes(char *, BOOL, UINT);

//==== MinimizeToTray Functions - see comments in Helpers.c ===================
BOOL GetDoAnimateMinimize(VOID);
VOID MinimizeWndToTray(HWND hWnd);
VOID RestoreWndFromTray(HWND hWnd);

//============== haccel work
VOID	HL_Init(HWND hwnd);
VOID	HL_LoadINI();
VOID	HL_SaveINI();
VOID	HL_Release();
VOID	HL_Trace(const char *fmt, ...);
VOID	HL_WTrace(const char *fmt, LPCWSTR word);
VOID	HL_WTrace2(const char *fmt, LPCWSTR word1, LPCWSTR word2);
BOOL	HL_Is_Empty(LPCWSTR txt);
BOOL	HL_Get_goto_number(LPTSTR txt, int *out, BOOL hex);
VOID	HL_Set_wheel_scroll(BOOL on);
VOID	HL_Reload_Settings();
VOID	HL_Get_last_dir(LPTSTR out);
void	HL_inplace_rev(WCHAR * s);
BOOL	HL_Explorer_cxt_menu(LPCWSTR path, void * parentWindow);
UINT_PTR CALLBACK HL_OFN__hook_proc(
  HWND hdlg,
  UINT uiMsg,
  WPARAM wParam,
  LPARAM lParam
  );

VOID	HL_Grep(VOID* lpf, BOOL grep);

void*	HL_Alloc(size_t);
void*	HL_Realloc(void*, size_t);
void	HL_Free(void*);
int		HL_Compare_files(LPCWSTR sz1, LPCWSTR sz2);
BOOL	hl_iswordchar(WCHAR ch);
BOOL	hl_isspace(WCHAR ch);

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

BOOL SetClipboardText(const HWND hwnd, const wchar_t* text);
BOOL ASCIItoUCS2(const char* lpSrc, wchar_t* lpDest, const int maxDest, const UINT nCodePage);

typedef enum
{
  ELI_HIDE = 0,
  ELI_SHOW,
  ELI_SHOW_NON_US
} ELanguageIndicatorMode;

void UpdateWindowTitle(HWND hwnd);
#ifdef __cplusplus
}//end extern "C"
#endif
///   End of Helpers.h   \\\
