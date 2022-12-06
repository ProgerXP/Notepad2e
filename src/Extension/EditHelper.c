#include "EditHelper.h"
#include <cassert>
#include "CommonUtils.h"
#include "CommentAwareLineWrapping.h"
#include "Dialogs.h"
#include "EditHelperEx.h"
#include "Helpers.h"
#include "Edit.h"
#include "ExtSelection.h"
#include "LexerUtils.h"
#include "MainWndHelper.h"
#include "Notepad2.h"
#include "resource.h"
#include "SciCall.h"
#include "StrToBase64.h"
#include "StrToHex.h"
#include "StrToQP.h"
#include "StrToURL.h"
#include "StringRecoding.h"
#include "Styles.h"
#include "Subclassing.h"
#include "Trace.h"
#include "Utils.h"

#define BRACES "()[]{}<>"
#define BRACES_WITH_QUOTES BRACES "'\"`"

WCHAR wchLastHTMLTag[TEXT_BUFFER_LENGTH] = L"<tag>";
WCHAR wchLastHTMLEndTag[TEXT_BUFFER_LENGTH] = L"</tag>";

extern NP2ENCODING mEncoding[];
extern int iEncoding;
extern int bFindWordMatchCase;
extern int bFindWordWrapAround;
extern TBBUTTON tbbMainWnd[];
extern HWND hwndToolbar;
extern HWND hwndMain;
extern HWND hDlgFindReplace;
extern int iOpenSaveFilterIndex;
extern BOOL bAlwaysOnTop;
extern int flagAlwaysOnTop;
extern PEDITLEXER pLexCurrent;

void n2e_SplitLines(const HWND hwnd, const int iLineSizeLimit, const BOOL bColumnWrap)
{
  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }

  const BOOL bAdjustSelection = bColumnWrap && (SciCall_GetSelStart() == SciCall_GetSelEnd());
  if (bAdjustSelection)
  {
    SciCall_SetSel(0, SciCall_GetLength());
  }
  EncodeStrWithCALW(hwnd, iLineSizeLimit);
  if (bAdjustSelection)
  {
    SciCall_SetSel(0, SciCall_GetLength());
  }
}

BOOL n2e_JoinLines_InitSelection()
{
  BOOL bContinueProcessing = TRUE;
  const int iSelStart = SciCall_GetSelStart();
  const int iSelEnd = SciCall_GetSelEnd();
  const int iSelEndNew = n2e_JoinLines_GetSelEnd(iSelStart, iSelEnd, &bContinueProcessing);
  if (bContinueProcessing && (iSelEndNew != iSelEnd))
  {
    SciCall_SetSel(iSelStart, iSelEndNew);
  }
  return bContinueProcessing;
}

void n2e_StripHTMLTags(const HWND hwnd)
{
#define HTML_TAG_LEFT "<"
#define HTML_TAG_RIGHT ">"

  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }
  struct Sci_TextToFind ttf1, ttf2;
  int res, len;
  int selbeg = SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  int selend = SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  len = SendMessage(hwnd, SCI_GETTEXTLENGTH, 0, 0);
  SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
  if (0 == selend - selbeg)
  {
    ttf1.chrg.cpMin = selbeg;
    ttf1.chrg.cpMax = 0;
    ttf1.lpstrText = HTML_TAG_LEFT;
    res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf1);
    if (-1 != res)
    {
      ttf2.chrg.cpMin = ttf1.chrgText.cpMax;
      ttf2.chrg.cpMax = len;
      ttf2.lpstrText = HTML_TAG_RIGHT;
      res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf2);
      if (-1 != res)
      {
        SendMessage(hwnd, SCI_DELETERANGE, ttf1.chrgText.cpMin, ttf2.chrgText.cpMax - ttf1.chrgText.cpMin);
        SendMessage(hwnd, SCI_SETSEL, ttf1.chrgText.cpMin, ttf1.chrgText.cpMin);
      }
    }
  }
  else
  {
    while (1)
    {
      ttf1.chrg.cpMin = selbeg;
      ttf1.chrg.cpMax = selend;
      ttf1.lpstrText = HTML_TAG_LEFT;
      res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf1);
      if (-1 != res)
      {
        ttf2.chrg.cpMin = ttf1.chrgText.cpMax;
        ttf2.chrg.cpMax = selend;
        ttf2.lpstrText = HTML_TAG_RIGHT;
        res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf2);
        if (-1 != res)
        {
          int dlen = ttf2.chrgText.cpMax - ttf1.chrgText.cpMin;
          SendMessage(hwnd, SCI_DELETERANGE, ttf1.chrgText.cpMin, dlen);
          selend -= dlen;
        }
        else
        {
          break;
        }
      }
      else
      {
        break;
      }
    }
    SendMessage(hwnd, SCI_SETSEL, selbeg, selend);
  }
  SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
}

BOOL n2e_ShowPromptIfSelectionModeIsRectangle(const HWND hwnd)
{
  if (n2e_IsRectangularSelection())
  {
    MsgBox(MBWARN, IDS_SELRECT);
    return TRUE;
  }
  return FALSE;
}

extern BOOL bAutoIndent;

LPSTR GetLinePrefix(const int iLine, LPBOOL pbLineEmpty)
{
  *pbLineEmpty = TRUE;

  LPSTR pszPrefix = n2e_GetTextRange(
    SciCall_PositionFromLine(iLine),
    SciCall_LineEndPosition(iLine));

  for (size_t i = 0; i < strlen(pszPrefix); ++i)
  {
    const char chCurrent = pszPrefix[i];
    if (!isspace(chCurrent) || (chCurrent == '\r') || (chCurrent == '\n'))
    {
      pszPrefix[i] = 0;
      *pbLineEmpty = (i == 0);
      break;
    }
  }

  return pszPrefix;
}

