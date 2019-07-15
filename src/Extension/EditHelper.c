#include "EditHelper.h"
#include <cassert>
#include "CommonUtils.h"
#include "Dialogs.h"
#include "Helpers.h"
#include "Edit.h"
#include "ExtSelection.h"
#include "Notepad2.h"
#include "resource.h"
#include "SciCall.h"
#include "StrToBase64.h"
#include "StrToHex.h"
#include "StrToQP.h"
#include "StrToURL.h"
#include "StringRecoding.h"
#include "Subclassing.h"
#include "Trace.h"
#include "Utils.h"

WCHAR wchLastHTMLTag[0xff] = L"<tag>";
WCHAR wchLastHTMLEndTag[0xff] = L"</tag>";

extern NP2ENCODING mEncoding[];
extern int iEncoding;
extern int bFindWordMatchCase;
extern int bFindWordWrapAround;
extern TBBUTTON tbbMainWnd[];
extern HWND hwndToolbar;
extern HWND hwndMain;
extern HWND hDlgFindReplace;

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
  if (SC_SEL_RECTANGLE == SendMessage(hwnd, SCI_GETSELECTIONMODE, 0, 0))
  {
    MsgBox(MBINFO, IDS_SELRECT);
    return TRUE;
  }
  return FALSE;
}

extern BOOL bAutoIndent;

