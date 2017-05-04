#include "EditHelper.h"
#include <cassert>

WCHAR	n2e_last_html_tag[0xff] = L"<tag>";
WCHAR	n2e_last_html_end_tag[0xff] = L"</tag>";

void n2e_StripHTMLTags(HWND hwnd)
{
  struct Sci_TextToFind ttf1, ttf2;
  int selbeg, selend, res, len;

  //
  selbeg = SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  selend = SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  len = SendMessage(hwnd, SCI_GETTEXTLENGTH, 0, 0);
  SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
  //
  if (0 == selend - selbeg)
  {
    ttf1.chrg.cpMin = selbeg;
    ttf1.chrg.cpMax = 0;
    ttf1.lpstrText = "<";
    res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf1);
    if (-1 != res)
    {
      ttf2.chrg.cpMin = ttf1.chrgText.cpMax;
      ttf2.chrg.cpMax = len;
      ttf2.lpstrText = ">";
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
      ttf1.lpstrText = "<";
      res = SendMessage(hwnd, SCI_FINDTEXT, 0, (LPARAM)&ttf1);
      if (-1 != res)
      {
        ttf2.chrg.cpMin = ttf1.chrgText.cpMax;
        ttf2.chrg.cpMax = selend;
        ttf2.lpstrText = ">";
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

BOOL IsSelectionModeValid(HWND hwnd)
{
  if (SC_SEL_RECTANGLE == SendMessage(hwnd, SCI_GETSELECTIONMODE, 0, 0))
  {
    MsgBox(MBINFO, IDS_SELRECT);
    return FALSE;
  }
  return TRUE;
}

extern BOOL bAutoIndent;

LPSTR GetLinePrefix(HWND hwnd, int iLine, LPBOOL pbLineEmpty)
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

void InsertNewLineWithPrefix(HWND hwnd, LPSTR pszPrefix, BOOL bInsertAbove)
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

void EditInsertNewLine(HWND hwnd, BOOL insertAbove)
{
  if (n2e_SelectionEditStop(N2E_SE_APPLY))
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

VOID	n2e_AdjustOffset(int *pos, BOOL in)
{
#ifdef _DEBUG
  N2E_Trace("UTF8 %d", mEncoding[iEncoding].uFlags & NCP_UTF8);
  N2E_Trace("8Bit %d", mEncoding[iEncoding].uFlags & NCP_8BIT);
  N2E_Trace("UNicode %d", mEncoding[iEncoding].uFlags & NCP_UNICODE);
  N2E_Trace("offset is %d", *pos);
#endif
}

void n2e_JumpToOffset(HWND hwnd, int iNewPos)
{
  n2e_AdjustOffset(&iNewPos, TRUE);
  SendMessage(hwnd, SCI_GOTOPOS, (WPARAM)iNewPos, 0);
}

void n2e_GetOffset(HWND hwnd, int *out)
{
  *out = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  n2e_AdjustOffset(out, FALSE);
}

int FindTextImpl(const HWND hwnd, const int searchFlags, struct TextToFind* pttf)
{
  return (int)SendMessage(hwnd, SCI_FINDTEXT, searchFlags, (LPARAM)pttf);
}

int FindTextTest(const HWND hwnd, const int searchFlags, const struct TextToFind* pttf, const int cpMin)
{
  struct TextToFind ttf = *pttf;
  ttf.chrg.cpMin = cpMin;
  return FindTextImpl(hwnd, searchFlags, &ttf);
}

BOOL CheckTextExists(const HWND hwnd, const int searchFlags, const struct TextToFind* pttf, const int iPos)
{
  return (FindTextTest(hwnd, searchFlags, pttf, iPos) >= 0);
}

void n2e_MsgCreate()
{
  // Tabs
  SendMessage(hwndEdit, SCI_SETUSETABS, !bTabsAsSpaces, 0);
  SendMessage(hwndEdit, SCI_SETTABINDENTS, bTabIndents, 0);
  SendMessage(hwndEdit, SCI_SETBACKSPACEUNINDENTS, bBackspaceUnindents, 0);
  SendMessage(hwndEdit, SCI_SETTABWIDTH, iTabWidth, 0);
  SendMessage(hwndEdit, SCI_SETINDENT, iIndentWidth, 0);
  // Indent Guides
  Style_SetIndentGuides(hwndEdit, bShowIndentGuides);
  // Word wrap
  if (!fWordWrap)
  {
    SendMessage(hwndEdit, SCI_SETWRAPMODE, SC_WRAP_NONE, 0);
  }
  else
  {
    SendMessage(hwndEdit, SCI_SETWRAPMODE, (iWordWrapMode == 0) ? SC_WRAP_WORD : SC_WRAP_CHAR, 0);
  }
  if (iWordWrapIndent == 5)
  {
    SendMessage(hwndEdit, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_SAME, 0);
  }
  else if (iWordWrapIndent == 6)
  {
    SendMessage(hwndEdit, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_INDENT, 0);
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
    SendMessage(hwndEdit, SCI_SETWRAPSTARTINDENT, i, 0);
    SendMessage(hwndEdit, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_FIXED, 0);
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
    SendMessage(hwndEdit, SCI_SETWRAPVISUALFLAGSLOCATION, wrapVisualFlagsLocation, 0);
    SendMessage(hwndEdit, SCI_SETWRAPVISUALFLAGS, wrapVisualFlags, 0);
  }
  else
  {
    SendMessage(hwndEdit, SCI_SETWRAPVISUALFLAGS, 0, 0);
  }
  // Long Lines
  if (bMarkLongLines)
  {
    SendMessage(hwndEdit, SCI_SETEDGEMODE, (iLongLineMode == EDGE_LINE) ? EDGE_LINE : EDGE_BACKGROUND, 0);
  }
  else
  {
    SendMessage(hwndEdit, SCI_SETEDGEMODE, EDGE_NONE, 0);
  }
  SendMessage(hwndEdit, SCI_SETEDGECOLUMN, iLongLinesLimit, 0);
  // Margins
  SendMessage(hwndEdit, SCI_SETMARGINWIDTHN, 2, 0);
  SendMessage(hwndEdit, SCI_SETMARGINWIDTHN, 1, (bShowSelectionMargin) ? 16 : 0);
  UpdateLineNumberWidth();
  // Nonprinting characters
  SendMessage(hwndEdit, SCI_SETVIEWWS, bViewWhiteSpace ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE, 0);
  SendMessage(hwndEdit, SCI_SETVIEWEOL, bViewEOLs, 0);
  SendMessage(hwndEdit, SCI_MOVECARETONRCLICK, bMoveCaretOnRightClick, 0);
}

void n2e_FindNextWord(HWND hwnd, LPCEDITFINDREPLACE lpref, BOOL next)
{
  struct Sci_TextRange	tr;
  struct Sci_TextToFind	ttf;
  static char* szPrevWord = NULL;
  int cpos, wlen, doclen, res, searchflags;
  BOOL has;
#define _N2E_SEARCH_FOR_WORD_LIMIT 0x100
  N2E_TRACE(L"look for next(%d) word", next);
  ZeroMemory(&ttf, sizeof(ttf));
  ttf.lpstrText = 0;
  cpos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  doclen = SendMessage(hwnd, SCI_GETTEXTLENGTH, 0, 0);
  tr.chrg.cpMin = SendMessage(hwnd, SCI_WORDSTARTPOSITION, cpos, TRUE);
  tr.chrg.cpMax = SendMessage(hwnd, SCI_WORDENDPOSITION, cpos, TRUE);
  wlen = tr.chrg.cpMax - tr.chrg.cpMin;
  res = 0;

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
  //
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
    if (iFindWordMatchCase != 0)
      searchflags |= SCFIND_MATCHCASE;

    res = FindTextImpl(hwnd, searchflags, &ttf);
    const BOOL bTextFound = (res >= 0);
    UpdateFindIcon(bTextFound && (FindTextTest(hwnd, searchflags, &ttf, res + 1) >= 0));

    if ((-1 == res) && (iFindWordWrapAround != 0))
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
      res = FindTextImpl(hwnd, searchflags, &ttf);
      UpdateFindIcon(res >= 0);
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

BOOL n2e_OpenNextFile(HWND hwnd, LPCWSTR file, BOOL next)
{
  WCHAR	dirname[MAX_PATH], odn[MAX_PATH], found_path[MAX_PATH], *filename;
  HANDLE	hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA	ffd;
  INT		cmp_res;
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
      cmp_res = _N2E_COMPARE_FILES(filename, ffd.cFileName);
      N2E_TRACE(L"%S vs %S = %d", ffd.cFileName, filename, cmp_res);
      if ((next && cmp_res >= 0) || (!next&&cmp_res <= 0))
      {
        continue;
      }
      if (*found_path)
      {
        cmp_res = _N2E_COMPARE_FILES(found_path, ffd.cFileName);
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

void n2e_UnwrapSelection(HWND hwnd, BOOL quote_mode)
{
  int cpos, len, temp, pos_left, pos_right;
  struct Sci_TextRange tr_1, tr_2;
  BOOL found;
  const static int max_region_to_scan = 1024;
  const static int max_brackets_to_skip = 100;
  cpos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  len = SendMessage(hwnd, SCI_GETTEXTLENGTH, 0, 0);
  tr_1.chrg.cpMax = cpos;
  tr_1.chrg.cpMin = max(0, cpos - max_region_to_scan);
  tr_1.lpstrText = NULL;
  tr_2.chrg.cpMin = cpos;
  tr_2.chrg.cpMax = min(len, cpos + max_region_to_scan);
  tr_2.lpstrText = NULL;
  {
    temp = abs(tr_1.chrg.cpMax - tr_1.chrg.cpMin);
    if (!temp) goto OUT_OF_UNWRAP;
    tr_1.lpstrText = (char*)n2e_Alloc(temp + 1);
    SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr_1);
  }
  {
    temp = abs(tr_2.chrg.cpMax - tr_2.chrg.cpMin);
    if (!temp) goto OUT_OF_UNWRAP;
    tr_2.lpstrText = (char*)n2e_Alloc(temp + 1);
    SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr_2);
  }
  pos_left = tr_1.chrg.cpMax, pos_right = tr_2.chrg.cpMin;
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
    int	  skipcl = 0, skipcr = 0;
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
          {
            N2E_TRACE("Left bracket found '%c'", lch);
            break;
          }
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

void n2e_EscapeHTML(HWND hwnd)
{
  int beg, end, res;
  size_t symb;
  BOOL changed;
  struct Sci_TextToFind ttf;
  const char* _source = "&<>";
  const char* _target[] = { "&amp;", "&lt;", "&gt;" };
  assert(strlen(_source) == COUNTOF(_target));
  SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
  beg = SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  end = SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
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
    res = 0;
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
    SendMessage(hwnd, SCI_SETSEL, beg, beg);
  }
  n2e_Free(ttf.lpstrText);
  SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
}

void UpdateFindIcon(const BOOL findOK)
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

void ResetFindIcon()
{
  UpdateFindIcon(TRUE);
}

void EditString2Hex(HWND hwnd)
{
  if (!IsSelectionModeValid(hwnd))
  {
    return;
  }
  EncodeStrToHex(hwnd);
}

void EditHex2String(HWND hwnd)
{
  if (!IsSelectionModeValid(hwnd))
  {
    return;
  }
  DecodeHexToStr(hwnd);
}