void InsertNewLineWithPrefix(const HWND hwnd, LPCSTR pszPrefix, const BOOL bInsertAbove)
{
  const BOOL bAutoIndentOrigin = bAutoIndent;
  bAutoIndent = 0;
  SendMessage(hwnd, SCI_NEWLINE, 0, 0);
  bAutoIndent = bAutoIndentOrigin;
  if (bInsertAbove)
  {
    SendMessage(hwnd, SCI_CHARLEFT, 0, 0);
  }
  if (pszPrefix && (strlen(pszPrefix) > 0))
  {
    const int iCurrentPos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
    SendMessage(hwnd, SCI_INSERTTEXT, iCurrentPos, (LPARAM)pszPrefix);
    SendMessage(hwnd, SCI_LINEEND, 0, 0);
  }
}

LPSTR FindPrefixForNonEmptyLine(const BOOL bSearchForward, int iScanLimit)
{
  BOOL bIsEmptyLine = FALSE;
  const int iLineCount = SciCall_GetLineCount();
  int iLine = SciCall_LineFromPosition(SciCall_GetCurrentPos());
  iLine += bSearchForward ? 1 : -1;
  while ((iScanLimit >= 1)
    && ((bSearchForward && (iLine < iLineCount)) || (!bSearchForward && (iLine >= 0))))
  {
    const int iNonSpaceCharPos = n2e_GetNonSpaceCharPos(iLine, TRUE);
    if (iNonSpaceCharPos >= 0)
    {
      return GetLinePrefix(iLine, &bIsEmptyLine);
    }
    iLine += bSearchForward ? 1 : -1;
    --iScanLimit;
  }
  return NULL;
}

void n2e_EditInsertNewLine(const HWND hwnd, const BOOL insertAbove)
{
  n2e_SelectionEditStop(hwnd, SES_APPLY);

  const int iCurPos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  const int iCurLine = SendMessage(hwnd, SCI_LINEFROMPOSITION, iCurPos, 0);
  const int iCurLineEndPos = SendMessage(hwnd, SCI_GETLINEENDPOSITION, iCurLine, 0);
  SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
  const int iPrevLine = (iCurLine > 0) ? iCurLine - 1 : 0;
  const int iScanLinesLimit = 5;
  LPSTR pszPrefixText = NULL;
  LPSTR pszForeignPrefixText = NULL;
  BOOL bIsEmptyPrefix = FALSE;

  if (insertAbove)
  {
    const int iPrevLineEndPos = (iPrevLine == iCurLine) ? 0 : SendMessage(hwnd, SCI_GETLINEENDPOSITION, iPrevLine, 0);
    if (bAutoIndent)
    {
      pszPrefixText = (n2e_GetNonSpaceCharPos(iCurLine, TRUE) >= 0)
        ? GetLinePrefix(iCurLine, &bIsEmptyPrefix)
        : NULL;
      pszForeignPrefixText = FindPrefixForNonEmptyLine(FALSE, iScanLinesLimit);
      
      LPSTR pszUsedPrefixText =
        (pszPrefixText && pszForeignPrefixText)
          ? (strlen(pszPrefixText) > strlen(pszForeignPrefixText))
            ? pszPrefixText
            : pszForeignPrefixText
          : pszPrefixText
            ? pszPrefixText
            : pszForeignPrefixText;

      SciCall_SetSel(iPrevLineEndPos, iPrevLineEndPos);
      InsertNewLineWithPrefix(hwnd, pszUsedPrefixText, (iPrevLine == iCurLine));
    }
    else
    {
      SendMessage(hwnd, SCI_SETSEL, iPrevLineEndPos, iPrevLineEndPos);
      SendMessage(hwnd, SCI_NEWLINE, 0, 0);
    }
  }
  else
  {
    if (bAutoIndent)
    {
      pszPrefixText = (n2e_GetNonSpaceCharPos(iCurLine, TRUE) > 0)
        ? GetLinePrefix(iCurLine, &bIsEmptyPrefix)
        : NULL;
      pszForeignPrefixText = FindPrefixForNonEmptyLine(TRUE, iScanLinesLimit);

      LPSTR pszUsedPrefixText =
        (pszPrefixText && pszForeignPrefixText)
          ? (strlen(pszPrefixText) > strlen(pszForeignPrefixText))
            ? pszPrefixText
            : pszForeignPrefixText
          : pszPrefixText
            ? pszPrefixText
            : pszForeignPrefixText;

      SciCall_SetSel(iCurLineEndPos, iCurLineEndPos);
      InsertNewLineWithPrefix(hwnd, pszUsedPrefixText, FALSE);
    }
    else
    {
      SendMessage(hwnd, SCI_SETSEL, iCurLineEndPos, iCurLineEndPos);
      SendMessage(hwnd, SCI_NEWLINE, 0, 0);
    }
  }
  SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
  n2e_Free(pszPrefixText);
  n2e_Free(pszForeignPrefixText);
}

void n2e_AdjustOffset(const int pos)
{
  N2E_TRACE_PLAIN("UTF8 %d", mEncoding[iEncoding].uFlags & NCP_UTF8);
  N2E_TRACE_PLAIN("8Bit %d", mEncoding[iEncoding].uFlags & NCP_8BIT);
  N2E_TRACE_PLAIN("UNicode %d", mEncoding[iEncoding].uFlags & NCP_UNICODE);
  N2E_TRACE_PLAIN("offset is %d", pos);
}

void n2e_JumpToOffset(const HWND hwnd, const int iNewPos)
{
  n2e_AdjustOffset(iNewPos);
  EditSelectEx(hwnd, iNewPos, iNewPos);
}

BOOL n2e_IsCommentStyle(PEDITSTYLE pStyle)
{
  return pStyle && (StrStrI(pStyle->pszName, L"comment") != NULL);
}

