#include <Windows.h>
#include <Shlwapi.h>
#include "Edit.h"
#include "../src/Extension/SciCall.h"
#include "Scintilla.h"
#include "BoostRegexSearch.h"
#include "../src/Extension/Lexers.h"
#include "../src/Extension/LexerUtils.h"
#include "../src/Extension/StringRecoding.h"

#ifndef COUNTOF
#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))
#endif

#ifndef N2E_TESTING
#include "Dialogs.h"
#include "Helpers.h"
#include "resource.h"
#include "ExtSelection.h"

extern LPMRULIST mruFind;

extern void BeginWaitCursor();
extern void EndWaitCursor();
extern void UpdateLineNumberWidth(HWND hwnd);
#endif

// [2e]: original code moved from Edit.c
//
//=============================================================================
//
//  EditSelectEx()
//
void EditSelectEx(HWND hwnd, int iAnchorPos, int iCurrentPos)
{
  SendMessage(hwnd, SCI_SETXCARETPOLICY, CARET_SLOP | CARET_STRICT | CARET_EVEN, 50);
#ifndef N2E_TESTING
  // [2e]: Disable ScrollYCaretPolicy in page-wise Edit Mode #337
  if (!(n2e_IsSelectionEditModeOn() && n2e_IsPageWiseSelectionEditMode()))
  {
    // [2e]: ScrollYCaretPolicy ini-option
    SendMessage(hwnd, SCI_SETYCARETPOLICY, CARET_SLOP | CARET_STRICT | CARET_EVEN, n2e_GetCaretSlop());
  }
#endif
  SendMessage(hwnd, SCI_SETSEL, iAnchorPos, iCurrentPos);
  SendMessage(hwnd, SCI_SETXCARETPOLICY, CARET_SLOP | CARET_EVEN, 50);
  SendMessage(hwnd, SCI_SETYCARETPOLICY, CARET_EVEN, 0);
}

// [2e]: original code moved from Helpers.c
//

/******************************************************************************
*
*  UnSlash functions
*  Mostly taken from SciTE, (c) Neil Hodgson, http://www.scintilla.org
*
/

/**
* Is the character an octal digit?
*/
static BOOL IsOctalDigit(char ch)
{
  return ch >= '0' && ch <= '7';
}

/**
 * If the character is an hexa digit, get its value.
 */
static int GetHexDigit(char ch)
{
  if (ch >= '0' && ch <= '9')
  {
    return ch - '0';
  }
  if (ch >= 'A' && ch <= 'F')
  {
    return ch - 'A' + 10;
  }
  if (ch >= 'a' && ch <= 'f')
  {
    return ch - 'a' + 10;
  }
  return -1;
}

/**
 * Convert C style \a, \b, \f, \n, \r, \t, \v, \xhh and \uhhhh into their indicated characters.
 */
unsigned int UnSlash(char *s, UINT cpEdit)
{
  char *sStart = s;
  char *o = s;

  while (*s)
  {
    if (*s == '\\')
    {
      s++;
      if (*s == 'a')
        *o = '\a';
      else if (*s == 'b')
        *o = '\b';
      else if (*s == 'f')
        *o = '\f';
      else if (*s == 'n')
        *o = '\n';
      else if (*s == 'r')
        *o = '\r';
      else if (*s == 't')
        *o = '\t';
      else if (*s == 'v')
        *o = '\v';
      else if (*s == 'x' || *s == 'u')
      {
        BOOL bShort = (*s == 'x');
        char ch[8];
        char *pch = ch;
        WCHAR val[2] = L"";
        int hex;
        val[0] = 0;
        hex = GetHexDigit(*(s + 1));
        if (hex >= 0)
        {
          s++;
          val[0] = hex;
          hex = GetHexDigit(*(s + 1));
          if (hex >= 0)
          {
            s++;
            val[0] *= 16;
            val[0] += hex;
            if (!bShort)
            {
              hex = GetHexDigit(*(s + 1));
              if (hex >= 0)
              {
                s++;
                val[0] *= 16;
                val[0] += hex;
                hex = GetHexDigit(*(s + 1));
                if (hex >= 0)
                {
                  s++;
                  val[0] *= 16;
                  val[0] += hex;
                }
              }
            }
          }
          if (val[0])
          {
            val[1] = 0;
            WideCharToMultiByte(cpEdit, 0, val, -1, ch, COUNTOF(ch), NULL, NULL);
            *o = *pch++;
            while (*pch)
              *++o = *pch++;
          }
          else
            o--;
        }
        else
          o--;
      }
      else
        *o = *s;
    }
    else
      *o = *s;
    o++;
    if (*s)
    {
      s++;
    }
  }
  *o = '\0';
  return (unsigned int)(o - sStart);
}

