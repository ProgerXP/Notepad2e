/******************************************************************************
*
*
* Notepad2
*
* Edit.c
*   Text File Editing Helper Stuff
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
#include <shlwapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "scintilla.h"
#include "scilexer.h"
#include "notepad2.h"
#include "edit.h"
#include "styles.h"
#include "dialogs.h"
#include "helpers.h"
#include "resource.h"
#include "Extension/EditHelper.h"
#include "Extension/EditHelperEx.h"
#include "Extension/ExtSelection.h"
#include "Extension/DPIHelper.h"
#include "Extension/SciCall.h"
#include "Extension/ProcessElevationUtils.h"
#include "Extension/Utils.h"


extern HWND  hwndMain;
extern HWND  hwndEdit;
extern HINSTANCE g_hInstance;
extern LPMALLOC  g_lpMalloc;
extern DWORD dwLastIOError;
extern HWND hDlgFindReplace;
extern HWND hDlgGotoLine;
extern UINT cpLastFind;
extern BOOL bReplaceInitialized;

static EDITFINDREPLACE efrSave;
static BOOL bSwitchedFindReplace = FALSE;
static int xFindReplaceDlgSave;
static int yFindReplaceDlgSave;
extern int xFindReplaceDlg;
extern int yFindReplaceDlg;

extern int iDefaultEncoding;
extern int iDefaultEOLMode;
extern int iLineEndings[3];
extern BOOL bFixLineEndings;
extern BOOL bAutoStripBlanks;


// Default Codepage and Character Set
extern int iDefaultCodePage;
extern int iDefaultCharSet;
extern BOOL bSkipUnicodeDetection;
extern BOOL bLoadASCIIasUTF8;
extern int iSrcEncoding;
extern int iWeakSrcEncoding;

extern int iEncoding;

// Supported Encodings
WCHAR wchANSI[8] = L"";
WCHAR wchOEM[8] = L"";

NP2ENCODING mEncoding[] = {
  { NCP_DEFAULT | NCP_RECODE, 0, "ansi,ansi,ascii,", 61000, L"" },
  { NCP_8BIT | NCP_RECODE, 0, "oem,oem,", 61001, L"" },
  { NCP_UNICODE | NCP_UNICODE_BOM, 0, "", 61002, L"" },
  { NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_UNICODE_BOM, 0, "", 61003, L"" },
  { NCP_UNICODE | NCP_RECODE, 0, "utf-16,utf16,unicode,", 61004, L"" },
  { NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_RECODE, 0, "utf-16be,utf16be,unicodebe,", 61005, L"" },
  { NCP_UTF8 | NCP_RECODE, 0, "utf-8,utf8,", 61006, L"" },
  { NCP_UTF8 | NCP_UTF8_SIGN, 0, "utf-8,utf8,", 61007, L"" },
  { NCP_8BIT | NCP_RECODE, CP_UTF7, "utf-7,utf7,", 61008, L"" },
  { NCP_8BIT | NCP_RECODE, 720, "DOS-720,dos720,", 61009, L"" },
  { NCP_8BIT | NCP_RECODE, 28596, "iso-8859-6,iso88596,arabic,csisolatinarabic,ecma114,isoir127,", 61010, L"" },
  { NCP_8BIT | NCP_RECODE, 10004, "x-mac-arabic,xmacarabic,", 61011, L"" },
  { NCP_8BIT | NCP_RECODE, 1256, "windows-1256,windows1256,cp1256", 61012, L"" },
  { NCP_8BIT | NCP_RECODE, 775, "ibm775,ibm775,cp500,", 61013, L"" },
  { NCP_8BIT | NCP_RECODE, 28594, "iso-8859-4,iso88594,csisolatin4,isoir110,l4,latin4,", 61014, L"" },
  { NCP_8BIT | NCP_RECODE, 1257, "windows-1257,windows1257,", 61015, L"" },
  { NCP_8BIT | NCP_RECODE, 852, "ibm852,ibm852,cp852,", 61016, L"" },
  { NCP_8BIT | NCP_RECODE, 28592, "iso-8859-2,iso88592,csisolatin2,isoir101,latin2,l2,", 61017, L"" },
  { NCP_8BIT | NCP_RECODE, 10029, "x-mac-ce,xmacce,", 61018, L"" },
  { NCP_8BIT | NCP_RECODE, 1250, "windows-1250,windows1250,xcp1250,", 61019, L"" },
  { NCP_8BIT | NCP_RECODE, 936, "gb2312,gb2312,chinese,cngb,csgb2312,csgb231280,gb231280,gbk,", 61020, L"" },
  { NCP_8BIT | NCP_RECODE, 10008, "x-mac-chinesesimp,xmacchinesesimp,", 61021, L"" },
  { NCP_8BIT | NCP_RECODE, 950, "big5,big5,cnbig5,csbig5,xxbig5,", 61022, L"" },
  { NCP_8BIT | NCP_RECODE, 10002, "x-mac-chinesetrad,xmacchinesetrad,", 61023, L"" },
  { NCP_8BIT | NCP_RECODE, 10082, "x-mac-croatian,xmaccroatian,", 61024, L"" },
  { NCP_8BIT | NCP_RECODE, 866, "cp866,cp866,ibm866,", 61025, L"" },
  { NCP_8BIT | NCP_RECODE, 28595, "iso-8859-5,iso88595,csisolatin5,csisolatincyrillic,cyrillic,isoir144,", 61026, L"" },
  { NCP_8BIT | NCP_RECODE, 20866, "koi8-r,koi8r,cskoi8r,koi,koi8,", 61027, L"" },
  { NCP_8BIT | NCP_RECODE, 21866, "koi8-u,koi8u,koi8ru,", 61028, L"" },
  { NCP_8BIT | NCP_RECODE, 10007, "x-mac-cyrillic,xmaccyrillic,", 61029, L"" },
  { NCP_8BIT | NCP_RECODE, 1251, "windows-1251,windows1251,xcp1251,", 61030, L"" },
  { NCP_8BIT | NCP_RECODE, 28603, "iso-8859-13,iso885913,", 61031, L"" },
  { NCP_8BIT | NCP_RECODE, 863, "ibm863,ibm863,", 61032, L"" },
  { NCP_8BIT | NCP_RECODE, 737, "ibm737,ibm737,", 61033, L"" },
  { NCP_8BIT | NCP_RECODE, 28597, "iso-8859-7,iso88597,csisolatingreek,ecma118,elot928,greek,greek8,isoir126,", 61034, L"" },
  { NCP_8BIT | NCP_RECODE, 10006, "x-mac-greek,xmacgreek,", 61035, L"" },
  { NCP_8BIT | NCP_RECODE, 1253, "windows-1253,windows1253,", 61036, L"" },
  { NCP_8BIT | NCP_RECODE, 869, "ibm869,ibm869,", 61037, L"" },
  { NCP_8BIT | NCP_RECODE, 862, "DOS-862,dos862,", 61038, L"" },
  { NCP_8BIT | NCP_RECODE, 38598, "iso-8859-8-i,iso88598i,logical,", 61039, L"" },
  { NCP_8BIT | NCP_RECODE, 28598, "iso-8859-8,iso88598,csisolatinhebrew,hebrew,isoir138,visual,", 61040, L"" },
  { NCP_8BIT | NCP_RECODE, 10005, "x-mac-hebrew,xmachebrew,", 61041, L"" },
  { NCP_8BIT | NCP_RECODE, 1255, "windows-1255,windows1255,", 61042, L"" },
  { NCP_8BIT | NCP_RECODE, 861, "ibm861,ibm861,", 61043, L"" },
  { NCP_8BIT | NCP_RECODE, 10079, "x-mac-icelandic,xmacicelandic,", 61044, L"" },
  { NCP_8BIT | NCP_RECODE, 10001, "x-mac-japanese,xmacjapanese,", 61045, L"" },
  { NCP_8BIT | NCP_RECODE, 932, "shift_jis,shiftjis,shiftjs,csshiftjis,cswindows31j,mskanji,xmscp932,xsjis,", 61046, L"" },
  { NCP_8BIT | NCP_RECODE, 10003, "x-mac-korean,xmackorean,", 61047, L"" },
  { NCP_8BIT | NCP_RECODE, 949, "windows-949,windows949,ksc56011987,csksc5601,euckr,isoir149,korean,ksc56011989", 61048, L"" },
  { NCP_8BIT | NCP_RECODE, 28593, "iso-8859-3,iso88593,latin3,isoir109,l3,", 61049, L"" },
  { NCP_8BIT | NCP_RECODE, 28605, "iso-8859-15,iso885915,latin9,l9,", 61050, L"" },
  { NCP_8BIT | NCP_RECODE, 865, "ibm865,ibm865,", 61051, L"" },
  { NCP_8BIT | NCP_RECODE, 437, "ibm437,ibm437,437,cp437,cspc8,codepage437,", 61052, L"" },
  { NCP_8BIT | NCP_RECODE, 858, "ibm858,ibm858,ibm00858,", 61053, L"" },
  { NCP_8BIT | NCP_RECODE, 860, "ibm860,ibm860,", 61054, L"" },
  { NCP_8BIT | NCP_RECODE, 10010, "x-mac-romanian,xmacromanian,", 61055, L"" },
  { NCP_8BIT | NCP_RECODE, 10021, "x-mac-thai,xmacthai,", 61056, L"" },
  { NCP_8BIT | NCP_RECODE, 874, "windows-874,windows874,dos874,iso885911,tis620,", 61057, L"" },
  { NCP_8BIT | NCP_RECODE, 857, "ibm857,ibm857,", 61058, L"" },
  { NCP_8BIT | NCP_RECODE, 28599, "iso-8859-9,iso88599,latin5,isoir148,l5,", 61059, L"" },
  { NCP_8BIT | NCP_RECODE, 10081, "x-mac-turkish,xmacturkish,", 61060, L"" },
  { NCP_8BIT | NCP_RECODE, 1254, "windows-1254,windows1254,", 61061, L"" },
  { NCP_8BIT | NCP_RECODE, 10017, "x-mac-ukrainian,xmacukrainian,", 61062, L"" },
  { NCP_8BIT | NCP_RECODE, 1258, "windows-1258,windows-258,", 61063, L"" },
  { NCP_8BIT | NCP_RECODE, 850, "ibm850,ibm850,", 61064, L"" },
  { NCP_8BIT | NCP_RECODE, 28591, "iso-8859-1,iso88591,cp819,latin1,ibm819,isoir100,latin1,l1,", 61065, L"" },
  { NCP_8BIT | NCP_RECODE, 10000, "macintosh,macintosh,", 61066, L"" },
  { NCP_8BIT | NCP_RECODE, 1252, "windows-1252,windows1252,cp367,cp819,ibm367,us,xansi,", 61067, L"" },
  { NCP_8BIT | NCP_RECODE, 37, "ebcdic-cp-us,ebcdiccpus,ebcdiccpca,ebcdiccpwt,ebcdiccpnl,ibm037,cp037,", 61068, L"" },
  { NCP_8BIT | NCP_RECODE, 500, "x-ebcdic-international,xebcdicinternational,", 61069, L"" },
  { NCP_8BIT | NCP_RECODE, 875, "x-EBCDIC-GreekModern,xebcdicgreekmodern,", 61070, L"" },
  { NCP_8BIT | NCP_RECODE, 1026, "CP1026,cp1026,csibm1026,ibm1026,", 61071, L"" },
  { NCP_8BIT | NCP_RECODE, 54936, "gb18030,gb18030,", 61072, L"" },
};


extern LPMRULIST mruFind;
extern LPMRULIST mruReplace;


//=============================================================================
//
//  EditCreate()
//
HWND EditCreate(HWND hwndParent)
{

  HWND hwnd;

  hwnd = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    L"Scintilla",
    NULL,
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
    0, 0, 0, 0,
    hwndParent,
    (HMENU)IDC_EDIT,
    g_hInstance,
    NULL);

  SendMessage(hwnd, SCI_SETCODEPAGE, iDefaultCodePage, 0);
  SendMessage(hwnd, SCI_SETEOLMODE, SC_EOL_CRLF, 0);
  SendMessage(hwnd, SCI_SETPASTECONVERTENDINGS, 1, 0);
  SendMessage(hwnd, SCI_SETMODEVENTMASK, n2e_SelectionGetSciEventMask(TRUE), 0);
  SendMessage(hwnd, SCI_USEPOPUP, FALSE, 0);
  SendMessage(hwnd, SCI_SETSCROLLWIDTH, 2048, 0);
  SendMessage(hwnd, SCI_SETSCROLLWIDTHTRACKING, TRUE, 0);
  SendMessage(hwnd, SCI_SETENDATLASTLINE, TRUE, 0);
  SendMessage(hwnd, SCI_SETCARETSTICKY, FALSE, 0);
  SendMessage(hwnd, SCI_SETXCARETPOLICY, CARET_SLOP | CARET_EVEN, 50);
  SendMessage(hwnd, SCI_SETYCARETPOLICY, CARET_EVEN, 0);
  SendMessage(hwnd, SCI_SETMULTIPLESELECTION, FALSE, 0);
  SendMessage(hwnd, SCI_SETADDITIONALSELECTIONTYPING, FALSE, 0);
  SendMessage(hwnd, SCI_SETVIRTUALSPACEOPTIONS, SCVS_NONE, 0);
  SendMessage(hwnd, SCI_SETADDITIONALCARETSBLINK, FALSE, 0);
  SendMessage(hwnd, SCI_SETADDITIONALCARETSVISIBLE, FALSE, 0);

  SendMessage(hwnd, SCI_ASSIGNCMDKEY, (SCK_NEXT + (SCMOD_CTRL << 16)), SCI_PARADOWN);
  SendMessage(hwnd, SCI_ASSIGNCMDKEY, (SCK_PRIOR + (SCMOD_CTRL << 16)), SCI_PARAUP);
  SendMessage(hwnd, SCI_ASSIGNCMDKEY, (SCK_NEXT + ((SCMOD_CTRL | SCMOD_SHIFT) << 16)), SCI_PARADOWNEXTEND);
  SendMessage(hwnd, SCI_ASSIGNCMDKEY, (SCK_PRIOR + ((SCMOD_CTRL | SCMOD_SHIFT) << 16)), SCI_PARAUPEXTEND);
  SendMessage(hwnd, SCI_ASSIGNCMDKEY, (SCK_HOME + (0 << 16)), SCI_VCHOMEWRAP);
  SendMessage(hwnd, SCI_ASSIGNCMDKEY, (SCK_END + (0 << 16)), SCI_LINEENDWRAP);
  SendMessage(hwnd, SCI_ASSIGNCMDKEY, (SCK_HOME + (SCMOD_SHIFT << 16)), SCI_VCHOMEWRAPEXTEND);
  SendMessage(hwnd, SCI_ASSIGNCMDKEY, (SCK_END + (SCMOD_SHIFT << 16)), SCI_LINEENDWRAPEXTEND);

  // Init default values for printing
  EditPrintInit();

  return (hwnd);

}


//=============================================================================
//
//  EditSetNewText()
//
extern BOOL bFreezeAppTitle;
extern FILEVARS fvCurFile;

void EditSetNewText(HWND hwnd, char *lpstrText, DWORD cbText)
{
  bFreezeAppTitle = TRUE;

  if (SendMessage(hwnd, SCI_GETREADONLY, 0, 0))
    SendMessage(hwnd, SCI_SETREADONLY, FALSE, 0);

  SendMessage(hwnd, SCI_CANCEL, 0, 0);
  SendMessage(hwnd, SCI_SETUNDOCOLLECTION, 0, 0);
  SendMessage(hwnd, SCI_EMPTYUNDOBUFFER, 0, 0);
  SendMessage(hwnd, SCI_CLEARALL, 0, 0);
  SendMessage(hwnd, SCI_MARKERDELETEALL, (WPARAM)-1, 0);
  SendMessage(hwnd, SCI_SETSCROLLWIDTH, 2048, 0);
  SendMessage(hwnd, SCI_SETXOFFSET, 0, 0);

  FileVars_Apply(hwnd, &fvCurFile);

  if (cbText > 0)
    SendMessage(hwnd, SCI_ADDTEXT, cbText, (LPARAM)lpstrText);

  SendMessage(hwnd, SCI_SETUNDOCOLLECTION, 1, 0);
  SendMessage(hwnd, EM_EMPTYUNDOBUFFER, 0, 0);
  SendMessage(hwnd, SCI_SETSAVEPOINT, 0, 0);
  SendMessage(hwnd, SCI_GOTOPOS, 0, 0);
  SendMessage(hwnd, SCI_CHOOSECARETX, 0, 0);

  bFreezeAppTitle = FALSE;
}


//=============================================================================
//
//  EditConvertText()
//
BOOL EditConvertText(HWND hwnd, UINT cpSource, UINT cpDest, BOOL bSetSavePoint)
{
  struct TextRange tr = { { 0, -1 }, NULL };
  int length, cbText, cbwText;
  char *pchText;
  WCHAR *pwchText;

  if (cpSource == cpDest)
    return (TRUE);

  length = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);

  if (length == 0)
  {
    SendMessage(hwnd, SCI_CANCEL, 0, 0);
    SendMessage(hwnd, SCI_SETUNDOCOLLECTION, 0, 0);
    SendMessage(hwnd, SCI_EMPTYUNDOBUFFER, 0, 0);
    SendMessage(hwnd, SCI_CLEARALL, 0, 0);
    SendMessage(hwnd, SCI_MARKERDELETEALL, (WPARAM)-1, 0);
    SendMessage(hwnd, SCI_SETCODEPAGE, cpDest, 0);
    SendMessage(hwnd, SCI_SETUNDOCOLLECTION, 1, 0);
    SendMessage(hwnd, EM_EMPTYUNDOBUFFER, 0, 0);
    SendMessage(hwnd, SCI_GOTOPOS, 0, 0);
    SendMessage(hwnd, SCI_CHOOSECARETX, 0, 0);

    if (bSetSavePoint)
      SendMessage(hwnd, SCI_SETSAVEPOINT, 0, 0);
  }
  else
  {
    pchText = GlobalAlloc(GPTR, length * 5 + 2);

    tr.lpstrText = pchText;
    SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

    pwchText = GlobalAlloc(GPTR, length * 3 + 2);
    cbwText = MultiByteToWideChar(cpSource, 0, pchText, length, pwchText, length * 3 + 2);
    cbText = WideCharToMultiByte(cpDest, 0, pwchText, cbwText, pchText, length * 5 + 2, NULL, NULL);

    SendMessage(hwnd, SCI_CANCEL, 0, 0);
    SendMessage(hwnd, SCI_SETUNDOCOLLECTION, 0, 0);
    SendMessage(hwnd, SCI_EMPTYUNDOBUFFER, 0, 0);
    SendMessage(hwnd, SCI_CLEARALL, 0, 0);
    SendMessage(hwnd, SCI_MARKERDELETEALL, (WPARAM)-1, 0);
    SendMessage(hwnd, SCI_SETCODEPAGE, cpDest, 0);
    SendMessage(hwnd, SCI_ADDTEXT, cbText, (LPARAM)pchText);
    SendMessage(hwnd, SCI_EMPTYUNDOBUFFER, 0, 0);
    SendMessage(hwnd, SCI_SETUNDOCOLLECTION, 1, 0);
    SendMessage(hwnd, SCI_GOTOPOS, 0, 0);
    SendMessage(hwnd, SCI_CHOOSECARETX, 0, 0);

    GlobalFree(pchText);
    GlobalFree(pwchText);
  }
  return (TRUE);
}


//=============================================================================
//
//  EditSetNewEncoding()
//
BOOL EditSetNewEncoding(HWND hwnd, int iCurrentEncoding, int iNewEncoding, BOOL bNoUI, BOOL bSetSavePoint)
{
  if (iCurrentEncoding != iNewEncoding)
  {
    if ((iCurrentEncoding == CPI_DEFAULT && iNewEncoding == CPI_DEFAULT) ||
        (iCurrentEncoding != CPI_DEFAULT && iNewEncoding != CPI_DEFAULT))
      return (TRUE);

    if (SendMessage(hwnd, SCI_GETLENGTH, 0, 0) == 0)
    {

      BOOL bIsEmptyUndoHistory =
        (SendMessage(hwnd, SCI_CANUNDO, 0, 0) == 0 && SendMessage(hwnd, SCI_CANREDO, 0, 0) == 0);

      if ((iCurrentEncoding == CPI_DEFAULT || iNewEncoding == CPI_DEFAULT) &&
        (bNoUI || bIsEmptyUndoHistory || InfoBox(MBYESNO, L"MsgConv2", IDS_ASK_ENCODING2) == IDYES))
      {

        EditConvertText(hwnd,
                        (mEncoding[iCurrentEncoding].uFlags & NCP_DEFAULT) ? iDefaultCodePage : SC_CP_UTF8,
                        (mEncoding[iNewEncoding].uFlags & NCP_DEFAULT) ? iDefaultCodePage : SC_CP_UTF8,
                        bSetSavePoint);

        return (TRUE);
      }

      else
        return (FALSE);
    }

    else if ((iCurrentEncoding == CPI_DEFAULT || iNewEncoding == CPI_DEFAULT) &&
      (bNoUI || InfoBox(MBYESNO, L"MsgConv1", IDS_ASK_ENCODING) == IDYES))
    {
      BeginWaitCursor();

      EditConvertText(hwnd,
                      (mEncoding[iCurrentEncoding].uFlags & NCP_DEFAULT) ? iDefaultCodePage : SC_CP_UTF8,
                      (mEncoding[iNewEncoding].uFlags & NCP_DEFAULT) ? iDefaultCodePage : SC_CP_UTF8,
                      FALSE);
      EndWaitCursor();

      return (TRUE);
    }

    else
      return (FALSE);
  }

  else
    return (FALSE);
}


//=============================================================================
//
//  EditGetClipboardText()
//
char* EditGetClipboardText(HWND hwnd)
{
  HANDLE hmem;
  WCHAR *pwch;
  char  *pmch;
  char  *ptmp;
  int    wlen, mlen, mlen2;
  UINT   codepage;
  int    eolmode;

  if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(GetParent(hwnd)))
    return (NULL);

  hmem = GetClipboardData(CF_UNICODETEXT);
  pwch = GlobalLock(hmem);

  wlen = lstrlenW(pwch);

  codepage = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);
  eolmode = (int)SendMessage(hwnd, SCI_GETEOLMODE, 0, 0);

  mlen = WideCharToMultiByte(codepage, 0, pwch, wlen + 1, NULL, 0, 0, 0) - 1;
  pmch = LocalAlloc(LPTR, mlen + 1);
  if (pmch)
    WideCharToMultiByte(codepage, 0, pwch, wlen + 1, pmch, mlen + 1, NULL, NULL);

  ptmp = LocalAlloc(LPTR, mlen * 2 + 1);
  if (ptmp)
  {
    char *s = pmch;
    char *d = ptmp;
    int i;
    for (i = 0; (i < mlen) && (*s != 0); i++)
    {
      if (*s == '\n' || *s == '\r')
      {
        if (eolmode == SC_EOL_CR)
        {
          *d++ = '\r';
        }
        else if (eolmode == SC_EOL_LF)
        {
          *d++ = '\n';
        }
        else
        { // eolmode == SC_EOL_CRLF
          *d++ = '\r';
          *d++ = '\n';
        }
        if ((*s == '\r') && (i + 1 < mlen) && (*(s + 1) == '\n'))
        {
          i++;
          s++;
        }
        s++;
      }
      else
      {
        *d++ = *s++;
      }
    }
    *d++ = 0;
    mlen2 = (int)(d - ptmp) - 1;

    LocalFree(pmch);
    pmch = LocalAlloc(LPTR, mlen2 + 1);
    lstrcpyA(pmch, ptmp);
    LocalFree(ptmp);
  }

  GlobalUnlock(hmem);
  CloseClipboard();

  return (pmch);
}


//=============================================================================
//
//  EditCopyAppend()
//
BOOL EditCopyAppend(HWND hwnd)
{
  HANDLE hOld = NULL;
  WCHAR  *pszOld = L"";

  HANDLE hNew;
  WCHAR  *pszNew;

  char  *pszText;
  int   cchTextW;
  WCHAR *pszTextW;

  // [2e]: "Copy Add (Ctrl+E)" not working when clipboard is empty
  const BOOL bClipboardDataAvailable = IsClipboardFormatAvailable(CF_UNICODETEXT);
  const WCHAR *pszSep = bClipboardDataAvailable ? L"\r\n" : L"";
  UINT  uCodePage;

  int iCurPos;
  int iAnchorPos;
  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);
  if (iCurPos != iAnchorPos)
  {
    if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
    {
      return FALSE;
    }
    int iSelCount =
      (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) -
      (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    pszText = LocalAlloc(LPTR, iSelCount + 1);
    (int)SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)pszText);
  }
  else
  {
    // [2e]: Ctrl+E update #28
    int line;
    struct Sci_TextRange tr;

    line = SendMessage(hwnd, SCI_LINEFROMPOSITION, iCurPos, 0);
    tr.chrg.cpMin = SendMessage(hwnd, SCI_POSITIONFROMLINE, line, 0);
    tr.chrg.cpMax = SendMessage(hwnd, SCI_GETLINEENDPOSITION, line, 0);
    if (tr.chrg.cpMax > tr.chrg.cpMin)
    {
      tr.lpstrText = LocalAlloc(LPTR, tr.chrg.cpMax - tr.chrg.cpMin + 1);
      SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
      pszText = tr.lpstrText;
    }
    else
    {
      int cchText = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
      pszText = LocalAlloc(LPTR, cchText + 1);
      SendMessage(hwnd, SCI_GETTEXT, (int)LocalSize(pszText), (LPARAM)pszText);
    }
    // [/2e]
  }
  uCodePage = (SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8) ? CP_UTF8 : CP_ACP;
  cchTextW = MultiByteToWideChar(uCodePage, 0, pszText, -1, NULL, 0);
  if (cchTextW > 0)
  {
    pszTextW = LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(pszSep) + cchTextW + 1));
    lstrcpy(pszTextW, pszSep);
    MultiByteToWideChar(uCodePage, 0, pszText, -1, StrEnd(pszTextW), (int)LocalSize(pszTextW) / sizeof(WCHAR));
  }
  else
  {
    pszTextW = L"";
  }

  LocalFree(pszText);

  if (!OpenClipboard(GetParent(hwnd)))
  {
    LocalFree(pszTextW);
    return (FALSE);
  }
  if (bClipboardDataAvailable)
  {
    hOld = GetClipboardData(CF_UNICODETEXT);
    pszOld = GlobalLock(hOld);
  }

  hNew = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                     sizeof(WCHAR) * (lstrlen(pszOld) + lstrlen(pszTextW) + 1));
  pszNew = GlobalLock(hNew);

  lstrcpy(pszNew, pszOld);
  lstrcat(pszNew, pszTextW);

  GlobalUnlock(hNew);
  if (bClipboardDataAvailable)
    GlobalUnlock(hOld);

  EmptyClipboard();
  SetClipboardData(CF_UNICODETEXT, hNew);
  CloseClipboard();

  return (TRUE);
}


//=============================================================================
//
//  EditDetectEOLMode() - moved here to handle Unicode files correctly
//
int EditDetectEOLMode(HWND hwnd, char *lpData, DWORD cbData)
{
  int iEOLMode = iLineEndings[iDefaultEOLMode];
  char *cp = (char *)lpData;
  if (!cp)
    return (iEOLMode);

  while (*cp && (*cp != '\x0D' && *cp != '\x0A'))
    cp++;

  if (*cp == '\x0D' && * (cp + 1) == '\x0A')
    iEOLMode = SC_EOL_CRLF;
  else if (*cp == '\x0D' && * (cp + 1) != '\x0A')
    iEOLMode = SC_EOL_CR;
  else if (*cp == '\x0A')
    iEOLMode = SC_EOL_LF;

  return (iEOLMode);
}



//=============================================================================
//
//  Encoding Helper Functions
//
void Encoding_InitDefaults()
{
  wsprintf(wchANSI, L" (%i)", GetACP());
  mEncoding[CPI_OEM].uCodePage = GetOEMCP();
  wsprintf(wchOEM, L" (%i)", mEncoding[CPI_OEM].uCodePage);
}

int Encoding_MapIniSetting(BOOL bLoad, int iSetting)
{
  if (bLoad)
  {
    switch (iSetting)
    {
      case 0:
        return CPI_DEFAULT;
      case 1:
        return CPI_UNICODEBOM;
      case 2:
        return CPI_UNICODEBEBOM;
      case 3:
        return CPI_UTF8;
      case 4:
        return CPI_UTF8SIGN;
      case 5:
        return CPI_OEM;
      case 6:
        return CPI_UNICODE;
      case 7:
        return CPI_UNICODEBE;
      case 8:
        return CPI_UTF7;
      default: {
          int i;
          for (i = CPI_UTF7 + 1; i < COUNTOF(mEncoding); i++)
          {
            if (mEncoding[i].uCodePage == (UINT)iSetting && Encoding_IsValid(i))
              return (i);
          }
          return CPI_DEFAULT;
        }
    }
  }
  else
  {
    switch (iSetting)
    {
      case CPI_DEFAULT:
        return 0;
      case CPI_UNICODEBOM:
        return 1;
      case CPI_UNICODEBEBOM:
        return 2;
      case CPI_UTF8:
        return 3;
      case CPI_UTF8SIGN:
        return 4;
      case CPI_OEM:
        return 5;
      case CPI_UNICODE:
        return 6;
      case CPI_UNICODEBE:
        return 7;
      case CPI_UTF7:
        return 8;
      default:
        return (mEncoding[iSetting].uCodePage);
    }
  }
}


void Encoding_GetLabel(int iEncoding)
{
  if (mEncoding[iEncoding].wchLabel[0] == 0)
  {
    WCHAR *pwsz;
    WCHAR wch[256] = L"";
    GetString(mEncoding[iEncoding].idsName, wch, COUNTOF(wch));
    if (pwsz = StrChr(wch, L';'))
    {
      if (pwsz = StrChr(CharNext(pwsz), L';'))
      {
        pwsz = CharNext(pwsz);
      }
    }
    if (!pwsz)
      pwsz = wch;
    StrCpyN(mEncoding[iEncoding].wchLabel, pwsz, COUNTOF(mEncoding[iEncoding].wchLabel));
  }
}


int Encoding_MatchW(LPCWSTR pwszTest)
{
  char tchTest[256];
  WideCharToMultiByte(CP_ACP, 0, pwszTest, -1, tchTest, COUNTOF(tchTest), NULL, NULL);
  return (Encoding_MatchA(tchTest));
}

int Encoding_MatchA(char *pchTest)
{
  int i;
  char  chTest[256];
  char *pchSrc = pchTest;
  char *pchDst = chTest;
  *pchDst++ = ',';
  while (*pchSrc)
  {
    if (IsCharAlphaNumericA(*pchSrc))
      *pchDst++ = *CharLowerA(pchSrc);
    pchSrc++;
  }
  *pchDst++ = ',';
  *pchDst = 0;
  for (i = 0; i < COUNTOF(mEncoding); i++)
  {
    if (StrStrIA(mEncoding[i].pszParseNames, chTest))
    {
      CPINFO cpi;
      if ((mEncoding[i].uFlags & NCP_INTERNAL) ||
          IsValidCodePage(mEncoding[i].uCodePage) &&
          GetCPInfo(mEncoding[i].uCodePage, &cpi))
        return (i);
      else
        return (-1);
    }
  }
  return (-1);
}

BOOL Encoding_IsValid(int iTestEncoding)
{
  CPINFO cpi;
  if (iTestEncoding >= 0 &&
      iTestEncoding < COUNTOF(mEncoding))
  {
    if ((mEncoding[iTestEncoding].uFlags & NCP_INTERNAL) ||
        IsValidCodePage(mEncoding[iTestEncoding].uCodePage) &&
        GetCPInfo(mEncoding[iTestEncoding].uCodePage, &cpi))
    {
      return (TRUE);
    }
  }
  return (FALSE);
}

typedef struct _ee
{
  int    id;
  WCHAR  wch[256];
} ENCODINGENTRY, *PENCODINGENTRY;

int CmpEncoding(const void *s1, const void *s2)
{
  return StrCmp(((PENCODINGENTRY)s1)->wch, ((PENCODINGENTRY)s2)->wch);
}

void Encoding_AddToListView(HWND hwnd, int idSel, BOOL bRecodeOnly)
{
  int i;
  int iSelItem = -1;
  LVITEM lvi;
  WCHAR wchBuf[256];
  PENCODINGENTRY pEE = LocalAlloc(LPTR, COUNTOF(mEncoding) * sizeof(ENCODINGENTRY));
  for (i = 0; i < COUNTOF(mEncoding); i++)
  {
    pEE[i].id = i;
    GetString(mEncoding[i].idsName, pEE[i].wch, COUNTOF(pEE[i].wch));
  }
  qsort(pEE, COUNTOF(mEncoding), sizeof(ENCODINGENTRY), CmpEncoding);
  ZeroMemory(&lvi, sizeof(LVITEM));
  lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
  lvi.pszText = wchBuf;
  for (i = 0; i < COUNTOF(mEncoding); i++)
  {
    int id = pEE[i].id;
    if (!bRecodeOnly || (mEncoding[id].uFlags & NCP_RECODE))
    {
      CPINFO cpi;
      WCHAR *pwsz;

      lvi.iItem = ListView_GetItemCount(hwnd);
      if (pwsz = StrChr(pEE[i].wch, L';'))
      {
        StrCpyN(wchBuf, CharNext(pwsz), COUNTOF(wchBuf));
        if (pwsz = StrChr(wchBuf, L';'))
          *pwsz = 0;
      }
      else
        StrCpyN(wchBuf, pEE[i].wch, COUNTOF(wchBuf));

      if (id == CPI_DEFAULT)
        StrCatN(wchBuf, wchANSI, COUNTOF(wchBuf));
      else if (id == CPI_OEM)
        StrCatN(wchBuf, wchOEM, COUNTOF(wchBuf));

      if ((mEncoding[id].uFlags & NCP_INTERNAL) ||
        (IsValidCodePage(mEncoding[id].uCodePage) &&
         GetCPInfo(mEncoding[id].uCodePage, &cpi)))
        lvi.iImage = 0;
      else
        lvi.iImage = 1;

      lvi.lParam = (LPARAM)id;
      ListView_InsertItem(hwnd, &lvi);

      if (idSel == id)
        iSelItem = lvi.iItem;
    }
  }

  LocalFree(pEE);

  if (iSelItem != -1)
  {
    ListView_SetItemState(hwnd, iSelItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(hwnd, iSelItem, FALSE);
  }
  else
  {
    ListView_SetItemState(hwnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);
    ListView_EnsureVisible(hwnd, 0, FALSE);
  }
}


BOOL Encoding_GetFromListView(HWND hwnd, int *pidEncoding)
{
  LVITEM lvi;

  lvi.iItem = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
  lvi.iSubItem = 0;
  lvi.mask = LVIF_PARAM;
  if (ListView_GetItem(hwnd, &lvi))
  {
    if (Encoding_IsValid((int)lvi.lParam))
    {
      *pidEncoding = (int)lvi.lParam;
      return (TRUE);
    }
    else
      MsgBox(MBWARN, IDS_ERR_ENCODINGNA);
  }
  return (FALSE);
}


void Encoding_AddToComboboxEx(HWND hwnd, int idSel, BOOL bRecodeOnly)
{
  int i;
  int iSelItem = -1;
  COMBOBOXEXITEM cbei;
  WCHAR wchBuf[256];
  PENCODINGENTRY pEE = LocalAlloc(LPTR, COUNTOF(mEncoding) * sizeof(ENCODINGENTRY));
  for (i = 0; i < COUNTOF(mEncoding); i++)
  {
    pEE[i].id = i;
    GetString(mEncoding[i].idsName, pEE[i].wch, COUNTOF(pEE[i].wch));
  }
  qsort(pEE, COUNTOF(mEncoding), sizeof(ENCODINGENTRY), CmpEncoding);

  ZeroMemory(&cbei, sizeof(COMBOBOXEXITEM));
  cbei.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM;
  cbei.pszText = wchBuf;
  cbei.cchTextMax = COUNTOF(wchBuf);
  cbei.iImage = 0;
  cbei.iSelectedImage = 0;

  for (i = 0; i < COUNTOF(mEncoding); i++)
  {
    int id = pEE[i].id;
    if (!bRecodeOnly || (mEncoding[id].uFlags & NCP_RECODE))
    {

      CPINFO cpi;
      WCHAR *pwsz;

      cbei.iItem = SendMessage(hwnd, CB_GETCOUNT, 0, 0);

      if (pwsz = StrChr(pEE[i].wch, L';'))
      {
        StrCpyN(wchBuf, CharNext(pwsz), COUNTOF(wchBuf));
        if (pwsz = StrChr(wchBuf, L';'))
          *pwsz = 0;
      }
      else
        StrCpyN(wchBuf, pEE[i].wch, COUNTOF(wchBuf));
      if (id == CPI_DEFAULT)
        StrCatN(wchBuf, wchANSI, COUNTOF(wchBuf));
      else if (id == CPI_OEM)
        StrCatN(wchBuf, wchOEM, COUNTOF(wchBuf));

      if ((mEncoding[id].uFlags & NCP_INTERNAL) ||
          (IsValidCodePage(mEncoding[id].uCodePage) &&
          GetCPInfo(mEncoding[id].uCodePage, &cpi)))
        cbei.iImage = 0;
      else
        cbei.iImage = 1;

      cbei.lParam = (LPARAM)id;
      SendMessage(hwnd, CBEM_INSERTITEM, 0, (LPARAM)&cbei);

      if (idSel == id)
        iSelItem = (int)cbei.iItem;
    }
  }

  LocalFree(pEE);

  if (iSelItem != -1)
    SendMessage(hwnd, CB_SETCURSEL, (WPARAM)iSelItem, 0);
}


BOOL Encoding_GetFromComboboxEx(HWND hwnd, int *pidEncoding)
{
  COMBOBOXEXITEM cbei;
  cbei.iItem = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  cbei.mask = CBEIF_LPARAM;

  if (SendMessage(hwnd, CBEM_GETITEM, 0, (LPARAM)&cbei))
  {
    if (Encoding_IsValid((int)cbei.lParam))
    {
      *pidEncoding = (int)cbei.lParam;
      return (TRUE);
    }
    else
      MsgBox(MBWARN, IDS_ERR_ENCODINGNA);
  }
  return (FALSE);
}

BOOL IsUnicode(const char *pBuffer, int cb, LPBOOL lpbBOM, LPBOOL lpbReverse)
{
  int i = 0xFFFF;

  BOOL bIsTextUnicode;

  BOOL bHasBOM;
  BOOL bHasRBOM;

  if (!pBuffer || cb < 2)
    return FALSE;

  if (!bSkipUnicodeDetection)
    bIsTextUnicode = IsTextUnicode(pBuffer, cb, &i);
  else
    bIsTextUnicode = FALSE;

  bHasBOM = (*((UNALIGNED PWCHAR) pBuffer) == 0xFEFF);
  bHasRBOM = (*((UNALIGNED PWCHAR) pBuffer) == 0xFFFE);

  if (i == 0xFFFF) // i doesn't seem to have been modified ...
    i = 0;

  if (bIsTextUnicode || bHasBOM || bHasRBOM ||
      ((i & (IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK)) &&
      !((i & IS_TEXT_UNICODE_UNICODE_MASK) && (i & IS_TEXT_UNICODE_REVERSE_MASK)) &&
      !(i & IS_TEXT_UNICODE_ODD_LENGTH) &&
      !(i & IS_TEXT_UNICODE_ILLEGAL_CHARS && !(i & IS_TEXT_UNICODE_REVERSE_SIGNATURE)) &&
      !((i & IS_TEXT_UNICODE_REVERSE_MASK) == IS_TEXT_UNICODE_REVERSE_STATISTICS)))
  {
    if (lpbBOM)
      *lpbBOM = (bHasBOM || bHasRBOM ||
                (i & (IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE)))
                ? TRUE : FALSE;
    if (lpbReverse)
      *lpbReverse = (bHasRBOM || (i & IS_TEXT_UNICODE_REVERSE_MASK)) ? TRUE : FALSE;

    return TRUE;
  }

  else

    return FALSE;
}

BOOL IsUTF8(const char *pTest, int nLength)
{
  static int byte_class_table[256] = {
    /*       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  */
    /* 00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 20 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 30 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 40 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 60 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 70 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 80 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 90 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* A0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* B0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* C0 */ 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    /* D0 */ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    /* E0 */ 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 7, 7,
    /* F0 */ 9, 10, 10, 10, 11, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
    /*       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  */
  };
  /* state table */
  typedef enum
  {
    kSTART = 0, kA, kB, kC, kD, kE, kF, kG, kERROR, kNumOfStates
  } utf8_state;
  static utf8_state state_table[] = {
    /*                            kSTART, kA,     kB,     kC,     kD,     kE,     kF,     kG,     kERROR */
    /* 0x00-0x7F: 0            */ kSTART, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0x80-0x8F: 1            */ kERROR, kSTART, kA, kERROR, kA, kB, kERROR, kB, kERROR,
    /* 0x90-0x9f: 2            */ kERROR, kSTART, kA, kERROR, kA, kB, kB, kERROR, kERROR,
    /* 0xa0-0xbf: 3            */ kERROR, kSTART, kA, kA, kERROR, kB, kB, kERROR, kERROR,
    /* 0xc0-0xc1, 0xf5-0xff: 4 */ kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xc2-0xdf: 5            */ kA, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xe0: 6                 */ kC, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xe1-0xec, 0xee-0xef: 7 */ kB, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xed: 8                 */ kD, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf0: 9                 */ kF, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf1-0xf3: 10           */ kE, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf4: 11                */ kG, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR
  };