PEDITSTYLE n2e_GetStyleById(const int iStyle)
{
  PEDITSTYLE pStyle = pLexCurrent->Styles;
  int i = 0;
  while (pStyle && (pStyle->iStyle >= 0))
  {
    if (((iStyle == 0) && pStyle->iStyle == iStyle)
      || (MULTI_STYLE_STYLE1(pStyle->iStyle) == iStyle)
      || (MULTI_STYLE_STYLE2(pStyle->iStyle) == iStyle)
      || (MULTI_STYLE_STYLE3(pStyle->iStyle) == iStyle)
      || (MULTI_STYLE_STYLE4(pStyle->iStyle) == iStyle))
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
  const int iTestPos = pED->m_tr.m_iSelStart + pED->m_tb.m_iPos + iPos;
  const HWND _hwnd = hwnd ? hwnd : hwndEdit;
  const DWORD dwStyle = (int)SendMessage(_hwnd, SCI_GETSTYLEAT, iTestPos, 0);
  const PEDITSTYLE pStyle = n2e_GetStyleById(dwStyle);
  return (pStyle
            && (StrStrI(pStyle->pszName, L"comment") != NULL)
            && n2e_IsSingleLineCommentStyle(pLexCurrent->iLexer, dwStyle));
    //|| (SciCall_GetLength() == iTestPos);
}

BOOL n2e_CommentStyleIsDefined(const HWND hwnd)
{
  PEDITSTYLE pStyle = pLexCurrent->Styles;
  int i = 0;
  while (pStyle && (pStyle->iStyle >= 0))
  {
    if (n2e_IsCommentStyle(pStyle))
      return TRUE;

    pStyle = &pLexCurrent->Styles[++i];
  }
  return FALSE;
}

int n2e_FindTextImpl(const HWND hwnd, LPCEDITFINDREPLACE lpefr, struct TextToFind* pttf)
{
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

int FindTextTest(const HWND hwnd, LPCEDITFINDREPLACE lpefr, const struct TextToFind* pttf, const int cpMin)
{
  struct TextToFind ttf = *pttf;
  ttf.chrg.cpMin = cpMin;
  return n2e_FindTextImpl(hwnd, lpefr, &ttf);
}

BOOL n2e_CheckTextExists(const HWND hwnd, LPCEDITFINDREPLACE lpefr, const struct TextToFind* pttf, const int iPos)
{
  return (FindTextTest(hwnd, lpefr, pttf, iPos) >= 0);
}

void n2e_FindNextWord(const HWND hwnd, LPCEDITFINDREPLACE lpefr, const BOOL next)
{
  EDITFINDREPLACE efr = *lpefr;
  struct Sci_TextRange tr;
  struct Sci_TextToFind ttf;
  int searchflags = SCFIND_WHOLEWORD;
  BOOL has = FALSE;
#define _N2E_SEARCH_FOR_WORD_LIMIT 0x100
  N2E_TRACE(L"look for next(%d) word", next);
  ZeroMemory(&ttf, sizeof(ttf));
  ttf.lpstrText = 0;
  const int cpos = SciCall_GetCurrentPos();
  const int doclen = SciCall_GetLength();
  if (SciCall_GetSelEnd() - SciCall_GetSelStart() > 0)
  {
    tr.chrg.cpMin = SciCall_GetSelStart();
    tr.chrg.cpMax = SciCall_GetSelEnd();
  }
  else
  {
    tr.chrg.cpMin = SciCall_GetWordStartPos(cpos, TRUE);
    tr.chrg.cpMax = SciCall_GetWordEndPos(cpos, TRUE);
  }
  int wlen = tr.chrg.cpMax - tr.chrg.cpMin;
  int res = 0;

  has = wlen > 0;

  // look up for new word for search
  if (!has)
  {
    tr.chrg.cpMin = cpos;
    tr.chrg.cpMax = min(cpos + _N2E_SEARCH_FOR_WORD_LIMIT, doclen);
    wlen = tr.chrg.cpMax - tr.chrg.cpMin;
    if (wlen > 0)
    {
      int counter;
      char symb;
      tr.lpstrText = n2e_GetTextRange(tr.chrg.cpMin, tr.chrg.cpMax);
      counter = 0;
      while (counter <= wlen)
      {
        ++counter;
        symb = tr.lpstrText[counter];
        if (N2E_IS_LITERAL(symb))
        {
          if (!res)
          {
            res = counter;
          }
        }
        else
        {
          if (res)
          {
            tr.lpstrText[counter] = '\0';
            ttf.lpstrText = tr.lpstrText + res;
            if (next)
            {
              tr.chrg.cpMax = cpos + counter;                
            }
            else
            {
              tr.chrg.cpMin = cpos;
            }
            break;
          }
        }
      }
    }
  }
  else
  {
    tr.lpstrText = n2e_GetTextRange(tr.chrg.cpMin, tr.chrg.cpMax);
    ttf.lpstrText = tr.lpstrText;
    res = 1;
  }

  if (res)
  {
    N2E_TRACE("search for '%s' ", ttf.lpstrText);
    n2e_FindMRUAdd(ttf.lpstrText);
    lstrcpyA(lpefr->szFind, ttf.lpstrText);
    lstrcpyA(lpefr->szFindUTF8, ttf.lpstrText);
    if (next)
    {
      ttf.chrg.cpMin = tr.chrg.cpMax;
      ttf.chrg.cpMax = doclen;
    }
    else
    {
      ttf.chrg.cpMin = tr.chrg.cpMin;
      ttf.chrg.cpMax = 0;
    }
    if (bFindWordMatchCase)
    {
      searchflags |= SCFIND_MATCHCASE;
    }
    efr.fuFlags = searchflags;

    res = n2e_FindTextImpl(hwnd, &efr, &ttf);
    const BOOL bTextFound = (res >= 0);
    n2e_UpdateFindIcon(bTextFound && (FindTextTest(hwnd, &efr, &ttf, res + 1) >= 0));

    if ((-1 == res) && (bFindWordWrapAround != 0))
    {
      if (next)
      {
        ttf.chrg.cpMin = 0;
        ttf.chrg.cpMax = tr.chrg.cpMin;
      }
      else
      {
        ttf.chrg.cpMin = doclen;
        ttf.chrg.cpMax = tr.chrg.cpMax;
      }
      res = n2e_FindTextImpl(hwnd, &efr, &ttf);
      n2e_UpdateFindIcon(res >= 0);
    }
    if (res >= 0)
    {
      EditSelectEx(hwnd, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
    }
    else
    {
      SciCall_SetCurrentPos(cpos);
    }
    if (tr.lpstrText)
    {
      n2e_Free(tr.lpstrText);
      tr.lpstrText = 0;
    }
  }
}

BOOL n2e_OpenNextFile(const HWND hwnd, LPCWSTR file, const BOOL next)
{
  WCHAR dirname[MAX_PATH], odn[MAX_PATH], found_path[MAX_PATH], *filename;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA ffd;
  INT cmp_res;
  *found_path = L'\0';
  filename = PathFindFileName(file);
  StrCpy(dirname, file);
  if (!PathRemoveFileSpec(dirname))
  {
    return FALSE;
  }
  if (L'\\' != dirname[lstrlen(dirname) - 1])
  {
    StrCat(dirname, L"\\");
  }
  StrCpy(odn, dirname);

  // [2e]: Open Next/Previous - use current dialog filter #277
  WCHAR szFileFilter[MAX_PATH] = { L"*" };
  if (iOpenSaveFilterIndex >= 1)
  {
    WCHAR szFilter[MAX_PATH] = { 0 };
    Style_GetOpenDlgFilterStr(szFilter, COUNTOF(szFilter));

    LPCWSTR psz = szFilter;
    int iFilterIndex = iOpenSaveFilterIndex;
    int iLineIndex = 0;
    while (*psz)
    {
      if (iLineIndex % 2)
      {
        --iFilterIndex;
        if (iFilterIndex == 0)
        {
          StrCpy(szFileFilter, psz);
          break;
        }
      }
      psz += wcslen(psz) + 1;
      ++iLineIndex;
    }
  }
  // [/2e]
  StrCat(dirname, szFileFilter);

  hFind = FindFirstFile(dirname, &ffd);
  if (INVALID_HANDLE_VALUE == hFind)
  {
    return FALSE;
  }
  do
  {
    if (0 == (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      cmp_res = n2e_CompareFiles(filename, ffd.cFileName);
      N2E_TRACE(L"%S vs %S = %d", ffd.cFileName, filename, cmp_res);
      if ((next && cmp_res >= 0) || (!next&&cmp_res <= 0))
      {
        continue;
      }
      if (*found_path)
      {
        cmp_res = n2e_CompareFiles(found_path, ffd.cFileName);
      }
      else
      {
        cmp_res = 0;
      }
      N2E_TRACE(L"%S vs %S = %d", ffd.cFileName, found_path, cmp_res);
      if ((next && cmp_res >= 0) || (!next&&cmp_res <= 0))
      {
        StrCpy(found_path, ffd.cFileName);
        N2E_TRACE(L"saved %S", found_path);
      }
    }
  } while (FindNextFile(hFind, &ffd));
  FindClose(hFind);
  if (*found_path)
  {
    StrCat(odn, found_path);
    N2E_TRACE(L"file to open %S", odn);
    FileLoad(FALSE, FALSE, FALSE, FALSE, odn);
  }
  return TRUE;
}

void n2e_UnwrapSelection(const HWND hwnd, const BOOL quote_mode)
{
  const static int max_region_to_scan = 1024;
  const auto pos = SciCall_GetCurrentPos();
  const auto len = SciCall_GetLength();
  int posStart = -1;
  int posEnd = -1;
  int p = pos;
  do
  {
    posStart = SciCall_BraceMatch(p, quote_mode);
    if (posStart < 0)
    {
      if (p == 0)
        break;
      p = SciCall_PositionBefore(p);
    }
  } while ((p >= 0) && (p >= pos - max_region_to_scan) && (posStart < 0));

  if (posStart >= 0)
    posEnd = SciCall_BraceMatch(posStart, quote_mode);

  if ((posStart >= 0) && (posEnd >= 0) && (max(posStart, posEnd) >= pos))
  {
    SciCall_BeginUndoAction();
    SciCall_DeleteRange(max(posStart, posEnd), 1);
    SciCall_DeleteRange(min(posStart, posEnd), 1);
    SciCall_SetSel(min(posStart, posEnd), max(posStart, posEnd) - 1);
    SciCall_EndUndoAction();
  }
}

void n2e_EscapeHTML(const HWND hwnd)
{
  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }
  size_t symb;
  BOOL changed = FALSE;
  struct Sci_TextToFind ttf;
  const char* _source = "&<>";
  const char* _target[] = { "&amp;", "&lt;", "&gt;" };
  assert(strlen(_source) == COUNTOF(_target));
  SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
  int beg = SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  int end = SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  if (beg == end)
  {
    beg = 0;
    end = SendMessage(hwnd, SCI_GETTEXTLENGTH, 0, 0);
  }
  char buffer[2] = { 0, 0 };
  ttf.lpstrText = buffer;
  changed = FALSE;
  for (symb = 0; symb < strlen(_source); ++symb)
  {
    ttf.chrg.cpMin = beg;
    ttf.chrg.cpMax = end;
    buffer[0] = _source[symb];
    int res = 0;
    while (-1 != res)
    {
      res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf);
      if (-1 != res)
      {
        {
          assert(ttf.chrgText.cpMax == ttf.chrgText.cpMin + 1);
          SendMessage(hwnd, SCI_DELETERANGE, ttf.chrgText.cpMin, 1);
          SendMessage(hwnd, SCI_INSERTTEXT, ttf.chrgText.cpMin, (LPARAM)_target[symb]);
          ttf.chrg.cpMin = ttf.chrgText.cpMax;
          end += strlen(_target[symb]) - 1;
          ttf.chrg.cpMax = end;
          changed = TRUE;
        }
      }
    }
  }
  if (changed)
  {
    SendMessage(hwnd, SCI_SETSEL, beg, end);
  }
  SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
}

void n2e_UpdateFindIcon(const BOOL findOK)
{
  TBBUTTON* pBtn = &tbbMainWnd[FIND_INFO_INDEX];
  pBtn->iBitmap = findOK ? ICON_FIND_OK : ICON_FIND_FAILED;

  TBBUTTONINFO tbbi = { 0 };
  tbbi.cbSize = sizeof(tbbi);
  tbbi.idCommand = pBtn->idCommand;
  tbbi.iImage = pBtn->iBitmap;
  tbbi.dwMask = TBIF_IMAGE;
  SendMessage(hwndToolbar, TB_SETBUTTONINFO, tbbi.idCommand, (LPARAM)&tbbi);
}

void n2e_UpdateFindIconAndFlashWindow(const BOOL findOK)
{
  n2e_UpdateFindIcon(findOK);

  FLASHWINFO fwi = {
    .cbSize = sizeof(FLASHWINFO),
    .hwnd = hwndMain,
    .dwFlags = FLASHW_CAPTION,
    .uCount = 1,
    .dwTimeout = 0
  };
  FlashWindowEx(&fwi);
}

void n2e_ResetFindIcon()
{
  n2e_UpdateFindIcon(TRUE);
}

void n2e_UpdateAlwaysOnTopButton()
{
  SendMessage(hwndToolbar, TB_SETSTATE, IDM_VIEW_ALWAYSONTOP, MAKELPARAM(TBSTATE_ENABLED | (((bAlwaysOnTop || flagAlwaysOnTop == 2) && flagAlwaysOnTop != 1) ? TBSTATE_PRESSED : 0), 0));
}

void n2e_EditString2Hex(const HWND hwnd)
{
  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }
  EncodeStrToHex(hwnd);
}

void n2e_EditHex2String(const HWND hwnd)
{
  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }
  DecodeHexToStr(hwnd);
}