/**
 * Convert C style \0oo into their indicated characters.
 * This is used to get control characters into the regular expresion engine.
 */
unsigned int UnSlashLowOctal(char *s)
{
  char *sStart = s;
  char *o = s;
  while (*s)
  {
    if ((s[0] == '\\') && (s[1] == '0') && IsOctalDigit(s[2]) && IsOctalDigit(s[3]))
    {
      *o = (char)(8 * (s[2] - '0') + (s[3] - '0'));
      s += 3;
    }
    else
    {
      *o = *s;
    }
    o++;
    if (*s)
      s++;
  }
  *o = '\0';
  return (unsigned int)(o - sStart);
}

void TransformBackslashes(char *pszInput, BOOL bRegEx, UINT cpEdit)
{
  if (bRegEx)
    UnSlashLowOctal(pszInput);
  else
    UnSlash(pszInput, cpEdit);
}
// [/2e]: original code moved from Helpers.c

BOOL n2e_IsRectangularSelection()
{
  return SciCall_GetSelectionMode() == SC_SEL_RECTANGLE;
}

BOOL n2e_ShowPromptIfSelectionModeIsRectangle(const HWND hwnd)
{
  if (n2e_IsRectangularSelection())
  {
#ifndef N2E_TESTING
    MsgBox(MBWARN, IDS_SELRECT);
#endif
    return TRUE;
  }
  return FALSE;
}

BOOL isEndedWithEOL(char *psz)
{
  const auto length = strlen(psz);
  if (length < 2)
    return FALSE;
  return (psz[length - 2] == '\\') && ((psz[length - 1] == 'r') || (psz[length - 1] == 'n'));
}

BOOL n2e_IsCommentStyle(PEDITSTYLE pStyle)
{
  return pStyle && (pStyle->i64Style != -1) && (StrStrI(pStyle->pszName, L"comment") != NULL);
}

PEDITSTYLE n2e_GetStyleById(const int iStyle)
{
  PEDITSTYLE pStyle = pLexCurrent->Styles;
  int i = 0;
  while (pStyle && (pStyle->i64Style != -1))
  {
    if (((iStyle == 0) && pStyle->i64Style == iStyle)
      || (MULTI_STYLE_STYLE1(pStyle->i64Style) == iStyle)
      || (MULTI_STYLE_STYLE2(pStyle->i64Style) == iStyle)
      || (MULTI_STYLE_STYLE3(pStyle->i64Style) == iStyle)
      || (MULTI_STYLE_STYLE4(pStyle->i64Style) == iStyle)
      || (MULTI_STYLE_STYLE5(pStyle->i64Style) == iStyle)
      || (MULTI_STYLE_STYLE6(pStyle->i64Style) == iStyle)
      || (MULTI_STYLE_STYLE7(pStyle->i64Style) == iStyle)
      || (MULTI_STYLE_STYLE8(pStyle->i64Style) == iStyle))
      break;

    pStyle = &pLexCurrent->Styles[++i];
  }
  return pStyle;
}

BOOL n2e_IsCommentStyleById(const int iStyle)
{
  return n2e_IsCommentStyle(n2e_GetStyleById(iStyle));
}

BOOL n2e_IsCommentStyleAtPos(const HWND hwnd, const int iPos)
{
  return n2e_IsCommentStyle(n2e_GetStyleById((int)SendMessage(hwnd, SCI_GETSTYLEAT, iPos, 0)));
}