#define BYTE_CLASS(b) (byte_class_table[(unsigned char)b])
#define NEXT_STATE(b,cur) (state_table[(BYTE_CLASS(b) * kNumOfStates) + (cur)])
  utf8_state current = kSTART;
  int i;
  const char *pt = pTest;
  int len = nLength;
  for (i = 0; i < len; i++, pt++)
  {
    current = NEXT_STATE(*pt, current);
    if (kERROR == current)
    {
      break;
    }
  }

  return (current == kSTART) ? TRUE : FALSE;
}

BOOL IsUTF7(const char *pTest, int nLength)
{
  int i;
  const char *pt = pTest;
  for (i = 0; i < nLength; i++)
  {
    if (*pt & 0x80 || !*pt)
      return FALSE;
    pt++;
  }

  return TRUE;
}


#define IsUTF8Signature(p) \
    ((*(p+0) == '\xEF' && *(p+1) == '\xBB' && *(p+2) == '\xBF'))


#define UTF8StringStart(p) \
    (IsUTF8Signature(p)) ? (p+3) : (p)


/* byte length of UTF-8 sequence based on value of first byte.
   for UTF-16 (21-bit space), max. code length is 4, so we only need to look
   at 4 upper bits.
 */
static const INT utf8_lengths[16] = {
  1, 1, 1, 1, 1, 1, 1, 1, /* 0000 to 0111 : 1 byte (plain ASCII) */
  0, 0, 0, 0,             /* 1000 to 1011 : not valid */
  2, 2,                   /* 1100, 1101 : 2 bytes */
  3,                      /* 1110 : 3 bytes */
  4                       /* 1111 :4 bytes */
};

/*++
Function :
    UTF8_mbslen_bytes [INTERNAL]

    Calculates the byte size of a NULL-terminated UTF-8 string.

Parameters :
    char *utf8_string : string to examine

Return value :
    size (in bytes) of a NULL-terminated UTF-8 string.
    -1 if invalid NULL-terminated UTF-8 string
--*/
static INT UTF8_mbslen_bytes(LPCSTR utf8_string)
{
  INT length = 0;
  INT code_size;
  BYTE byte;

  while (*utf8_string)
  {
    byte = (BYTE)* utf8_string;
    if ((byte <= 0xF7) && (0 != (code_size = utf8_lengths[byte >> 4])))
    {
      length += code_size;
      utf8_string += code_size;
    }
    else
    {
      /* we got an invalid byte value but need to count it,
         it will be later ignored during the string conversion */
      length++;
      utf8_string++;
    }
  }
  length++; /* include NULL terminator */
  return length;
}

/*++
Function :
    UTF8_mbslen [INTERNAL]

    Calculates the character size of a NULL-terminated UTF-8 string.

Parameters :
    char *utf8_string : string to examine
    int byte_length : byte size of string

Return value :
    size (in characters) of a UTF-8 string.
    -1 if invalid UTF-8 string
--*/
static INT UTF8_mbslen(LPCSTR source, INT byte_length)
{
  INT wchar_length = 0;
  INT code_size;
  BYTE byte;

  while (byte_length > 0)
  {
    byte = (BYTE)* source;
    /* UTF-16 can't encode 5-byte and 6-byte sequences, so maximum value
       for first byte is 11110111. Use lookup table to determine sequence
       length based on upper 4 bits of first byte */
    if ((byte <= 0xF7) && (0 != (code_size = utf8_lengths[byte >> 4])))
    {
      /* 1 sequence == 1 character */
      wchar_length++;
      if (code_size == 4)
      {
        wchar_length++;
      }
      source += code_size;      /* increment pointer */
      byte_length -= code_size; /* decrement counter*/
    }
    else
    {
      /*
         unlike UTF8_mbslen_bytes, we ignore the invalid characters.
         we only report the number of valid characters we have encountered
         to match the Windows behavior.
      */
      source++;
      byte_length--;
    }
  }
  return wchar_length;
}


//=============================================================================
//
//  EditLoadFile()
//
BOOL EditLoadFile(
  HWND hwnd,
  LPCWSTR pszFile,
  BOOL bSkipEncodingDetection,
  int* iEncoding,
  int* iEOLMode,
  BOOL *pbUnicodeErr,
  BOOL *pbFileTooBig)
{

  HANDLE hFile;

  DWORD  dwFileSize;
  DWORD  dwFileSizeLimit;
  DWORD  dwBufSize;
  BOOL   bReadSuccess;

  char* lpData;
  DWORD cbData;
  int _iDefaultEncoding;

  BOOL bBOM;
  BOOL bReverse = FALSE;

  *pbUnicodeErr = FALSE;
  *pbFileTooBig = FALSE;

  hFile = CreateFile(pszFile,
                     GENERIC_READ,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
  dwLastIOError = GetLastError();

  if (hFile == INVALID_HANDLE_VALUE)
  {
    iSrcEncoding = -1;
    iWeakSrcEncoding = -1;
    return FALSE;
  }

  // calculate buffer limit
  dwFileSize = GetFileSize(hFile, NULL);
  dwBufSize = dwFileSize + 10;

  // Check if a warning message should be displayed for large files
  dwFileSizeLimit = IniGetInt(L"Settings2", L"FileLoadWarningMB", 1);
  if (dwFileSizeLimit != 0 && dwFileSizeLimit * 1024 * 1024 < dwFileSize)
  {
    if (InfoBox(MBYESNO, L"MsgFileSizeWarning", IDS_WARNLOADBIGFILE) != IDYES)
    {
      CloseHandle(hFile);
      *pbFileTooBig = TRUE;
      iSrcEncoding = -1;
      iWeakSrcEncoding = -1;
      return FALSE;
    }
  }

  lpData = GlobalAlloc(GPTR, dwBufSize);
  // [2e]: Loading large file silently stops #126
  dwLastIOError = GetLastError();
  if (!lpData)
  {
    CloseHandle(hFile);
    *pbFileTooBig = FALSE;
    iSrcEncoding = -1;
    iWeakSrcEncoding = -1;
    return FALSE;
  }
  // [/2e]
  bReadSuccess = ReadFile(hFile, lpData, (DWORD)GlobalSize(lpData) - 2, &cbData, NULL);
  dwLastIOError = GetLastError();
  CloseHandle(hFile);

  if (!bReadSuccess)
  {
    GlobalFree(lpData);
    iSrcEncoding = -1;
    iWeakSrcEncoding = -1;
    return FALSE;
  }

  if (!Encoding_IsValid(iDefaultEncoding))
    iDefaultEncoding = CPI_DEFAULT;

  _iDefaultEncoding = iDefaultEncoding;
  if (iWeakSrcEncoding != -1 && Encoding_IsValid(iWeakSrcEncoding))
    _iDefaultEncoding = iWeakSrcEncoding;

  *iEncoding = CPI_DEFAULT;

  if (cbData == 0)
  {
    FileVars_Init(NULL, 0, &fvCurFile);
    *iEOLMode = iLineEndings[iDefaultEOLMode];
    if (iSrcEncoding == -1)
    {
      if (bLoadASCIIasUTF8)
        *iEncoding = CPI_UTF8;
      else
        *iEncoding = _iDefaultEncoding;
    }
    else
      *iEncoding = iSrcEncoding;
    SendMessage(hwnd, SCI_SETCODEPAGE, (mEncoding[*iEncoding].uFlags & NCP_DEFAULT) ? iDefaultCodePage : SC_CP_UTF8, 0);
    EditSetNewText(hwnd, "", 0);
    SendMessage(hwnd, SCI_SETEOLMODE, iLineEndings[iDefaultEOLMode], 0);
    GlobalFree(lpData);
  }

  else if (!bSkipEncodingDetection && (iSrcEncoding == -1 || iSrcEncoding == CPI_UNICODE || iSrcEncoding == CPI_UNICODEBE) &&
           (iSrcEncoding == CPI_UNICODE || iSrcEncoding == CPI_UNICODEBE || IsUnicode(lpData, cbData, &bBOM, &bReverse)) &&
           (iSrcEncoding == CPI_UNICODE || iSrcEncoding == CPI_UNICODEBE || !IsUTF8Signature(lpData))) // check for UTF-8 signature
  {
    char *lpDataUTF8;

    if (iSrcEncoding == CPI_UNICODE)
    {
      bBOM = (*((UNALIGNED PWCHAR) lpData) == 0xFEFF);
      bReverse = FALSE;
    }
    else if (iSrcEncoding == CPI_UNICODEBE)
      bBOM = (*((UNALIGNED PWCHAR) lpData) == 0xFFFE);

    if (iSrcEncoding == CPI_UNICODEBE || bReverse)
    {
      _swab(lpData, lpData, cbData);
      if (bBOM)
        *iEncoding = CPI_UNICODEBEBOM;
      else
        *iEncoding = CPI_UNICODEBE;
    }
    else
    {
      if (bBOM)
        *iEncoding = CPI_UNICODEBOM;
      else
        *iEncoding = CPI_UNICODE;
    }

    lpDataUTF8 = GlobalAlloc(GPTR, (cbData * 3) + 2);
    cbData = WideCharToMultiByte(CP_UTF8, 0, (bBOM) ? (LPWSTR)lpData + 1 : (LPWSTR)lpData,
                                 (bBOM) ? (cbData) / sizeof(WCHAR) : cbData / sizeof(WCHAR) + 1, lpDataUTF8, (int)GlobalSize(lpDataUTF8), NULL, NULL);

    if (cbData == 0)
    {
      cbData = WideCharToMultiByte(CP_ACP, 0, (bBOM) ? (LPWSTR)lpData + 1 : (LPWSTR)lpData,
                                   (-1), lpDataUTF8, (int)GlobalSize(lpDataUTF8), NULL, NULL);
      *pbUnicodeErr = TRUE;
    }

    GlobalFree(lpData);
    SendMessage(hwnd, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
    EditSetNewText(hwnd, "", 0);
    FileVars_Init(lpDataUTF8, cbData - 1, &fvCurFile);
    EditSetNewText(hwnd, lpDataUTF8, cbData - 1);
    *iEOLMode = EditDetectEOLMode(hwnd, lpDataUTF8, cbData - 1);
    GlobalFree(lpDataUTF8);
  }
  else
  {
    FileVars_Init(lpData, cbData, &fvCurFile);
    if (!bSkipEncodingDetection && (iSrcEncoding == -1 || iSrcEncoding == CPI_UTF8 || iSrcEncoding == CPI_UTF8SIGN) &&
      ((IsUTF8Signature(lpData) ||
        FileVars_IsUTF8(&fvCurFile) ||
        (iSrcEncoding == CPI_UTF8 || iSrcEncoding == CPI_UTF8SIGN) ||
        (IsUTF8(lpData, cbData) &&
        (((UTF8_mbslen_bytes(UTF8StringStart(lpData)) - 1 !=
           UTF8_mbslen(UTF8StringStart(lpData), IsUTF8Signature(lpData) ? cbData - 3 : cbData)) ||
          mEncoding[_iDefaultEncoding].uFlags & NCP_UTF8 ||
          bLoadASCIIasUTF8))))) && !(FileVars_IsNonUTF8(&fvCurFile) &&
          (iSrcEncoding != CPI_UTF8 && iSrcEncoding != CPI_UTF8SIGN)))
    {
      SendMessage(hwnd, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
      EditSetNewText(hwnd, "", 0);
      if (IsUTF8Signature(lpData))
      {
        EditSetNewText(hwnd, UTF8StringStart(lpData), cbData - 3);
        *iEncoding = CPI_UTF8SIGN;
        *iEOLMode = EditDetectEOLMode(hwnd, UTF8StringStart(lpData), cbData - 3);
      }
      else
      {
        EditSetNewText(hwnd, lpData, cbData);
        *iEncoding = CPI_UTF8;
        *iEOLMode = EditDetectEOLMode(hwnd, lpData, cbData);
      }
      GlobalFree(lpData);
    }

    else
    {
      UINT uCodePage = CP_UTF8;
      LPWSTR lpDataWide;
      int cbDataWide;

      if (iSrcEncoding != -1)
        *iEncoding = iSrcEncoding;
      else
      {
        *iEncoding = FileVars_GetEncoding(&fvCurFile);
        if (*iEncoding == -1)
        {
          if (fvCurFile.mask & FV_ENCODING)
            *iEncoding = CPI_DEFAULT;
          else
          {
            if (iWeakSrcEncoding == -1)
              *iEncoding = _iDefaultEncoding;
            else if (mEncoding[iWeakSrcEncoding].uFlags & NCP_INTERNAL)
              *iEncoding = iDefaultEncoding;
            else
              *iEncoding = _iDefaultEncoding;
          }
        }
      }

      if ((mEncoding[*iEncoding].uFlags & NCP_8BIT && mEncoding[*iEncoding].uCodePage != CP_UTF7) ||
          (mEncoding[*iEncoding].uCodePage == CP_UTF7 && IsUTF7(lpData, cbData)))
      {

        uCodePage = mEncoding[*iEncoding].uCodePage;

        lpDataWide = GlobalAlloc(GPTR, cbData * 2 + 16);
        cbDataWide = MultiByteToWideChar(uCodePage, 0, lpData, cbData, lpDataWide, (int)GlobalSize(lpDataWide) / sizeof(WCHAR));

        GlobalFree(lpData);
        lpData = GlobalAlloc(GPTR, cbDataWide * 3 + 16);

        cbData = WideCharToMultiByte(CP_UTF8, 0, lpDataWide, cbDataWide, lpData, (int)GlobalSize(lpData), NULL, NULL);
        GlobalFree(lpDataWide);

        SendMessage(hwnd, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
        EditSetNewText(hwnd, "", 0);
        EditSetNewText(hwnd, lpData, cbData);
        *iEOLMode = EditDetectEOLMode(hwnd, lpData, cbData);
        GlobalFree(lpData);
      }

      else
      {

        SendMessage(hwnd, SCI_SETCODEPAGE, iDefaultCodePage, 0);
        EditSetNewText(hwnd, "", 0);
        EditSetNewText(hwnd, lpData, cbData);
        *iEncoding = CPI_DEFAULT;
        *iEOLMode = EditDetectEOLMode(hwnd, lpData, cbData);
        GlobalFree(lpData);
      }
    }
  }

  iSrcEncoding = -1;
  iWeakSrcEncoding = -1;
  return TRUE;

}


//=============================================================================
//
//  EditSaveFile()
//
BOOL EditSaveFile(
  HWND hwnd,
  LPCWSTR pszFile,
  int iEncoding,
  BOOL *pbCancelDataLoss,
  BOOL bSaveCopy)
{

  HANDLE hFile;
  BOOL   bWriteSuccess;

  char* lpData;
  DWORD cbData;
  DWORD dwBytesWritten;

  *pbCancelDataLoss = FALSE;

  hFile = n2e_CreateFile(pszFile,
                     GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     NULL,
                     OPEN_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
  dwLastIOError = GetLastError();

  // failure could be due to missing attributes (2k/XP)
  if (hFile == INVALID_HANDLE_VALUE)
  {
    DWORD dwAttributes = GetFileAttributes(pszFile);
    if (dwAttributes != INVALID_FILE_ATTRIBUTES)
    {
      dwAttributes = dwAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
      hFile = n2e_CreateFile(pszFile,
                         GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL | dwAttributes,
                         NULL);
      dwLastIOError = GetLastError();
    }
  }

  if (hFile == INVALID_HANDLE_VALUE)
    return FALSE;

  // ensure consistent line endings
  if (bFixLineEndings)
  {
    SendMessage(hwnd, SCI_CONVERTEOLS, SendMessage(hwnd, SCI_GETEOLMODE, 0, 0), 0);
    EditFixPositions(hwnd);
  }
  // strip trailing blanks
  if (bAutoStripBlanks)
    EditStripTrailingBlanks(hwnd, TRUE);

  // get text
  cbData = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
  lpData = GlobalAlloc(GPTR, cbData + 1);
  SendMessage(hwnd, SCI_GETTEXT, GlobalSize(lpData), (LPARAM)lpData);

  if (cbData == 0)
  {
    bWriteSuccess = n2e_SetEndOfFile(hFile);
    dwLastIOError = GetLastError();
  }

  else
  {
    if (mEncoding[iEncoding].uFlags & NCP_UNICODE)
    {
      LPWSTR lpDataWide;
      int    cbDataWide;

      n2e_SetEndOfFile(hFile);

      lpDataWide = GlobalAlloc(GPTR, cbData * 2 + 16);
      cbDataWide = MultiByteToWideChar(CP_UTF8, 0, lpData, cbData, lpDataWide, (int)GlobalSize(lpDataWide) / sizeof(WCHAR));

      if (mEncoding[iEncoding].uFlags & NCP_UNICODE_BOM)
      {
        if (mEncoding[iEncoding].uFlags & NCP_UNICODE_REVERSE)
          n2e_WriteFile(hFile, (LPCVOID) "\xFE\xFF", 2, &dwBytesWritten, NULL);
        else
          n2e_WriteFile(hFile, (LPCVOID) "\xFF\xFE", 2, &dwBytesWritten, NULL);
      }

      if (mEncoding[iEncoding].uFlags & NCP_UNICODE_REVERSE)
        _swab((char *)lpDataWide, (char *)lpDataWide, cbDataWide * sizeof(WCHAR));

      bWriteSuccess = n2e_WriteFile(hFile, lpDataWide, cbDataWide * sizeof(WCHAR), &dwBytesWritten, NULL);
      dwLastIOError = GetLastError();

      GlobalFree(lpDataWide);
      GlobalFree(lpData);
    }

    else if (mEncoding[iEncoding].uFlags & NCP_UTF8)
    {
      n2e_SetEndOfFile(hFile);

      if (mEncoding[iEncoding].uFlags & NCP_UTF8_SIGN)
        n2e_WriteFile(hFile, (LPCVOID) "\xEF\xBB\xBF", 3, &dwBytesWritten, NULL);

      bWriteSuccess = n2e_WriteFile(hFile, lpData, cbData, &dwBytesWritten, NULL);
      dwLastIOError = GetLastError();

      GlobalFree(lpData);
    }
    else if (mEncoding[iEncoding].uFlags & NCP_8BIT)
    {
      BOOL bCancelDataLoss = FALSE;
      UINT uCodePage = mEncoding[iEncoding].uCodePage;

      LPWSTR lpDataWide = GlobalAlloc(GPTR, cbData * 2 + 16);
      int    cbDataWide = MultiByteToWideChar(CP_UTF8, 0, lpData, cbData, lpDataWide, (int)GlobalSize(lpDataWide) / sizeof(WCHAR));

      // Special cases: 42, 50220, 50221, 50222, 50225, 50227, 50229, 54936, 57002-11, 65000, 65001
      if (uCodePage == CP_UTF7 || uCodePage == 54936)
      {
        GlobalFree(lpData);
        lpData = GlobalAlloc(GPTR, GlobalSize(lpDataWide) * 2);
      }
      else
        ZeroMemory(lpData, GlobalSize(lpData));

      if (uCodePage == CP_UTF7 || uCodePage == 54936)
        cbData = WideCharToMultiByte(uCodePage, 0, lpDataWide, cbDataWide, lpData, (int)GlobalSize(lpData), NULL, NULL);
      else
      {
        cbData = WideCharToMultiByte(uCodePage, WC_NO_BEST_FIT_CHARS, lpDataWide, cbDataWide, lpData, (int)GlobalSize(lpData), NULL,
                                     &bCancelDataLoss);
        if (!bCancelDataLoss)
        {
          cbData = WideCharToMultiByte(uCodePage, 0, lpDataWide, cbDataWide, lpData, (int)GlobalSize(lpData), NULL, NULL);
          bCancelDataLoss = FALSE;
        }
      }
      GlobalFree(lpDataWide);

      if (!bCancelDataLoss || InfoBox(MBOKCANCEL, L"MsgConv3", IDS_ERR_UNICODE2) == IDOK)
      {
        n2e_SetEndOfFile(hFile);
        bWriteSuccess = n2e_WriteFile(hFile, lpData, cbData, &dwBytesWritten, NULL);
        dwLastIOError = GetLastError();
      }
      else
      {
        bWriteSuccess = FALSE;
        *pbCancelDataLoss = TRUE;
      }

      GlobalFree(lpData);
    }

    else
    {
      n2e_SetEndOfFile(hFile);
      bWriteSuccess = n2e_WriteFile(hFile, lpData, cbData, &dwBytesWritten, NULL);
      dwLastIOError = GetLastError();
      GlobalFree(lpData);
    }
  }

  n2e_CloseHandle(hFile);

  if (bWriteSuccess)
  {
    if (!bSaveCopy)
      SendMessage(hwnd, SCI_SETSAVEPOINT, 0, 0);

    return TRUE;
  }

  else
    return FALSE;

}


//=============================================================================
//
//  EditInvertCase()
//
void EditInvertCase(HWND hwnd)
{
  int cchTextW;
  int iCurPos;
  int iAnchorPos;
  UINT cpEdit;
  int i;
  BOOL bChanged = FALSE;

  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurPos != iAnchorPos)
  {
    if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
    {
      int iSelCount = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) -
        (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);

      char  *pszText = GlobalAlloc(GPTR, (iSelCount)+2);
      LPWSTR pszTextW = GlobalAlloc(GPTR, (iSelCount * 2) + 2);

      if (pszText == NULL || pszTextW == NULL)
      {
        GlobalFree(pszText);
        GlobalFree(pszTextW);
        return;
      }

      SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)pszText);

      cpEdit = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);

      cchTextW = MultiByteToWideChar(cpEdit, 0, pszText, iSelCount, pszTextW, (int)GlobalSize(pszTextW) / sizeof(WCHAR));

      for (i = 0; i < cchTextW; i++)
      {
        if (IsCharUpperW(pszTextW[i]))
        {
          pszTextW[i] = LOWORD(CharLowerW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i], 0)));
          bChanged = TRUE;
        }
        else if (IsCharLowerW(pszTextW[i]))
        {
          pszTextW[i] = LOWORD(CharUpperW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i], 0)));
          bChanged = TRUE;
        }
      }

      if (bChanged)
      {

        WideCharToMultiByte(cpEdit, 0, pszTextW, cchTextW, pszText, (int)GlobalSize(pszText), NULL, NULL);

        SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
        SendMessage(hwnd, SCI_CLEAR, 0, 0);
        SendMessage(hwnd, SCI_ADDTEXT, (WPARAM)iSelCount, (LPARAM)pszText);
        SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
        SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

      }

      GlobalFree(pszText);
      GlobalFree(pszTextW);
    }
  }
}