void n2e_EditString2Base64(const HWND hwnd)
{
  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }
  EncodeStrToBase64(hwnd);
}

void n2e_EditBase642String(const HWND hwnd)
{
  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }
  DecodeBase64ToStr(hwnd);
}

void n2e_EditString2QP(const HWND hwnd)
{
  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }
  EncodeStrToQP(hwnd);
}

void n2e_EditQP2String(const HWND hwnd)
{
  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }
  DecodeQPToStr(hwnd);
}

void n2e_EditString2URL(const HWND hwnd)
{
  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }
  EncodeStrToURL(hwnd);
}

void n2e_EditURL2String(const HWND hwnd)
{
  if (n2e_ShowPromptIfSelectionModeIsRectangle(hwnd))
  {
    return;
  }
  DecodeURLToStr(hwnd);
}

LPCWSTR GetControlIDAsString(const UINT nCtrlID)
{
  static WCHAR wchBuffer[20];
  return _itow(nCtrlID, wchBuffer, 16);
}

UINT GetCheckboxState(const HWND hwnd, const UINT nCtrlID)
{
  return IsDlgButtonChecked(hwnd, nCtrlID);
}

void SaveCheckboxState(const HWND hwnd, const UINT nCtrlID)
{
  SetProp(hwnd, GetControlIDAsString(nCtrlID), (HANDLE)GetCheckboxState(hwnd, nCtrlID));
}