BOOL n2e_IsSingleLineCommentStyleAtPos(const HWND hwnd, const int iLexer, const int iPos, EncodingData* pED)
{
#ifdef N2E_TESTING
  return (iLexer != SCLEX_NULL)
    && TextBuffer_IsTextAtPos(&pED->m_tb, n2e_GetSingleLineCommentPrefix(iLexer), iPos - n2e_GetSingleLineCommentPrefixLength(iLexer))
    && (!n2e_SingleLineCommentPrefixIsWord(iLexer)
      || TextBuffer_IsAnyFollowingCharAtLine(&pED->m_tb, lpstrWhiteSpacesAndEOLs, iPos));
#else
  const int iTestPos = pED->m_tr.m_iSelStart + pED->m_tb.m_iPos + iPos;
  const HWND _hwnd = hwnd ? hwnd : hwndEdit;
  const DWORD dwStyle = (int)SendMessage(_hwnd, SCI_GETSTYLEAT, iTestPos, 0);
  const PEDITSTYLE pStyle = n2e_GetStyleById(dwStyle);
  return (pStyle && (pStyle->i64Style != -1)
    && (StrStrI(pStyle->pszName, L"comment") != NULL)
    && n2e_IsSingleLineCommentStyle(pLexCurrent->iLexer, dwStyle));
  //|| (SciCall_GetLength() == iTestPos);
#endif
}

BOOL n2e_CommentStyleIsDefined(const HWND hwnd)
{
  PEDITSTYLE pStyle = pLexCurrent->Styles;
  int i = 0;
  while (pStyle && (pStyle->i64Style != -1))
  {
    if (n2e_IsCommentStyle(pStyle))
      return TRUE;

    pStyle = &pLexCurrent->Styles[++i];
  }
  return FALSE;
}

BOOL n2e_IsFindReplaceAvailable(LPCEDITFINDREPLACE lpefr)
{
#ifdef N2E_TESTING
  return TRUE;
#else
  if (((lpefr->fuFlags & SCFIND_REGEXP) == 0) || n2e_IsUnicodeEncodingMode() || n2e_IsUTF8EncodingMode())
    return TRUE;

  if (InfoBox(MBYESNO, L"MsgICURegexWarning", IDS_WARN_ICU_REGEX) != IDYES)
    return FALSE;

  SendMessage(hwndMain, WM_COMMAND, MAKELONG(IDM_ENCODING_UTF8, 1), 0);

  const BOOL res = n2e_IsUnicodeEncodingMode() || (iEncoding == CPI_UTF8);
  if (res)
  {
    const UINT uCPEdit = (UINT)SendMessage(lpefr->hwnd, SCI_GETCODEPAGE, 0, 0);
    GetDlgItemTextA2W(uCPEdit, hDlgFindReplace, IDC_FINDTEXT, lpefr->szFind, COUNTOF(lpefr->szFind));
  }
  return res;
#endif
}

int n2e_FindTextImpl(const HWND hwnd, LPCEDITFINDREPLACE lpefr, struct TextToFind* pttf)
{
#ifndef N2E_TESTING
  // [2e]: Always save Find strings to MRU #440
  MRU_AddA(mruFind, pttf->lpstrText);
#endif

  lpefr->fuFlags |= SCFIND_REGEXP_EMPTYMATCH_ALLOWATSTART | SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH | SCFIND_REGEXP_SKIPCRLFASONE;

  int iPos = -1;
  BOOL bContinueSearch = TRUE;
  while (bContinueSearch)
  {
    iPos = (int)SendMessage(hwnd, SCI_FINDTEXT, lpefr->fuFlags, (LPARAM)pttf);
    bContinueSearch = (iPos >= 0);

    if (bContinueSearch)
    {
      switch (lpefr->iSearchInComments)
      {
      case SIC_ALWAYS:
        bContinueSearch = FALSE;
        break;
      case SIC_ONLY:
        bContinueSearch = !n2e_IsCommentStyleAtPos(hwnd, iPos);
        break;
      case SIC_NEVER:
        bContinueSearch = n2e_IsCommentStyleAtPos(hwnd, iPos);
        break;
      }
    }
    if (bContinueSearch)
    {
      if (pttf->chrg.cpMin <= pttf->chrg.cpMax)
      {
        pttf->chrg.cpMin = iPos + 1;
      }
      else
      {
        pttf->chrg.cpMin = iPos;
      }
    }
  }
  return iPos;
}