//=============================================================================
//
//  EditTitleCase()
//
void EditTitleCase(HWND hwnd)
{
  int cchTextW;
  int iCurPos;
  int iAnchorPos;
  UINT cpEdit;
  int i;
  BOOL bNewWord = TRUE;
  BOOL bWordEnd = TRUE;
  BOOL bChanged = FALSE;

  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurPos != iAnchorPos)
  {
    if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
    {
      int iSelCount = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) -
                      (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);

      char  *pszText = GlobalAlloc(GPTR, (iSelCount)+2);
      LPWSTR pszTextW = GlobalAlloc(GPTR, (iSelCount * 2) + 2);

      if (pszText == NULL || pszTextW == NULL)
      {
        GlobalFree(pszText);
        GlobalFree(pszTextW);
        return;
      }

      SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)pszText);

      cpEdit = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);

      cchTextW = MultiByteToWideChar(cpEdit, 0, pszText, iSelCount, pszTextW, (int)GlobalSize(pszTextW) / sizeof(WCHAR));

      if (IsW7())
      {

        LPWSTR pszMappedW = LocalAlloc(LPTR, LocalSize(pszTextW));

        if (LCMapString(
                        LOCALE_SYSTEM_DEFAULT, LCMAP_LINGUISTIC_CASING |/*LCMAP_TITLECASE*/0x00000300,
                        pszTextW, cchTextW, pszMappedW, (int)LocalSize(pszMappedW) / sizeof(WCHAR)))
        {
          StrCpyN(pszTextW, pszMappedW, (int)GlobalSize(pszTextW) / sizeof(WCHAR));
          bChanged = TRUE;
        }
        else
          bChanged = FALSE;

        LocalFree(pszMappedW);
      }
      else
      {

        for (i = 0; i < cchTextW; i++)
        {
          BOOL bAlphaNumeric = IsCharAlphaNumericW(pszTextW[i]);
          if (!bAlphaNumeric && (!StrChr(L"\x0027\x2019\x0060\x00B4", pszTextW[i]) || bWordEnd))
          {
            bNewWord = TRUE;
          }
          else
          {
            if (bNewWord)
            {
              if (IsCharLowerW(pszTextW[i]))
              {
                pszTextW[i] = LOWORD(CharUpperW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i], 0)));
                bChanged = TRUE;
              }
            }
            else
            {
              if (IsCharUpperW(pszTextW[i]))
              {
                pszTextW[i] = LOWORD(CharLowerW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i], 0)));
                bChanged = TRUE;
              }
            }
            bNewWord = FALSE;
          }
          bWordEnd = !bAlphaNumeric;
        }
      }

      if (bChanged)
      {
        WideCharToMultiByte(cpEdit, 0, pszTextW, cchTextW, pszText, (int)GlobalSize(pszText), NULL, NULL);

        SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
        SendMessage(hwnd, SCI_CLEAR, 0, 0);
        SendMessage(hwnd, SCI_ADDTEXT, (WPARAM)iSelCount, (LPARAM)pszText);
        SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
        SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

      }

      GlobalFree(pszText);
      GlobalFree(pszTextW);
    }
  }
}


//=============================================================================
//
//  EditSentenceCase()
//
void EditSentenceCase(HWND hwnd)
{
  int cchTextW;
  int iCurPos;
  int iAnchorPos;
  UINT cpEdit;
  int i;
  BOOL bNewSentence = TRUE;
  BOOL bChanged = FALSE;

  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurPos != iAnchorPos)
  {
    if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
    {
      int iSelCount = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) -
                      (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);

      char  *pszText = GlobalAlloc(GPTR, (iSelCount)+2);
      LPWSTR pszTextW = GlobalAlloc(GPTR, (iSelCount * 2) + 2);

      if (pszText == NULL || pszTextW == NULL)
      {
        GlobalFree(pszText);
        GlobalFree(pszTextW);
        return;
      }

      SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)pszText);

      cpEdit = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);

      cchTextW = MultiByteToWideChar(cpEdit, 0, pszText, iSelCount, pszTextW, (int)GlobalSize(pszTextW) / sizeof(WCHAR));

      for (i = 0; i < cchTextW; i++)
      {
        if (StrChr(L".;!?\r\n", pszTextW[i]))
        {
          bNewSentence = TRUE;
        }
        else
        {
          if (IsCharAlphaNumericW(pszTextW[i]))
          {
            if (bNewSentence)
            {
              if (IsCharLowerW(pszTextW[i]))
              {
                pszTextW[i] = LOWORD(CharUpperW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i], 0)));
                bChanged = TRUE;
              }
              bNewSentence = FALSE;
            }
            else
            {
              if (IsCharUpperW(pszTextW[i]))
              {
                pszTextW[i] = LOWORD(CharLowerW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i], 0)));
                bChanged = TRUE;
              }
            }
          }
        }
      }

      if (bChanged)
      {

        WideCharToMultiByte(cpEdit, 0, pszTextW, cchTextW, pszText, (int)GlobalSize(pszText), NULL, NULL);
        SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
        SendMessage(hwnd, SCI_CLEAR, 0, 0);
        SendMessage(hwnd, SCI_ADDTEXT, (WPARAM)iSelCount, (LPARAM)pszText);
        SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
        SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
      }

      GlobalFree(pszText);
      GlobalFree(pszTextW);
    }
  }
}


//=============================================================================
//
//  EditURLEncode()
//
void EditURLEncode(HWND hwnd)
{
  int cchTextW;
  int iCurPos;
  int iAnchorPos;
  UINT cpEdit;

  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurPos != iAnchorPos)
  {
    if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
    {
      int iSelCount = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) -
                      (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
      char*  pszText;
      LPWSTR pszTextW;

      DWORD  cchEscaped;
      char*  pszEscaped;
      DWORD  cchEscapedW;
      LPWSTR pszEscapedW;

      pszText = LocalAlloc(LPTR, (iSelCount)+2);
      if (pszText == NULL)
      {
        return;
      }

      pszTextW = LocalAlloc(LPTR, (iSelCount * 2) + 2);
      if (pszTextW == NULL)
      {
        LocalFree(pszText);
        return;
      }

      SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)pszText);
      cpEdit = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);
      cchTextW = MultiByteToWideChar(cpEdit, 0, pszText, iSelCount, pszTextW, (int)LocalSize(pszTextW) / sizeof(WCHAR));

      pszEscaped = LocalAlloc(LPTR, LocalSize(pszText) * 3);
      if (pszEscaped == NULL)
      {
        LocalFree(pszText);
        LocalFree(pszTextW);
        return;
      }

      pszEscapedW = LocalAlloc(LPTR, LocalSize(pszTextW) * 3);
      if (pszEscapedW == NULL)
      {
        LocalFree(pszText);
        LocalFree(pszTextW);
        LocalFree(pszEscaped);
        return;
      }

      cchEscapedW = (int)LocalSize(pszEscapedW) / sizeof(WCHAR);
      UrlEscape(pszTextW, pszEscapedW, &cchEscapedW, URL_ESCAPE_SEGMENT_ONLY);

      cchEscaped = WideCharToMultiByte(cpEdit, 0, pszEscapedW, cchEscapedW, pszEscaped, (int)LocalSize(pszEscaped), NULL, NULL);

      if (iCurPos < iAnchorPos)
        iAnchorPos = iCurPos + cchEscaped;
      else
        iCurPos = iAnchorPos + cchEscaped;

      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
      SendMessage(hwnd, SCI_CLEAR, 0, 0);
      SendMessage(hwnd, SCI_ADDTEXT, (WPARAM)cchEscaped, (LPARAM)pszEscaped);
      SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
      SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

      LocalFree(pszText);
      LocalFree(pszTextW);
      LocalFree(pszEscaped);
      LocalFree(pszEscapedW);
    }
  }
}


//=============================================================================
//
//  EditURLDecode()
//
void EditURLDecode(HWND hwnd)
{
  int cchTextW;
  int iCurPos;
  int iAnchorPos;
  UINT cpEdit;

  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurPos != iAnchorPos)
  {
    if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
    {
      int iSelCount = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) -
                      (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);

      char*  pszText;
      LPWSTR pszTextW;

      DWORD  cchUnescaped;
      char*  pszUnescaped;
      DWORD  cchUnescapedW;
      LPWSTR pszUnescapedW;

      pszText = LocalAlloc(LPTR, (iSelCount)+2);
      if (pszText == NULL)
      {
        return;
      }

      pszTextW = LocalAlloc(LPTR, (iSelCount * 2) + 2);
      if (pszTextW == NULL)
      {
        LocalFree(pszText);
        return;
      }

      SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)pszText);
      cpEdit = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);
      cchTextW = MultiByteToWideChar(cpEdit, 0, pszText, iSelCount, pszTextW, (int)LocalSize(pszTextW) / sizeof(WCHAR));

      pszUnescaped = LocalAlloc(LPTR, LocalSize(pszText) * 3);
      if (pszUnescaped == NULL)
      {
        LocalFree(pszText);
        LocalFree(pszTextW);
        return;
      }

      pszUnescapedW = LocalAlloc(LPTR, LocalSize(pszTextW) * 3);
      if (pszUnescapedW == NULL)
      {
        LocalFree(pszText);
        LocalFree(pszTextW);
        LocalFree(pszUnescaped);
        return;
      }

      cchUnescapedW = (int)LocalSize(pszUnescapedW) / sizeof(WCHAR);
      UrlUnescape(pszTextW, pszUnescapedW, &cchUnescapedW, 0);

      cchUnescaped = WideCharToMultiByte(cpEdit, 0, pszUnescapedW, cchUnescapedW, pszUnescaped, (int)LocalSize(pszUnescaped), NULL, NULL);

      if (iCurPos < iAnchorPos)
        iAnchorPos = iCurPos + cchUnescaped;
      else
        iCurPos = iAnchorPos + cchUnescaped;

      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
      SendMessage(hwnd, SCI_CLEAR, 0, 0);
      SendMessage(hwnd, SCI_ADDTEXT, (WPARAM)cchUnescaped, (LPARAM)pszUnescaped);
      SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
      SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

      LocalFree(pszText);
      LocalFree(pszTextW);
      LocalFree(pszUnescaped);
      LocalFree(pszUnescapedW);
    }
  }
}


//=============================================================================
//
//  EditEscapeCChars()
//
void EditEscapeCChars(HWND hwnd)
{
  if (SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) - SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0))
  {
    if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
    {
      EDITFINDREPLACE efr = { "", "", "", "", 0, 0, 0, 0, 0, 0, hwnd };
      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
      lstrcpyA(efr.szFind, "\\");
      lstrcpyA(efr.szReplace, "\\\\");
      EditReplaceAllInSelection(hwnd, &efr, FALSE);
      lstrcpyA(efr.szFind, "\"");
      lstrcpyA(efr.szReplace, "\\\"");
      EditReplaceAllInSelection(hwnd, &efr, FALSE);
      lstrcpyA(efr.szFind, "\'");
      lstrcpyA(efr.szReplace, "\\\'");
      EditReplaceAllInSelection(hwnd, &efr, FALSE);
      SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
    }
  }
}


//=============================================================================
//
//  EditUnescapeCChars()
//
void EditUnescapeCChars(HWND hwnd)
{
  if (SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) - SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0))
  {
    if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
    {
      EDITFINDREPLACE efr = { "", "", "", "", 0, 0, 0, 0, 0, 0, hwnd };

      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

      lstrcpyA(efr.szFind, "\\\\");
      lstrcpyA(efr.szReplace, "\\");
      EditReplaceAllInSelection(hwnd, &efr, FALSE);

      lstrcpyA(efr.szFind, "\\\"");
      lstrcpyA(efr.szReplace, "\"");
      EditReplaceAllInSelection(hwnd, &efr, FALSE);

      lstrcpyA(efr.szFind, "\\\'");
      lstrcpyA(efr.szReplace, "\'");
      EditReplaceAllInSelection(hwnd, &efr, FALSE);

      SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
    }
  }
}


//=============================================================================
//
//  EditChar2Hex()
//
void EditChar2Hex(HWND hwnd)
{
  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {

    int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

    if (SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0) ==
        SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0) &&
        iSelEnd == (int)SendMessage(hwnd, SCI_POSITIONAFTER, (WPARAM)iSelStart, 0))
    {

      char  ch[32];
      WCHAR wch[32];
      UINT  cp = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);

      SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)ch);

      if (ch[0] == 0)
        lstrcpyA(ch, "\\x00");

      else
      {
        MultiByteToWideChar(cp, 0, ch, -1, wch, COUNTOF(wch));
        if (wch[0] <= 0xFF)
          wsprintfA(ch, "\\x%02X", wch[0] & 0xFF);
        else
          wsprintfA(ch, "\\u%04X", wch[0]);
      }

      SendMessage(hwnd, SCI_REPLACESEL, 0, (LPARAM)ch);
      SendMessage(hwnd, SCI_SETSEL, iSelStart, iSelStart + lstrlenA(ch));
    }
  }
}


//=============================================================================
//
//  EditHex2Char()
//
void EditHex2Char(HWND hwnd)
{
  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {

    char ch[32];
    int  i;
    BOOL bTrySelExpand = FALSE;

    int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

    if (iSelEnd - iSelStart)
    {

      if (SendMessage(hwnd, SCI_GETSELTEXT, 0, 0) <= COUNTOF(ch))
      {

        SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)ch);

        if (StrChrIA(ch, ' ') || StrChrIA(ch, '\t') || StrChrIA(ch, '\r') || StrChrIA(ch, '\n') || StrChrIA(ch, '-'))
          return;

        if (StrCmpNIA(ch, "\\x", 2) == 0 || StrCmpNIA(ch, "\\u", 2) == 0)
        {
          ch[0] = '0';
          ch[1] = 'x';
        }

        else if (StrChrIA("xu", ch[0]))
        {
          ch[0] = '0';
          bTrySelExpand = TRUE;
        }

        if (sscanf(ch, "%x", &i) == 1)
        {
          int cch;
          if (i == 0)
          {
            ch[0] = 0;
            cch = 1;
          }
          else
          {
            UINT  cp = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);
            WCHAR wch[4];
            wsprintf(wch, L"%lc", (WCHAR)i);
            cch = WideCharToMultiByte(cp, 0, wch, -1, ch, COUNTOF(ch), NULL, NULL) - 1;
            if (bTrySelExpand && (char)SendMessage(hwnd, SCI_GETCHARAT, (WPARAM)iSelStart - 1, 0) == '\\')
            {
              iSelStart--;
            }
          }
          SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iSelStart, 0);
          SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iSelEnd, 0);
          SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cch, (LPARAM)ch);
          SendMessage(hwnd, SCI_SETSEL, iSelStart, iSelStart + cch);
        }
      }
    }
  }
}


//=============================================================================
//
//  EditModifyNumber()
//
void EditModifyNumber(HWND hwnd, BOOL bIncrease)
{
  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {

    int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

    if (iSelEnd - iSelStart)
    {

      char chFormat[32] = "";
      char chNumber[32];
      int  iNumber;
      int  iWidth;

      if (SendMessage(hwnd, SCI_GETSELTEXT, 0, 0) <= COUNTOF(chNumber))
      {
        SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)chNumber);

        if (StrChrIA(chNumber, '-'))
          return;

        if (!StrChrIA(chNumber, 'x') && sscanf(chNumber, "%d", &iNumber) == 1)
        {
          iWidth = lstrlenA(chNumber);
          if (iNumber >= 0)
          {
            if (bIncrease && iNumber < INT_MAX)
              iNumber++;
            if (!bIncrease && iNumber > 0)
              iNumber--;
            wsprintfA(chFormat, "%%0%ii", iWidth);
            wsprintfA(chNumber, chFormat, iNumber);
            SendMessage(hwnd, SCI_REPLACESEL, 0, (LPARAM)chNumber);
            SendMessage(hwnd, SCI_SETSEL, iSelStart, iSelStart + lstrlenA(chNumber));
          }
        }
        else if (sscanf(chNumber, "%x", &iNumber) == 1)
        {
          int i;
          BOOL bUppercase = FALSE;
          iWidth = lstrlenA(chNumber) - 2;
          if (iNumber >= 0)
          {
            if (bIncrease && iNumber < INT_MAX)
              iNumber++;
            if (!bIncrease && iNumber > 0)
              iNumber--;
            for (i = lstrlenA(chNumber) - 1; i >= 0; i--)
            {
              if (IsCharLowerA(chNumber[i]))
                break;
              else if (IsCharUpper(chNumber[i]))
              {
                bUppercase = TRUE;
                break;
              }
            }
            if (bUppercase)
              wsprintfA(chFormat, "%%#0%iX", iWidth);
            else
              wsprintfA(chFormat, "%%#0%ix", iWidth);
            wsprintfA(chNumber, chFormat, iNumber);
            SendMessage(hwnd, SCI_REPLACESEL, 0, (LPARAM)chNumber);
            SendMessage(hwnd, SCI_SETSEL, iSelStart, iSelStart + lstrlenA(chNumber));
          }
        }
      }
    }
  }
}


//=============================================================================
//
//  EditTabsToSpaces()
//
void EditTabsToSpaces(HWND hwnd, int nTabWidth, BOOL bOnlyIndentingWS)
{
  char *pszText;
  LPWSTR pszTextW;
  int cchTextW;
  int iTextW;
  LPWSTR pszConvW;
  int cchConvW;
  int cchConvM;
  int i, j;
  int iLine;
  int iCurPos;
  int iAnchorPos;
  int iSelStart;
  int iSelEnd;
  int iSelCount;
  UINT cpEdit;
  struct TextRange tr;
  BOOL bIsLineStart = TRUE;
  BOOL bModified = FALSE;

  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }

  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurPos == iAnchorPos)
    return;
  else
  {
    iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  }

  iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
  iSelStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);

  iSelCount = iSelEnd - iSelStart;

  pszText = GlobalAlloc(GPTR, (iSelCount)+2);
  if (pszText == NULL)
    return;

  pszTextW = GlobalAlloc(GPTR, (iSelCount * 2) + 2);
  if (pszTextW == NULL)
  {
    GlobalFree(pszText);
    return;
  }

  tr.chrg.cpMin = iSelStart;
  tr.chrg.cpMax = iSelEnd;
  tr.lpstrText = pszText;
  SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

  cpEdit = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);

  cchTextW = MultiByteToWideChar(cpEdit, 0, pszText, iSelCount, pszTextW, (int)GlobalSize(pszTextW) / sizeof(WCHAR));
  GlobalFree(pszText);

  pszConvW = GlobalAlloc(GPTR, cchTextW * sizeof(WCHAR) * nTabWidth + 2);
  if (pszConvW == NULL)
  {
    GlobalFree(pszTextW);
    return;
  }

  cchConvW = 0;

  // Contributed by Homam
  // Thank you very much!
  i = 0;
  for (iTextW = 0; iTextW < cchTextW; iTextW++)
  {
    WCHAR w = pszTextW[iTextW];
    if (w == L'\t' && (!bOnlyIndentingWS || bOnlyIndentingWS && bIsLineStart))
    {
      for (j = 0; j < nTabWidth - i % nTabWidth; j++)
        pszConvW[cchConvW++] = L' ';
      i = 0;
      bModified = TRUE;
    }
    else
    {
      i++;
      if (w == L'\n' || w == L'\r')
      {
        i = 0;
        bIsLineStart = TRUE;
      }
      else if (w != L' ')
        bIsLineStart = FALSE;
      pszConvW[cchConvW++] = w;
    }
  }

  GlobalFree(pszTextW);
  if (bModified)
  {
    pszText = GlobalAlloc(GPTR, cchConvW * 3);
    cchConvM = WideCharToMultiByte(cpEdit, 0, pszConvW, cchConvW, pszText, (int)GlobalSize(pszText), NULL, NULL);
    GlobalFree(pszConvW);
    if (iAnchorPos > iCurPos)
    {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchConvM;
    }
    else
    {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchConvM;
    }

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
    SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iSelStart, 0);
    SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iSelEnd, 0);
    SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cchConvM, (LPARAM)pszText);
    SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

    GlobalFree(pszText);
  }

  else
    GlobalFree(pszConvW);
}