UINT RestoreCheckboxState(const HWND hwnd, const UINT nCtrlID)
{
  return GetProp(hwnd, GetControlIDAsString(nCtrlID)) ? BST_CHECKED : BST_UNCHECKED;
}

BOOL n2e_IsCheckboxChecked(const HWND hwnd, const UINT nCtrlID, const BOOL bCheckRestoredState)
{
  return (GetCheckboxState(hwnd, nCtrlID) == BST_CHECKED)
    || (bCheckRestoredState && (RestoreCheckboxState(hwnd, nCtrlID) == BST_CHECKED));
}

void UpdateCheckboxState(const HWND hwnd, const UINT nCtrlID, const BOOL bRestoreState, const BOOL bEnabled)
{
  CheckDlgButton(hwnd, nCtrlID, bRestoreState ? RestoreCheckboxState(hwnd, nCtrlID) : BST_UNCHECKED);
  EnableWindow(GetDlgItem(hwnd, nCtrlID), bEnabled);
}

void n2e_SaveCheckboxes(const HWND hwnd)
{
  SaveCheckboxState(hwnd, IDC_FINDWORD);
  SaveCheckboxState(hwnd, IDC_FINDSTART);
  SaveCheckboxState(hwnd, IDC_FINDTRANSFORMBS);
}

void UpdateCheckboxesImpl(HWND hwnd, const UINT nCtrlID, const BOOL bInitialUpdate)
{
  const BOOL bRegexModeChanged = (nCtrlID == IDC_FINDREGEXP)
    || ((nCtrlID == IDC_FINDTRANSFORMBS) && (GetCheckboxState(hwnd, nCtrlID) == BST_CHECKED) && (GetCheckboxState(hwnd, IDC_FINDREGEXP) == BST_CHECKED));

  switch (nCtrlID)
  {
    case IDC_FINDWORD:
      CheckDlgButton(hwnd, IDC_FINDSTART, BST_UNCHECKED);
      break;
    case IDC_FINDSTART:
      CheckDlgButton(hwnd, IDC_FINDWORD, BST_UNCHECKED);
      break;
    case IDC_FINDREGEXP:
      CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_UNCHECKED);
      break;
    case IDC_FINDTRANSFORMBS:
      CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
      break;
    default:
      if (bInitialUpdate)
        break;
      return;
  }

  const BOOL bIsRegexMode = (GetCheckboxState(hwnd, IDC_FINDREGEXP) == BST_CHECKED);
  if (bInitialUpdate || (bRegexModeChanged && bIsRegexMode))
  {
    if (bInitialUpdate && n2e_IsCheckboxChecked(hwnd, IDC_FINDWORD, FALSE))
    {
      CheckDlgButton(hwnd, IDC_FINDSTART, BST_UNCHECKED);
    }
    SaveCheckboxState(hwnd, IDC_FINDTRANSFORMBS);
    SaveCheckboxState(hwnd, IDC_FINDWORD);
    SaveCheckboxState(hwnd, IDC_FINDSTART);
  }
  if (bRegexModeChanged || bIsRegexMode)
  {
    UpdateCheckboxState(hwnd, IDC_FINDWORD, TRUE, !bIsRegexMode);
    UpdateCheckboxState(hwnd, IDC_FINDSTART, TRUE, !bIsRegexMode);
    if (nCtrlID != IDC_FINDTRANSFORMBS)
    {
      UpdateCheckboxState(hwnd, IDC_FINDTRANSFORMBS, !bIsRegexMode, TRUE);
    }
  }
  else
  {
    SaveCheckboxState(hwnd, IDC_FINDREGEXP);
    UpdateCheckboxState(hwnd, IDC_FINDREGEXP, bIsRegexMode, TRUE);
  }
}

