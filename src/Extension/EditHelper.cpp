#include "EditHelper.h"
#include "Dialogs.h"
#include <string>
#include <Shlobj.h>
#include <WTypes.h>
#include <regex>
#include "scintilla.h"
#include "resource.h"
#include "Styles.h"

extern "C"
{
  WCHAR	hl_last_html_tag[0xff] = L"<tag>";
  WCHAR	hl_last_html_end_tag[0xff] = L"</tag>";

  extern UINT		_hl_ctx_menu_type;
  extern	VOID	HL_Trace(const char *fmt, ...);
  
  LPCONTEXTMENU2	g_IContext2 = NULL;
  LPCONTEXTMENU3	g_IContext3 = NULL;
  //
  VOID Invoke(int cmd, LPCONTEXTMENU menu, HWND win, LPCWSTR path)
  {
    if (cmd > 0) {
      std::wstring pathFolder = path;
      PathRemoveFileSpec((LPWSTR)pathFolder.data());

      CMINVOKECOMMANDINFOEX info ={0};
      info.cbSize = sizeof(info);
      info.fMask = CMIC_MASK_UNICODE;
      info.hwnd = win;
      info.lpVerb  = MAKEINTRESOURCEA(cmd - 1);
      info.lpVerbW = MAKEINTRESOURCEW(cmd - 1);
      info.nShow = SW_SHOWNORMAL;
      info.lpDirectoryW = pathFolder.c_str();
      menu->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
    }
  }
  //
  LRESULT CALLBACK HookWndProc(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam)
  {
    switch (message) {
      case WM_MENUCHAR:	// only supported by IContextMenu3
        if (g_IContext3) {
          LRESULT lResult = 0;
          g_IContext3->HandleMenuMsg2(message, wParam, lParam, &lResult);
          return (lResult);
        }
        break;
      case WM_DRAWITEM:
      case WM_MEASUREITEM:
        if (wParam) {
          break;    // if wParam != 0 then the message is not menu-related
        }
      case WM_INITMENUPOPUP:
        if (g_IContext2) {
          g_IContext2->HandleMenuMsg(message, wParam, lParam);
        }
        else {	// version 3
          g_IContext3->HandleMenuMsg(message, wParam, lParam);
        }
        return (message == WM_INITMENUPOPUP ? 0 : TRUE); // inform caller that
        // we handled WM_INITPOPUPMENU by ourself
        break;
      default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  BOOL GetContextMenu(LPCWSTR path, void **ppContextMenu, int &iMenuType)
  {
    *ppContextMenu = NULL;
    LPCONTEXTMENU icm1 = NULL;
    // first we retrieve the normal IContextMenu
    // interface (every object should have it)
    LPCITEMIDLIST idChild = 0;
    IShellFolder *ifolder = 0;
    ITEMIDLIST *id = 0;
    std::wstring windowsPath = path;
    std::replace(windowsPath.begin(), windowsPath.end(), '/', '\\');
    HRESULT result = SHParseDisplayName(windowsPath.c_str(), 0, &id, 0, 0);
    if (!SUCCEEDED(result) || !id) {
      return false;
    }
    result = SHBindToParent(id, IID_IShellFolder, (void **)&ifolder, &idChild);
    ifolder->GetUIObjectOf(NULL,
                           1,
                           (LPCITEMIDLIST *)&idChild,
                           IID_IContextMenu,
                           NULL,
                           (void **)&icm1);
    if (icm1) {
        // since we got an IContextMenu interface we can
        // now obtain the higher version interfaces via that
      if (icm1->QueryInterface(IID_IContextMenu3, ppContextMenu) == NOERROR) {
        iMenuType = 3;
      }
      else if (icm1->QueryInterface(IID_IContextMenu2,
               ppContextMenu) == NOERROR) {
        iMenuType = 2;
      }
      if (*ppContextMenu) {
        icm1->Release();    // we can now release version 1 interface,
      }
      // cause we got a higher one
      else {
        iMenuType = 1;
        *ppContextMenu = icm1;    // since no higher versions were found
      }  // redirect ppContextMenu to version 1 interface
    }
    else {
      return (FALSE);    // something went wrong
    }
    return (TRUE); // success
  }

  BOOL HL_Explorer_cxt_menu(LPCWSTR path, void *parentWindow)
  {
    int iMenuType = 0;
    // to know which version of IContextMenu is supported
    LPCONTEXTMENU pContextMenu;
    // common pointer to IContextMenu and higher version interface
    if (!GetContextMenu(path, (void **)&pContextMenu, iMenuType)) {
      return FALSE;    // something went wrong
    }
    //
    HMENU	h_menu = CreatePopupMenu();
    // lets fill the our popupmenu
    pContextMenu->QueryContextMenu(h_menu,
                                   0, 1, 0x7FFF, _hl_ctx_menu_type);
    WNDPROC OldWndProc = NULL;
    //
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *)&osvi);
    BOOL	bIsWindowsXPorLater =
      ((osvi.dwMajorVersion > 5) ||
       ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1)));
    HL_Trace("win version %d (%d - %d) . XP ? : %d", WINVER, osvi.dwMajorVersion, osvi.dwMinorVersion, !bIsWindowsXPorLater);
    if (iMenuType > 1) { // only version 2 and 3 supports menu messages
      OldWndProc = (WNDPROC)SetWindowLong((HWND)parentWindow,
                                          GWL_WNDPROC, (DWORD)HookWndProc);
      if (iMenuType == 2) {
        g_IContext2 = (LPCONTEXTMENU2)pContextMenu;
      }
      else {	// version 3
        g_IContext3 = (LPCONTEXTMENU3)pContextMenu;
      }
    }
    else {
      OldWndProc = NULL;
    }
    POINT pt;
    GetCursorPos(&pt);
    int iCmd = TrackPopupMenuEx(h_menu, TPM_RETURNCMD, pt.x, pt.y, (HWND)parentWindow, NULL);
    Invoke(iCmd, pContextMenu, (HWND)parentWindow, path);
    if (OldWndProc) {
      SetWindowLong((HWND)parentWindow, GWL_WNDPROC, (DWORD)OldWndProc);
    }
    pContextMenu->Release();
    return (TRUE);
  }

  void HL_Strip_html_tags(HWND hwnd)
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
    if (HLS_Edit_selection_stop(HL_SE_APPLY))
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

  VOID	HL_Adjust_offset(int *pos, BOOL in)
  {
#ifdef _DEBUG
    HL_Trace("UTF8 %d", mEncoding[iEncoding].uFlags & NCP_UTF8);
    HL_Trace("8Bit %d", mEncoding[iEncoding].uFlags & NCP_8BIT);
    HL_Trace("UNicode %d", mEncoding[iEncoding].uFlags & NCP_UNICODE);
    HL_Trace("offset is %d", *pos);
#endif
  }

  void HL_Jump_offset(HWND hwnd, int iNewPos)
  {
    HL_Adjust_offset(&iNewPos, TRUE);
    SendMessage(hwnd, SCI_GOTOPOS, (WPARAM)iNewPos, 0);
  }

  void HL_Get_offset(HWND hwnd, int *out)
  {
    *out = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
    HL_Adjust_offset(out, FALSE);
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

  void HL_Msg_create()
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

  void HL_Find_next_word(HWND hwnd, LPCEDITFINDREPLACE lpref, BOOL next)
  {
    struct Sci_TextRange	tr;
    struct Sci_TextToFind	ttf;
    static char* szPrevWord = NULL;
    int cpos, wlen, doclen, res, searchflags;
    BOOL has;
#define _HL_SEARCH_FOR_WORD_LIMIT 0x100
    HL_TRACE(L"look for next(%d) word", next);
    ZeroMemory(&ttf, sizeof(ttf));
    ttf.lpstrText = 0;
    cpos = SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
    doclen = SendMessage(hwnd, SCI_GETTEXTLENGTH, 0, 0);
    tr.chrg.cpMin = SendMessage(hwnd, SCI_WORDSTARTPOSITION, cpos, TRUE);
    tr.chrg.cpMax = SendMessage(hwnd, SCI_WORDENDPOSITION, cpos, TRUE);
    wlen = tr.chrg.cpMax - tr.chrg.cpMin;
    res = 0;

    tr.lpstrText = (char*)HL_Alloc(wlen + 1);
    SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

    const int iSelCount = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0) -
      (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);

    HL_Free(tr.lpstrText);

    if (iSelCount > 0)
    {
      const size_t prevWordLength = szPrevWord ? strlen(szPrevWord) + 1 : 0;
      if (szPrevWord && (prevWordLength > 0))
      {
        tr.lpstrText = (char*)HL_Alloc(prevWordLength);
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
        tr.chrg.cpMin = next ? cpos : max(cpos - _HL_SEARCH_FOR_WORD_LIMIT, 0);
        tr.chrg.cpMax = next ? min(cpos + _HL_SEARCH_FOR_WORD_LIMIT, doclen) : cpos;
        wlen = tr.chrg.cpMax - tr.chrg.cpMin;
        if (wlen > 0)
        {
          int counter;
          char symb;
          //
          tr.lpstrText = (char*)HL_Alloc(wlen + 1);
          SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
          counter = 0;
          while (counter <= wlen)
          {
            ++counter;
            symb = next ? tr.lpstrText[counter] : tr.lpstrText[wlen - counter];
            if (HL_IS_LITERAL(symb))
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
        tr.lpstrText = (char*)HL_Alloc(wlen + 1);
        SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
        ttf.lpstrText = tr.lpstrText;
        res = 1;
      }
    }
    //
    if (res)
    {
      HL_TRACE("search for '%s' ", ttf.lpstrText);
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
        HL_Free(szPrevWord);
        szPrevWord = (char*)HL_Alloc(strlen(lpstrText) + 1);
        lstrcpynA(szPrevWord, lpstrText, strlen(lpstrText) + 1);
      }
      if (tr.lpstrText)
      {
        HL_Free(tr.lpstrText);
        tr.lpstrText = 0;
      }
    }
  }

  BOOL HL_Open_nextFs_file(HWND hwnd, LPCWSTR file, BOOL next)
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
#define _HL_COMPARE_FILES( F1 , F2 )  (HL_Compare_files(F1,F2))
    do
    {
      if (0 == (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
        cmp_res = _HL_COMPARE_FILES(filename, ffd.cFileName);
        HL_TRACE(L"%S vs %S = %d", ffd.cFileName, filename, cmp_res);
        if ((next && cmp_res >= 0) || (!next&&cmp_res <= 0))
        {
          continue;
        }
        if (*found_path)
        {
          cmp_res = _HL_COMPARE_FILES(found_path, ffd.cFileName);
        }
        else
        {
          cmp_res = 0;
        }
        HL_TRACE(L"%S vs %S = %d", ffd.cFileName, found_path, cmp_res);
        if ((next && cmp_res >= 0) || (!next&&cmp_res <= 0))
        {
          StrCpy(found_path, ffd.cFileName);
          HL_TRACE(L"saved %S", found_path);
        }
      }
    } while (FindNextFile(hFind, &ffd));
    FindClose(hFind);
    if (*found_path)
    {
      StrCat(odn, found_path);
      HL_TRACE(L"file to open %S", odn);
      FileLoad(FALSE, FALSE, FALSE, FALSE, odn);
    }
    return TRUE;
  }

  void HL_Unwrap_selection(HWND hwnd, BOOL quote_mode)
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
      tr_1.lpstrText = (char*)HL_Alloc(temp + 1);
      SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr_1);
    }
    {
      temp = abs(tr_2.chrg.cpMax - tr_2.chrg.cpMin);
      if (!temp) goto OUT_OF_UNWRAP;
      tr_2.lpstrText = (char*)HL_Alloc(temp + 1);
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
            HL_TRACE("Left quote found '%c'", lch);
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
            HL_TRACE("Right quote found '%c'", rch);
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
      int*  skipl = (int*)HL_Alloc(max_brackets_to_skip * sizeof(int));

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
                  HL_TRACE("Skipped braces pair found '%c'", *tchl);
                  skipl[ti] = -1;
                  goto NEXT;
                }
              }
            }
            {
              HL_TRACE("Left bracket found '%c'", lch);
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
                HL_TRACE("Skip right bracket '%c' (%d to skip)", rch, skipcr);
                --skipcr;
              }
              else
              {
                HL_TRACE("Right bracket found '%c'", rch);
                break;
              }
            }
            else
            {
              tchr = NULL;
              HL_TRACE("Bad right bracket found '%c'", rch);
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
      HL_Free(skipl);
      found = tchr && tchl;
    }
    // remove
    if (found)
    {
      HL_TRACE("removing braces OR quotes at %d and %d", pos_left - 1, pos_right);
      SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
      SendMessage(hwnd, SCI_DELETERANGE, pos_left - 1, 1);
      SendMessage(hwnd, SCI_DELETERANGE, pos_right - 1 /*remember offset from prev line*/, 1);
      SendMessage(hwnd, SCI_SETSEL, pos_left - 1, pos_right - 1);
      SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
    }
  OUT_OF_UNWRAP:
    HL_Free(tr_1.lpstrText);
    HL_Free(tr_2.lpstrText);
  }

  void HL_Escape_html(HWND hwnd)
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
    ttf.lpstrText = (char*)HL_Alloc(2);
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
    HL_Free(ttf.lpstrText);
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

  int isValidRegex(LPCSTR str)
  {
    try
    {
      std::regex reg(str);
      return 1;
    }
    catch (...)
    {
      return 0;
    }
  }
}