//=============================================================================
//
//  EditSpacesToTabs()
//
void EditSpacesToTabs(HWND hwnd, int nTabWidth, BOOL bOnlyIndentingWS)
{
  char *pszText;
  LPWSTR pszTextW;
  int cchTextW;
  int iTextW;
  LPWSTR pszConvW;
  int cchConvW;
  int cchConvM;
  int i, j, t;
  int iLine;
  int iCurPos;
  int iAnchorPos;
  int iSelStart;
  int iSelEnd;
  int iSelCount;
  UINT cpEdit;
  struct TextRange tr;
  WCHAR space[256];
  BOOL bIsLineStart = TRUE;
  BOOL bModified = FALSE;

  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }

  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurPos == iAnchorPos)
    return;

  else
  {
    iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  }

  iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
  iSelStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);

  iSelCount = iSelEnd - iSelStart;

  pszText = GlobalAlloc(GPTR, (iSelCount)+2);
  if (pszText == NULL)
    return;

  pszTextW = GlobalAlloc(GPTR, (iSelCount * 2) + 2);
  if (pszTextW == NULL)
  {
    GlobalFree(pszText);
    return;
  }

  tr.chrg.cpMin = iSelStart;
  tr.chrg.cpMax = iSelEnd;
  tr.lpstrText = pszText;
  SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

  cpEdit = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);

  cchTextW = MultiByteToWideChar(cpEdit, 0, pszText, iSelCount, pszTextW, (int)GlobalSize(pszTextW) / sizeof(WCHAR));
  GlobalFree(pszText);

  pszConvW = GlobalAlloc(GPTR, cchTextW * sizeof(WCHAR) + 2);
  if (pszConvW == NULL)
  {
    GlobalFree(pszTextW);
    return;
  }

  cchConvW = 0;

  // Contributed by Homam
  // Thank you very much!
  i = j = 0;
  for (iTextW = 0; iTextW < cchTextW; iTextW++)
  {
    WCHAR w = pszTextW[iTextW];
    if ((w == L' ' || w == L'\t') && (!bOnlyIndentingWS || bOnlyIndentingWS && bIsLineStart))
    {
      space[j++] = w;
      if (j == nTabWidth - i % nTabWidth || w == L'\t')
      {
        if (j > 1 || pszTextW[iTextW + 1] == L' ' || pszTextW[iTextW + 1] == L'\t')
          pszConvW[cchConvW++] = L'\t';
        else
          pszConvW[cchConvW++] = w;
        i = j = 0;
        bModified = bModified || (w != pszConvW[cchConvW - 1]);
      }
    }
    else
    {
      i += j + 1;
      if (j > 0)
      {
        for (t = 0; t < j; t++)
          pszConvW[cchConvW++] = space[t];
        j = 0;
      }
      if (w == L'\n' || w == L'\r')
      {
        i = 0;
        bIsLineStart = TRUE;
      }
      else
        bIsLineStart = FALSE;
      pszConvW[cchConvW++] = w;
    }
  }
  if (j > 0)
  {
    for (t = 0; t < j; t++)
      pszConvW[cchConvW++] = space[t];
  }

  GlobalFree(pszTextW);

  if (bModified || cchConvW != cchTextW)
  {
    pszText = GlobalAlloc(GPTR, cchConvW * 3);

    cchConvM = WideCharToMultiByte(cpEdit, 0, pszConvW, cchConvW, pszText, (int)GlobalSize(pszText), NULL, NULL);
    GlobalFree(pszConvW);

    if (iAnchorPos > iCurPos)
    {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchConvM;
    }
    else
    {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchConvM;
    }

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
    SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iSelStart, 0);
    SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iSelEnd, 0);
    SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cchConvM, (LPARAM)pszText);
    SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

    GlobalFree(pszText);
  }

  else
    GlobalFree(pszConvW);
}


//=============================================================================
//
//  EditMoveUp()
//
void EditMoveUp(HWND hwnd)
{

  int iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);
  int iCurLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iCurPos, 0);
  int iAnchorLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iAnchorPos, 0);

  if (iCurLine == iAnchorLine)
  {
    int iLineCurPos = iCurPos - (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iCurLine, 0);
    int iLineAnchorPos = iAnchorPos - (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iAnchorLine, 0);
    if (iCurLine > 0)
    {

      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
      SendMessage(hwnd, SCI_LINETRANSPOSE, 0, 0);
      SendMessage(hwnd, SCI_SETSEL,
                  (WPARAM)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iAnchorLine - 1, 0) + iLineAnchorPos,
                  (LPARAM)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iCurLine - 1, 0) + iLineCurPos);
      SendMessage(hwnd, SCI_CHOOSECARETX, 0, 0);
      SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
    }
  }
  else if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {

    int iLineSrc = min(iCurLine, iAnchorLine) - 1;
    if (iLineSrc >= 0)
    {
      DWORD cLine;
      char *pLine;
      int iLineSrcStart;
      int iLineSrcEnd;
      int iLineDest;
      int iLineDestStart;

      cLine = (int)SendMessage(hwnd, SCI_GETLINE, (WPARAM)iLineSrc, 0);
      pLine = LocalAlloc(LPTR, cLine + 1);
      SendMessage(hwnd, SCI_GETLINE, (WPARAM)iLineSrc, (LPARAM)pLine);

      iLineSrcStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineSrc, 0);
      iLineSrcEnd = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineSrc + 1, 0);

      iLineDest = max(iCurLine, iAnchorLine);
      if (max(iCurPos, iAnchorPos) <= SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineDest, 0))
      {
        if (iLineDest >= 1)
          iLineDest--;
      }

      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

      SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iLineSrcStart, 0);
      SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iLineSrcEnd, 0);
      SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM) "");

      iLineDestStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineDest, 0);

      SendMessage(hwnd, SCI_INSERTTEXT, (WPARAM)iLineDestStart, (LPARAM)pLine);

      LocalFree(pLine);

      if (iLineDest == SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0) - 1)
      {
        char chaEOL[] = "\r\n";
        int iEOLMode = (int)SendMessage(hwnd, SCI_GETEOLMODE, 0, 0);
        if (iEOLMode == SC_EOL_CR)
          chaEOL[1] = 0;
        else if (iEOLMode == SC_EOL_LF)
        {
          chaEOL[0] = '\n';
          chaEOL[1] = 0;
        }

        SendMessage(hwnd, SCI_INSERTTEXT, (WPARAM)iLineDestStart, (LPARAM)chaEOL);
        SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)
                    SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLineDest, 0), 0);
        SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)
                    SendMessage(hwnd, SCI_GETLENGTH, 0, 0), 0);
        SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM) "");
      }

      if (iCurPos < iAnchorPos)
      {
        iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iCurLine - 1, 0);
        iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineDest, 0);
      }
      else
      {
        iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iAnchorLine - 1, 0);
        iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineDest, 0);
      }
      SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);

      SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
    }
  }
}


//=============================================================================
//
//  EditMoveDown()
//
void EditMoveDown(HWND hwnd)
{

  int iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);
  int iCurLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iCurPos, 0);
  int iAnchorLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iAnchorPos, 0);

  if (iCurLine == iAnchorLine)
  {

    int iLineCurPos = iCurPos - (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iCurLine, 0);
    int iLineAnchorPos = iAnchorPos - (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iAnchorLine, 0);
    if (iCurLine < SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0) - 1)
    {
      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
      SendMessage(hwnd, SCI_GOTOLINE, (WPARAM)iCurLine + 1, 0);
      SendMessage(hwnd, SCI_LINETRANSPOSE, 0, 0);
      SendMessage(hwnd, SCI_SETSEL,
                  (WPARAM)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iAnchorLine + 1, 0) + iLineAnchorPos,
                  (LPARAM)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iCurLine + 1, 0) + iLineCurPos);
      SendMessage(hwnd, SCI_CHOOSECARETX, 0, 0);
      SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
    }
  }
  else if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {

    int iLineSrc = max(iCurLine, iAnchorLine) + 1;
    if (max(iCurPos, iAnchorPos) <= SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineSrc - 1, 0))
    {
      if (iLineSrc >= 1)
        iLineSrc--;
    }

    if (iLineSrc <= SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0) - 1)
    {

      DWORD cLine;
      char *pLine;
      int iLineSrcStart;
      int iLineSrcEnd;
      int iLineDest;
      int iLineDestStart;

      BOOL bLastLine = (iLineSrc == SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0) - 1);

      if (bLastLine &&
          (SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLineSrc, 0) -
           SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineSrc, 0) == 0) &&
          (SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLineSrc - 1, 0) -
           SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineSrc - 1, 0) == 0))
        return;

      if (bLastLine)
      {
        char chaEOL[] = "\r\n";
        int iEOLMode = (int)SendMessage(hwnd, SCI_GETEOLMODE, 0, 0);
        if (iEOLMode == SC_EOL_CR)
           chaEOL[1] = 0;
        else if (iEOLMode == SC_EOL_LF)
        {
          chaEOL[0] = '\n';
          chaEOL[1] = 0;
        }
        SendMessage(hwnd, SCI_APPENDTEXT, (WPARAM)lstrlenA(chaEOL), (LPARAM)chaEOL);
      }

      cLine = (int)SendMessage(hwnd, SCI_GETLINE, (WPARAM)iLineSrc, 0);
      pLine = LocalAlloc(LPTR, cLine + 3);
      SendMessage(hwnd, SCI_GETLINE, (WPARAM)iLineSrc, (LPARAM)pLine);

      iLineSrcStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineSrc, 0);
      iLineSrcEnd = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineSrc + 1, 0);

      iLineDest = min(iCurLine, iAnchorLine);

      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

      SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iLineSrcStart, 0);
      SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iLineSrcEnd, 0);
      SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM) "");

      iLineDestStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineDest, 0);

      SendMessage(hwnd, SCI_INSERTTEXT, (WPARAM)iLineDestStart, (LPARAM)pLine);

      if (bLastLine)
      {
        SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)
                    SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)
                                SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0) - 2, 0), 0);
        SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)
                    SendMessage(hwnd, SCI_GETLENGTH, 0, 0), 0);
        SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM) "");
      }

      LocalFree(pLine);

      if (iCurPos < iAnchorPos)
      {
        iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iCurLine + 1, 0);
        iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineSrc + 1, 0);
      }
      else
      {
        iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iAnchorLine + 1, 0);
        iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineSrc + 1, 0);
      }
      SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
      SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
    }
  }
}


//=============================================================================
//
//  EditModifyLines()
//
void EditModifyLines(HWND hwnd, LPCWSTR pwszPrefix, LPCWSTR pwszAppend)
{
  char  mszPrefix1[256 * 3] = "";
  char  mszPrefix2[256 * 3] = "";
  BOOL  bPrefixNum = FALSE;
  int   iPrefixNum = 0;
  int   iPrefixNumWidth = 1;
  char *pszPrefixNumPad = "";
  char  mszAppend1[256 * 3] = "";
  char  mszAppend2[256 * 3] = "";
  BOOL  bAppendNum = FALSE;
  int   iAppendNum = 0;
  int   iAppendNumWidth = 1;
  char *pszAppendNumPad = "";
  int   mbcp;

  int iSelStart = SciCall_GetSelStart();
  int iSelEnd = SciCall_GetSelEnd();
  // [2e]: Alt + M improvements #119
  if (iSelStart == iSelEnd)
  {
    iSelStart = 0;
    iSelEnd = SciCall_GetLength();
  }
  // [/2e]

  if (SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8)
    mbcp = CP_UTF8;
  else
    mbcp = CP_ACP;

  if (lstrlen(pwszPrefix))
    WideCharToMultiByte(mbcp, 0, pwszPrefix, -1, mszPrefix1, COUNTOF(mszPrefix1), NULL, NULL);
  if (lstrlen(pwszAppend))
    WideCharToMultiByte(mbcp, 0, pwszAppend, -1, mszAppend1, COUNTOF(mszAppend1), NULL, NULL);

  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    char *p;
    int  i;

    int iLine;

    int iLineStart = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
    int iLineEnd = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0);

    if (iSelEnd <= SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd, 0))
    {
      if (iLineEnd - iLineStart >= 1)
        iLineEnd--;
    }

    if (lstrlenA(mszPrefix1))
    {

      p = mszPrefix1;
      while (!bPrefixNum && (p = StrStrA(p, "$(")))
      {

        if (StrCmpNA(p, "$(I)", CSTRLEN("$(I)")) == 0)
        {
          *p = 0;
          StrCpyA(mszPrefix2, p + CSTRLEN("$(I)"));
          bPrefixNum = TRUE;
          iPrefixNum = 0;
          for (i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "";
        }

        else if (StrCmpNA(p, "$(0I)", CSTRLEN("$(0I)")) == 0)
        {
          *p = 0;
          StrCpyA(mszPrefix2, p + CSTRLEN("$(0I)"));
          bPrefixNum = TRUE;
          iPrefixNum = 0;
          for (i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "0";
        }

        else if (StrCmpNA(p, "$(N)", CSTRLEN("$(N)")) == 0)
        {
          *p = 0;
          StrCpyA(mszPrefix2, p + CSTRLEN("$(N)"));
          bPrefixNum = TRUE;
          iPrefixNum = 1;
          for (i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "";
        }
        else if (StrCmpNA(p, "$(0N)", CSTRLEN("$(0N)")) == 0)
        {
          *p = 0;
          StrCpyA(mszPrefix2, p + CSTRLEN("$(0N)"));
          bPrefixNum = TRUE;
          iPrefixNum = 1;
          for (i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "0";
        }

        else if (StrCmpNA(p, "$(L)", CSTRLEN("$(L)")) == 0)
        {
          *p = 0;
          StrCpyA(mszPrefix2, p + CSTRLEN("$(L)"));
          bPrefixNum = TRUE;
          iPrefixNum = iLineStart + 1;
          for (i = iLineEnd + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "";
        }

        else if (StrCmpNA(p, "$(0L)", CSTRLEN("$(0L)")) == 0)
        {
          *p = 0;
          StrCpyA(mszPrefix2, p + CSTRLEN("$(0L)"));
          bPrefixNum = TRUE;
          iPrefixNum = iLineStart + 1;
          for (i = iLineEnd + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "0";
        }
        p += CSTRLEN("$(");
      }
    }

    if (lstrlenA(mszAppend1))
    {

      p = mszAppend1;
      while (!bAppendNum && (p = StrStrA(p, "$(")))
      {

        if (StrCmpNA(p, "$(I)", CSTRLEN("$(I)")) == 0)
        {
          *p = 0;
          StrCpyA(mszAppend2, p + CSTRLEN("$(I)"));
          bAppendNum = TRUE;
          iAppendNum = 0;
          for (i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "";
        }
        else if (StrCmpNA(p, "$(0I)", CSTRLEN("$(0I)")) == 0)
        {
          *p = 0;
          StrCpyA(mszAppend2, p + CSTRLEN("$(0I)"));
          bAppendNum = TRUE;
          iAppendNum = 0;
          for (i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "0";
        }
        else if (StrCmpNA(p, "$(N)", CSTRLEN("$(N)")) == 0)
        {
          *p = 0;
          StrCpyA(mszAppend2, p + CSTRLEN("$(N)"));
          bAppendNum = TRUE;
          iAppendNum = 1;
          for (i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "";
        }

        else if (StrCmpNA(p, "$(0N)", CSTRLEN("$(0N)")) == 0)
        {
          *p = 0;
          StrCpyA(mszAppend2, p + CSTRLEN("$(0N)"));
          bAppendNum = TRUE;
          iAppendNum = 1;
          for (i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "0";
        }
        else if (StrCmpNA(p, "$(L)", CSTRLEN("$(L)")) == 0)
        {
          *p = 0;
          StrCpyA(mszAppend2, p + CSTRLEN("$(L)"));
          bAppendNum = TRUE;
          iAppendNum = iLineStart + 1;
          for (i = iLineEnd + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "";
        }

        else if (StrCmpNA(p, "$(0L)", CSTRLEN("$(0L)")) == 0)
        {
          *p = 0;
          StrCpyA(mszAppend2, p + CSTRLEN("$(0L)"));
          bAppendNum = TRUE;
          iAppendNum = iLineStart + 1;
          for (i = iLineEnd + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "0";
        }
        p += CSTRLEN("$(");
      }
    }

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      int iPos;

      if (lstrlen(pwszPrefix))
      {
        char mszInsert[512 * 3];
        lstrcpyA(mszInsert, mszPrefix1);
        if (bPrefixNum)
        {
          char tchFmt[64];
          char tchNum[64];
          wsprintfA(tchFmt, "%%%s%ii", pszPrefixNumPad, iPrefixNumWidth);
          wsprintfA(tchNum, tchFmt, iPrefixNum);
          lstrcatA(mszInsert, tchNum);
          lstrcatA(mszInsert, mszPrefix2);
          iPrefixNum++;
        }
        iPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
        SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iPos, 0);
        SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iPos, 0);
        SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)lstrlenA(mszInsert), (LPARAM)mszInsert);
      }

      if (lstrlen(pwszAppend))
      {

        char mszInsert[512 * 3];
        lstrcpyA(mszInsert, mszAppend1);
        if (bAppendNum)
        {
          char tchFmt[64];
          char tchNum[64];
          wsprintfA(tchFmt, "%%%s%ii", pszAppendNumPad, iAppendNumWidth);
          wsprintfA(tchNum, tchFmt, iAppendNum);
          lstrcatA(mszInsert, tchNum);
          lstrcatA(mszInsert, mszAppend2);
          iAppendNum++;
        }
        iPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);
        SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iPos, 0);
        SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iPos, 0);
        SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)lstrlenA(mszInsert), (LPARAM)mszInsert);
      }
    }
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

    if (iSelStart != iSelEnd)
    {
      int iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
      int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);
      if (iCurPos < iAnchorPos)
      {
        iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0);
        iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd + 1, 0);
      }
      else
      {
        iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0);
        iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd + 1, 0);
      }
      SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
    }

  }
}


//=============================================================================
//
//  EditAlignText()
//
void EditAlignText(HWND hwnd, int nMode)
{
#define BUFSIZE_ALIGN 1024

  int   mbcp;
  BOOL  bModified = FALSE;

  int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  int iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8)
    mbcp = CP_UTF8;
  else
    mbcp = CP_ACP;

  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    int iLine;
    int iMinIndent = BUFSIZE_ALIGN;
    int iMaxLength = 0;

    int iLineStart = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
    int iLineEnd = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0);

    if (iSelEnd <= SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd, 0))
    {
      if (iLineEnd - iLineStart >= 1)
        iLineEnd--;
    }
    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      int iLineEndPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);
      int iLineIndentPos = (int)SendMessage(hwnd, SCI_GETLINEINDENTPOSITION, (WPARAM)iLine, 0);

      if (iLineIndentPos != iLineEndPos)
      {

        int iIndentCol = (int)SendMessage(hwnd, SCI_GETLINEINDENTATION, (WPARAM)iLine, 0);
        int iEndCol;
        char ch;
        int iTail;

        iTail = iLineEndPos - 1;
        ch = (char)SendMessage(hwnd, SCI_GETCHARAT, (WPARAM)iTail, 0);
        while (iTail >= iLineStart && (ch == ' ' || ch == '\t'))
        {
          iTail--;
          ch = (char)SendMessage(hwnd, SCI_GETCHARAT, (WPARAM)iTail, 0);
          iLineEndPos--;
        }
        iEndCol = (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iLineEndPos, 0);

        iMinIndent = min(iMinIndent, iIndentCol);
        iMaxLength = max(iMaxLength, iEndCol);
      }
    }
    if (iMaxLength < BUFSIZE_ALIGN)
    {

      for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
      {
        int iIndentPos = (int)SendMessage(hwnd, SCI_GETLINEINDENTPOSITION, (WPARAM)iLine, 0);
        int iEndPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);

        if (iIndentPos == iEndPos && iEndPos > 0)
        {

          if (!bModified)
          {
            SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
            bModified = TRUE;
          }

          SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0), 0);
          SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iEndPos, 0);
          SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM) "");
        }
        else
        {
          char  tchLineBuf[BUFSIZE_ALIGN * 3] = "";
          WCHAR wchLineBuf[BUFSIZE_ALIGN * 3] = L"";
          WCHAR *pWords[BUFSIZE_ALIGN * 3 / 2];
          WCHAR *p = wchLineBuf;

          int iWords = 0;
          int iWordsLength = 0;
          int cchLine = (int)SendMessage(hwnd, SCI_GETLINE, (WPARAM)iLine, (LPARAM)tchLineBuf);

          if (!bModified)
          {
            SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
            bModified = TRUE;
          }

          MultiByteToWideChar(mbcp, 0, tchLineBuf, cchLine, wchLineBuf, COUNTOF(wchLineBuf));
          StrTrim(wchLineBuf, L"\r\n\t ");

          while (*p)
          {
            if (*p != L' ' && *p != L'\t')
            {
              pWords[iWords++] = p++;
              iWordsLength++;
              while (*p && *p != L' ' && *p != L'\t')
              {
                p++;
                iWordsLength++;
              }
            }
            else
              *p++ = 0;
          }

          if (iWords > 0)
          {
            if (nMode == ALIGN_JUSTIFY || nMode == ALIGN_JUSTIFY_EX)
            {
              BOOL bNextLineIsBlank = FALSE;
              if (nMode == ALIGN_JUSTIFY_EX)
              {
                if (SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0) <= iLine + 1)
                  bNextLineIsBlank = TRUE;

                else
                {

                  int iLineEndPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine + 1, 0);
                  int iLineIndentPos = (int)SendMessage(hwnd, SCI_GETLINEINDENTPOSITION, (WPARAM)iLine + 1, 0);

                  if (iLineIndentPos == iLineEndPos)
                    bNextLineIsBlank = TRUE;
                }
              }

              if ((nMode == ALIGN_JUSTIFY || nMode == ALIGN_JUSTIFY_EX) &&
                  iWords > 1 && iWordsLength >= 2 &&
                  ((nMode != ALIGN_JUSTIFY_EX || !bNextLineIsBlank || iLineStart == iLineEnd) ||
                  (bNextLineIsBlank && iWordsLength > (iMaxLength - iMinIndent) * 0.75)))
              {
                int iGaps = iWords - 1;
                int iSpacesPerGap = (iMaxLength - iMinIndent - iWordsLength) / iGaps;
                int iExtraSpaces = (iMaxLength - iMinIndent - iWordsLength) % iGaps;
                int i, j;
                int iPos;

                WCHAR wchNewLineBuf[BUFSIZE_ALIGN * 3];
                StrCpy(wchNewLineBuf, pWords[0]);
                p = StrEnd(wchNewLineBuf);
                for (i = 1; i < iWords; i++)
                {
                  for (j = 0; j < iSpacesPerGap; j++)
                  {
                    *p++ = L' ';
                    *p = 0;
                  }
                  if (i > iGaps - iExtraSpaces)
                  {
                    *p++ = L' ';
                    *p = 0;
                  }
                  StrCat(p, pWords[i]);
                  p = StrEnd(p);
                }

                WideCharToMultiByte(mbcp, 0, wchNewLineBuf, -1, tchLineBuf, COUNTOF(tchLineBuf), NULL, NULL);

                iPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
                SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iPos, 0);
                iPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);
                SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iPos, 0);
                SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)lstrlenA(tchLineBuf), (LPARAM)tchLineBuf);

                SendMessage(hwnd, SCI_SETLINEINDENTATION, (WPARAM)iLine, (LPARAM)iMinIndent);
              }
              else
              {
                int i;
                int iPos;

                WCHAR wchNewLineBuf[BUFSIZE_ALIGN];
                StrCpy(wchNewLineBuf, pWords[0]);
                p = StrEnd(wchNewLineBuf);

                for (i = 1; i < iWords; i++)
                {
                  *p++ = L' ';
                  *p = 0;
                  StrCat(wchNewLineBuf, pWords[i]);
                  p = StrEnd(p);
                }

                WideCharToMultiByte(mbcp, 0, wchNewLineBuf, -1, tchLineBuf, COUNTOF(tchLineBuf), NULL, NULL);

                iPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
                SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iPos, 0);
                iPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);
                SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iPos, 0);
                SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)lstrlenA(tchLineBuf), (LPARAM)tchLineBuf);

                SendMessage(hwnd, SCI_SETLINEINDENTATION, (WPARAM)iLine, (LPARAM)iMinIndent);
              }
            }
            else
            {

              int iExtraSpaces = iMaxLength - iMinIndent - iWordsLength - iWords + 1;
              int iOddSpaces = iExtraSpaces % 2;
              int i;
              int iPos;

              WCHAR wchNewLineBuf[BUFSIZE_ALIGN * 3] = L"";
              p = wchNewLineBuf;

              if (nMode == ALIGN_RIGHT)
              {
                for (i = 0; i < iExtraSpaces; i++)
                  *p++ = L' ';
                *p = 0;
              }
              if (nMode == ALIGN_CENTER)
              {
                for (i = 1; i < iExtraSpaces - iOddSpaces; i += 2)
                  *p++ = L' ';
                *p = 0;
              }
              for (i = 0; i < iWords; i++)
              {
                StrCat(p, pWords[i]);
                if (i < iWords - 1)
                  StrCat(p, L" ");
                if (nMode == ALIGN_CENTER && iWords > 1 && iOddSpaces > 0 && i + 1 >= iWords / 2)
                {
                  StrCat(p, L" ");
                  iOddSpaces--;
                }
                p = StrEnd(p);
              }

              WideCharToMultiByte(mbcp, 0, wchNewLineBuf, -1, tchLineBuf, COUNTOF(tchLineBuf), NULL, NULL);

              if (nMode == ALIGN_RIGHT || nMode == ALIGN_CENTER)
              {
                SendMessage(hwnd, SCI_SETLINEINDENTATION, (WPARAM)iLine, (LPARAM)iMinIndent);
                iPos = (int)SendMessage(hwnd, SCI_GETLINEINDENTPOSITION, (WPARAM)iLine, 0);
              }
              else
                iPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
              SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iPos, 0);
              iPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);
              SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iPos, 0);
              SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)lstrlenA(tchLineBuf), (LPARAM)tchLineBuf);

              if (nMode == ALIGN_LEFT)
                SendMessage(hwnd, SCI_SETLINEINDENTATION, (WPARAM)iLine, (LPARAM)iMinIndent);
            }
          }
        }
      }
      if (bModified)
        SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
    }
    else
      MsgBox(MBWARN, IDS_BUFFERTOOSMALL);

    if (iCurPos < iAnchorPos)
    {
      iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0);
      iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd + 1, 0);
    }
    else
    {
      iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0);
      iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd + 1, 0);
    }
    SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);

  }
}


//=============================================================================
//
//  EditEncloseSelection()
//
void EditEncloseSelection(HWND hwnd, LPCWSTR pwszOpen, LPCWSTR pwszClose)
{
  char  mszOpen[256 * 3] = "";
  char  mszClose[256 * 3] = "";
  int   mbcp;

  int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

  // [2e]: Ignore newlines in Alt+X #30
  char  skip[] = "\r\n\t ";
  char  ch = 0;
  while (iSelStart < iSelEnd)
  {
    ch = (char)SendMessage(hwnd, SCI_GETCHARAT, iSelStart, 0);
    if (strchr(skip, ch))
    {
      ++iSelStart;
    }
    else
    {
      break;
    }
  }
  while (iSelStart < iSelEnd)
  {
    ch = (char)SendMessage(hwnd, SCI_GETCHARAT, iSelEnd - 1, 0);
    if (strchr(skip, ch))
    {
      --iSelEnd;
    }
    else
    {
      break;
    }
  }
  // [/2e]

  if (SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8)
    mbcp = CP_UTF8;
  else
    mbcp = CP_ACP;

  if (lstrlen(pwszOpen))
    WideCharToMultiByte(mbcp, 0, pwszOpen, -1, mszOpen, COUNTOF(mszOpen), NULL, NULL);
  if (lstrlen(pwszClose))
    WideCharToMultiByte(mbcp, 0, pwszClose, -1, mszClose, COUNTOF(mszClose), NULL, NULL);

  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

    if (lstrlenA(mszOpen))
    {
      SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iSelStart, 0);
      SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iSelStart, 0);
      SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)lstrlenA(mszOpen), (LPARAM)mszOpen);
    }
    if (lstrlenA(mszClose))
    {
      SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iSelEnd + lstrlenA(mszOpen), 0);
      SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iSelEnd + lstrlenA(mszOpen), 0);
      SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)lstrlenA(mszClose), (LPARAM)mszClose);
    }

    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

    // Fix selection
    if (iSelStart == iSelEnd)
      SendMessage(hwnd, SCI_SETSEL, (WPARAM)iSelStart + lstrlenA(mszOpen), (WPARAM)iSelStart + lstrlenA(mszOpen));

    else
    {
      int iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
      int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);
      if (iCurPos < iAnchorPos)
      {
        iCurPos = iSelStart + lstrlenA(mszOpen);
        iAnchorPos = iSelEnd + lstrlenA(mszOpen);
      }
      else
      {
        iAnchorPos = iSelStart + lstrlenA(mszOpen);
        iCurPos = iSelEnd + lstrlenA(mszOpen);
      }
      SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
    }

  }
}