void n2e_EditFindReplaceUpdateCheckboxes(const HWND hwnd, const UINT nCtrlID)
{
  UpdateCheckboxesImpl(hwnd, nCtrlID, FALSE);
}

void n2e_EditFindReplaceInitialUpdateCheckboxes(const HWND hwnd)
{
  UpdateCheckboxesImpl(hwnd, 0, TRUE);
}

void remove_char(char* str, char c)
{
  char *pr = str, *pw = str;
  while (*pr)
  {
    *pw = *pr++;
    pw += (*pw != c);
  }
  *pw = '\0';
}

int n2e_MultiByteToWideChar(LPCSTR lpMultiByteStr, const int cbMultiByte, LPWSTR lpWideCharStr, const int cchWideChar)
{
  return MultiByteToWideChar(SciCall_GetCodePage(), 0, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

LPWSTR n2e_MultiByteToWideString(LPCSTR text)
{
  const int textLength = n2e_MultiByteToWideChar(text, -1, NULL, 0);
  LPWSTR pWideText = n2e_Alloc(textLength * 2);
  n2e_MultiByteToWideChar(text, -1, pWideText, textLength);
  return pWideText;
}

BOOL n2e_FilteredPasteFromClipboard(const HWND hwnd)
{
  char *pClip = EditGetClipboardText(hwndEdit);
  if (pClip)
  {
    remove_char(pClip, '\r');
    remove_char(pClip, '\n');
    const LPWSTR pWideText = n2e_MultiByteToWideString(pClip);
    SendMessage(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)pWideText);
    n2e_Free(pWideText);
    LocalFree(pClip);
    return TRUE;
  }
  return FALSE;
}

BOOL n2e_CheckWindowClassName(const HWND hwnd, LPCWSTR lpwstrClassname)
{
  WCHAR wchClassName[MAX_PATH];
  RealGetWindowClass(hwnd, wchClassName, _countof(wchClassName));
  return (_wcsicmp(wchClassName, lpwstrClassname) == 0);
}

LRESULT CALLBACK n2e_FilterClipboardEditWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_PASTE:
      n2e_FilteredPasteFromClipboard(hwnd);
      return 0;
    default:
      break;
  }
  return n2e_CallOriginalWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK n2e_FindEditWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_PASTE:
      n2e_FilteredPasteFromClipboard(hwnd);
      return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDACC_BACK:
          {
            const HWND hwndCombo = GetParent(hwnd);
            const UINT idControl = GetWindowLong(hwndCombo, GWL_ID);
            WCHAR buf[1024];
            GetWindowText(hwnd, buf, COUNTOF(buf)-1);
            const int car = LOWORD(SendMessage(hwnd, EM_GETSEL, 0, 0));
            const int len = min(lstrlen(buf) - 1, car - 1);
            int cou = len;
            BOOL got = FALSE;
            int curr = 0, prev = 0;
            while (cou >= 0)
            {
              WCHAR ch = buf[cou];
              if (iswspace(ch))
              {
                curr = 0;
              }
              else if (N2E_IS_LITERAL(ch))
              {
                curr = 1;
              }
              else
              {
                curr = -1;
              }
              if (got)
              {
                if (!curr)
                {
                  break;
                }
                else if (curr != prev)
                {
                  break;
                }
              }
              else
              {
                got = curr;
              }
              prev = curr;
              --cou;
            }
            if (cou != len)
            {
              SendMessage(hwnd, EM_SETSEL, cou + 1, car);
              SendMessage(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)L"");
            }
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  const auto res = n2e_CallOriginalWindowProc(hwnd, uMsg, wParam, lParam);
  if ((uMsg == WM_SETTEXT) || (uMsg == EM_REPLACESEL))
  {
    const HWND hwndCombo = GetParent(hwnd);
    PostMessage(GetParent(hwndCombo), WM_COMMAND, MAKELONG(GetWindowLong(hwndCombo, GWL_ID), CBN_EDITCHANGE), 0);
  }
  return res;
}

BOOL n2e_EnableClipboardFiltering(const HWND hwnd, const UINT idEdit)
{
  const HWND hwndEditCtrl = GetDlgItem(hwnd, idEdit);
  return n2e_CheckWindowClassName(hwndEditCtrl, WC_EDIT)
          ? n2e_SubclassWindow(hwndEditCtrl, n2e_FilterClipboardEditWndProc)
          : FALSE;
}

BOOL n2e_SubclassFindEditInCombo(const HWND hwnd, const UINT idCombo)
{
  const HWND hwndCombo = GetDlgItem(hwnd, idCombo);
  const HWND hwndEditCtrl = FindWindowEx(hwndCombo, NULL, WC_EDIT, NULL);
  if (hwndEditCtrl && !n2e_IsSubclassedWindow(hwndEditCtrl))
  {
    n2e_SubclassWindow(hwndEditCtrl, n2e_FindEditWndProc);
    return TRUE;
  }
  return FALSE;
}

extern WCHAR last_selected[MAX_PATH];