LPSTR GetLinePrefix(const HWND hwnd, const int iLine, LPBOOL pbLineEmpty)
{
  *pbLineEmpty = TRUE;
  LPSTR pszPrefix = NULL;

  const int iCurPos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  const int iLineStart = SendMessage(hwnd, SCI_POSITIONFROMLINE, iLine, 0);
  const int iLinePrefixLength = iCurPos - iLineStart;
  pszPrefix = (LPSTR)GlobalAlloc(GPTR, iLinePrefixLength + 1);
  struct TextRange tr = { { iLineStart, iCurPos }, pszPrefix };
  SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

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

void FreeLinePrefix(LPSTR pszPrefix)
{
  if (pszPrefix)
  {
    GlobalFree(pszPrefix);
  }
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

void n2e_EditInsertNewLine(const HWND hwnd, const BOOL insertAbove)
{
  if (n2e_SelectionEditStop(SES_APPLY))
  {
    return;
  }
  const int iCurPos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  const int iCurLine = SendMessage(hwnd, SCI_LINEFROMPOSITION, iCurPos, 0);
  const int iCurLineEndPos = SendMessage(hwnd, SCI_GETLINEENDPOSITION, iCurLine, 0);
  SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
  const int iPrevLine = (iCurLine > 0) ? iCurLine - 1 : 0;
  LPSTR pszPrefixText = NULL;
  BOOL bIsEmptyPrefix = FALSE;

  if (insertAbove)
  {
    const int iPrevLineEndPos = (iPrevLine == iCurLine) ? 0 : SendMessage(hwnd, SCI_GETLINEENDPOSITION, iPrevLine, 0);
    if (bAutoIndent)
    {
      const int iLineStart = SendMessage(hwnd, SCI_POSITIONFROMLINE, iCurLine, 0);
      const int iLinePrefixLength = iCurPos - iLineStart;
      bIsEmptyPrefix = (iLinePrefixLength == 0);
      if (!bIsEmptyPrefix)
      {
        pszPrefixText = GetLinePrefix(hwnd, iCurLine, &bIsEmptyPrefix);
      }
      if (bIsEmptyPrefix)
      {
        SendMessage(hwnd, SCI_SETSEL, iPrevLineEndPos, iPrevLineEndPos);
        SendMessage(hwnd, SCI_NEWLINE, 0, 0);
        if (iPrevLine == iCurLine)
        {
          const int iNewPrevLineEndPos = SendMessage(hwnd, SCI_GETLINEENDPOSITION, iPrevLine, 0);
          SendMessage(hwnd, SCI_SETSEL, iNewPrevLineEndPos, iNewPrevLineEndPos);
        }
      }
      else
      {
        SendMessage(hwnd, SCI_SETSEL, iPrevLineEndPos, iPrevLineEndPos);
        InsertNewLineWithPrefix(hwnd, pszPrefixText, (iPrevLine == iCurLine));
      }
    }
    else
    {
      SendMessage(hwnd, SCI_SETSEL, iPrevLineEndPos, iPrevLineEndPos);
      SendMessage(hwnd, SCI_NEWLINE, 0, 0);
    }
  }
  else
  {
    const BOOL isLineEnd = (iCurPos == iCurLineEndPos);
    if (isLineEnd && bAutoIndent)
    {
      SendMessage(hwnd, SCI_SETSEL, iCurLineEndPos, iCurLineEndPos);
      pszPrefixText = GetLinePrefix(hwnd, iCurLine - 1, &bIsEmptyPrefix);
      InsertNewLineWithPrefix(hwnd, pszPrefixText, FALSE);
    }
    else
    {
      SendMessage(hwnd, SCI_SETSEL, iCurLineEndPos, iCurLineEndPos);
      SendMessage(hwnd, SCI_NEWLINE, 0, 0);
    }
  }
  SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
  FreeLinePrefix(pszPrefixText);
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
  SendMessage(hwnd, SCI_GOTOPOS, (WPARAM)iNewPos, 0);
}

int n2e_FindTextImpl(const HWND hwnd, const int searchFlags, const struct TextToFind* pttf)
{
  return (int)SendMessage(hwnd, SCI_FINDTEXT, searchFlags, (LPARAM)pttf);
}

int FindTextTest(const HWND hwnd, const int searchFlags, const struct TextToFind* pttf, const int cpMin)
{
  struct TextToFind ttf = *pttf;
  ttf.chrg.cpMin = cpMin;
  return n2e_FindTextImpl(hwnd, searchFlags, &ttf);
}

BOOL n2e_CheckTextExists(const HWND hwnd, const int searchFlags, const struct TextToFind* pttf, const int iPos)
{
  return (FindTextTest(hwnd, searchFlags, pttf, iPos) >= 0);
}

void n2e_FindNextWord(const HWND hwnd, LPCEDITFINDREPLACE lpref, const BOOL next)
{
  struct Sci_TextRange tr;
  struct Sci_TextToFind ttf;
  static char* szPrevWord = NULL;
  int searchflags = 0;
  BOOL has = FALSE;
#define _N2E_SEARCH_FOR_WORD_LIMIT 0x100
  N2E_TRACE(L"look for next(%d) word", next);
  ZeroMemory(&ttf, sizeof(ttf));
  ttf.lpstrText = 0;
  const int cpos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  const int doclen = SendMessage(hwnd, SCI_GETTEXTLENGTH, 0, 0);
  tr.chrg.cpMin = SendMessage(hwnd, SCI_WORDSTARTPOSITION, cpos, TRUE);
  tr.chrg.cpMax = SendMessage(hwnd, SCI_WORDENDPOSITION, cpos, TRUE);
  int wlen = tr.chrg.cpMax - tr.chrg.cpMin;
  int res = 0;

  tr.lpstrText = (char*)n2e_Alloc(wlen + 1);
  SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

  const int iSelCount = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) -
                        (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);

  n2e_Free(tr.lpstrText);

  if (iSelCount > 0)
  {
    const size_t prevWordLength = szPrevWord ? strlen(szPrevWord) + 1 : 0;
    if (szPrevWord && (prevWordLength > 0))
    {
      tr.lpstrText = (char*)n2e_Alloc(prevWordLength);
      lstrcpynA(tr.lpstrText, szPrevWord, prevWordLength);
      ttf.lpstrText = tr.lpstrText;
      res = 1;
    }
  }
  if (res == 0)
  {
    has = wlen > 0;

    // look up for new word for search
    if (!has)
    {
      tr.chrg.cpMin = next ? cpos : max(cpos - _N2E_SEARCH_FOR_WORD_LIMIT, 0);
      tr.chrg.cpMax = next ? min(cpos + _N2E_SEARCH_FOR_WORD_LIMIT, doclen) : cpos;
      wlen = tr.chrg.cpMax - tr.chrg.cpMin;
      if (wlen > 0)
      {
        int counter;
        char symb;
        //
        tr.lpstrText = (char*)n2e_Alloc(wlen + 1);
        SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
        counter = 0;
        while (counter <= wlen)
        {
          ++counter;
          symb = next ? tr.lpstrText[counter] : tr.lpstrText[wlen - counter];
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
              if (next)
              {
                tr.chrg.cpMax = cpos + counter;
                tr.lpstrText[counter] = '\0';
                ttf.lpstrText = tr.lpstrText + res;
              }
              else
              {
                tr.chrg.cpMin = cpos - res;
                tr.lpstrText[wlen - res + 1] = '\0';
                ttf.lpstrText = tr.lpstrText + wlen - counter + 1;
              }
              break;
            }
          }
        }
      }
    }
    else
    {
      tr.lpstrText = (char*)n2e_Alloc(wlen + 1);
      SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
      ttf.lpstrText = tr.lpstrText;
      res = 1;
    }
  }

  if (res)
  {
    N2E_TRACE("search for '%s' ", ttf.lpstrText);
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
    searchflags = SCFIND_WHOLEWORD;
    if (bFindWordMatchCase)
    {
      searchflags |= SCFIND_MATCHCASE;
    }

    res = n2e_FindTextImpl(hwnd, searchflags, &ttf);
    const BOOL bTextFound = (res >= 0);
    n2e_UpdateFindIcon(bTextFound && (FindTextTest(hwnd, searchflags, &ttf, res + 1) >= 0));

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
      res = n2e_FindTextImpl(hwnd, searchflags, &ttf);
      n2e_UpdateFindIcon(res >= 0);
    }
    if (res >= 0)
    {
      EditSelectEx(hwnd, ttf.chrgText.cpMin, ttf.chrgText.cpMax);
    }
    else
    {
      SendMessage(hwnd, SCI_SETCURRENTPOS, cpos, 0);
    }
    if (ttf.lpstrText)
    {
      const char* lpstrText = ttf.lpstrText;
      n2e_Free(szPrevWord);
      szPrevWord = (char*)n2e_Alloc(strlen(lpstrText) + 1);
      lstrcpynA(szPrevWord, lpstrText, strlen(lpstrText) + 1);
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
  StrCat(dirname, L"*");
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
  struct Sci_TextRange tr_1, tr_2;
  BOOL found = FALSE;
  const static int max_region_to_scan = 1024;
  const static int max_brackets_to_skip = 100;
  const int cpos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  const int len = SendMessage(hwnd, SCI_GETTEXTLENGTH, 0, 0);
  tr_1.chrg.cpMax = cpos;
  tr_1.chrg.cpMin = max(0, cpos - max_region_to_scan);
  tr_1.lpstrText = NULL;
  tr_2.chrg.cpMin = cpos;
  tr_2.chrg.cpMax = min(len, cpos + max_region_to_scan);
  tr_2.lpstrText = NULL;

  int temp = abs(tr_1.chrg.cpMax - tr_1.chrg.cpMin);
  if (!temp) goto OUT_OF_UNWRAP;
  tr_1.lpstrText = (char*)n2e_Alloc(temp + 1);
  SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr_1);

  temp = abs(tr_2.chrg.cpMax - tr_2.chrg.cpMin);
  if (!temp) goto OUT_OF_UNWRAP;
  tr_2.lpstrText = (char*)n2e_Alloc(temp + 1);
  SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr_2);

  int pos_left = tr_1.chrg.cpMax;
  int pos_right = tr_2.chrg.cpMin;
  found = FALSE;
  if (quote_mode)
  {
    const char* _quotes = "\"'`";
    char* qchl = NULL;
    // search left
    while (1)
    {
      char lch = tr_1.lpstrText[pos_left - tr_1.chrg.cpMin - 1];
      if (lch)
      {
        if (qchl = (char*)strchr(_quotes, lch))
        {
          N2E_TRACE("Left quote found '%c'", lch);
          break;
        }
      }
      if (--pos_left <= tr_1.chrg.cpMin)
      {
        qchl = NULL;
        break;
      }
    }

    // go right
    while (qchl)
    {
      char rch = tr_2.lpstrText[pos_right - tr_2.chrg.cpMin];
      if (rch)
      {
        if (rch == *qchl)
        {
          N2E_TRACE("Right quote found '%c'", rch);
          break;
        }
      }
      if (++pos_right > tr_2.chrg.cpMax)
      {
        qchl = NULL;
        break;
      }
    }
    found = NULL != qchl;
  }
  else
  {
    const char* _left_braces = "<{([";
    const char* _right_braces = ">})]";
    char* tchl = NULL, *tchr = NULL, *qchl = NULL;
    int   skipcl = 0, skipcr = 0;
    int*  skipl = (int*)n2e_Alloc(max_brackets_to_skip * sizeof(int));

    // search left
  RESUME_SEARCH:
    while (1)
    {
      char lch = tr_1.lpstrText[pos_left - tr_1.chrg.cpMin - 1];
      if (lch)
      {
        if (tchl = (char*)strchr(_left_braces, lch))
        {
          if (skipcl)
          {
            int ti = 0;
            for (; ti < skipcl; ++ti)
            {
              if (tchl - _left_braces == skipl[ti])
              {
                N2E_TRACE("Skipped braces pair found '%c'", *tchl);
                skipl[ti] = -1;
                goto NEXT;
              }
            }
          }
          N2E_TRACE("Left bracket found '%c'", lch);
          break;
        }
        if (tchl = (char*)strchr(_right_braces, lch))
        {
          skipl[skipcl++] = tchl - _right_braces;
        }
      }
    NEXT:
      if (--pos_left <= tr_1.chrg.cpMin)
      {
        tchl = NULL;
        break;
      }
    }
    // go right
    while (tchl)
    {
      char rch = tr_2.lpstrText[pos_right - tr_2.chrg.cpMin];
      if (rch)
      {
        if (tchr = (char*)strchr(_right_braces, rch))
        {
          if (tchr - _right_braces == tchl - _left_braces)
          {
            if (skipcr)
            {
              N2E_TRACE("Skip right bracket '%c' (%d to skip)", rch, skipcr);
              --skipcr;
            }
            else
            {
              N2E_TRACE("Right bracket found '%c'", rch);
              break;
            }
          }
          else
          {
            tchr = NULL;
            N2E_TRACE("Bad right bracket found '%c'", rch);
          }
        }
        if (tchr = (char*)strchr(_left_braces, rch))
        {
          if (tchr == tchl)
          {
            ++skipcr;
          }
        }
      }
      if (++pos_right > tr_2.chrg.cpMax)
      {
        tchr = NULL;
        break;
      }
    }
    if (tchl && !tchr && --pos_left > tr_1.chrg.cpMin)
    {
      tchl = NULL;
      tchr = NULL;
      skipcr = 0;
      pos_right = tr_2.chrg.cpMin;
      goto RESUME_SEARCH;
    }
    n2e_Free(skipl);
    found = tchr && tchl;
  }
  // remove
  if (found)
  {
    N2E_TRACE("removing braces OR quotes at %d and %d", pos_left - 1, pos_right);
    SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
    SendMessage(hwnd, SCI_DELETERANGE, pos_left - 1, 1);
    SendMessage(hwnd, SCI_DELETERANGE, pos_right - 1 /*remember offset from prev line*/, 1);
    SendMessage(hwnd, SCI_SETSEL, pos_left - 1, pos_right - 1);
    SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
  }