//=============================================================================
//
//  EditToggleLineComments()
//
void EditToggleLineComments(HWND hwnd, LPCWSTR pwszComment, BOOL bInsertAtStart)
{
  char  mszComment[256 * 3] = "";
  int   cchComment;
  int   mbcp;
  int   iAction = 0;

  int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  int iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);

  if (SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8)
    mbcp = CP_UTF8;
  else
    mbcp = CP_ACP;

  if (lstrlen(pwszComment))
    WideCharToMultiByte(mbcp, 0, pwszComment, -1, mszComment, COUNTOF(mszComment), NULL, NULL);
  cchComment = lstrlenA(mszComment);

  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd) && cchComment)
  {
    int iLine;
    int iCommentCol = 0;

    int iLineStart = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
    int iLineEnd = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0);

    if (iSelEnd <= SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd, 0))
    {
      if (iLineEnd - iLineStart >= 1)
        iLineEnd--;
    }

    if (!bInsertAtStart)
    {
      iCommentCol = 1024;
      for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
      {
        int iLineEndPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);
        int iLineIndentPos = (int)SendMessage(hwnd, SCI_GETLINEINDENTPOSITION, (WPARAM)iLine, 0);
        if (iLineIndentPos != iLineEndPos)
        {
          int iIndentColumn = (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iLineIndentPos, 0);
          iCommentCol = min(iCommentCol, iIndentColumn);
        }
      }
    }

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      int iCommentPos;
      int iIndentPos = (int)SendMessage(hwnd, SCI_GETLINEINDENTPOSITION, (WPARAM)iLine, 0);
      char tchBuf[32];
      struct TextRange tr;
      if (iIndentPos == SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0))
        continue;

      tr.chrg.cpMin = iIndentPos;
      tr.chrg.cpMax = tr.chrg.cpMin + min(31, cchComment);
      tr.lpstrText = tchBuf;
      SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

      if (StrCmpNIA(tchBuf, mszComment, cchComment) == 0)
      {
        switch (iAction)
        {
          case 0:
            iAction = 2;
          case 2:
            SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iIndentPos, 0);
            SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iIndentPos + cchComment, 0);
            SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM) "");
            break;
          case 1:
            break;
        }
      }
      else
      {
        switch (iAction)
        {
          case 0:
            iAction = 1;
          case 1:
            iCommentPos = (int)SendMessage(hwnd, SCI_FINDCOLUMN, (WPARAM)iLine, (LPARAM)iCommentCol);
            SendMessage(hwnd, SCI_INSERTTEXT, (WPARAM)iCommentPos, (LPARAM)mszComment);
            break;
          case 2:
            break;
        }
      }
    }
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

    if (iSelStart != iSelEnd)
    {
      int iAnchorPos;
      if (iCurPos == iSelStart)
      {
        iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0);
        iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd + 1, 0);
      }
      else
      {
        iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0);
        iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd + 1, 0);
      }
      SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
    }

  }
}


//=============================================================================
//
//  EditPadWithSpaces()
//
void EditPadWithSpaces(HWND hwnd, BOOL bSkipEmpty, BOOL bNoUndoGroup)
{
  char *pmszPadStr;
  int iMaxColumn = 0;
  int iLine;
  BOOL bIsRectangular = FALSE;
  BOOL bReducedSelection = FALSE;

  int iSelStart;
  int iSelEnd;

  int iLineStart;
  int iLineEnd;

  int iRcCurLine;
  int iRcAnchorLine;
  int iRcCurCol;
  int iRcAnchorCol;

  if (SC_SEL_RECTANGLE != SendMessage(hwnd, SCI_GETSELECTIONMODE, 0, 0))
  {

    iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

    iLineStart = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
    iLineEnd = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0);

    if (iLineStart == iLineEnd)
    {
      iLineStart = 0;
      iLineEnd = (int)SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0) - 1;
    }
    else
    {
      if (iSelEnd <= SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd, 0))
      {
        if (iLineEnd - iLineStart >= 1)
        {
          iLineEnd--;
          bReducedSelection = TRUE;
        }
      }
    }

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {

      int iPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);
      iMaxColumn = max(iMaxColumn, (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iPos, 0));
    }
  }
  else
  {

    int iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
    int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

    iRcCurLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iCurPos, 0);
    iRcAnchorLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iAnchorPos, 0);

    iRcCurCol = (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iCurPos, 0);
    iRcAnchorCol = (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iAnchorPos, 0);

    bIsRectangular = TRUE;

    iLineStart = 0;
    iLineEnd = (int)SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0) - 1;

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {

      int iPos = (int)SendMessage(hwnd, SCI_GETLINESELENDPOSITION, (WPARAM)iLine, 0);
      if (iPos != INVALID_POSITION)
        iMaxColumn = max(iMaxColumn, (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iPos, 0));
    }
  }

  pmszPadStr = LocalAlloc(LPTR, (iMaxColumn + 1) * sizeof(char));
  if (pmszPadStr)
  {

    FillMemory(pmszPadStr, LocalSize(pmszPadStr), ' ');

    if (!bNoUndoGroup)
      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {

      int iPos;
      int iPadLen;
      int iLineSelEndPos;

      iLineSelEndPos = (int)SendMessage(hwnd, SCI_GETLINESELENDPOSITION, (WPARAM)iLine, 0);
      if (bIsRectangular && INVALID_POSITION == iLineSelEndPos)
        continue;

      iPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);
      if (bIsRectangular && iPos > iLineSelEndPos)
        continue;
      if (bSkipEmpty && (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0) >= iPos)
        continue;

      iPadLen = iMaxColumn - (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iPos, 0);

      SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iPos, 0);
      SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iPos, 0);
      SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)iPadLen, (LPARAM)pmszPadStr);
    }
    if (!bNoUndoGroup)
      SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

    LocalFree(pmszPadStr);
  }

  if (!bIsRectangular &&
      SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0) !=
      SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0))
  {
    int iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
    int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);
    if (iCurPos < iAnchorPos)
    {
      iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0);
      if (!bReducedSelection)
        iAnchorPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLineEnd, 0);
      else
        iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd + 1, 0);
    }
    else
    {
      iAnchorPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0);
      if (!bReducedSelection)
        iCurPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLineEnd, 0);
      else
        iCurPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd + 1, 0);
    }
    SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
  }

  else if (bIsRectangular)
  {
    int iCurPos = (int)SendMessage(hwnd, SCI_FINDCOLUMN, (WPARAM)iRcCurLine, (LPARAM)iRcCurCol);
    int iAnchorPos = (int)SendMessage(hwnd, SCI_FINDCOLUMN, (WPARAM)iRcAnchorLine, (LPARAM)iRcAnchorCol);
    SendMessage(hwnd, SCI_SETRECTANGULARSELECTIONCARET, (WPARAM)iCurPos, 0);
    SendMessage(hwnd, SCI_SETRECTANGULARSELECTIONANCHOR, (WPARAM)iAnchorPos, 0);
  }
}


//=============================================================================
//
//  EditStripFirstCharacter()
//
void EditStripFirstCharacter(HWND hwnd)
{
  int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

  if (iSelStart == iSelEnd)
  {
    iSelStart = 0;
    iSelEnd = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
  }

  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    int iLine;

    int iLineStart = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
    int iLineEnd = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0);

    if (iSelStart > SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0))
      iLineStart++;

    if (iSelEnd <= SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd, 0))
      iLineEnd--;

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      int iPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
      if (SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0) - iPos > 0)
      {
        SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iPos, 0);
        SendMessage(hwnd, SCI_SETTARGETEND,
                    (WPARAM)SendMessage(hwnd, SCI_POSITIONAFTER, (WPARAM)iPos, 0), 0);
        SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM) "");
      }
    }
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
  }
}


//=============================================================================
//
//  EditStripLastCharacter()
//
void EditStripLastCharacter(HWND hwnd)
{
  int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

  if (iSelStart == iSelEnd)
  {
    iSelStart = 0;
    iSelEnd = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
  }

  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    int iLine;

    int iLineStart = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
    int iLineEnd = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0);

    if (iSelStart >= SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLineStart, 0))
      iLineStart++;

    if (iSelEnd < SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLineEnd, 0))
      iLineEnd--;

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      int iStartPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
      int iEndPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine, 0);
      if (iEndPos - iStartPos > 0)
      {
        SendMessage(hwnd, SCI_SETTARGETSTART,
                   (WPARAM)SendMessage(hwnd, SCI_POSITIONBEFORE, (WPARAM)iEndPos, 0), 0);
        SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iEndPos, 0);
        SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM) "");
      }
    }
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
  }
}


//=============================================================================
//
//  EditStripTrailingBlanks()
//
void EditStripTrailingBlanks(HWND hwnd, BOOL bIgnoreSelection)
{
  // Check if there is any selection... simply use a regular expression replace!
  if (!bIgnoreSelection &&
      (SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) - SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0) != 0))
  {
    if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
    {
      EDITFINDREPLACE efrTrim = { "[ \t]+$", "", "", "", SCFIND_REGEXP, 0, 0, 0, 0, 0, hwnd };
      EditReplaceAllInSelection(hwnd, &efrTrim, FALSE);
    }
  }
  // Code from SciTE...
  else
  {
    int line;
    int maxLines;
    int lineStart;
    int lineEnd;
    int i;
    char ch;

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
    maxLines = (int)SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0);
    for (line = 0; line < maxLines; line++)
    {
      lineStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, line, 0);
      lineEnd = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, line, 0);
      i = lineEnd - 1;
      ch = (char)SendMessage(hwnd, SCI_GETCHARAT, i, 0);
      while ((i >= lineStart) && ((ch == ' ') || (ch == '\t')))
      {
        i--;
        ch = (char)SendMessage(hwnd, SCI_GETCHARAT, i, 0);
      }
      if (i < (lineEnd - 1))
      {
        SendMessage(hwnd, SCI_SETTARGETSTART, i + 1, 0);
        SendMessage(hwnd, SCI_SETTARGETEND, lineEnd, 0);
        SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM) "");
      }
    }
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
  }
}


//=============================================================================
//
//  EditCompressSpaces()
//
void EditCompressSpaces(HWND hwnd)
{
  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
    int iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
    int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);
    int iLineStart = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
    int iLineEnd = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0);
    int iLength = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);

    char *pszIn;
    char *pszOut;
    BOOL bIsLineStart, bIsLineEnd;
    BOOL bModified = FALSE;
    if (iSelStart != iSelEnd)
    {
      int cch = (int)SendMessage(hwnd, SCI_GETSELTEXT, 0, 0);
      pszIn = LocalAlloc(LPTR, cch);
      pszOut = LocalAlloc(LPTR, cch);
      SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)pszIn);
      bIsLineStart =
        (iSelStart == SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0));
      bIsLineEnd =
        (iSelEnd == SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLineEnd, 0));
    }
    else
    {
      int cch = iLength + 1;
      pszIn = LocalAlloc(GPTR, cch);
      pszOut = LocalAlloc(GPTR, cch);
      SendMessage(hwnd, SCI_GETTEXT, (WPARAM)cch, (LPARAM)pszIn);
      bIsLineStart = TRUE;
      bIsLineEnd = TRUE;
    }
    if (pszIn && pszOut)
    {
      char *ci, *co = pszOut;
      for (ci = pszIn; *ci; ci++)
      {
        if (*ci == ' ' || *ci == '\t')
        {
          if (*ci == '\t')
            bModified = TRUE;
          while (*(ci + 1) == ' ' || *(ci + 1) == '\t')
          {
            ci++;
            bModified = TRUE;
          }
          if (!bIsLineStart && (*(ci + 1) != '\n' && * (ci + 1) != '\r'))
            *co++ = ' ';
          else
            bModified = TRUE;
        }
        else
        {
          if (*ci == '\n' || *ci == '\r')
            bIsLineStart = TRUE;
          else
            bIsLineStart = FALSE;
          *co++ = *ci;
        }
      }
      if (bIsLineEnd && co > pszOut && * (co - 1) == ' ')
      {
        *--co = 0;
        bModified = TRUE;
      }

      if (bModified)
      {
        if (iSelStart != iSelEnd)
          SendMessage(hwnd, SCI_TARGETFROMSELECTION, 0, 0);
        else
        {
          SendMessage(hwnd, SCI_SETTARGETSTART, 0, 0);
          SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iLength, 0);
        }
        SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
        SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)-1, (LPARAM)pszOut);
        if (iCurPos > iAnchorPos)
        {
          iCurPos = (int)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0);
          iAnchorPos = (int)SendMessage(hwnd, SCI_GETTARGETSTART, 0, 0);
          SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
        }
        else if (iCurPos < iAnchorPos)
        {
          iCurPos = (int)SendMessage(hwnd, SCI_GETTARGETSTART, 0, 0);
          iAnchorPos = (int)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0);
          SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
        }
        SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
      }
    }
    if (pszIn)
      LocalFree(pszIn);
    if (pszOut)
      LocalFree(pszOut);
  }
}


//=============================================================================
//
//  EditRemoveBlankLines()
//
void EditRemoveBlankLines(HWND hwnd, BOOL bMerge)
{
  int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

  if (iSelStart == iSelEnd)
  {
    iSelStart = 0;
    iSelEnd = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
  }
  if (!n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    int iLine;

    int iLineStart = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
    int iLineEnd = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0);

    if (iSelStart > SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0))
      iLineStart++;

    if (iSelEnd <= SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd, 0) &&
        iLineEnd != SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0) - 1)
      iLineEnd--;

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

    for (iLine = iLineStart; iLine <= iLineEnd;)
    {
      int nBlanks = 0;
      while (iLine + nBlanks <= iLineEnd &&
             SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine + nBlanks, 0) ==
             SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iLine + nBlanks, 0))
        nBlanks++;

      if (nBlanks == 0 || (nBlanks == 1 && bMerge))
        iLine += nBlanks + 1;

      else
      {
        int iTargetStart;
        int iTargetEnd;

        if (bMerge)
          nBlanks--;
        iTargetStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
        iTargetEnd = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine + nBlanks, 0);
        SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iTargetStart, 0);
        SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iTargetEnd, 0);
        SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM) "");
        if (bMerge)
          iLine++;
        iLineEnd -= nBlanks;
      }
    }
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
  }
}


//=============================================================================
//
//  EditWrapToColumn()
//
void EditWrapToColumn(HWND hwnd, int nColumn)
{
  char* pszText;
  LPWSTR pszTextW;
  int cchTextW;
  int iTextW;
  LPWSTR pszConvW;
  int cchConvW;
  int cchConvM;
  int iLineLength;
  int iLine;
  int iCurPos;
  int iAnchorPos;
  int iSelStart;
  int iSelEnd;
  int iSelCount;
  UINT cpEdit;
  struct TextRange tr;
  int   cEOLMode;
  WCHAR wszEOL[] = L"\r\n";
  int   cchEOL = 2;
  BOOL bModified = FALSE;

  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }

  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurPos == iAnchorPos)
    return;

  else
  {
    iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  }

  iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
  iSelStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);

  iSelCount = iSelEnd - iSelStart;

  pszText = GlobalAlloc(GPTR, (iSelCount)+2);
  if (pszText == NULL)
    return;

  pszTextW = GlobalAlloc(GPTR, (iSelCount * 2) + 2);
  if (pszTextW == NULL)
  {
    GlobalFree(pszText);
    return;
  }

  tr.chrg.cpMin = iSelStart;
  tr.chrg.cpMax = iSelEnd;
  tr.lpstrText = pszText;
  SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

  cpEdit = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);

  cchTextW = MultiByteToWideChar(cpEdit, 0, pszText, iSelCount, pszTextW, (int)GlobalSize(pszTextW) / sizeof(WCHAR));
  GlobalFree(pszText);

  pszConvW = GlobalAlloc(GPTR, cchTextW * sizeof(WCHAR) * 3 + 2);
  if (pszConvW == NULL)
  {
    GlobalFree(pszTextW);
    return;
  }

  cEOLMode = (int)SendMessage(hwnd, SCI_GETEOLMODE, 0, 0);
  if (cEOLMode == SC_EOL_CR)
    cchEOL = 1;
  else if (cEOLMode == SC_EOL_LF)
  {
    cchEOL = 1;
    wszEOL[0] = L'\n';
  }

  cchConvW = 0;
  iLineLength = 0;

#define ISDELIMITER(wc) StrChr(L",;.:-+%&¦|/*?!\"\'~´#=",wc)
#define ISWHITE(wc) StrChr(L" \t",wc)
#define ISWORDEND(wc) (StrChr(L" \t\r\n",wc))

  for (iTextW = 0; iTextW < cchTextW; iTextW++)
  {
    WCHAR w;
    w = pszTextW[iTextW];
    if (ISWHITE(w))
    {
      int iNextWordLen = 0;
      WCHAR w2;

      while (pszTextW[iTextW + 1] == L' ' || pszTextW[iTextW + 1] == L'\t')
      {
        iTextW++;
        bModified = TRUE;
      } // Modified: left out some whitespaces

      w2 = pszTextW[iTextW + 1];
      while (w2 != L'\0' && !ISWORDEND(w2))
      {
        iNextWordLen++;
        w2 = pszTextW[iTextW + iNextWordLen + 1];
      }

      if (iNextWordLen > 0)
      {
        if (iLineLength + iNextWordLen + 1 > nColumn)
        {
          pszConvW[cchConvW++] = wszEOL[0];
          if (cchEOL > 1)
            pszConvW[cchConvW++] = wszEOL[1];
          iLineLength = 0;
          bModified = TRUE;
        }
        else
        {
          if (iLineLength > 0)
          {
            pszConvW[cchConvW++] = L' ';
            iLineLength++;
          }
        }
      }
    }
    else
    {
      pszConvW[cchConvW++] = w;
      if (w == L'\r' || w == L'\n')
      {
        iLineLength = 0;
      }
      else
      {
        iLineLength++;
      }
    }
  }

  GlobalFree(pszTextW);

  if (bModified)
  {
    pszText = GlobalAlloc(GPTR, cchConvW * 3);

    cchConvM = WideCharToMultiByte(cpEdit, 0, pszConvW, cchConvW, pszText, (int)GlobalSize(pszText), NULL, NULL);
    GlobalFree(pszConvW);

    if (iAnchorPos > iCurPos)
    {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchConvM;
    }
    else
    {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchConvM;
    }

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
    SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iSelStart, 0);
    SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iSelEnd, 0);
    SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cchConvM, (LPARAM)pszText);
    SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

    GlobalFree(pszText);
  }

  else
    GlobalFree(pszConvW);
}


//=============================================================================
//
//  EditJoinLinesEx()
//
void EditJoinLinesEx(HWND hwnd)
{
  char* pszText;
  char* pszJoin;
  int   cchJoin = 0;
  int i;
  int iLine;
  int iCurPos;
  int iAnchorPos;
  int iSelStart;
  int iSelEnd;
  int iSelCount;
  struct TextRange tr;
  int  cEOLMode;
  char szEOL[] = "\r\n";
  int  cchEOL = 2;
  BOOL bModified = FALSE;

  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }

  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurPos == iAnchorPos)
    return;

  else
  {
    iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  }

  iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
  iSelStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
  // [2e]: Join Lines/Paragraphs - ignore trailing break #135
  iSelEnd = n2e_JoinParagraphs_GetSelEnd(iSelEnd);

  iSelCount = iSelEnd - iSelStart;

  pszText = LocalAlloc(LPTR, (iSelCount)+2);
  if (pszText == NULL)
    return;

  pszJoin = LocalAlloc(LPTR, LocalSize(pszText));
  if (pszJoin == NULL)
  {
    LocalFree(pszText);
    return;
  }

  tr.chrg.cpMin = iSelStart;
  tr.chrg.cpMax = iSelEnd;
  tr.lpstrText = pszText;
  SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

  cEOLMode = (int)SendMessage(hwnd, SCI_GETEOLMODE, 0, 0);
  if (cEOLMode == SC_EOL_CR)
    cchEOL = 1;
  else if (cEOLMode == SC_EOL_LF)
  {
    cchEOL = 1;
    szEOL[0] = '\n';
  }

  cchJoin = 0;
  for (i = 0; i < iSelCount; i++)
  {
    if (pszText[i] == '\r' || pszText[i] == '\n')
    {
      if (pszText[i] == '\r' && pszText[i + 1] == '\n')
        i++;
      if (!StrChrA("\r\n", pszText[i + 1]) && pszText[i + 1] != 0)
      {
        pszJoin[cchJoin++] = ' ';
        bModified = TRUE;
      }
      else
      {
        while (StrChrA("\r\n", pszText[i + 1]))
        {
          i++;
          bModified = TRUE;
        }
        if (pszText[i + 1] != 0)
        {
          pszJoin[cchJoin++] = szEOL[0];
          if (cchEOL > 1)
            pszJoin[cchJoin++] = szEOL[1];
          if (cchJoin > cchEOL)
          {
            pszJoin[cchJoin++] = szEOL[0];
            if (cchEOL > 1)
              pszJoin[cchJoin++] = szEOL[1];
          }
        }
      }
    }
    else
    {
      pszJoin[cchJoin++] = pszText[i];
    }
  }

  LocalFree(pszText);

  if (bModified)
  {
    if (iAnchorPos > iCurPos)
    {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchJoin;
    }
    else
    {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchJoin;
    }

    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
    SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)iSelStart, 0);
    SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)iSelEnd, 0);
    SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cchJoin, (LPARAM)pszJoin);
    SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
  }

  LocalFree(pszJoin);
}


//=============================================================================
//
//  EditSortLines()
//
typedef struct _SORTLINE
{
  WCHAR *pwszLine;
  WCHAR *pwszSortEntry;
} SORTLINE;

static FARPROC pfnStrCmpLogicalW;
typedef int(__stdcall *FNSTRCMP) (LPCWSTR, LPCWSTR);

int CmpStd(const void *s1, const void *s2)
{
  int cmp = StrCmp(((SORTLINE *)s1)->pwszSortEntry, ((SORTLINE *)s2)->pwszSortEntry);
  return (cmp) ? cmp : StrCmp(((SORTLINE *)s1)->pwszLine, ((SORTLINE *)s2)->pwszLine);
}

int CmpStdRev(const void *s1, const void *s2)
{
  int cmp = -1 * StrCmp(((SORTLINE *)s1)->pwszSortEntry, ((SORTLINE *)s2)->pwszSortEntry);
  return (cmp) ? cmp : -1 * StrCmp(((SORTLINE *)s1)->pwszLine, ((SORTLINE *)s2)->pwszLine);
}

int CmpLogical(const void *s1, const void *s2)
{
  int cmp = (int)pfnStrCmpLogicalW(((SORTLINE *)s1)->pwszSortEntry, ((SORTLINE *)s2)->pwszSortEntry);
  if (cmp == 0)
  {
    cmp = (int)pfnStrCmpLogicalW(((SORTLINE *)s1)->pwszLine, ((SORTLINE *)s2)->pwszLine);
  }
  if (cmp)
    return cmp;
  else
  {
    cmp = StrCmp(((SORTLINE *)s1)->pwszSortEntry, ((SORTLINE *)s2)->pwszSortEntry);
    return (cmp) ? cmp : StrCmp(((SORTLINE *)s1)->pwszLine, ((SORTLINE *)s2)->pwszLine);
  }
}

int CmpLogicalRev(const void *s1, const void *s2)
{
  int cmp = -1 * (int)pfnStrCmpLogicalW(((SORTLINE *)s1)->pwszSortEntry, ((SORTLINE *)s2)->pwszSortEntry);
  if (cmp == 0)
  {
    cmp = -1 * (int)pfnStrCmpLogicalW(((SORTLINE *)s1)->pwszLine, ((SORTLINE *)s2)->pwszLine);
  }
  if (cmp)
    return cmp;
  else
  {
    cmp = -1 * StrCmp(((SORTLINE *)s1)->pwszSortEntry, ((SORTLINE *)s2)->pwszSortEntry);
    return (cmp) ? cmp : -1 * StrCmp(((SORTLINE *)s1)->pwszLine, ((SORTLINE *)s2)->pwszLine);
  }
}