LRESULT CALLBACK n2e_OpenDialogWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
          {
            WCHAR buf[MAX_PATH];
            CommDlg_OpenSave_GetSpec(hwnd, buf, COUNTOF(buf));
            LPWSTR final_str = buf;
            if (wcsstr(last_selected, buf))
            {
              final_str = last_selected;
              N2E_TRACE("OFN drop window text %S ", buf);
            }
            WIN32_FIND_DATA fd = { 0 };
            BOOL bFolderExists = FALSE;
            HANDLE hFind = FindFirstFile(final_str, &fd);
            if (hFind != INVALID_HANDLE_VALUE)
            {
              do
              {
                if (((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                    && (StrCmpI(final_str, fd.cFileName) == 0))
                {
                  bFolderExists = TRUE;
                  break;
                }
              } while (FindNextFile(hFind, &fd));
              FindClose(hFind);
            }
            if (!bFolderExists)
            {
              OFNOTIFY ofn = { 0 };
              ofn.hdr.code = CDN_FILEOK;               
              SendMessage(FindWindowEx(hwnd, NULL, WC_DIALOG, NULL), WM_NOTIFY, MAKEWPARAM(IDOK, 0), (LPARAM)&ofn);
            }
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  return n2e_CallOriginalWindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL n2e_SubclassOpenDialog(const HWND hwnd)
{
  return n2e_SubclassWindow(hwnd, n2e_OpenDialogWndProc);
}

const WCHAR* _left_braces = L"<{([";
const WCHAR* _right_braces = L">})]";

void n2e_Init_EditInsertTagDlg(const HWND hwnd)
{
  const int len = lstrlen(wchLastHTMLTag);
  int k = 0;
  while (1)
  {
    if (len > k * 2 + 1 &&
        StrChr(_left_braces, wchLastHTMLTag[k]) &&
        StrChr(_right_braces, wchLastHTMLTag[len - k - 1]))
    {
      ++k;
    }
    else
    {
      break;
    }
  }
  if (k)
  {
    PostMessage(GetDlgItem(hwnd, 100), EM_SETSEL, k, len - k);
  }
  else
  {
    PostMessage(GetDlgItem(hwnd, 100), EM_SETSEL, 0, len);
  }
}

BOOL n2e_IsValidClosingTagA(LPCSTR pTag)
{
  const UINT codePage = SendMessage(hwndEdit, SCI_GETCODEPAGE, 0, 0);
  const int textLength = MultiByteToWideChar(codePage, 0, pTag, -1, NULL, 0);
  LPWSTR pwchTag = LocalAlloc(LPTR, textLength * 2);
  MultiByteToWideChar(codePage, 0, pTag, -1, pwchTag, textLength);

  const BOOL res = n2e_IsValidClosingTagW(pwchTag);
  LocalFree(pwchTag);
  return res;;
}

BOOL n2e_IsValidClosingTagW(LPCWSTR pwchTag)
{
  LPCWSTR arrInvalidClosingTags[17] = {
    L"</area>",
    L"</base>",
    L"</basefont>",
    L"</bgsound>",
    L"</br>",
    L"</col>",
    L"</embed>",
    L"</frame>",
    L"</hr>",
    L"</img>",
    L"</input>",
    L"</keygen>",
    L"</link>",
    L"</meta>",
    L"</param>",
    L"</source>",
    L"</track>"
  };

  for (int i = 0; i < _countof(arrInvalidClosingTags); ++i)
  {
    if (lstrcmpi(pwchTag, arrInvalidClosingTags[i]) == 0)
    {
      return FALSE;
    }
  }
  return TRUE;
}

WCHAR* n2e_GetClosingTagText_EditInsertTagDlg(WCHAR* wchBuf)
{
  static WCHAR wchIns[TEXT_BUFFER_LENGTH];
  WCHAR *pwCur;
  int  cchIns = 2;
  BOOL bClear = TRUE;
  BOOL bCopy = FALSE;

  if (lstrlen(wchBuf) >= 3)
  {
    if (((StrCmpNI(wchBuf, L"<!--", 4) == 0) && (StrStrI(wchBuf + 4, L"-->") != NULL))
        || ((StrCmpNI(wchBuf, L"<!DOCTYPE", 9) == 0) && (StrStrI(wchBuf + 9, L">") != NULL)))
    {
      bClear = TRUE;
      bCopy = FALSE;
    }
    else if (StrChr(_left_braces, *wchBuf))
    {
      int open_tag_len = 0;
      wchIns[0] = *wchBuf;
      // detect len of open tag
      while (StrChr(_left_braces, *(wchBuf + (++open_tag_len))))
      {
        wchIns[open_tag_len] = *(wchBuf + open_tag_len);
      }
      wchIns[open_tag_len] = L'/';
      wchIns[open_tag_len + 1] = L'\0';
      // get next char
      pwCur = wchBuf + open_tag_len;
      cchIns += open_tag_len - 1;

      // extract tag
      // trim left
      while (
        *pwCur &&
        !N2E_IS_LITERAL(*pwCur)
        )
      {
        *pwCur++;
      }
      while (
        *pwCur &&
        !StrChr(_left_braces, *pwCur) &&
        !StrChr(_right_braces, *pwCur) &&
        N2E_IS_LITERAL(*pwCur))
      {
        wchIns[cchIns++] = *pwCur++;
      }
      // get end of string
      while (
        *pwCur &&
        !StrChr(_right_braces, *pwCur))
      {
        pwCur++;
      }
      // if not short version
      if (*(pwCur - 1) != L'/')
      {
        while (open_tag_len--)
        {
          wchIns[cchIns++] = _right_braces[StrChr(_left_braces, wchIns[open_tag_len]) - _left_braces];
        }
        wchIns[cchIns] = L'\0';
        if (cchIns > 3 && n2e_IsValidClosingTagW(wchIns))
        {
          bClear = FALSE;
        }
      }
      else
      {
        bCopy = TRUE;
      }
      N2E_WTRACE_PLAIN("wchIns %s", wchIns);
      N2E_WTRACE_PLAIN("pwCur %s", pwCur);
    }
    else
    {
      bCopy = TRUE;
    }
  }
  else
  {
    bCopy = TRUE;
  }
  if (bCopy)
  {
    return wchBuf;
  }
  else if (bClear)
  {
    return L"";
  }
  return wchIns;
}

void n2e_SaveTagsData_EditInsertTagDlg(PTAGSDATA pdata)
{
  // correct pwsz1 according to pwsz2
  int idx = 0, len = 0;
  len = lstrlen(pdata->pwsz1);
  while (len > 0 && StrChr(_right_braces, pdata->pwsz1[len - 1]))
  {
    pdata->pwsz1[--len] = L'\0';
  }
  while (1)
  {
    int k;
    WCHAR const* br = StrChr(_left_braces, pdata->pwsz1[idx++]);
    if (!br)
    {
      break;
    }
    for (k = idx; k >= 0; --k)
    {
      pdata->pwsz1[len + k + 1] = pdata->pwsz1[len + k];
    }
    pdata->pwsz1[len] = _right_braces[br - _left_braces];
    N2E_WTRACE_PLAIN("pdata->pwsz1 %s", pdata->pwsz1);
  }
  lstrcpy(wchLastHTMLTag, pdata->pwsz1);
  lstrcpy(wchLastHTMLEndTag, pdata->pwsz2);
}

void n2e_int2bin(unsigned int val, LPWSTR binString)
{
  int bitCount = 0;
  int i;
  WCHAR binString_temp[MAX_PATH];

  do
  {
    binString_temp[bitCount++] = '0' + val % 2;
    val /= 2;
  } while (val > 0);

  /* Reverse the binary string */
  for (i = 0; i < bitCount; i++)
    binString[i] = binString_temp[bitCount - i - 1];

  binString[bitCount] = 0;
}

BOOL n2e_IsExpressionEvaluationEnabled()
{
  switch (iEvaluateMathExpression)
  {
    case EEM_DISABLED:
    default:
      return FALSE;
    case EEM_SELECTION:
    case EEM_LINE:
      return TRUE;
  }
}

int n2e_GetExpressionTextRange(int* piStart, int* piEnd)
{
  *piStart = *piEnd = 0;
  switch (iEvaluateMathExpression)
  {
    case EEM_DISABLED:
      break;
    case EEM_SELECTION:
      *piStart = SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0);
      *piEnd = SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0);
      break;
    case EEM_LINE:
      *piStart = SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0);
      *piEnd = SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0);
      if (*piEnd == *piStart)
      {
        const int iCurLine = (int)SendMessage(hwndEdit, SCI_LINEFROMPOSITION, *piStart, 0);
        *piStart = SendMessage(hwndEdit, SCI_POSITIONFROMLINE, iCurLine, 0);
        *piEnd = SendMessage(hwndEdit, SCI_GETLINEENDPOSITION, iCurLine, 0);
      }
      break;
  }
  return *piEnd - *piStart;
}