OUT_OF_UNWRAP:
  n2e_Free(tr_1.lpstrText);
  n2e_Free(tr_2.lpstrText);
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
  ttf.lpstrText = (char*)n2e_Alloc(2);
  ttf.lpstrText[1] = '\0';
  changed = FALSE;
  for (symb = 0; symb < strlen(_source); ++symb)
  {
    ttf.chrg.cpMin = beg;
    ttf.chrg.cpMax = end;
    ttf.lpstrText[0] = _source[symb];
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
  n2e_Free(ttf.lpstrText);
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

void n2e_ResetFindIcon()
{
  n2e_UpdateFindIcon(TRUE);
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
    SaveCheckboxState(hwnd, IDC_FINDTRANSFORMBS);
    SaveCheckboxState(hwnd, IDC_FINDWORD);
    SaveCheckboxState(hwnd, IDC_FINDSTART);
  }
  if (bRegexModeChanged || bIsRegexMode)
  {
    UpdateCheckboxState(hwnd, IDC_FINDWORD, !bIsRegexMode, !bIsRegexMode);
    UpdateCheckboxState(hwnd, IDC_FINDSTART, !bIsRegexMode, !bIsRegexMode);
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

BOOL n2e_FilteredPasteFromClipboard(const HWND hwnd)
{
  char *pClip = EditGetClipboardText(hwndEdit);
  if (pClip)
  {
    remove_char(pClip, '\r');
    remove_char(pClip, '\n');
    const UINT codePage = SendMessage(hwndEdit, SCI_GETCODEPAGE, 0, 0);
    const int textLength = MultiByteToWideChar(codePage, 0, pClip, -1, NULL, 0);
    LPWSTR pWideText = LocalAlloc(LPTR, textLength * 2);
    MultiByteToWideChar(codePage, 0, pClip, -1, pWideText, textLength);
    SendMessage(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)pWideText);
    LocalFree(pWideText);
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
  static WCHAR wchIns[256];
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

BOOL n2e_IsFindReplaceAvailable(LPCEDITFINDREPLACE lpefr)
{
#ifndef ICU_BUILD
  return TRUE;
#else
  if (((lpefr->fuFlags & SCFIND_REGEXP) == 0) || n2e_IsUnicodeEncodingMode() || (iEncoding == CPI_UTF8))
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