void EditSortLines(HWND hwnd, int iSortFlags)
{
  int iCurPos;
  int iAnchorPos;
  int iSelStart;
  int iSelEnd;
  int iLineStart;
  int iLineEnd;
  int iLineCount;

  BOOL bIsRectangular = FALSE;
  int iRcCurLine;
  int iRcAnchorLine;
  int iRcCurCol;
  int iRcAnchorCol;

  int  i, iLine;
  int  cchTotal = 0;
  int  ichlMax = 3;

  SORTLINE *pLines;
  char  *pmszResult;
  char  *pmszBuf;

  UINT uCodePage;
  DWORD cEOLMode;
  char mszEOL[] = "\r\n";

  UINT iTabWidth;
  UINT iSortColumn;

  BOOL bLastDup = FALSE;
  FNSTRCMP pfnStrCmp;

  pfnStrCmpLogicalW = GetProcAddress(GetModuleHandle(L"shlwapi"), "StrCmpLogicalW");
  pfnStrCmp = (iSortFlags & SORT_NOCASE) ? StrCmpIW : StrCmpW;

  iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  // [2e]: Alt+O: sort all on no selection #133
  if ((iCurPos != iAnchorPos) && (SC_SEL_RECTANGLE == SendMessage(hwnd, SCI_GETSELECTIONMODE, 0, 0)))
  {

    iRcCurLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iCurPos, 0);
    iRcAnchorLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iAnchorPos, 0);

    iRcCurCol = (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iCurPos, 0);
    iRcAnchorCol = (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iAnchorPos, 0);

    bIsRectangular = TRUE;

    iLineStart = min(iRcCurLine, iRcAnchorLine);
    iLineEnd = max(iRcCurLine, iRcAnchorLine);

    iSortColumn = min(iRcCurCol, iRcAnchorCol);
  }

  else
  {

    iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

    // [2e]: Alt+O: sort all on no selection #133
    if (iSelStart == iSelEnd)
    {
      iSelStart = 0;
      iSelEnd = SciCall_GetLength();
    }
    // [/2e]

    iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
    iSelStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);

    iLineStart = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
    iLineEnd = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelEnd, 0);

    if (iSelEnd <= SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd, 0))
      iLineEnd--;

    iSortColumn = (UINT)SendMessage(hwnd, SCI_GETCOLUMN,
                                    (WPARAM)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0), 0);
  }

  iLineCount = iLineEnd - iLineStart + 1;
  if (iLineCount < 2)
    return;

  uCodePage = (SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8) ? CP_UTF8 : CP_ACP;

  cEOLMode = (DWORD)SendMessage(hwnd, SCI_GETEOLMODE, 0, 0);
  if (cEOLMode == SC_EOL_CR)
  {
    mszEOL[1] = 0;
  }
  else if (cEOLMode == SC_EOL_LF)
  {
    mszEOL[0] = '\n';
    mszEOL[1] = 0;
  }

  iTabWidth = (UINT)SendMessage(hwnd, SCI_GETTABWIDTH, 0, 0);

  SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
  if (bIsRectangular)
    EditPadWithSpaces(hwnd, !(iSortFlags & SORT_SHUFFLE), TRUE);

  pLines = LocalAlloc(LPTR, sizeof(SORTLINE) * iLineCount);
  i = 0;
  for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
  {

    char *pmsz;
    int cchw;
    int cchm = (int)SendMessage(hwnd, SCI_GETLINE, (WPARAM)iLine, 0);

    pmsz = LocalAlloc(LPTR, cchm + 1);
    SendMessage(hwnd, SCI_GETLINE, (WPARAM)iLine, (LPARAM)pmsz);
    StrTrimA(pmsz, "\r\n");
    cchTotal += cchm;
    ichlMax = max(ichlMax, cchm);

    cchw = MultiByteToWideChar(uCodePage, 0, pmsz, -1, NULL, 0) - 1;
    if (cchw > 0)
    {
      UINT col = 0, tabs = iTabWidth;
      pLines[i].pwszLine = LocalAlloc(LPTR, sizeof(WCHAR) * (cchw + 1));
      MultiByteToWideChar(uCodePage, 0, pmsz, -1, pLines[i].pwszLine, (int)LocalSize(pLines[i].pwszLine) / sizeof(WCHAR));
      pLines[i].pwszSortEntry = pLines[i].pwszLine;
      if (iSortFlags & SORT_COLUMN)
      {
        while (*(pLines[i].pwszSortEntry))
        {
          if (*(pLines[i].pwszSortEntry) == L'\t')
          {
            if (col + tabs <= iSortColumn)
            {
              col += tabs;
              tabs = iTabWidth;
              pLines[i].pwszSortEntry = CharNext(pLines[i].pwszSortEntry);
            }
            else
              break;
          }
          else if (col < iSortColumn)
          {
            col++;
            if (--tabs == 0)
              tabs = iTabWidth;
            pLines[i].pwszSortEntry = CharNext(pLines[i].pwszSortEntry);
          }
          else
            break;
        }
      }
    }
    else
    {
      pLines[i].pwszLine = StrDup(L"");
      pLines[i].pwszSortEntry = pLines[i].pwszLine;
    }
    LocalFree(pmsz);
    i++;
  }
  if (iSortFlags & SORT_DESCENDING)
  {
    if (iSortFlags & SORT_LOGICAL && pfnStrCmpLogicalW)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpLogicalRev);
    else
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStdRev);
  }
  else if (iSortFlags & SORT_SHUFFLE)
  {
    srand((UINT)GetTickCount());
    for (i = iLineCount - 1; i > 0; i--)
    {
      int j = rand() % i;
      SORTLINE sLine;
      sLine.pwszLine = pLines[i].pwszLine;
      sLine.pwszSortEntry = pLines[i].pwszSortEntry;
      pLines[i] = pLines[j];
      pLines[j].pwszLine = sLine.pwszLine;
      pLines[j].pwszSortEntry = sLine.pwszSortEntry;
    }
  }
  else
  {
    if (iSortFlags & SORT_LOGICAL && pfnStrCmpLogicalW)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpLogical);
    else
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStd);
  }

  pmszResult = LocalAlloc(LPTR, cchTotal + 2 * iLineCount + 1);
  pmszBuf = LocalAlloc(LPTR, ichlMax + 1);

  for (i = 0; i < iLineCount; i++)
  {
    BOOL bDropLine = FALSE;
    if (pLines[i].pwszLine && ((iSortFlags & SORT_SHUFFLE) || lstrlen(pLines[i].pwszLine)))
    {
      if (!(iSortFlags & SORT_SHUFFLE))
      {
        if (iSortFlags & SORT_MERGEDUP || iSortFlags & SORT_UNIQDUP || iSortFlags & SORT_UNIQUNIQ)
        {
          if (i < iLineCount - 1)
          {
            if (pfnStrCmp(pLines[i].pwszLine, pLines[i + 1].pwszLine) == 0)
            {
              bLastDup = TRUE;
              bDropLine = (iSortFlags & SORT_MERGEDUP || iSortFlags & SORT_UNIQDUP);
            }
            else
            {
              bDropLine = (!bLastDup && (iSortFlags & SORT_UNIQUNIQ)) || (bLastDup && (iSortFlags & SORT_UNIQDUP));
              bLastDup = FALSE;
            }
          }
          else
          {
            bDropLine = (!bLastDup && (iSortFlags & SORT_UNIQUNIQ)) || (bLastDup && (iSortFlags & SORT_UNIQDUP));
            bLastDup = FALSE;
          }
        }
      }
      if (!bDropLine)
      {
        WideCharToMultiByte(uCodePage, 0, pLines[i].pwszLine, -1, pmszBuf, (int)LocalSize(pmszBuf), NULL, NULL);
        StrCatBuffA(pmszResult, pmszBuf, (int)LocalSize(pmszResult));
        StrCatBuffA(pmszResult, mszEOL, (int)LocalSize(pmszResult));
      }
    }
  }

  LocalFree(pmszBuf);

  for (i = 0; i < iLineCount; i++)
  {
    if (pLines[i].pwszLine)
      LocalFree(pLines[i].pwszLine);
  }
  LocalFree(pLines);

  if (!bIsRectangular)
  {
    if (iAnchorPos > iCurPos)
    {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + lstrlenA(pmszResult);
    }
    else
    {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + lstrlenA(pmszResult);
    }
  }

  SendMessage(hwnd, SCI_SETTARGETSTART, (WPARAM)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineStart, 0), 0);
  SendMessage(hwnd, SCI_SETTARGETEND, (WPARAM)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLineEnd + 1, 0), 0);
  SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)lstrlenA(pmszResult), (LPARAM)pmszResult);
  SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

  LocalFree(pmszResult);

  if (!bIsRectangular)
    SendMessage(hwnd, SCI_SETSEL, (WPARAM)iAnchorPos, (LPARAM)iCurPos);

  else
  {
    int iTargetStart = (int)SendMessage(hwnd, SCI_GETTARGETSTART, 0, 0);
    int iTargetEnd = (int)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0);
    SendMessage(hwnd, SCI_CLEARSELECTIONS, 0, 0);
    if (iTargetStart != iTargetEnd)
    {
      iTargetEnd -= lstrlenA(mszEOL);
      if (iRcAnchorLine > iRcCurLine)
      {
        iCurPos = (int)SendMessage(hwnd, SCI_FINDCOLUMN,
                                   (WPARAM)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iTargetStart, 0), (LPARAM)iRcCurCol);
        iAnchorPos = (int)SendMessage(hwnd, SCI_FINDCOLUMN,
                                      (WPARAM)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iTargetEnd, 0), (LPARAM)iRcAnchorCol);
      }
      else
      {
        iCurPos = (int)SendMessage(hwnd, SCI_FINDCOLUMN,
                                   (WPARAM)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iTargetEnd, 0), (LPARAM)iRcCurCol);
        iAnchorPos = (int)SendMessage(hwnd, SCI_FINDCOLUMN,
                                      (WPARAM)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iTargetStart, 0), (LPARAM)iRcAnchorCol);
      }
      if (iCurPos != iAnchorPos)
      {
        SendMessage(hwnd, SCI_SETRECTANGULARSELECTIONCARET, (WPARAM)iCurPos, 0);
        SendMessage(hwnd, SCI_SETRECTANGULARSELECTIONANCHOR, (WPARAM)iAnchorPos, 0);
      }
      else
        SendMessage(hwnd, SCI_SETSEL, (WPARAM)iTargetStart, (LPARAM)iTargetStart);
    }
    else
      SendMessage(hwnd, SCI_SETSEL, (WPARAM)iTargetStart, (LPARAM)iTargetStart);
  }
}


//=============================================================================
//
//  EditJumpTo()
//
void EditJumpTo(HWND hwnd, int iNewLine, int iNewCol)
{
  int iMaxLine = (int)SendMessage(hwnd, SCI_GETLINECOUNT, 0, 0);

  // Jumpt to end with line set to -1
  if (iNewLine == -1)
  {
    SendMessage(hwnd, SCI_DOCUMENTEND, 0, 0);
    return;
  }

  // Line maximum is iMaxLine
  iNewLine = min(iNewLine, iMaxLine);

  // Column minimum is 1
  iNewCol = max(iNewCol, 1);

  if (iNewLine > 0 && iNewLine <= iMaxLine && iNewCol > 0)
  {
    int iNewPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iNewLine - 1, 0);
    int iLineEndPos = (int)SendMessage(hwnd, SCI_GETLINEENDPOSITION, (WPARAM)iNewLine - 1, 0);
    while (iNewCol - 1 > SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iNewPos, 0))
    {
      if (iNewPos >= iLineEndPos)
        break;

      iNewPos = (int)SendMessage(hwnd, SCI_POSITIONAFTER, (WPARAM)iNewPos, 0);
    }

    SendMessage(hwnd, SCI_SETXCARETPOLICY, CARET_SLOP | CARET_STRICT | CARET_EVEN, 50);
    SendMessage(hwnd, SCI_SETYCARETPOLICY, CARET_SLOP | CARET_STRICT | CARET_EVEN, 5);

    iNewPos = min(iNewPos, iLineEndPos);
    SendMessage(hwnd, SCI_GOTOPOS, (WPARAM)iNewPos, 0);
    SendMessage(hwnd, SCI_CHOOSECARETX, 0, 0);

    SendMessage(hwnd, SCI_SETXCARETPOLICY, CARET_SLOP | CARET_EVEN, 50);
    SendMessage(hwnd, SCI_SETYCARETPOLICY, CARET_EVEN, 0);
  }
}


//=============================================================================
//
//  EditSelectEx()
//
void EditSelectEx(HWND hwnd, int iAnchorPos, int iCurrentPos)
{
  SendMessage(hwnd, SCI_SETXCARETPOLICY, CARET_SLOP | CARET_STRICT | CARET_EVEN, 50);
  // [2e]: ScrollYCaretPolicy ini-option
  const int linesOnScreen = SendMessage(hwnd, SCI_LINESONSCREEN, 0, 0);
  int yCaretSlop = 5;
  switch (iScrollYCaretPolicy)
  {
    case SCP_LEGACY:
    default:
      // legacy Notepad2 behavior
      break;
    case SCP_THIRD:
      yCaretSlop = linesOnScreen / 3;
      break;
    case SCP_HALF:
      yCaretSlop = linesOnScreen / 2;
      break;
  }
  // [/2e]
  SendMessage(hwnd, SCI_SETYCARETPOLICY, CARET_SLOP | CARET_STRICT | CARET_EVEN, yCaretSlop);
  SendMessage(hwnd, SCI_SETSEL, iAnchorPos, iCurrentPos);
  SendMessage(hwnd, SCI_SETXCARETPOLICY, CARET_SLOP | CARET_EVEN, 50);
  SendMessage(hwnd, SCI_SETYCARETPOLICY, CARET_EVEN, 0);
}


//=============================================================================
//
//  EditFixPositions()
//
void EditFixPositions(HWND hwnd)
{
  int iMaxPos = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
  int iCurrentPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurrentPos > 0 && iCurrentPos < iMaxPos)
  {
    int iNewPos = (int)SendMessage(hwnd, SCI_POSITIONAFTER, (WPARAM)
                                   (int)SendMessage(hwnd, SCI_POSITIONBEFORE, (WPARAM)iCurrentPos, 0), 0);
    if (iNewPos != iCurrentPos)
    {
      SendMessage(hwnd, SCI_SETCURRENTPOS, (WPARAM)iNewPos, 0);
      iCurrentPos = iNewPos;
    }
  }
  if (iAnchorPos != iCurrentPos && iAnchorPos > 0 && iAnchorPos < iMaxPos)
  {
    int iNewPos = (int)SendMessage(hwnd, SCI_POSITIONAFTER, (WPARAM)
                                   (int)SendMessage(hwnd, SCI_POSITIONBEFORE, (WPARAM)iAnchorPos, 0), 0);
    if (iNewPos != iAnchorPos)
      SendMessage(hwnd, SCI_SETANCHOR, (WPARAM)iNewPos, 0);
  }
}


//=============================================================================
//
//  EditEnsureSelectionVisible()
//
void EditEnsureSelectionVisible(HWND hwnd)
{
  int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);
  int iCurrentPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  SendMessage(hwnd, SCI_ENSUREVISIBLE,
              (WPARAM)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iAnchorPos, 0), 0);
  if (iAnchorPos != iCurrentPos)
    SendMessage(hwnd, SCI_ENSUREVISIBLE,
                (WPARAM)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iCurrentPos, 0), 0);

  EditSelectEx(hwnd, iAnchorPos, iCurrentPos);
}


//=============================================================================
//
//  EditGetExcerpt()
//
void EditGetExcerpt(HWND hwnd, LPWSTR lpszExcerpt, DWORD cchExcerpt)
{
  WCHAR tch[256] = L"";
  WCHAR *p;
  DWORD cch = 0;
  UINT cpEdit;
  struct TextRange tr;
  char*  pszText;
  LPWSTR pszTextW;

  int iCurPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  if (iCurPos == iAnchorPos || SC_SEL_RECTANGLE == SendMessage(hwnd, SCI_GETSELECTIONMODE, 0, 0))
  {
    lstrcpy(lpszExcerpt, L"");
    return;
  }

  tr.chrg.cpMin = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  tr.chrg.cpMax = min((int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0), (LONG)(tr.chrg.cpMin + COUNTOF(tch)));
  tr.chrg.cpMax = min((int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0), tr.chrg.cpMax);

  pszText = LocalAlloc(LPTR, (tr.chrg.cpMax - tr.chrg.cpMin) + 2);
  pszTextW = LocalAlloc(LPTR, ((tr.chrg.cpMax - tr.chrg.cpMin) * 2) + 2);

  if (pszText && pszTextW)
  {
    tr.lpstrText = pszText;
    SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
    cpEdit = (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0);
    MultiByteToWideChar(cpEdit, 0, pszText, tr.chrg.cpMax - tr.chrg.cpMin, pszTextW, (int)GlobalSize(pszTextW) / sizeof(WCHAR));
    for (p = pszTextW; *p && cch < COUNTOF(tch) - 1; p++)
    {
      if (*p == L'\r' || *p == L'\n' || *p == L'\t' || *p == L' ')
      {
        tch[cch++] = L' ';
        while (*(p + 1) == L'\r' || *(p + 1) == L'\n' || *(p + 1) == L'\t' || *(p + 1) == L' ')
          p++;
      }
      else
        tch[cch++] = *p;
    }
    tch[cch++] = L'\0';
    StrTrim(tch, L" ");
  }

  if (cch == 1)
    lstrcpy(tch, L" ... ");

  if (cch > cchExcerpt)
  {
    tch[cchExcerpt - 2] = L'.';
    tch[cchExcerpt - 3] = L'.';
    tch[cchExcerpt - 4] = L'.';
  }
  StrCpyN(lpszExcerpt, tch, cchExcerpt);

  if (pszText)
    LocalFree(pszText);
  if (pszTextW)
    LocalFree(pszTextW);
}

//=============================================================================
//
//  EditFindReplaceDlgProcW()
//
INT_PTR CALLBACK EditFindReplaceDlgProcW(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  LPEDITFINDREPLACE lpefr;
  int i;
  WCHAR tch[512 + 32];
  BOOL bCloseDlg;
  BOOL bIsFindDlg;

  static UINT uCPEdit;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {

        DPI_INIT();

        int cchSelection;
        char *lpszSelection;
        char *lpsz;

        WCHAR tch[128];
        HMENU hmenu;

        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        lpefr = (LPEDITFINDREPLACE)lParam;

        // Get the current code page for Unicode conversion
        uCPEdit = (UINT)SendMessage(lpefr->hwnd, SCI_GETCODEPAGE, 0, 0);

        // Load MRUs
        for (i = 0; i < MRU_Enum(mruFind, 0, NULL, 0); i++)
        {
          MRU_Enum(mruFind, i, tch, COUNTOF(tch));
          SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_ADDSTRING, 0, (LPARAM)tch);
        }
        for (i = 0; i < MRU_Enum(mruReplace, 0, NULL, 0); i++)
        {
          MRU_Enum(mruReplace, i, tch, COUNTOF(tch));
          SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_ADDSTRING, 0, (LPARAM)tch);
        }

        if (!bSwitchedFindReplace)
        {
          cchSelection = (int)SendMessage(lpefr->hwnd, SCI_GETSELECTIONEND, 0, 0) -
                         (int)SendMessage(lpefr->hwnd, SCI_GETSELECTIONSTART, 0, 0);

          if (cchSelection <= 500)
          {
            cchSelection = (int)SendMessage(lpefr->hwnd, SCI_GETSELTEXT, 0, 0);
            lpszSelection = GlobalAlloc(GPTR, cchSelection + 2);
            SendMessage(lpefr->hwnd, SCI_GETSELTEXT, 0, (LPARAM)lpszSelection);

            // Check lpszSelection and truncate bad chars
            lpsz = StrChrA(lpszSelection, 13);
            if (lpsz)
              *lpsz = '\0';

            lpsz = StrChrA(lpszSelection, 10);
            if (lpsz)
              *lpsz = '\0';

            lpsz = StrChrA(lpszSelection, 9);
            if (lpsz)
              *lpsz = '\0';

            SetDlgItemTextA2W(uCPEdit, hwnd, IDC_FINDTEXT, lpszSelection);
            GlobalFree(lpszSelection);
          }
        }

        SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_LIMITTEXT, 500, 0);
        SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_SETEXTENDEDUI, TRUE, 0);

        if (!GetWindowTextLengthW(GetDlgItem(hwnd, IDC_FINDTEXT)))
          SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_FINDTEXT, lpefr->szFindUTF8);

        if (GetDlgItem(hwnd, IDC_REPLACETEXT))
        {
          SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_LIMITTEXT, 500, 0);
          SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_SETEXTENDEDUI, TRUE, 0);
          SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_REPLACETEXT, lpefr->szReplaceUTF8);
        }

        if (lpefr->fuFlags & SCFIND_MATCHCASE)
          CheckDlgButton(hwnd, IDC_FINDCASE, BST_CHECKED);

        if (lpefr->fuFlags & SCFIND_WHOLEWORD)
          CheckDlgButton(hwnd, IDC_FINDWORD, BST_CHECKED);

        if (lpefr->fuFlags & SCFIND_WORDSTART)
          CheckDlgButton(hwnd, IDC_FINDSTART, BST_CHECKED);

        if (lpefr->fuFlags & SCFIND_REGEXP)
          CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_CHECKED);

        if (lpefr->bTransformBS)
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);

        if (lpefr->bNoFindWrap)
          CheckDlgButton(hwnd, IDC_NOWRAP, BST_CHECKED);

        if (GetDlgItem(hwnd, IDC_REPLACE))
        {
          if (bSwitchedFindReplace)
          {
            if (lpefr->bFindClose)
              CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
          }
          else
          {
            if (lpefr->bReplaceClose)
              CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
          }
        }
        else
        {
          if (bSwitchedFindReplace)
          {
            if (lpefr->bReplaceClose)
              CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
          }
          else
          {
            if (lpefr->bFindClose)
              CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
          }
        }

        if (!bSwitchedFindReplace)
        {
          if (xFindReplaceDlg == 0 || yFindReplaceDlg == 0)
            CenterDlgInParent(hwnd);
          else
            SetDlgPos(hwnd, xFindReplaceDlg, yFindReplaceDlg);
        }
        else
        {
          SetDlgPos(hwnd, xFindReplaceDlgSave, yFindReplaceDlgSave);
          bSwitchedFindReplace = FALSE;
          CopyMemory(lpefr, &efrSave, sizeof(EDITFINDREPLACE));
        }

        n2e_SaveCheckboxes(hwnd);
        n2e_EditFindReplaceInitialUpdateCheckboxes(hwnd);

        hmenu = GetSystemMenu(hwnd, FALSE);
        GetString(SC_SAVEPOS, tch, COUNTOF(tch));
        InsertMenu(hmenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, SC_SAVEPOS, tch);
        GetString(SC_RESETPOS, tch, COUNTOF(tch));
        InsertMenu(hmenu, 1, MF_BYPOSITION | MF_STRING | MF_ENABLED, SC_RESETPOS, tch);
        InsertMenu(hmenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        // [2e]: Match indicator
        n2e_ResetFindIcon();
        // [2e]: Ctrl+H: Replace input behaviour #121
        n2e_SubclassFindEditInCombo(hwnd, IDC_FINDTEXT);
        n2e_SubclassFindEditInCombo(hwnd, IDC_REPLACETEXT);
        // [/2e]
      }
      return TRUE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        case IDC_FINDTEXT:
        case IDC_REPLACETEXT: {
            BOOL bEnable = (GetWindowTextLengthW(GetDlgItem(hwnd, IDC_FINDTEXT)) ||
                            CB_ERR != SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_GETCURSEL, 0, 0));
            // [2e]: disable search for invalid regex values
            if (bEnable && (IsDlgButtonChecked(hwnd, IDC_FINDREGEXP) == BST_CHECKED))
            {
              char szFind[512], szReplace[512];
              GetDlgItemTextA2W(uCPEdit, hwnd, IDC_FINDTEXT, szFind, COUNTOF(szFind));
              // [2e]: insert empty groups to make any back-references (\1..\9) valid to pass regexp check #145
              #define FAKE_REGEXP_GROUPS "()()()()()()()()()"
              strcpy(szReplace, FAKE_REGEXP_GROUPS);
              LPSTR szReplaceTail = szReplace + COUNTOF(FAKE_REGEXP_GROUPS) - 1;
              const int szReplaceTailSize = COUNTOF(szReplace) - COUNTOF(FAKE_REGEXP_GROUPS) - 1;
              GetDlgItemTextA2W(uCPEdit, hwnd, IDC_REPLACETEXT, szReplaceTail, szReplaceTailSize);
              bEnable = (n2e_isValidRegex(szFind) != 0) && (n2e_isValidRegex(szReplace) != 0);
            }
            // [/2e]

            EnableWindow(GetDlgItem(hwnd, IDOK), bEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_FINDPREV), bEnable);
            EnableWindow(GetDlgItem(hwnd, ID_GREP), bEnable);
            EnableWindow(GetDlgItem(hwnd, ID_UNGREP), bEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_REPLACE), bEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_REPLACEALL), bEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_REPLACEINSEL), bEnable);

            if (HIWORD(wParam) == CBN_CLOSEUP)
            {
              LONG lSelEnd;
              SendDlgItemMessage(hwnd, LOWORD(wParam), CB_GETEDITSEL, 0, (LPARAM)&lSelEnd);
              SendDlgItemMessage(hwnd, LOWORD(wParam), CB_SETEDITSEL, 0, MAKELPARAM(lSelEnd, lSelEnd));
            }
          }
          break;

        case IDC_FINDREGEXP:
          n2e_EditFindReplaceUpdateCheckboxes(hwnd, IDC_FINDREGEXP);
          PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_FINDTEXT, 1), 0);
          break;

        case IDC_FINDTRANSFORMBS:
          n2e_EditFindReplaceUpdateCheckboxes(hwnd, IDC_FINDTRANSFORMBS);
          PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_FINDTEXT, 1), 0);
          break;

        case IDOK:
        case IDC_FINDPREV:
        case IDC_REPLACE:
        case IDC_REPLACEALL:
        case IDC_REPLACEINSEL:
        case IDACC_SELTONEXT:
        case IDACC_SELTOPREV:
        case IDMSG_SWITCHTOFIND:
        case IDMSG_SWITCHTOREPLACE:
        case IDMSG_SWITCHTOGOTO:
        // [2e]: Grep / Ungrep #29
        case ID_GREP:
        case ID_UNGREP:
        // [/2e]

          lpefr = (LPEDITFINDREPLACE)GetWindowLongPtr(hwnd, DWLP_USER);

          bIsFindDlg = (GetDlgItem(hwnd, IDC_REPLACE) == NULL);

          if ((bIsFindDlg && (LOWORD(wParam) == IDMSG_SWITCHTOREPLACE))
              || (!bIsFindDlg && (LOWORD(wParam) == IDMSG_SWITCHTOFIND))
              || (LOWORD(wParam) == IDMSG_SWITCHTOGOTO))
          {
            GetDlgPos(hwnd, &xFindReplaceDlgSave, &yFindReplaceDlgSave);
            bSwitchedFindReplace = TRUE;
            // [2e]: Match indicator
            n2e_ResetFindIcon();
            CopyMemory(&efrSave, lpefr, sizeof(EDITFINDREPLACE));
          }

          // Get current code page for Unicode conversion
          uCPEdit = (UINT)SendMessage(lpefr->hwnd, SCI_GETCODEPAGE, 0, 0);
          cpLastFind = uCPEdit;

          if (!bSwitchedFindReplace &&
              !GetDlgItemTextA2W(uCPEdit, hwnd, IDC_FINDTEXT, lpefr->szFind, COUNTOF(lpefr->szFind)))
          {

            EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_FINDPREV), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_REPLACE), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_REPLACEALL), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_REPLACEINSEL), FALSE);
            return TRUE;
          }

          if (GetDlgItem(hwnd, IDC_REPLACETEXT))
            GetDlgItemTextA2W(uCPEdit, hwnd, IDC_REPLACETEXT, lpefr->szReplace, COUNTOF(lpefr->szReplace));

          lpefr->fuFlags = 0;

          const BOOL bIsRegexMode = (IsDlgButtonChecked(hwnd, IDC_FINDREGEXP) == BST_CHECKED);
          if (bIsRegexMode)
            lpefr->fuFlags |= SCFIND_REGEXP | SCFIND_POSIX;

          if (IsDlgButtonChecked(hwnd, IDC_FINDCASE) == BST_CHECKED)
            lpefr->fuFlags |= SCFIND_MATCHCASE;

          if (n2e_IsCheckboxChecked(hwnd, IDC_FINDWORD, bIsRegexMode))
            lpefr->fuFlags |= SCFIND_WHOLEWORD;

          if (n2e_IsCheckboxChecked(hwnd, IDC_FINDSTART, bIsRegexMode))
            lpefr->fuFlags |= SCFIND_WORDSTART;

          lpefr->bTransformBS =
            (IsDlgButtonChecked(hwnd, IDC_FINDTRANSFORMBS) == BST_CHECKED) ? TRUE : FALSE;

          lpefr->bNoFindWrap = (IsDlgButtonChecked(hwnd, IDC_NOWRAP) == BST_CHECKED) ? TRUE : FALSE;

          if (bIsFindDlg)
          {
            lpefr->bFindClose = (IsDlgButtonChecked(hwnd, IDC_FINDCLOSE) == BST_CHECKED) ? TRUE : FALSE;
          }
          else
          {
            lpefr->bReplaceClose = (IsDlgButtonChecked(hwnd, IDC_FINDCLOSE) == BST_CHECKED) ? TRUE : FALSE;
          }

          if (!bSwitchedFindReplace)
          {
            // Save MRUs
            if (lstrlenA(lpefr->szFind))
            {
              if (GetDlgItemTextA2W(CP_UTF8, hwnd, IDC_FINDTEXT, lpefr->szFindUTF8, COUNTOF(lpefr->szFindUTF8)))
              {
                GetDlgItemText(hwnd, IDC_FINDTEXT, tch, COUNTOF(tch));
                MRU_Add(mruFind, tch);
              }
            }
            if (lstrlenA(lpefr->szReplace))
            {
              if (GetDlgItemTextA2W(CP_UTF8, hwnd, IDC_REPLACETEXT, lpefr->szReplaceUTF8, COUNTOF(lpefr->szReplaceUTF8)))
              {
                GetDlgItemText(hwnd, IDC_REPLACETEXT, tch, COUNTOF(tch));
                MRU_Add(mruReplace, tch);
              }
            }
            else
              lstrcpyA(lpefr->szReplaceUTF8, "");
          }
          else
          {
            GetDlgItemTextA2W(CP_UTF8, hwnd, IDC_FINDTEXT, lpefr->szFindUTF8, COUNTOF(lpefr->szFindUTF8));
            if (!GetDlgItemTextA2W(CP_UTF8, hwnd, IDC_REPLACETEXT, lpefr->szReplaceUTF8, COUNTOF(lpefr->szReplaceUTF8)))
              lstrcpyA(lpefr->szReplaceUTF8, "");
          }
          
          if (bIsFindDlg)
          {
            bCloseDlg = lpefr->bFindClose;
          }
          else
          {
            if (LOWORD(wParam) == IDOK)
              bCloseDlg = FALSE;
            else
              bCloseDlg = lpefr->bReplaceClose;
          }

          // Reload MRUs
          SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_RESETCONTENT, 0, 0);
          SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_RESETCONTENT, 0, 0);

          for (i = 0; i < MRU_Enum(mruFind, 0, NULL, 0); i++)
          {
            MRU_Enum(mruFind, i, tch, COUNTOF(tch));
            SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_ADDSTRING, 0, (LPARAM)tch);
          }
          for (i = 0; i < MRU_Enum(mruReplace, 0, NULL, 0); i++)
          {
            MRU_Enum(mruReplace, i, tch, COUNTOF(tch));
            SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_ADDSTRING, 0, (LPARAM)tch);
          }

          SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_FINDTEXT, lpefr->szFindUTF8);
          SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_REPLACETEXT, lpefr->szReplaceUTF8);

          if (!bSwitchedFindReplace)
            SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetFocus()), 1);

          switch (LOWORD(wParam))
          {
            case IDOK: // find next
            case IDACC_SELTONEXT:
              if (!bIsFindDlg)
                bReplaceInitialized = TRUE;
              // [2e]: Find/Replace - handle Ctrl+Enter #273
              if (HIBYTE(GetKeyState(VK_CONTROL)))
              {
                int iPos = SciCall_GetCurrentPos();
                int iAnchor = SciCall_GetAnchor();
                SciCall_SetCurrentPos(0);
                SciCall_SetAnchor(0);
                if (!EditFindNext(lpefr->hwnd, lpefr, FALSE))
                {
                  SciCall_SetCurrentPos(iPos);
                  SciCall_SetAnchor(iAnchor);
                  bCloseDlg = FALSE;
                }
              }
              // [/2e]
              else
              {
                bCloseDlg &= EditFindNext(lpefr->hwnd, lpefr, LOWORD(wParam) == IDACC_SELTONEXT);
              }
              break;

            case IDC_FINDPREV: // find previous
            case IDACC_SELTOPREV:
              if (!bIsFindDlg)
                bReplaceInitialized = TRUE;
              bCloseDlg &= EditFindPrev(lpefr->hwnd, lpefr, LOWORD(wParam) == IDACC_SELTOPREV);
              break;

            case IDC_REPLACE:
              bReplaceInitialized = TRUE;
              bCloseDlg &= EditReplace(lpefr->hwnd, lpefr);
              break;

            case IDC_REPLACEALL:
              bReplaceInitialized = TRUE;
              bCloseDlg &= EditReplaceAll(lpefr->hwnd, lpefr, TRUE);
              break;

            case IDC_REPLACEINSEL:
              bReplaceInitialized = TRUE;
              bCloseDlg &= EditReplaceAllInSelection(lpefr->hwnd, lpefr, TRUE);
              break;

            // [2e]: Grep / Ungrep #29
            case ID_GREP:
              bCloseDlg = n2e_Grep(lpefr, TRUE);
              break;

            case ID_UNGREP:
              bCloseDlg = n2e_Grep(lpefr, FALSE);
              break;
            // [/2e]
          }

          if (bCloseDlg)
          {
            // [2e]: Match indicator
            n2e_ResetFindIcon();
            DestroyWindow(hwnd);
            hDlgFindReplace = NULL;
          }
          break;


        case IDCANCEL:
          // [2e]: Match indicator
          n2e_ResetFindIcon();
          DestroyWindow(hwnd);
          break;

        case IDACC_FIND:
          PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_FIND, 1), 0);
          break;

        case IDACC_REPLACE:
          PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_REPLACE, 1), 0);
          break;

        case IDACC_GOTO:
          PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_GOTOLINE, 1), 0);
          break;

        case IDACC_SAVEPOS:
          GetDlgPos(hwnd, &xFindReplaceDlg, &yFindReplaceDlg);
          break;

        case IDACC_RESETPOS:
          CenterDlgInParent(hwnd);
          xFindReplaceDlg = yFindReplaceDlg = 0;
          break;

        case IDACC_FINDNEXT:
          PostMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, 1), 0);
          break;

        case IDACC_FINDPREV:
          PostMessage(hwnd, WM_COMMAND, MAKELONG(IDC_FINDPREV, 1), 0);
          break;

        case IDACC_REPLACENEXT:
          if (GetDlgItem(hwnd, IDC_REPLACE) != NULL)
          {
            PostMessage(hwnd, WM_COMMAND, MAKELONG(IDC_REPLACE, 1), 0);
          }
          break;

        case IDACC_SAVEFIND:
          SendMessage(hwndMain, WM_COMMAND, MAKELONG(IDM_EDIT_SAVEFIND, 1), 0);
          lpefr = (LPEDITFINDREPLACE)GetWindowLongPtr(hwnd, DWLP_USER);
          SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_FINDTEXT, lpefr->szFindUTF8);
          CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_UNCHECKED);
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_FINDTEXT)), 1);
          break;
      }

      return TRUE;


    case WM_SYSCOMMAND:
      if (wParam == SC_SAVEPOS)
      {
        PostMessage(hwnd, WM_COMMAND, MAKELONG(IDACC_SAVEPOS, 0), 0);
        return TRUE;
      }
      else if (wParam == SC_RESETPOS)
      {
        PostMessage(hwnd, WM_COMMAND, MAKELONG(IDACC_RESETPOS, 0), 0);
        return TRUE;
      }
      else
        return FALSE;


    case WM_NOTIFY: {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        switch (pnmhdr->code)
        {
          case NM_CLICK:
          case NM_RETURN:
            switch (pnmhdr->idFrom)
            {
            case IDC_TOGGLEFIND:
              PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_FIND, 1), 0);
              break;
            case IDC_TOGGLEREPLACE:
              PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_REPLACE, 1), 0);
              break;
            case IDC_TOGGLEGOTO:
              PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_GOTOLINE, 1), 0);
              break;
            }
            break;
        }
      }
      break;

  }

  return FALSE;

}