void n2e_CopyEvaluatedExpressionToClipboard()
{
  const int iEvaluateMathExpressionOrigin = iEvaluateMathExpression;
  iEvaluateMathExpression = EEM_LINE;
  char arrchText[MAX_EXPRESSION_LENGTH] = { 0 };
  WCHAR arrwchValue[MAX_EXPRESSION_LENGTH] = { 0 };
  if (n2e_FormatEvaluatedExpression(hwndEdit,
        arrchText, COUNTOF(arrchText),
        arrwchValue, COUNTOF(arrwchValue), FALSE))
  {
    if (flagPasteBoard)
      bLastCopyFromMe = TRUE;
    n2e_SetClipboardText(hwndMain, arrwchValue);
  }
  iEvaluateMathExpression = iEvaluateMathExpressionOrigin;
}

BOOL n2e_IsFindReplaceAvailable(LPCEDITFINDREPLACE lpefr)
{
#ifndef ICU_BUILD
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

LPCSTR n2e_FormatLineText(LPSTR buf, const int iLineStart, const int iLineIndex, const int iDocumentLineIndex,
  LPCSTR lpPrefixAbsFormat, LPCSTR lpPrefixAbsZeroFormat,
  LPCSTR lpPrefixRelFormat, LPCSTR lpPrefixRelZeroFormat,
  LPCSTR lpPrefixRel0Format, LPCSTR lpPrefixRel0ZeroFormat)
{
  n2e_ReplaceSubstringFormat(&buf[0], "$(L)", lpPrefixAbsFormat, iDocumentLineIndex);
  n2e_ReplaceSubstringFormat(&buf[0], "$(0L)", lpPrefixAbsZeroFormat, iDocumentLineIndex);
  n2e_ReplaceSubstringFormat(&buf[0], "$(N)", lpPrefixRelFormat, iLineIndex - iLineStart + 1);
  n2e_ReplaceSubstringFormat(&buf[0], "$(0N)", lpPrefixRelZeroFormat, iLineIndex - iLineStart + 1);
  n2e_ReplaceSubstringFormat(&buf[0], "$(I)", lpPrefixRel0Format, iLineIndex - iLineStart);
  n2e_ReplaceSubstringFormat(&buf[0], "$(0I)", lpPrefixRel0ZeroFormat, iLineIndex - iLineStart);
  return buf;
}

LPCSTR n2e_GetBracesList()
{
  return bTreatQuotesAsBraces ? BRACES_WITH_QUOTES : BRACES;
}

BOOL n2e_InitTextFromSelection(HWND hwnd, const UINT uiControlID, HWND _hwndEdit, const BOOL bAllowEmptyString)
{
  const UINT uCPEdit = (UINT)SendMessage(_hwndEdit, SCI_GETCODEPAGE, 0, 0);

  int cchSelection = (int)SendMessage(_hwndEdit, SCI_GETSELECTIONEND, 0, 0) -
    (int)SendMessage(_hwndEdit, SCI_GETSELECTIONSTART, 0, 0);

  if ((bAllowEmptyString || (cchSelection > 0)) && (cchSelection <= 500))
  {
    cchSelection = (int)SendMessage(_hwndEdit, SCI_GETSELTEXT, 0, 0);
    char* lpszSelection = GlobalAlloc(GPTR, cchSelection + 2);
    SendMessage(_hwndEdit, SCI_GETSELTEXT, 0, (LPARAM)lpszSelection);

    // Check lpszSelection and truncate bad chars
    char* lpsz = StrChrA(lpszSelection, 13);
    if (lpsz)
      *lpsz = '\0';

    lpsz = StrChrA(lpszSelection, 10);
    if (lpsz)
      *lpsz = '\0';

    lpsz = StrChrA(lpszSelection, 9);
    if (lpsz)
      *lpsz = '\0';

    SetDlgItemTextA2W(uCPEdit, hwnd, uiControlID, lpszSelection);
    GlobalFree(lpszSelection);

    return TRUE;
  }
  return FALSE;
}