int n2e_regexReplaceFilter(const void* ptr, const int pos, const int mode)
{
  // #TODO: code cleanup
  const ESearchInComments sic = (ESearchInComments)mode;
  switch (sic)
	{
	case SIC_ALWAYS:
    return 1;
  case SIC_ONLY:
    return n2e_IsCommentStyleAtPos(hwndEdit, pos);
  case SIC_NEVER:
    return !n2e_IsCommentStyleAtPos(hwndEdit, pos);
  default:
    break;
  }
  return 1;
}

BOOL n2e_EditReplaceAllImpl(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bShowInfo, BOOL bProcessSelectionOnly)
{
  char szFind2[TEXT_BUFFER_LENGTH];
  char *pszReplace2;

  if (bProcessSelectionOnly && n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
    return FALSE;

  if (!lstrlenA(lpefr->szFind))
    return FALSE;

  // [2e]: ICU build: missing regexp warnings #232
  if (!n2e_IsFindReplaceAvailable(lpefr))
    return FALSE;

#ifndef N2E_TESTING
  // Show wait cursor...
  BeginWaitCursor();
#endif

  lstrcpynA(szFind2, lpefr->szFind, COUNTOF(szFind2));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2, (lpefr->fuFlags & SCFIND_REGEXP),
    (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0));

  if (lstrlenA(szFind2) == 0)
  {
#ifndef N2E_TESTING
    InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
#endif
    return FALSE;
  }

  // [2e]: Extremely slow Replace when changing line count #363
  SciCall_SetSkipUIUpdate(1);

#ifndef N2E_TESTING
  if (lstrcmpA(lpefr->szReplace, "^c") == 0)
  {
    pszReplace2 = EditGetClipboardText(hwnd);
  }
  else
#endif
  {
    pszReplace2 = StrDupA(lpefr->szReplace);
    if (lpefr->bTransformBS)
      TransformBackslashes(pszReplace2, (lpefr->fuFlags & SCFIND_REGEXP),
      (UINT)SendMessage(hwnd, SCI_GETCODEPAGE, 0, 0));
  }

  if (!pszReplace2)
    pszReplace2 = StrDupA("");

  SciCall_BeginUndoAction();
  struct Sci_RegexReplace rr = { 0 };
  rr.chrg.cpMin = bProcessSelectionOnly ? SciCall_GetSelStart() : 0;
  rr.chrg.cpMax = bProcessSelectionOnly ? SciCall_GetSelEnd() : SciCall_GetLength();
  rr.lpstrRegex = szFind2;
  rr.lpstrRegexReplace = pszReplace2;
  rr.filterFunc = n2e_regexReplaceFilter;
  rr.filterFuncParam = lpefr->iSearchInComments;
  SciCall_RegexReplaceText(lpefr->fuFlags, &rr);

  if (rr.count)
  {
    if (bProcessSelectionOnly && (SciCall_GetSelEnd() < SendMessage(hwnd, SCI_GETTARGETEND, 0, 0)))
    {
      int iAnchorPos = (int)SendMessage(hwnd, SCI_GETANCHOR, 0, 0);
      int iCurrentPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);

      if (iAnchorPos > iCurrentPos)
        iAnchorPos = (int)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0);
      else
        iCurrentPos = (int)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0);

      EditSelectEx(hwnd, iAnchorPos, iCurrentPos);
    }
  }
  SciCall_EndUndoAction();

  // [2e]: Extremely slow Replace when changing line count #363
  SciCall_SetSkipUIUpdate(0);

#ifndef N2E_TESTING
  // [2e]: Gutter not updated on Replace #206
  VIEW_COMMAND(UpdateLineNumberWidth);
  // Remove wait cursor
  EndWaitCursor();

  if (bShowInfo)
  {
    if (rr.count > 0)
      InfoBox(0, L"MsgReplaceCount", IDS_REPLCOUNT, rr.count);
    else
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
  }
#endif

  LocalFree(pszReplace2);
  return TRUE;

}

BOOL EditReplaceAllInSelection(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bShowInfo)
{
  return n2e_EditReplaceAllImpl(hwnd, lpefr, bShowInfo, TRUE);
}

BOOL EditReplaceAll(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bShowInfo)
{
  return n2e_EditReplaceAllImpl(hwnd, lpefr, bShowInfo, FALSE);
}