//=============================================================================
//
//  EditFindReplaceDlg()
//
HWND EditFindReplaceDlg(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bReplace)
{

  HWND hDlg;

  lpefr->hwnd = hwnd;

  hDlg = CreateThemedDialogParam(g_hInstance,
                                 (bReplace) ? MAKEINTRESOURCEW(IDD_REPLACE) : MAKEINTRESOURCEW(IDD_FIND),
                                 GetParent(hwnd),
                                 EditFindReplaceDlgProcW,
                                 (LPARAM)lpefr);

  ShowWindow(hDlg, SW_SHOW);

  return hDlg;

}


//=============================================================================
//
//  EditFindNext()
//
BOOL EditFindNext(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL fExtendSelection)
{

  struct TextToFind ttf;
  int iPos;
  int iSelPos, iSelAnchor;
  char szFind2[512];
  BOOL bSuppressNotFound = FALSE;

  if (!lstrlenA(lpefr->szFind))
    return FALSE;

  // [2e]: Boost regex and Cyrillic #162
  if (!n2e_IsFindReplaceAvailable(lpefr))
    return FALSE;

  lstrcpynA(szFind2, lpefr->szFind, COUNTOF(szFind2));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2, (lpefr->fuFlags & SCFIND_REGEXP),
                         (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0));

  if (lstrlenA(szFind2) == 0)
  {
    InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
    return FALSE;
  }

  iSelPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iSelAnchor = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  ZeroMemory(&ttf, sizeof(ttf));

  ttf.chrg.cpMin = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  ttf.chrg.cpMax = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
  ttf.lpstrText = szFind2;

  iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)&ttf);

  const BOOL bTextFound = (iPos >= 0);
  // [2e]: Match indicator
  n2e_UpdateFindIcon(bTextFound && n2e_CheckTextExists(hwnd, lpefr->fuFlags, &ttf, iPos + 1));
  if (!bTextFound && ttf.chrg.cpMin > 0 && !lpefr->bNoFindWrap && !fExtendSelection)
  {
    if (IDOK == InfoBox(MBOKCANCEL, L"MsgFindWrap1", IDS_FIND_WRAPFW))
    {
      ttf.chrg.cpMin = 0;
      iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)&ttf);
      // [2e]: Match indicator
      n2e_UpdateFindIcon((iPos >= 0) && n2e_CheckTextExists(hwnd, lpefr->fuFlags, &ttf, iPos + 1));
    }
    else
    {
      bSuppressNotFound = TRUE;
      // [2e]: Match indicator
      if (!IsWindowVisible(hDlgFindReplace))
      {
        n2e_ResetFindIcon();
      }
      // [/2e]
    }
  }

  if (iPos == -1)
  {
    // notfound
    if (!bSuppressNotFound)
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
    return FALSE;
  }

  if (!fExtendSelection)
    EditSelectEx(hwnd, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
  else
    EditSelectEx(hwnd, min(iSelAnchor, iSelPos), ttf.chrgText.cpMax);

  return TRUE;

}


//=============================================================================
//
//  EditFindPrev()
//
BOOL EditFindPrev(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL fExtendSelection)
{

  struct TextToFind ttf;
  int iPos;
  int iSelPos, iSelAnchor;
  int iLength;
  char szFind2[512];
  BOOL bSuppressNotFound = FALSE;

  if (!lstrlenA(lpefr->szFind))
    return FALSE;

  // [2e]: Boost regex and Cyrillic #162
  if (!n2e_IsFindReplaceAvailable(lpefr))
    return FALSE;

  lstrcpynA(szFind2, lpefr->szFind, COUNTOF(szFind2));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2, (lpefr->fuFlags & SCFIND_REGEXP),
                         (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0));

  if (lstrlenA(szFind2) == 0)
  {
    InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
    return FALSE;
  }

  iSelPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  iSelAnchor = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);

  ZeroMemory(&ttf, sizeof(ttf));

  ttf.chrg.cpMin = max(0, (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0));
  ttf.chrg.cpMax = 0;
  ttf.lpstrText = szFind2;

  iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)&ttf);

  const BOOL bTextFound = (iPos >= 0);
  // [2e]: Match indicator
  n2e_UpdateFindIcon(bTextFound && n2e_CheckTextExists(hwnd, lpefr->fuFlags, &ttf, iPos - 1));
  iLength = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
  if (!bTextFound && ttf.chrg.cpMin < iLength && !lpefr->bNoFindWrap && !fExtendSelection)
  {
    if (IDOK == InfoBox(MBOKCANCEL, L"MsgFindWrap2", IDS_FIND_WRAPRE))
    {
      ttf.chrg.cpMin = iLength;
      iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)&ttf);
      // [2e]: Match indicator
      n2e_UpdateFindIcon((iPos >= 0) && n2e_CheckTextExists(hwnd, lpefr->fuFlags, &ttf, iPos - 1));
    }
    else
    {
      bSuppressNotFound = TRUE;
      // [2e]: Match indicator
      if (!IsWindowVisible(hDlgFindReplace))
      {
        n2e_ResetFindIcon();
      }
      // [/2e]
    }
  }

  if (iPos == -1)
  {
    // notfound
    if (!bSuppressNotFound)
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
    return FALSE;
  }

  if (!fExtendSelection)
    EditSelectEx(hwnd, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
  else
    EditSelectEx(hwnd, max(iSelPos, iSelAnchor), ttf.chrgText.cpMin);

  return TRUE;

}


//=============================================================================
//
//  EditReplace()
//
BOOL EditReplace(HWND hwnd, LPCEDITFINDREPLACE lpefr)
{

  struct TextToFind ttf;
  int iPos;
  int iSelStart;
  int iSelEnd;
  int iReplaceMsg = (lpefr->fuFlags & SCFIND_REGEXP) ? SCI_REPLACETARGETRE : SCI_REPLACETARGET;
  char szFind2[512];
  char *pszReplace2;
  BOOL bSuppressNotFound = FALSE;

  if (!lstrlenA(lpefr->szFind))
    return FALSE;

  // [2e]: ICU build: missing regexp warnings #232
  if (!n2e_IsFindReplaceAvailable(lpefr))
    return FALSE;

  lstrcpynA(szFind2, lpefr->szFind, COUNTOF(szFind2));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2, (lpefr->fuFlags & SCFIND_REGEXP),
                         (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0));

  if (lstrlenA(szFind2) == 0)
  {
    InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
    return FALSE;
  }

  if (lstrcmpA(lpefr->szReplace, "^c") == 0)
  {
    iReplaceMsg = SCI_REPLACETARGET;
    pszReplace2 = EditGetClipboardText(hwnd);
  }
  else
  {
    pszReplace2 = StrDupA(lpefr->szReplace);
    if (lpefr->bTransformBS)
      TransformBackslashes(pszReplace2, (lpefr->fuFlags & SCFIND_REGEXP),
                           (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0));
  }

  if (!pszReplace2)
    pszReplace2 = StrDupA("");

  iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

  ZeroMemory(&ttf, sizeof(ttf));

  ttf.chrg.cpMin = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0); // Start!
  ttf.chrg.cpMax = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
  ttf.lpstrText = szFind2;

  iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)&ttf);

  if (iPos == -1 && ttf.chrg.cpMin > 0 && !lpefr->bNoFindWrap)
  {
    if (IDOK == InfoBox(MBOKCANCEL, L"MsgFindWrap1", IDS_FIND_WRAPFW))
    {
      ttf.chrg.cpMin = 0;
      iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)&ttf);
    }
    else
      bSuppressNotFound = TRUE;
  }

  if (iPos == -1)
  {
    // notfound
    LocalFree(pszReplace2);
    if (!bSuppressNotFound)
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
    return FALSE;
  }

  if (iSelStart != ttf.chrgText.cpMin || iSelEnd != ttf.chrgText.cpMax)
  {
    LocalFree(pszReplace2);
    EditSelectEx(hwnd, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
    return FALSE;
  }

  SendMessage(hwnd, SCI_SETTARGETSTART, ttf.chrgText.cpMin, 0);
  SendMessage(hwnd, SCI_SETTARGETEND, ttf.chrgText.cpMax, 0);
  SendMessage(hwnd, iReplaceMsg, (WPARAM)-1, (LPARAM)pszReplace2);

  ttf.chrg.cpMin = (int)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0);
  ttf.chrg.cpMax = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);

  iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)&ttf);

  bSuppressNotFound = FALSE;
  if (iPos == -1 && ttf.chrg.cpMin > 0 && !lpefr->bNoFindWrap)
  {
    if (IDOK == InfoBox(MBOKCANCEL, L"MsgFindWrap1", IDS_FIND_WRAPFW))
    {
      ttf.chrg.cpMin = 0;
      iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)&ttf);
    }
    else
      bSuppressNotFound = TRUE;
  }

  // [2e]: Gutter not updated on Replace #206
  UpdateLineNumberWidth();
  if (iPos != -1)
    EditSelectEx(hwnd, ttf.chrgText.cpMin, ttf.chrgText.cpMax);

  else
  {
    EditSelectEx(hwnd,
                 (int)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0),
                 (int)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0));
    if (!bSuppressNotFound)
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
  }

  LocalFree(pszReplace2);
  return TRUE;

}


//=============================================================================
//
//  EditReplaceAll()
//
BOOL EditReplaceAll(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bShowInfo)
{

  struct TextToFind ttf;
  int iPos;
  int iCount = 0;
  int iReplaceMsg = (lpefr->fuFlags & SCFIND_REGEXP) ? SCI_REPLACETARGETRE : SCI_REPLACETARGET;
  char szFind2[512];
  char *pszReplace2;
  BOOL bRegexStartOfLine;
  BOOL bRegexStartOrEndOfLine;

  if (!lstrlenA(lpefr->szFind))
    return FALSE;

  // [2e]: Boost regex and Cyrillic #162
  if (!n2e_IsFindReplaceAvailable(lpefr))
    return FALSE;

  // Show wait cursor...
  BeginWaitCursor();

  lstrcpynA(szFind2, lpefr->szFind, COUNTOF(szFind2));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2, (lpefr->fuFlags & SCFIND_REGEXP),
                         (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0));

  if (lstrlenA(szFind2) == 0)
  {
    InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
    return FALSE;
  }

  bRegexStartOfLine =
    (szFind2[0] == '^');
  bRegexStartOrEndOfLine =
    (lpefr->fuFlags & SCFIND_REGEXP &&
    (!lstrcmpA(szFind2, "$") || !lstrcmpA(szFind2, "^") || !lstrcmpA(szFind2, "^$")));

  if (lstrcmpA(lpefr->szReplace, "^c") == 0)
  {
    iReplaceMsg = SCI_REPLACETARGET;
    pszReplace2 = EditGetClipboardText(hwnd);
  }
  else
  {
    pszReplace2 = StrDupA(lpefr->szReplace);
    if (lpefr->bTransformBS)
      TransformBackslashes(pszReplace2, (lpefr->fuFlags & SCFIND_REGEXP),
                           (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0));
  }

  if (!pszReplace2)
    pszReplace2 = StrDupA("");

  ZeroMemory(&ttf, sizeof(ttf));

  ttf.chrg.cpMin = 0;
  ttf.chrg.cpMax = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
  ttf.lpstrText = szFind2;

  while ((iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)&ttf)) != -1)
  {
    int iReplacedLen;

    if (iCount == 0 && bRegexStartOrEndOfLine)
    {
      if (0 == SendMessage(hwnd, SCI_GETLINEENDPOSITION, 0, 0))
      {
        iPos = 0;
        ttf.chrgText.cpMin = 0;
        ttf.chrgText.cpMax = 0;
      }
    }

    if (++iCount == 1)
    {
      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
    }
    SendMessage(hwnd, SCI_SETTARGETSTART, ttf.chrgText.cpMin, 0);
    SendMessage(hwnd, SCI_SETTARGETEND, ttf.chrgText.cpMax, 0);
    iReplacedLen = (int)SendMessage(hwnd, iReplaceMsg, (WPARAM)-1, (LPARAM)pszReplace2);
    ttf.chrg.cpMin = ttf.chrgText.cpMin + iReplacedLen;
    ttf.chrg.cpMax = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
    if (ttf.chrg.cpMin == ttf.chrg.cpMax)
      break;

    if (ttf.chrgText.cpMin == ttf.chrgText.cpMax &&
        !(bRegexStartOrEndOfLine && iReplacedLen > 0))
      ttf.chrg.cpMin = (int)SendMessage(hwnd, SCI_POSITIONAFTER, ttf.chrg.cpMin, 0);
    if (bRegexStartOfLine)
    {
      int iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)ttf.chrg.cpMin, 0);
      int ilPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
      if (ilPos == ttf.chrg.cpMin)
        ttf.chrg.cpMin = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine + 1, 0);
      if (ttf.chrg.cpMin == ttf.chrg.cpMax)
        break;
    }
  }

  if (iCount)
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);

  // [2e]: Gutter not updated on Replace #206
  UpdateLineNumberWidth();
  // Remove wait cursor
  EndWaitCursor();

  if (bShowInfo)
  {
    if (iCount > 0)
      InfoBox(0, L"MsgReplaceCount", IDS_REPLCOUNT, iCount);
    else
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
  }

  LocalFree(pszReplace2);
  return TRUE;

}


//=============================================================================
//
//  EditReplaceAllInSelection()
//
BOOL EditReplaceAllInSelection(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bShowInfo)
{

  struct TextToFind ttf;
  int iPos;
  int iCount = 0;
  int iReplaceMsg = (lpefr->fuFlags & SCFIND_REGEXP) ? SCI_REPLACETARGETRE : SCI_REPLACETARGET;
  BOOL fCancel = FALSE;
  char szFind2[512];
  char *pszReplace2;
  BOOL bRegexStartOfLine;
  BOOL bRegexStartOrEndOfLine;

  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return FALSE;
  }

  if (!lstrlenA(lpefr->szFind))
    return FALSE;

  // [2e]: ICU build: missing regexp warnings #232
  if (!n2e_IsFindReplaceAvailable(lpefr))
    return FALSE;

  // Show wait cursor...
  BeginWaitCursor();

  lstrcpynA(szFind2, lpefr->szFind, COUNTOF(szFind2));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2, (lpefr->fuFlags & SCFIND_REGEXP),
                         (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0));
  if (lstrlenA(szFind2) == 0)
  {
    InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
    return FALSE;
  }

  bRegexStartOfLine =
    (szFind2[0] == '^');
  bRegexStartOrEndOfLine =
    (lpefr->fuFlags & SCFIND_REGEXP &&
    (!lstrcmpA(szFind2, "$") || !lstrcmpA(szFind2, "^") || !lstrcmpA(szFind2, "^$")));

  if (lstrcmpA(lpefr->szReplace, "^c") == 0)
  {
    iReplaceMsg = SCI_REPLACETARGET;
    pszReplace2 = EditGetClipboardText(hwnd);
  }
  else
  {
    pszReplace2 = StrDupA(lpefr->szReplace);
    if (lpefr->bTransformBS)
      TransformBackslashes(pszReplace2, (lpefr->fuFlags & SCFIND_REGEXP),
                           (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0));
  }

  if (!pszReplace2)
    pszReplace2 = StrDupA("");

  ZeroMemory(&ttf, sizeof(ttf));

  ttf.chrg.cpMin = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  ttf.chrg.cpMax = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);
  ttf.lpstrText = szFind2;

  while ((iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)&ttf)) != -1 && !fCancel)
  {
    if (ttf.chrgText.cpMin >= SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0) &&
        ttf.chrgText.cpMax <= SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0))
    {
      int iReplacedLen;

      if (ttf.chrg.cpMin == 0 && iCount == 0 && bRegexStartOrEndOfLine)
      {
        if (0 == SendMessage(hwnd, SCI_GETLINEENDPOSITION, 0, 0))
        {
          iPos = 0;
          ttf.chrgText.cpMin = 0;
          ttf.chrgText.cpMax = 0;
        }
      }

      if (++iCount == 1)
        SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);

      SendMessage(hwnd, SCI_SETTARGETSTART, ttf.chrgText.cpMin, 0);
      SendMessage(hwnd, SCI_SETTARGETEND, ttf.chrgText.cpMax, 0);
      iReplacedLen = (int)SendMessage(hwnd, iReplaceMsg, (WPARAM)-1, (LPARAM)pszReplace2);

      ttf.chrg.cpMin = ttf.chrgText.cpMin + iReplacedLen;
      ttf.chrg.cpMax = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);

      if (ttf.chrg.cpMin == ttf.chrg.cpMax)
        fCancel = TRUE;

      if (ttf.chrgText.cpMin == ttf.chrgText.cpMax &&
          !(bRegexStartOrEndOfLine && iReplacedLen > 0))
        ttf.chrg.cpMin = (int)SendMessage(hwnd, SCI_POSITIONAFTER, ttf.chrg.cpMin, 0);
      if (bRegexStartOfLine)
      {
        int iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)ttf.chrg.cpMin, 0);
        int ilPos = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
        if (ilPos == ttf.chrg.cpMin)
          ttf.chrg.cpMin = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine + 1, 0);
        if (ttf.chrg.cpMin == ttf.chrg.cpMax)
          break;
      }
    }

    else
      // gone across selection, cancel
      fCancel = TRUE;
  }

  if (iCount)
  {
    if (SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) <
        SendMessage(hwnd, SCI_GETTARGETEND, 0, 0))
    {

      int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);
      int iCurrentPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);

      if (iAnchorPos > iCurrentPos)
        iAnchorPos = (int)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0);
      else
        iCurrentPos = (int)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0);

      EditSelectEx(hwnd, iAnchorPos, iCurrentPos);
    }

    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
  }

  // [2e]: Gutter not updated on Replace #206
  UpdateLineNumberWidth();
  // Remove wait cursor
  EndWaitCursor();

  if (bShowInfo)
  {
    if (iCount > 0)
      InfoBox(0, L"MsgReplaceCount", IDS_REPLCOUNT, iCount);
    else
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
  }

  LocalFree(pszReplace2);
  return TRUE;

}


//=============================================================================
//
//  EditLinenumDlgProc()
//
INT_PTR CALLBACK EditLinenumDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  LPCGOTOPARAMS lpgoto = NULL;
  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        
        // [2e]: Find/Replace - add Go to Go To #259
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        lpgoto = (LPGOTOPARAMS)lParam;

        if (!lpgoto || lpgoto->bForceDefaultInit || !strlen(lpgoto->lpefr->szFindUTF8))
        {
          lpgoto->bForceDefaultInit = FALSE;
          int iCurLine = (int)SendMessage(hwndEdit, SCI_LINEFROMPOSITION, SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0), 0) + 1;
          SetDlgItemInt(hwnd, IDC_LINENUM, iCurLine, FALSE);
        }
        else
        {
          SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_LINENUM, lpgoto->lpefr->szFindUTF8);
        }
        // [/2e]
        SendDlgItemMessage(hwnd, IDC_LINENUM, EM_LIMITTEXT, 15, 0);
        SendDlgItemMessage(hwnd, IDC_COLNUM, EM_LIMITTEXT, 15, 0);
        SendDlgItemMessage(hwnd, IDC_POSNUM, EM_LIMITTEXT, 15, 0);

        DPI_INIT();

        if (!bSwitchedFindReplace)
        {
          if (xFindReplaceDlg == 0 || yFindReplaceDlg == 0)
            CenterDlgInParent(hwnd);
          else
            SetDlgPos(hwnd, xFindReplaceDlg, yFindReplaceDlg);
        }
        else
        {
          SetDlgPos(hwnd, xFindReplaceDlgSave, yFindReplaceDlgSave);
          bSwitchedFindReplace = FALSE;
        }
      }
      return TRUE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        // [2e]: Find/Replace - add Go to Go To #259
        case IDACC_FIND:
        case IDACC_REPLACE:
          lpgoto = (LPGOTOPARAMS)GetWindowLongPtr(hwnd, DWLP_USER);
          GetDlgItemTextA2W(CP_UTF8, hwnd, IDC_LINENUM, lpgoto->lpefr->szFindUTF8, COUNTOF(lpgoto->lpefr->szFindUTF8));
          PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG((LOWORD(wParam) == IDACC_FIND) ? IDM_EDIT_FIND : IDM_EDIT_REPLACE, 1), 0);
          break;
        // [/2e]


        case IDACC_SAVEPOS:
          GetDlgPos(hwnd, &xFindReplaceDlg, &yFindReplaceDlg);
          break;


        case IDMSG_SWITCHTOFIND:
        case IDMSG_SWITCHTOREPLACE:
          GetDlgPos(hwnd, &xFindReplaceDlg, &yFindReplaceDlg);
          break;


        case IDOK: {
            // [2e]: Go-to absolute offset #2
            int iNewCol;
            int iNewLine;
            int iMaxLine;
            int iMaxPos;
            int iNewPos;
            TCHAR wsLine[0xff];
            TCHAR wsCol[0xff];
            TCHAR wsPos[0xff];
            GetDlgItemText(hwnd, IDC_LINENUM, wsLine, 0xff);
            GetDlgItemText(hwnd, IDC_COLNUM, wsCol, 0xff);
            GetDlgItemText(hwnd, IDC_POSNUM, wsPos, 0xff);
            iMaxLine = SendMessage(hwndEdit, SCI_GETLINECOUNT, 0, 0);
            iMaxPos = SendMessage(hwndEdit, SCI_GETTEXTLENGTH, 0, 0);
            if (!n2e_IsTextEmpty(wsPos))
            {
              if (n2e_GetGotoNumber(wsPos, &iNewPos, TRUE) &&
                  iNewPos >= 0 &&
                  iNewPos < iMaxPos)
              {
                n2e_JumpToOffset(hwndEdit, iNewPos);
                EndDialog(hwnd, IDOK);
              }
              else
              {
                PostMessage(hwnd, WM_NEXTDLGCTL,
                            (WPARAM)(GetDlgItem(hwnd, IDC_POSNUM)), 1);
              }
            }
            else if (
              n2e_GetGotoNumber(wsLine, &iNewLine, FALSE)
              )
            {
              if (!n2e_IsTextEmpty(wsCol))
              {
                if (!n2e_GetGotoNumber(wsCol, &iNewCol, FALSE) || iNewCol <= 0)
                {
                  PostMessage(hwnd, WM_NEXTDLGCTL,
                              (WPARAM)(GetDlgItem(hwnd, IDC_COLNUM)), 1);
                  break;
                }
              }
              else
              {
                iNewCol = 1;
              }
              if (iNewLine > 0
                  && iNewLine <= iMaxLine
                  )
              {
                EditJumpTo(hwndEdit, iNewLine, iNewCol);
                DestroyWindow(hwnd);
                hDlgGotoLine = NULL;
              }
              else
              {
                PostMessage(hwnd, WM_NEXTDLGCTL,
                            (WPARAM)(GetDlgItem(hwnd, IDC_LINENUM)), 1);
              }
            }
            else
            {
              PostMessage(hwnd, WM_NEXTDLGCTL,
                          (WPARAM)(GetDlgItem(hwnd, IDC_LINENUM)), 1);
            }
            // [/2e]
          }
          break;
        case IDCANCEL:
          DestroyWindow(hwnd);
          hDlgGotoLine = NULL;
          break;

      }

      return TRUE;

    case WM_NOTIFY: {

      LPNMHDR pnmhdr = (LPNMHDR)lParam;
      switch (pnmhdr->code)
      {
        case NM_CLICK:
        case NM_RETURN:
          switch (pnmhdr->idFrom)
          {
            case IDC_TOGGLEFIND:
              PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_FIND, 1), 0);
              break;
            case IDC_TOGGLEREPLACE:
              PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_REPLACE, 1), 0);
              break;
          }
          break;
      }
    }
    break;

  }

  return FALSE;

}


//=============================================================================
//
//  EditLinenumDlg()
//
HWND EditLinenumDlg(HWND hwnd, LPCGOTOPARAMS lpgoto)
{

  HWND hDlg;
  
  lpgoto->lpefr->hwnd = NULL;

  hDlg = CreateThemedDialogParam(g_hInstance, MAKEINTRESOURCEW(IDD_LINENUM), 
                                 GetParent(hwnd),  EditLinenumDlgProc, (LPARAM)lpgoto);

  ShowWindow(hDlg, SW_SHOW);

  return hDlg;

}


//=============================================================================
//
//  EditModifyLinesDlg()
//
//  Controls: 100 Input
//            101 Input
//
typedef struct _modlinesdata
{
  LPWSTR pwsz1;
  LPWSTR pwsz2;
} MODLINESDATA, *PMODLINESDATA;


INT_PTR CALLBACK EditModifyLinesDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static PMODLINESDATA pdata;

  static int id_hover;
  static int id_capture;

  static HFONT hFontNormal;
  static HFONT hFontHover;

  static HCURSOR hCursorNormal;
  static HCURSOR hCursorHover;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        LOGFONT lf;

        id_hover = 0;
        id_capture = 0;

        if (NULL == (hFontNormal = (HFONT)SendDlgItemMessage(hwnd, 200, WM_GETFONT, 0, 0)))
          hFontNormal = GetStockObject(DEFAULT_GUI_FONT);
        GetObject(hFontNormal, sizeof(LOGFONT), &lf);
        lf.lfUnderline = TRUE;
        hFontHover = CreateFontIndirect(&lf);

        hCursorNormal = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
        if (!(hCursorHover = LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND))))
          hCursorHover = LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_ARROW));

        pdata = (PMODLINESDATA)lParam;
        SetDlgItemTextW(hwnd, 100, pdata->pwsz1);
        SendDlgItemMessage(hwnd, 100, EM_LIMITTEXT, 255, 0);
        SetDlgItemTextW(hwnd, 101, pdata->pwsz2);
        SendDlgItemMessage(hwnd, 101, EM_LIMITTEXT, 255, 0);

        // [2e]: Remove line breaks from Alt+M #173
        n2e_EnableClipboardFiltering(hwnd, 100);
        n2e_EnableClipboardFiltering(hwnd, 101);
        // [/2e]

        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;

    case WM_DESTROY:
      DeleteObject(hFontHover);
      return FALSE;

    case WM_NCACTIVATE:
      if (!(BOOL)wParam)
      {
        if (id_hover != 0)
        {
          int _id_hover = id_hover;
          id_hover = 0;
          id_capture = 0;
        }
      }
      return FALSE;
    case WM_CTLCOLORSTATIC: {
        DWORD dwId = GetWindowLong((HWND)lParam, GWL_ID);
        HDC hdc = (HDC)wParam;
        if (dwId >= 200 && dwId <= 205)
        {
          SetBkMode(hdc, TRANSPARENT);
          if (GetSysColorBrush(COLOR_HOTLIGHT))
          {
            SetTextColor(hdc, GetSysColor(COLOR_HOTLIGHT));
          }
          else
          {
            SetTextColor(hdc, RGB(0, 0, 255));
          }
          SelectObject(hdc, hFontHover);
          return (LONG_PTR)GetSysColorBrush(COLOR_BTNFACE);
        }
      }
      break;
    case WM_MOUSEMOVE: {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        HWND hwndHover = ChildWindowFromPoint(hwnd, pt);
        DWORD dwId = GetWindowLong(hwndHover, GWL_ID);
        if (GetActiveWindow() == hwnd)
        {
          if (dwId >= 200 && dwId <= 205)
          {
            if (id_capture == dwId || id_capture == 0)
            {
              if (id_hover != id_capture || id_hover == 0)
              {
                id_hover = dwId;
              }
            }
            else if (id_hover != 0)
            {
              int _id_hover = id_hover;
              id_hover = 0;
            }
          }
          else if (id_hover != 0)
          {
            int _id_hover = id_hover;
            id_hover = 0;
          }
          SetCursor(id_hover != 0 ? hCursorHover : hCursorNormal);
        }
      }
      break;
    case WM_LBUTTONDOWN: {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        HWND hwndHover = ChildWindowFromPoint(hwnd, pt);
        DWORD dwId = GetWindowLong(hwndHover, GWL_ID);
        if (dwId >= 200 && dwId <= 205)
        {
          GetCapture();
          id_hover = dwId;
          id_capture = dwId;
        }
        SetCursor(id_hover != 0 ? hCursorHover : hCursorNormal);
      }
      break;
    case WM_LBUTTONUP: {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        HWND hwndHover = ChildWindowFromPoint(hwnd, pt);
        DWORD dwId = GetWindowLong(hwndHover, GWL_ID);
        if (id_capture != 0)
        {
          ReleaseCapture();
          if (id_hover == id_capture)
          {
            int id_focus = GetWindowLong(GetFocus(), GWL_ID);
            if (id_focus == 100 || id_focus == 101)
            {
              WCHAR wch[8];
              GetDlgItemText(hwnd, id_capture, wch, COUNTOF(wch));
              SendDlgItemMessage(hwnd, id_focus, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)wch);
            }
          }
          id_capture = 0;
        }
        SetCursor(id_hover != 0 ? hCursorHover : hCursorNormal);
      }
      break;

    case WM_CANCELMODE:
      if (id_capture != 0)
      {
        ReleaseCapture();
        id_hover = 0;
        id_capture = 0;
        SetCursor(hCursorNormal);
      }
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK: {
            GetDlgItemTextW(hwnd, 100, pdata->pwsz1, 256);
            GetDlgItemTextW(hwnd, 101, pdata->pwsz2, 256);
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
//  EditModifyLinesDlg()
//
BOOL EditModifyLinesDlg(HWND hwnd, LPWSTR pwsz1, LPWSTR pwsz2)
{

  INT_PTR iResult;
  MODLINESDATA data = { pwsz1, pwsz2 };

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCEW(IDD_MODIFYLINES),
    hwnd,
    EditModifyLinesDlgProc,
    (LPARAM)&data);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  EditAlignDlgProc()
//
//  Controls: 100 Radio Button
//            101 Radio Button
//            102 Radio Button
//            103 Radio Button
//            104 Radio Button
//
INT_PTR CALLBACK EditAlignDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static int *piAlignMode;
  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        piAlignMode = (int *)lParam;
        CheckRadioButton(hwnd, 100, 104, *piAlignMode + 100);
        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK: {
            *piAlignMode = 0;
            if (IsDlgButtonChecked(hwnd, 100) == BST_CHECKED)
              *piAlignMode = ALIGN_LEFT;
            else if (IsDlgButtonChecked(hwnd, 101) == BST_CHECKED)
              *piAlignMode = ALIGN_RIGHT;
            else if (IsDlgButtonChecked(hwnd, 102) == BST_CHECKED)
              *piAlignMode = ALIGN_CENTER;
            else if (IsDlgButtonChecked(hwnd, 103) == BST_CHECKED)
              *piAlignMode = ALIGN_JUSTIFY;
            else if (IsDlgButtonChecked(hwnd, 104) == BST_CHECKED)
              *piAlignMode = ALIGN_JUSTIFY_EX;
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
//  EditAlignDlg()
//
BOOL EditAlignDlg(HWND hwnd, int *piAlignMode)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCEW(IDD_ALIGN),
    hwnd,
    EditAlignDlgProc,
    (LPARAM)piAlignMode);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  EditEncloseSelectionDlgProc()
//
//  Controls: 100 Input
//            101 Input
//
typedef struct _encloseselectiondata
{
  LPWSTR pwsz1;
  LPWSTR pwsz2;
} ENCLOSESELDATA, *PENCLOSESELDATA;


INT_PTR CALLBACK EditEncloseSelectionDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  // [2e]: Alt+Q: close off if a single bracket #37
  const WCHAR* _left_braces = L"<{([";
  const WCHAR* _right_braces = L">})]";
  const WCHAR* _special_symbs = L"`~!@#%^*-_+=|\\/:;\"',.?";
  // [/2e]
  static PENCLOSESELDATA pdata;
  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        pdata = (PENCLOSESELDATA)lParam;
        SendDlgItemMessage(hwnd, 100, EM_LIMITTEXT, 255, 0);
        SetDlgItemTextW(hwnd, 100, pdata->pwsz1);
        SendDlgItemMessage(hwnd, 101, EM_LIMITTEXT, 255, 0);
        SetDlgItemTextW(hwnd, 101, pdata->pwsz2);
        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        // [2e]: Alt+Q: close off if a single bracket #37
        case 100: {
            if (HIWORD(wParam) == EN_CHANGE)
            {
              WCHAR wcIns[256], wcBuf[256];
              WCHAR* br;
              BOOL brackets;
              int bCount;
              GetDlgItemTextW(hwnd, 100, wcBuf, 255);
              *wcIns = L'\0';
              bCount = 0;
              brackets = TRUE;
              if (!*wcBuf)
              {
                bCount = -1;
              }
              else
              {
                while (br = StrChr(_left_braces, *(wcBuf + bCount)))
                {
                  wcIns[bCount++] = _right_braces[br - _left_braces];
                }
                wcIns[bCount] = '\0';
              }
              if (0 == bCount)
              {
                br = StrChr(_special_symbs, *(wcBuf));
                while (br &&  *br == *(wcBuf + bCount))
                {
                  wcIns[bCount++] = *br;
                  brackets = FALSE;
                }
                wcIns[bCount] = '\0';
              }

              if (bCount)
              {
                if (brackets)
                {
                  n2e_InplaceRev(wcIns);
                }
                SetDlgItemTextW(hwnd, 101, wcIns);
              }
            }
          }
          break;
        // [/2e]
        case IDOK: {
            GetDlgItemTextW(hwnd, 100, pdata->pwsz1, 256);
            GetDlgItemTextW(hwnd, 101, pdata->pwsz2, 256);
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
//  EditEncloseSelectionDlg()
//
BOOL EditEncloseSelectionDlg(HWND hwnd, LPWSTR pwszOpen, LPWSTR pwszClose)
{

  INT_PTR iResult;
  ENCLOSESELDATA data = { pwszOpen, pwszClose };

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCEW(IDD_ENCLOSESELECTION),
    hwnd,
    EditEncloseSelectionDlgProc,
    (LPARAM)&data);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  EditInsertTagDlgProc()
//
//  Controls: 100 Input
//            101 Input
//
INT_PTR CALLBACK EditInsertTagDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static PTAGSDATA pdata;
  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        pdata = (PTAGSDATA)lParam;
        SendDlgItemMessage(hwnd, 100, EM_LIMITTEXT, 254, 0);
        SetDlgItemTextW(hwnd, 100, wchLastHTMLTag);
        SendDlgItemMessage(hwnd, 101, EM_LIMITTEXT, 255, 0);
        SetDlgItemTextW(hwnd, 101, wchLastHTMLEndTag);
        SetFocus(GetDlgItem(hwnd, 100));
        n2e_Init_EditInsertTagDlg(hwnd);
        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return FALSE;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case 100:
          if (HIWORD(wParam) == EN_CHANGE)
          {
            WCHAR wchBuf[256];
            GetDlgItemTextW(hwnd, 100, wchBuf, 256);
            SetDlgItemTextW(hwnd, 101, n2e_GetClosingTagText_EditInsertTagDlg(wchBuf));
          }
          break;
        case IDOK:
          GetDlgItemTextW(hwnd, 100, pdata->pwsz1, 256);
          GetDlgItemTextW(hwnd, 101, pdata->pwsz2, 256);
          n2e_SaveTagsData_EditInsertTagDlg(pdata);
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
//  EditInsertTagDlg()
//
BOOL EditInsertTagDlg(HWND hwnd, LPWSTR pwszOpen, LPWSTR pwszClose)
{
  INT_PTR iResult;
  TAGSDATA data = { pwszOpen, pwszClose };

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCEW(IDD_INSERTTAG),
    hwnd,
    EditInsertTagDlgProc,
    (LPARAM)&data);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  EditSortDlgProc()
//
//  Controls: 100-102 Radio Button
//            103-108 Check Box
//
INT_PTR CALLBACK EditSortDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static int *piSortFlags;
  static BOOL bEnableLogicalSort;

  switch (umsg)
  {
    DPI_CHANGED_HANDLER();

    case WM_INITDIALOG: {
        piSortFlags = (int *)lParam;
        if (*piSortFlags & SORT_DESCENDING)
          CheckRadioButton(hwnd, 100, 102, 101);
        else if (*piSortFlags & SORT_SHUFFLE)
        {
          CheckRadioButton(hwnd, 100, 102, 102);
          EnableWindow(GetDlgItem(hwnd, 103), FALSE);
          EnableWindow(GetDlgItem(hwnd, 104), FALSE);
          EnableWindow(GetDlgItem(hwnd, 105), FALSE);
          EnableWindow(GetDlgItem(hwnd, 106), FALSE);
          EnableWindow(GetDlgItem(hwnd, 107), FALSE);
        }
        else
          CheckRadioButton(hwnd, 100, 102, 100);
        if (*piSortFlags & SORT_MERGEDUP)
          CheckDlgButton(hwnd, 103, BST_CHECKED);
        if (*piSortFlags & SORT_UNIQDUP)
        {
          CheckDlgButton(hwnd, 104, BST_CHECKED);
          EnableWindow(GetDlgItem(hwnd, 103), FALSE);
        }
        if (*piSortFlags & SORT_UNIQUNIQ)
          CheckDlgButton(hwnd, 105, BST_CHECKED);
        if (*piSortFlags & SORT_NOCASE)
          CheckDlgButton(hwnd, 106, BST_CHECKED);
        if (GetProcAddress(GetModuleHandle(L"shlwapi"), "StrCmpLogicalW"))
        {
          if (*piSortFlags & SORT_LOGICAL)
            CheckDlgButton(hwnd, 107, BST_CHECKED);
          bEnableLogicalSort = TRUE;
        }
        else
        {
          EnableWindow(GetDlgItem(hwnd, 107), FALSE);
          bEnableLogicalSort = FALSE;
        }
        if (SC_SEL_RECTANGLE != SendMessage(hwndEdit, SCI_GETSELECTIONMODE, 0, 0))
        {
          *piSortFlags &= ~SORT_COLUMN;
          EnableWindow(GetDlgItem(hwnd, 108), FALSE);
        }
        else
        {
          *piSortFlags |= SORT_COLUMN;
          CheckDlgButton(hwnd, 108, BST_CHECKED);
        }
        DPI_INIT();
        CenterDlgInParent(hwnd);
      }
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK: {
            *piSortFlags = 0;
            if (IsDlgButtonChecked(hwnd, 101) == BST_CHECKED)
              *piSortFlags |= SORT_DESCENDING;
            if (IsDlgButtonChecked(hwnd, 102) == BST_CHECKED)
              *piSortFlags |= SORT_SHUFFLE;
            if (IsDlgButtonChecked(hwnd, 103) == BST_CHECKED)
              *piSortFlags |= SORT_MERGEDUP;
            if (IsDlgButtonChecked(hwnd, 104) == BST_CHECKED)
              *piSortFlags |= SORT_UNIQDUP;
            if (IsDlgButtonChecked(hwnd, 105) == BST_CHECKED)
              *piSortFlags |= SORT_UNIQUNIQ;
            if (IsDlgButtonChecked(hwnd, 106) == BST_CHECKED)
              *piSortFlags |= SORT_NOCASE;
            if (IsDlgButtonChecked(hwnd, 107) == BST_CHECKED)
              *piSortFlags |= SORT_LOGICAL;
            if (IsDlgButtonChecked(hwnd, 108) == BST_CHECKED)
              *piSortFlags |= SORT_COLUMN;
            EndDialog(hwnd, IDOK);
          }
          break;
        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
        case 100:
        case 101:
          EnableWindow(GetDlgItem(hwnd, 103), IsDlgButtonChecked(hwnd, 105) != BST_CHECKED);
          EnableWindow(GetDlgItem(hwnd, 104), TRUE);
          EnableWindow(GetDlgItem(hwnd, 105), TRUE);
          EnableWindow(GetDlgItem(hwnd, 106), TRUE);
          EnableWindow(GetDlgItem(hwnd, 107), bEnableLogicalSort);
          break;
        case 102:
          EnableWindow(GetDlgItem(hwnd, 103), FALSE);
          EnableWindow(GetDlgItem(hwnd, 104), FALSE);
          EnableWindow(GetDlgItem(hwnd, 105), FALSE);
          EnableWindow(GetDlgItem(hwnd, 106), FALSE);
          EnableWindow(GetDlgItem(hwnd, 107), FALSE);
          break;
        case 104:
          EnableWindow(GetDlgItem(hwnd, 103), IsDlgButtonChecked(hwnd, 104) != BST_CHECKED);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  EditSortDlg()
//
BOOL EditSortDlg(HWND hwnd, int *piSortFlags)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
    g_hInstance,
    MAKEINTRESOURCEW(IDD_SORT),
    hwnd,
    EditSortDlgProc,
    (LPARAM)piSortFlags);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  FileVars_Init()
//
extern BOOL bNoEncodingTags;
extern int fNoFileVariables;

BOOL FileVars_Init(char *lpData, DWORD cbData, LPFILEVARS lpfv)
{

  int i;
  char tch[512];
  BOOL bDisableFileVariables = FALSE;

  ZeroMemory(lpfv, sizeof(FILEVARS));
  if ((fNoFileVariables && bNoEncodingTags) || !lpData || !cbData)
    return (TRUE);

  lstrcpynA(tch, lpData, min(cbData + 1, COUNTOF(tch)));

  if (!fNoFileVariables)
  {
    if (FileVars_ParseInt(tch, "enable-local-variables", &i) && (!i))
      bDisableFileVariables = TRUE;

    if (!bDisableFileVariables)
    {

      if (FileVars_ParseInt(tch, "tab-width", &i))
      {
        lpfv->iTabWidth = max(min(i, 256), 1);
        lpfv->mask |= FV_TABWIDTH;
      }

      if (FileVars_ParseInt(tch, "c-basic-indent", &i))
      {
        lpfv->iIndentWidth = max(min(i, 256), 0);
        lpfv->mask |= FV_INDENTWIDTH;
      }

      if (FileVars_ParseInt(tch, "indent-tabs-mode", &i))
      {
        lpfv->bTabsAsSpaces = (i) ? FALSE : TRUE;
        lpfv->mask |= FV_TABSASSPACES;
      }

      if (FileVars_ParseInt(tch, "c-tab-always-indent", &i))
      {
        lpfv->bTabIndents = (i) ? TRUE : FALSE;
        lpfv->mask |= FV_TABINDENTS;
      }

      if (FileVars_ParseInt(tch, "truncate-lines", &i))
      {
        lpfv->fWordWrap = (i) ? FALSE : TRUE;
        lpfv->mask |= FV_WORDWRAP;
      }

      if (FileVars_ParseInt(tch, "fill-column", &i))
      {
        lpfv->iLongLinesLimit = max(min(i, 4096), 0);
        lpfv->mask |= FV_LONGLINESLIMIT;
      }
    }
  }

  if (!IsUTF8Signature(lpData) && !bNoEncodingTags && !bDisableFileVariables)
  {
    if (FileVars_ParseStr(tch, "encoding", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
    else if (FileVars_ParseStr(tch, "charset", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
    else if (FileVars_ParseStr(tch, "coding", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
  }

  if (!fNoFileVariables && !bDisableFileVariables)
  {
    if (FileVars_ParseStr(tch, "mode", lpfv->tchMode, COUNTOF(lpfv->tchMode)))
      lpfv->mask |= FV_MODE;
  }

  if (lpfv->mask == 0 && cbData > COUNTOF(tch))
  {

    lstrcpynA(tch, lpData + cbData - COUNTOF(tch) + 1, COUNTOF(tch));

    if (!fNoFileVariables)
    {
      if (FileVars_ParseInt(tch, "enable-local-variables", &i) && (!i))
        bDisableFileVariables = TRUE;

      if (!bDisableFileVariables)
      {

        if (FileVars_ParseInt(tch, "tab-width", &i))
        {
          lpfv->iTabWidth = max(min(i, 256), 1);
          lpfv->mask |= FV_TABWIDTH;
        }

        if (FileVars_ParseInt(tch, "c-basic-indent", &i))
        {
          lpfv->iIndentWidth = max(min(i, 256), 0);
          lpfv->mask |= FV_INDENTWIDTH;
        }

        if (FileVars_ParseInt(tch, "indent-tabs-mode", &i))
        {
          lpfv->bTabsAsSpaces = (i) ? FALSE : TRUE;
          lpfv->mask |= FV_TABSASSPACES;
        }

        if (FileVars_ParseInt(tch, "c-tab-always-indent", &i))
        {
          lpfv->bTabIndents = (i) ? TRUE : FALSE;
          lpfv->mask |= FV_TABINDENTS;
        }

        if (FileVars_ParseInt(tch, "truncate-lines", &i))
        {
          lpfv->fWordWrap = (i) ? FALSE : TRUE;
          lpfv->mask |= FV_WORDWRAP;
        }

        if (FileVars_ParseInt(tch, "fill-column", &i))
        {
          lpfv->iLongLinesLimit = max(min(i, 4096), 0);
          lpfv->mask |= FV_LONGLINESLIMIT;
        }
      }
    }

    if (!IsUTF8Signature(lpData) && !bNoEncodingTags && !bDisableFileVariables)
    {

      if (FileVars_ParseStr(tch, "encoding", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
        lpfv->mask |= FV_ENCODING;
      else if (FileVars_ParseStr(tch, "charset", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
        lpfv->mask |= FV_ENCODING;
      else if (FileVars_ParseStr(tch, "coding", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
        lpfv->mask |= FV_ENCODING;
    }

    if (!fNoFileVariables && !bDisableFileVariables)
    {
      if (FileVars_ParseStr(tch, "mode", lpfv->tchMode, COUNTOF(lpfv->tchMode)))
        lpfv->mask |= FV_MODE;
    }
  }

  if (lpfv->mask & FV_ENCODING)
    lpfv->iEncoding = Encoding_MatchA(lpfv->tchEncoding);

  return (TRUE);
}


//=============================================================================
//
//  FileVars_Apply()
//
extern int iTabWidth;
extern int iTabWidthG;
extern int iIndentWidth;
extern int iIndentWidthG;
extern BOOL bTabsAsSpaces;
extern BOOL bTabsAsSpacesG;
extern BOOL bTabIndents;
extern BOOL bTabIndentsG;
extern int fWordWrap;
extern int fWordWrapG;
extern int iWordWrapMode;
extern int iLongLinesLimit;
extern int iLongLinesLimitG;
extern int iWrapCol;

BOOL FileVars_Apply(HWND hwnd, LPFILEVARS lpfv)
{

  if (lpfv->mask & FV_TABWIDTH)
    iTabWidth = lpfv->iTabWidth;
  else
    iTabWidth = iTabWidthG;
  SendMessage(hwnd, SCI_SETTABWIDTH, iTabWidth, 0);

  if (lpfv->mask & FV_INDENTWIDTH)
    iIndentWidth = lpfv->iIndentWidth;
  else if (lpfv->mask & FV_TABWIDTH)
    iIndentWidth = 0;
  else
    iIndentWidth = iIndentWidthG;
  SendMessage(hwnd, SCI_SETINDENT, iIndentWidth, 0);

  if (lpfv->mask & FV_TABSASSPACES)
    bTabsAsSpaces = lpfv->bTabsAsSpaces;
  else
    bTabsAsSpaces = bTabsAsSpacesG;
  SendMessage(hwnd, SCI_SETUSETABS, !bTabsAsSpaces, 0);

  if (lpfv->mask & FV_TABINDENTS)
    bTabIndents = lpfv->bTabIndents;
  else
    bTabIndents = bTabIndentsG;
  SendMessage(hwndEdit, SCI_SETTABINDENTS, bTabIndents, 0);

  if (lpfv->mask & FV_WORDWRAP)
    fWordWrap = lpfv->fWordWrap;
  else
    fWordWrap = fWordWrapG;
  if (!fWordWrap)
    SendMessage(hwndEdit, SCI_SETWRAPMODE, SC_WRAP_NONE, 0);
  else
    SendMessage(hwndEdit, SCI_SETWRAPMODE, (iWordWrapMode == 0) ? SC_WRAP_WORD : SC_WRAP_CHAR, 0);

  if (lpfv->mask & FV_LONGLINESLIMIT)
    iLongLinesLimit = lpfv->iLongLinesLimit;
  else
    iLongLinesLimit = iLongLinesLimitG;
  SendMessage(hwnd, SCI_SETEDGECOLUMN, iLongLinesLimit, 0);

  iWrapCol = 0;

  return (TRUE);
}


//=============================================================================
//
//  FileVars_ParseInt()
//
BOOL FileVars_ParseInt(char *pszData, char *pszName, int *piValue)
{

  char tch[32];
  char *pvStart = pszData;
  char chPrev;
  char *pvEnd;
  int  itok;

  while (pvStart = StrStrIA(pvStart, pszName))
  {
    chPrev = (pvStart > pszData) ? *(pvStart - 1) : 0;
    if (!IsCharAlphaNumericA(chPrev) && chPrev != '-' && chPrev != '_')
    {
      pvStart += lstrlenA(pszName);
      while (*pvStart == ' ')
        pvStart++;
      if (*pvStart == ':' || *pvStart == '=')
        break;
    }
    else
      pvStart += lstrlenA(pszName);
  }

  if (pvStart)
  {

    while (*pvStart && StrChrIA(":=\"' \t", *pvStart))
      pvStart++;

    lstrcpynA(tch, pvStart, COUNTOF(tch));
    pvEnd = tch;
    while (*pvEnd && IsCharAlphaNumericA(*pvEnd))
      pvEnd++;
    *pvEnd = 0;
    StrTrimA(tch, " \t:=\"'");

    itok = sscanf(tch, "%i", piValue);
    if (itok == 1)
      return (TRUE);

    if (tch[0] == 't')
    {
      *piValue = 1;
      return (TRUE);
    }

    if (tch[0] == 'n' || tch[0] == 'f')
    {
      *piValue = 0;
      return (TRUE);
    }
  }
  return (FALSE);
}


//=============================================================================
//
//  FileVars_ParseStr()
//
BOOL FileVars_ParseStr(char *pszData, char *pszName, char *pszValue, int cchValue)
{

  char tch[32];
  char *pvStart = pszData;
  char chPrev;
  char *pvEnd;
  BOOL bQuoted = FALSE;

  while (pvStart = StrStrIA(pvStart, pszName))
  {
    chPrev = (pvStart > pszData) ? *(pvStart - 1) : 0;
    if (!IsCharAlphaNumericA(chPrev) && chPrev != '-' && chPrev != '_')
    {
      pvStart += lstrlenA(pszName);
      while (*pvStart == ' ')
        pvStart++;
      if (*pvStart == ':' || *pvStart == '=')
        break;
    }
    else
      pvStart += lstrlenA(pszName);
  }

  if (pvStart)
  {

    while (*pvStart && StrChrIA(":=\"' \t", *pvStart))
    {
      if (*pvStart == '\'' || *pvStart == '"')
        bQuoted = TRUE;
      pvStart++;
    }
    lstrcpynA(tch, pvStart, COUNTOF(tch));

    pvEnd = tch;
    while (*pvEnd && (IsCharAlphaNumericA(*pvEnd) || StrChrIA("+-/_", *pvEnd) || (bQuoted && *pvEnd == ' ')))
      pvEnd++;
    *pvEnd = 0;
    StrTrimA(tch, " \t:=\"'");

    lstrcpynA(pszValue, tch, cchValue);
    return (TRUE);
  }
  return (FALSE);
}


//=============================================================================
//
//  FileVars_IsUTF8()
//
BOOL FileVars_IsUTF8(LPFILEVARS lpfv)
{
  if (lpfv->mask & FV_ENCODING)
  {
    if (lstrcmpiA(lpfv->tchEncoding, "utf-8") == 0 ||
        lstrcmpiA(lpfv->tchEncoding, "utf8") == 0)
      return (TRUE);
  }
  return (FALSE);
}


//=============================================================================
//
//  FileVars_IsNonUTF8()
//
BOOL FileVars_IsNonUTF8(LPFILEVARS lpfv)
{
  if (lpfv->mask & FV_ENCODING)
  {
    if (lstrlenA(lpfv->tchEncoding) &&
        lstrcmpiA(lpfv->tchEncoding, "utf-8") != 0 &&
        lstrcmpiA(lpfv->tchEncoding, "utf8") != 0)
      return (TRUE);
  }
  return (FALSE);
}


//=============================================================================
//
//  FileVars_IsValidEncoding()
//
BOOL FileVars_IsValidEncoding(LPFILEVARS lpfv)
{
  CPINFO cpi;
  if (lpfv->mask & FV_ENCODING &&
      lpfv->iEncoding >= 0 &&
      lpfv->iEncoding < COUNTOF(mEncoding))
  {
    if ((mEncoding[lpfv->iEncoding].uFlags & NCP_INTERNAL) ||
        IsValidCodePage(mEncoding[lpfv->iEncoding].uCodePage) &&
        GetCPInfo(mEncoding[lpfv->iEncoding].uCodePage, &cpi))
    {
      return (TRUE);
    }
  }
  return (FALSE);
}


//=============================================================================
//
//  FileVars_GetEncoding()
//
int FileVars_GetEncoding(LPFILEVARS lpfv)
{
  if (lpfv->mask & FV_ENCODING)
    return (lpfv->iEncoding);
  else
    return (-1);
}
