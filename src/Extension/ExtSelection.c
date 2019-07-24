#include "stdafx.h"
#include <cassert>
#include "CommonUtils.h"
#include "ExtSelection.h"
#include "Scintilla.h"
#include "SciCall.h"
#include "Helpers.h"
#include "Trace.h"
#include "Utils.h"

/************************************************************************/
/* multiple words matches selection, some are located on other pages    */
/************************************************************************/
#define N2E_SELECT_INDICATOR 9
/************************************************************************/
/* single word matches                                                  */
/************************************************************************/
#define N2E_SELECT_INDICATOR_SINGLE 10
/************************************************************************/
/* multiple words matches selection, all are visible on current page    */
/************************************************************************/
#define N2E_SELECT_INDICATOR_PAGE 11
/************************************************************************/
/* "selection edit"-mode, Ctrl+Tab to activate                          */
/************************************************************************/
#define N2E_SELECT_INDICATOR_EDIT 12

#define N2E_SELECT_MAX_SIZE 0xff
#define N2E_SELECT_MAX_COUNT 0xff

BOOL bHighlightSelection = TRUE;
BOOL bEditSelection = FALSE;
BOOL bHighlightAll = TRUE;
BOOL bEditSelectionInit = FALSE;
BOOL bEditSelectionExit = FALSE;

extern BOOL bHighlightLineIfWindowInactive;
extern long iMaxSearchDistance;
extern EWordNavigationMode iWordNavigationMode;
extern HWND hwndMain;
extern HWND hwndEdit;

typedef struct tagHLSEdata
{
  long  pos;
  long  len;
  char* original;
} SE_DATA, *LPSE_DATA;

typedef enum
{
  PCM_NONE = 0,
  PCM_ROLLBACK = 1 << 0,
  PCM_MODIFIED = 1 << 1
} EProcessChangesMode;

SE_DATA arrEditSelections[N2E_SELECT_MAX_COUNT];
long iEditSelectionsCount = 0;
struct Sci_TextRange trEditSelection;
long iOriginalSelectionLength = 0;
long iEditSelectionOffest = 0;
BOOL bEditSelectionWholeWordMode = TRUE;
BOOL bEditSelectionStrictMode = TRUE;
char *pEditSelectionOriginalWord = NULL;

BOOL case_compare(const char* a, const char* b, BOOL ignore_case)
{
  if (ignore_case)
  {
    return 0 == _stricmp(a, b);
  }
  return 0 == strcmp(a, b);
}

int n2e_SelectionKeyAction(int key, int msg)
{
  if (n2e_IsSelectionEditModeOn())
  {
    if (VK_RETURN == key)
    {
      if (WM_CHAR == msg)
      {
        n2e_SelectionEditStop(SES_APPLY);
      }
      return 0;
    }
  }
  return -1;
}

void n2e_EditSelectionInit(LPCWSTR lpSection, const int iDefaultSection, const int iIndicator,
  LPCWSTR lpAlphaSetting, const int iDefaultAlpha,
  LPCWSTR lpLineAlphaSetting, const int iDefaultLineAlpha,
  LPCWSTR lpColorSetting, const COLORREF iDefaultColor,
  LPCWSTR lpUnderSetting, const int iDefaultUnder)
{
  const int indi_style = IniGetInt(N2E_INI_SECTION, lpSection, iDefaultSection);
  if (indi_style >= 0)
  {
    SendMessage(hwndEdit, SCI_INDICSETSTYLE, iIndicator, indi_style);
    SendMessage(hwndEdit, SCI_INDICSETALPHA, iIndicator, IniGetInt(N2E_INI_SECTION, lpAlphaSetting, iDefaultAlpha));
    SendMessage(hwndEdit, SCI_INDICSETOUTLINEALPHA, iIndicator, IniGetInt(N2E_INI_SECTION, lpLineAlphaSetting, iDefaultLineAlpha));
    SendMessage(hwndEdit, SCI_INDICSETFORE, iIndicator, IniGetInt(N2E_INI_SECTION, lpColorSetting, iDefaultColor));
    SendMessage(hwndEdit, SCI_INDICSETUNDER, iIndicator, IniGetInt(N2E_INI_SECTION, lpUnderSetting, iDefaultUnder));
  }
}

void n2e_EditInit()
{
  SendMessage(hwndEdit, SCI_SETCARETLINEVISIBLEALWAYS, bHighlightLineIfWindowInactive, 0);
  SendMessage(hwndEdit, SCI_SETWORDNAVIGATIONMODE, iWordNavigationMode, 0);

#define DEFAULT_SECTION 6
#define EXTENDED_SECTION 7

  n2e_EditSelectionInit(L"SelectionType", DEFAULT_SECTION, N2E_SELECT_INDICATOR,
                        L"SelectionAlpha", 0,
                        L"SelectionLineAlpha", 0,
                        L"SelectionColor", RGB(0x00, 0xAA, 0x00),
                        L"SelectionUnder", 0);
  
  n2e_EditSelectionInit(L"SingleSelectionType", DEFAULT_SECTION, N2E_SELECT_INDICATOR_SINGLE,
                        L"SingleSelectionAlpha", 0,
                        L"SingleSelectionLineAlpha", 0,
                        L"SingleSelectionColor", RGB(0x90, 0x00, 0x00),
                        L"SingleSelectionUnder", 0);

  n2e_EditSelectionInit(L"PageSelectionType", EXTENDED_SECTION, N2E_SELECT_INDICATOR_PAGE,
                        L"PageSelectionAlpha", 50,
                        L"PageSelectionLineAlpha", 255,
                        L"PageSelectionColor", RGB(0x99, 0x99, 0x00),
                        L"PageSelectionUnder", 1);

  n2e_EditSelectionInit(L"EditSelectionType", EXTENDED_SECTION, N2E_SELECT_INDICATOR_EDIT,
                        L"EditSelectionAlpha", 50,
                        L"EditSelectionLineAlpha", 255,
                        L"EditSelectionColor", RGB(0x00, 0x00, 0xFF),
                        L"EditSelectionUnder", 1);
  
  n2e_proc_action = n2e_SelectionKeyAction;
  trEditSelection.lpstrText = 0;
}

void n2e_SelectionRelease()
{
  int k = 0;
  if (trEditSelection.lpstrText)
  {
    n2e_Free(trEditSelection.lpstrText);
    trEditSelection.lpstrText = 0;
  }
  if (pEditSelectionOriginalWord)
  {
    n2e_Free(pEditSelectionOriginalWord);
    pEditSelectionOriginalWord = 0;
  }
  for (k = 0; k < COUNTOF(arrEditSelections); ++k)
  {
    SE_DATA* se = &arrEditSelections[k];
    if (se->original)
    {
      n2e_Free(se->original);
      se->original = NULL;
    }
  }
}

int n2e_SelectionGetWraps(const int beg, const int end)
{
  int k = 0;
  int out = 0;
  int len = SendMessage(hwndEdit, SCI_GETLINECOUNT, 0, 0);
  for (k = beg; k < end && k + beg < len; ++k)
  {
    out += SendMessage(hwndEdit, SCI_WRAPCOUNT, beg + k, 0) - 1;
  }
  return out;
}

int n2e_HighlightWord(LPCSTR word)
{
  int res = 0;
  int cnt = 0;
  int lstart, lwrap, lrange, len, curr;
  int old;
  struct Sci_TextToFind ttf;
  struct Sci_TextToFind ttf1;
  len = SendMessage(hwndEdit, SCI_GETTEXTLENGTH, 0, 0);
  curr = SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
  if (bHighlightAll)
  {
    lstart = SendMessage(hwndEdit, SCI_GETFIRSTVISIBLELINE, 0, 0);
    lstart = (int)SendMessage(hwndEdit, SCI_DOCLINEFROMVISIBLE, lstart, 0);
  }
  else
  {
    lstart = SendMessage(hwndEdit, SCI_LINEFROMPOSITION, curr, 0);
  }
  lrange = bHighlightAll
    ? min(SendMessage(hwndEdit, SCI_LINESONSCREEN, 0, 0), SendMessage(hwndEdit, SCI_GETLINECOUNT, 0, 0))
    : 0;
  ttf.chrg.cpMin = SendMessage(hwndEdit, SCI_POSITIONFROMLINE, lstart, 0);
  ttf.chrg.cpMax = SendMessage(hwndEdit, SCI_GETLINEENDPOSITION, lstart + lrange, 0) + 1;
  old = SendMessage(hwndEdit, SCI_GETINDICATORCURRENT, 0, 0);
  SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, N2E_SELECT_INDICATOR, 0);
  SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, 0, len);
  SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, N2E_SELECT_INDICATOR_EDIT, 0);
  SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, 0, len);
  SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, N2E_SELECT_INDICATOR_PAGE, 0);
  SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, 0, len);
  SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, N2E_SELECT_INDICATOR_SINGLE, 0);
  SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, 0, len);
  if (word)
  {
    int search_opt = SCFIND_WHOLEWORD;
    int wlen = strlen(word);
    int curr_indi = N2E_SELECT_INDICATOR_SINGLE;
    BOOL bPreviousMatchIsVisible = FALSE;
    if (bEditSelectionInit)
    {
      iEditSelectionsCount = 0;
      if (!bEditSelectionWholeWordMode)
      {
        search_opt = SCFIND_MATCHCASE;
      }
      if (pEditSelectionOriginalWord)
      {
        if (strlen(pEditSelectionOriginalWord) != wlen + 1)
        {
          pEditSelectionOriginalWord = n2e_Realloc(pEditSelectionOriginalWord, wlen + 1);
        }
      }
      else
      {
        pEditSelectionOriginalWord = n2e_Alloc(wlen + 1);
      }
      strcpy(pEditSelectionOriginalWord, word);
    }
    // 2 first words
    {
      ttf1.chrg.cpMin = max(0, ttf.chrg.cpMin - iMaxSearchDistance);
      ttf1.chrg.cpMax = min(len, ttf.chrg.cpMin + iMaxSearchDistance);
    }
    ttf1.lpstrText = (LPSTR)word;
    res = SendMessage(hwndEdit, SCI_FINDTEXT, search_opt, (LPARAM)&ttf1);
    bPreviousMatchIsVisible = ttf1.chrgText.cpMin >= ttf.chrg.cpMin && ttf1.chrgText.cpMin < ttf.chrg.cpMax;
    while (1)
    {
      ttf1.chrg.cpMin = ttf1.chrgText.cpMax;
      res = SendMessage(hwndEdit, SCI_FINDTEXT, search_opt, (LPARAM)&ttf1);
      if (-1 != res)
      {
        // current match is visible
        if (
          ttf1.chrgText.cpMin >= ttf.chrg.cpMin &&
          ttf1.chrgText.cpMin < ttf.chrg.cpMax
          )
        {
          if (bPreviousMatchIsVisible)
          {
            if (bEditSelectionInit)
            {
              curr_indi = N2E_SELECT_INDICATOR_EDIT;
              bEditSelection = TRUE;
              iOriginalSelectionLength = wlen;
              break;
            }
            else
            {
              curr_indi = N2E_SELECT_INDICATOR_PAGE;
            }
          }
          else
          {
            curr_indi = N2E_SELECT_INDICATOR;
            if (!bEditSelectionInit)
            {
              break;
            }
          }
          bPreviousMatchIsVisible = TRUE;
        }
        else
        {
          curr_indi = N2E_SELECT_INDICATOR;
        }
        if (ttf1.chrgText.cpMin >= ttf.chrg.cpMax && N2E_SELECT_INDICATOR == curr_indi)
        {
          if (bEditSelectionInit)
          {
            bEditSelection = TRUE;
            iOriginalSelectionLength = wlen;
          }
          break;
        }
        ttf1.chrg.cpMin = ttf1.chrgText.cpMax;
      }
      else
      {
        break;
      }
    }
    SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, curr_indi, 0);
    if (bEditSelectionInit && !n2e_IsSelectionEditModeOn())
    {
      bEditSelectionInit = FALSE;
    }
    lwrap = 0;
    if (bEditSelectionInit)
    {
      lwrap = n2e_SelectionGetWraps(lstart, lstart + lrange);
    }
    ttf.lpstrText = (LPSTR)word;
    while (1)
    {
      res = SendMessage(hwndEdit, SCI_FINDTEXT, search_opt, (LPARAM)&ttf);
      if (-1 != res)
      {
        if (bEditSelectionInit)
        {
          int line = SendMessage(hwndEdit, SCI_LINEFROMPOSITION, ttf.chrgText.cpMax, 0);
          if (ttf.chrgText.cpMin < trEditSelection.chrg.cpMin && ttf.chrgText.cpMax > trEditSelection.chrg.cpMin)
          {
            N2E_TRACE("SKIP collision SELECTION");
            ttf.chrg.cpMin = ttf.chrgText.cpMax;
            continue;
          }
          N2E_TRACE("[%d] line__ %d (%d , %d , %d) ", ttf.chrgText.cpMin, line, lwrap, lstart, lrange);
          if (line <= lrange + lstart)
          {
            LPSE_DATA dt = &arrEditSelections[iEditSelectionsCount++];
            dt->pos = ttf.chrgText.cpMin;
            dt->len = wlen;
            if (dt->original)
            {
              n2e_Free(dt->original);
            }
            dt->original = n2e_Alloc(wlen + 1);
            {
              struct Sci_TextRange str;
              str.chrg.cpMin = dt->pos;
              str.chrg.cpMax = dt->pos + wlen;
              str.lpstrText = dt->original;
              SendMessage(hwndEdit, SCI_GETTEXTRANGE, 0, (LPARAM)&str);
            }
          }
          else
          {
            N2E_TRACE("out of loop");
            break;
          }
        }
        SendMessage(hwndEdit, SCI_INDICATORFILLRANGE, ttf.chrgText.cpMin, ttf.chrgText.cpMax - ttf.chrgText.cpMin);
        cnt++;
        ttf.chrg.cpMin = ttf.chrgText.cpMax;
      }
      else
      {
        break;
      }
    }
  }
  SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, old, 0);
  return cnt;
}

void n2e_SelectionGetWord()
{
  int sel_len = 0, cpos = 0;
  if (trEditSelection.lpstrText)
  {
    n2e_Free(trEditSelection.lpstrText);
    trEditSelection.lpstrText = 0;
  }
  cpos = SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
  if (bEditSelectionInit)
  {
    trEditSelection.chrg.cpMin = SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0);
    trEditSelection.chrg.cpMax = SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0);
    sel_len = trEditSelection.chrg.cpMax - trEditSelection.chrg.cpMin;
    iEditSelectionOffest = SciCall_GetCurrentPos();
    bEditSelectionWholeWordMode = FALSE;
    if (sel_len < 1)
    {
      sel_len = 0;
    }
  }
  if (0 == sel_len)
  {
    trEditSelection.chrg.cpMin = SendMessage(hwndEdit, SCI_WORDSTARTPOSITION, cpos, TRUE);
    trEditSelection.chrg.cpMax = SendMessage(hwndEdit, SCI_WORDENDPOSITION, cpos, TRUE);
    sel_len = trEditSelection.chrg.cpMax - trEditSelection.chrg.cpMin;
    bEditSelectionWholeWordMode = TRUE;
  }
  if (sel_len > (!bEditSelectionInit || bEditSelectionWholeWordMode) ? 1 : 0)
  {
    trEditSelection.lpstrText = n2e_Alloc(sel_len + 1);
    SendMessage(hwndEdit, SCI_GETTEXTRANGE, 0, (LPARAM)&trEditSelection);
  }
  else
  {
    trEditSelection.chrg.cpMin = 0;
    trEditSelection.chrg.cpMax = 0;
  }
}

void n2e_SelectionHighlightTurn()
{
  if (bHighlightSelection)
  {
    n2e_SelectionGetWord();
    if ((n2e_HighlightWord(trEditSelection.lpstrText) < 2) && !bHighlightAll)
    {
      bHighlightAll = TRUE;
      n2e_HighlightWord(trEditSelection.lpstrText);
    }
  }
  else
  {
    const int old = SendMessage(hwndEdit, SCI_GETINDICATORCURRENT, 0, 0);
    const int len = SendMessage(hwndEdit, SCI_GETTEXTLENGTH, 0, 0);
    SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, N2E_SELECT_INDICATOR, 0);
    SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, 0, len);
    SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, N2E_SELECT_INDICATOR_SINGLE, 0);
    SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, 0, len);
    SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, N2E_SELECT_INDICATOR_PAGE, 0);
    SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, 0, len);
    SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, N2E_SELECT_INDICATOR_EDIT, 0);
    SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, 0, len);
    SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, old, 0);
  }
}

BOOL n2e_SelectionProcessChanges(const EProcessChangesMode opt)
{
  int old_ind;
  int new_len = 0;
  int k = 0;
  int doc_len = SendMessage(hwndEdit, SCI_GETTEXTLENGTH, 0, 0);
  BOOL out = TRUE;
  BOOL work = TRUE;
  BOOL cur_se = FALSE;
  BOOL rollback = opt & PCM_ROLLBACK;
  char *old_word = 0;
  struct Sci_TextRange tr;
  int cur_pos = SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
  int delta_len = 0;
  tr.lpstrText = 0;
  old_ind = SendMessage(hwndEdit, SCI_GETINDICATORCURRENT, 0, 0);
  if (cur_pos < trEditSelection.chrg.cpMin || cur_pos > trEditSelection.chrg.cpMax)
  {
    N2E_TRACE("OUT OF BOUND  SE exit (applied) %d %d %d", cur_pos, trEditSelection.chrg.cpMin, trEditSelection.chrg.cpMax);
    out = FALSE;
    goto _EXIT;
  }
  if (trEditSelection.chrg.cpMax < trEditSelection.chrg.cpMin)
  {
    N2E_TRACE("critical SE exit");
    out = FALSE;
    goto _EXIT;
  }
  new_len = trEditSelection.chrg.cpMax - trEditSelection.chrg.cpMin;
  old_word = n2e_Alloc(iOriginalSelectionLength + 1);
  tr.lpstrText = n2e_Alloc(iOriginalSelectionLength + 1);
  SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, N2E_SELECT_INDICATOR_EDIT, 0);
  assert(trEditSelection.lpstrText);
  strcpy(old_word, trEditSelection.lpstrText);
  if (rollback)
  {
    if (0 == pEditSelectionOriginalWord)
    {
      N2E_TRACE("NO original word!");
      goto _EXIT;
    }
    N2E_TRACE("ROLLBACK to TR '%s' (%d - %d) ", pEditSelectionOriginalWord, trEditSelection.chrg.cpMin, trEditSelection.chrg.cpMax);
    new_len = strlen(pEditSelectionOriginalWord);
    if (trEditSelection.lpstrText && new_len != strlen(trEditSelection.lpstrText))
    {
      trEditSelection.lpstrText = n2e_Realloc(trEditSelection.lpstrText, strlen(pEditSelectionOriginalWord) + 1);
    }
  }
  else
  {
    if (trEditSelection.lpstrText && (trEditSelection.chrg.cpMax - trEditSelection.chrg.cpMin) != iOriginalSelectionLength)
    {
      trEditSelection.lpstrText = n2e_Realloc(trEditSelection.lpstrText, trEditSelection.chrg.cpMax - trEditSelection.chrg.cpMin + 1);
    }
    SendMessage(hwndEdit, SCI_GETTEXTRANGE, 0, (LPARAM)&trEditSelection);
    if (case_compare(old_word, trEditSelection.lpstrText, 0/*_hl_se_mode_whole_word*/))
    {
      N2E_TRACE("case (%d) compare exit!", bEditSelectionWholeWordMode);
      goto _EXIT;
    }
  }
  /*
  clear cur edit
  */
  SendMessage(hwndEdit, SCI_SETMODEVENTMASK, n2e_SelectionGetSciEventMask(FALSE), 0);
  for (k = 0; k < iEditSelectionsCount; ++k)
  {
    LPSE_DATA se = &arrEditSelections[k];
    // shifting
    N2E_TRACE("start shift: pos:%d cur:%d delta:%d", se->pos, trEditSelection.chrg.cpMin, delta_len);
    se->pos += delta_len;
    if (!rollback && se->pos > trEditSelection.chrg.cpMin)
    {
      se->pos += (new_len - iOriginalSelectionLength);
    }
    // check collisions
    if (trEditSelection.chrg.cpMax > se->pos && se->pos > trEditSelection.chrg.cpMin)
    {
      N2E_TRACE("critical SE exit");
      out = FALSE;
      goto _EXIT;
    }
    SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, se->pos, se->len);
    se->len = new_len;
    if (bEditSelectionStrictMode)
    {
      /*
      edited item
      */
      work = FALSE;
      cur_se = (se->pos == trEditSelection.chrg.cpMin && !rollback);
      N2E_TRACE("start check: pos:%d cur:%d delta:%d", se->pos, trEditSelection.chrg.cpMin, delta_len);
      if (!cur_se)
      {
        tr.chrg.cpMin = se->pos;
        tr.chrg.cpMax = se->pos + iOriginalSelectionLength;
        doc_len = SendMessage(hwndEdit, SCI_GETTEXTLENGTH, 0, 0);
        if (tr.chrg.cpMax > doc_len)
        {
          N2E_TRACE("!!!SE item last pos out of document (cur pos %d len %d doclen %d) . ", se->pos, iOriginalSelectionLength, doc_len);
          break;
        }
        SendMessage(hwndEdit, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
        work = case_compare(tr.lpstrText, old_word, bEditSelectionWholeWordMode);
      }
      else
      {
        work = FALSE;
        N2E_TRACE("cur pos!")
      }
    }
    else
    {
      work = TRUE;
    }
    if (work)
    {
      SendMessage(hwndEdit, SCI_SETTARGETSTART, se->pos, 0);
      SendMessage(hwndEdit, SCI_SETTARGETEND, se->pos + iOriginalSelectionLength, 0);
      if (rollback)
      {
        assert(case_compare(pEditSelectionOriginalWord, se->original, TRUE));
        SendMessage(hwndEdit, SCI_REPLACETARGET, -1, (LPARAM)se->original);
      }
      else
      {
        SendMessage(hwndEdit, SCI_REPLACETARGET, -1, (LPARAM)trEditSelection.lpstrText);
      }
      delta_len += (new_len - iOriginalSelectionLength);
      if (se->pos < trEditSelection.chrg.cpMax)
      {
        trEditSelection.chrg.cpMin += (new_len - iOriginalSelectionLength);
        trEditSelection.chrg.cpMax += (new_len - iOriginalSelectionLength);
      }
    }
    else if (!cur_se)
    {
      N2E_TRACE("!!!SE mismatch error at idx %d pos %d expect '%s' but got '%s' then skip item",
               k, se->pos, old_word, tr.lpstrText);
    }
    SendMessage(hwndEdit, SCI_INDICATORFILLRANGE, se->pos, new_len);
    N2E_TRACE("new se pos %d = %d (%d). delta %d", k, se->pos, se->len, delta_len);
  }
_EXIT:
  iOriginalSelectionLength = new_len;
  SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, old_ind, 0);
  if (old_word)
  {
    n2e_Free(old_word);
    old_word = 0;
  }
  if (tr.lpstrText)
  {
    n2e_Free(tr.lpstrText);
    tr.lpstrText = 0;
  }
  N2E_TRACE("new range is %d : %d . curpos is %d", trEditSelection.chrg.cpMin, trEditSelection.chrg.cpMax, cur_pos);
  SendMessage(hwndEdit, SCI_SETMODEVENTMASK, n2e_SelectionGetSciEventMask(TRUE), 0);
  return out;
}

BOOL n2e_IsSelectionEditModeOn()
{
  return bEditSelection;
}

void n2e_SelectionEditStart(const BOOL highlightAll)
{
  bHighlightAll = highlightAll;
  // if mode already ON - then turn it OFF
  if (n2e_IsSelectionEditModeOn())
  {
    n2e_SelectionEditStop(SES_APPLY);
    return;
  }
  bEditSelectionInit = TRUE;
  iEditSelectionsCount = 0;
  n2e_SelectionHighlightTurn();
  bEditSelectionInit = FALSE;
  if (n2e_IsSelectionEditModeOn())
  {
    if (SciCall_GetSelStart() == SciCall_GetSelEnd())
    {
      SendMessage(hwndEdit, SCI_SETSEL, trEditSelection.chrg.cpMin, trEditSelection.chrg.cpMax);
    }
    SendMessage(hwndEdit, SCI_BEGINUNDOACTION, 0, 0);
    bEditSelectionExit = FALSE;
  }
}

BOOL n2e_SelectionEditStop(const ESelectionEditStopMode mode)
{
  bEditSelectionInit = FALSE;
  if (n2e_IsSelectionEditModeOn())
  {
    if (mode & SES_REJECT)
    {
      n2e_SelectionProcessChanges(PCM_ROLLBACK);
      SciCall_SetSel(iEditSelectionOffest, iEditSelectionOffest);
    }
    else
    {
      const int pos = SciCall_GetCurrentPos();
      SciCall_SetSel(pos, pos);
    }
    bEditSelection = FALSE;
    bHighlightAll = TRUE;

    n2e_SelectionHighlightTurn();
    SendMessage(hwndEdit, SCI_ENDUNDOACTION, 0, 0);
    return TRUE;
  }
  return FALSE;
}

void n2e_SelectionUpdate(const ESelectionUpdateMode place)
{
  if (n2e_IsSelectionEditModeOn())
  {
    EProcessChangesMode opt = PCM_NONE;
    if (bEditSelectionExit)
    {
      n2e_SelectionEditStop(SES_APPLY);
      return;
    }
    if (SUM_MODIF == place)
    {
      opt |= PCM_MODIFIED;
    }
    else
    {
      if (!n2e_SelectionProcessChanges(opt))
      {
        n2e_SelectionEditStop(SES_APPLY);
      }
    }
  }
  else
  {
    n2e_SelectionHighlightTurn();
  }
}

void n2e_SelectionNotificationHandler(const int code, const struct SCNotification *scn)
{
  switch (code)
  {
    case SCN_UPDATEUI:
      if (bHighlightSelection)
      {
        n2e_SelectionUpdate(SUM_UPDATE);
      }
      break;
    case SCN_MODIFIED:
      if ((scn->modificationType & (SC_MOD_CONTAINER|SC_PERFORMED_UNDO)) == (SC_MOD_CONTAINER|SC_PERFORMED_UNDO))
      {
        PostMessage(hwndEdit, SCI_GOTOPOS, (WPARAM)scn->token, 0);
      }
      else if (bHighlightSelection)
      {
        if (n2e_IsSelectionEditModeOn())
        {
          if (scn->modificationType & SC_MOD_INSERTTEXT)
          {
            N2E_TRACE("MODIF INSERT pos:%d len%d lines:%d text:%s", scn->position, scn->length, scn->linesAdded, scn->text);
            trEditSelection.chrg.cpMax += scn->length;
          }
          else if (scn->modificationType & SC_MOD_DELETETEXT)
          {
            N2E_TRACE("MODIF DELETE pos:%d len%d lines:%d text:%s", scn->position, scn->length, scn->linesAdded, scn->text);
            trEditSelection.chrg.cpMax -= scn->length;
          }
          else if (scn->modificationType & SC_PERFORMED_USER)
          {
            N2E_TRACE("MODIF PERFORMED USER");
          }
          else if (scn->modificationType & SC_PERFORMED_UNDO)
          {
            N2E_TRACE("MODIF PERFORMED UNDO");
          }
          else if (scn->modificationType & SC_PERFORMED_REDO)
          {
            N2E_TRACE("MODIF PERFORMED REDO");
          }
          else if (scn->modificationType & SC_MOD_BEFOREINSERT)
          {
            N2E_TRACE("MODIF BEFORE INSERT pos:%d len%d ", scn->position, scn->length);
          }
          else if (scn->modificationType & SC_MOD_BEFOREDELETE)
          {
            N2E_TRACE("MODIF BEFORE DELETE pos:%d len%d ", scn->position, scn->length);
          }
          else if (scn->modificationType & SC_MULTILINEUNDOREDO)
          {
            N2E_TRACE("MODIF MULTILINE UNDO");
          }
          else if (scn->modificationType & SC_STARTACTION)
          {
            N2E_TRACE("MODIF START ACTION");
          }
        }
      }
      break;
    case SCN_SAVEPOINTREACHED:
    case SCEN_KILLFOCUS:
      n2e_SelectionEditStop(SES_APPLY);
      break;
  }
}

UINT n2e_SelectionGetSciEventMask(const BOOL range_not)
{
  UINT out = SC_PERFORMED_UNDO | SC_PERFORMED_REDO;
  if (range_not)
  {
    out |= SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT;
  }
  return out;
}

void n2e_OnMouseVanishEvent(const BOOL showCursor)
{
  static int hideCursorCounter = 0;
  if (showCursor)
  {
    if (hideCursorCounter > 0)
    {
      ShowCursor(TRUE);
      --hideCursorCounter;
    }
  }
  else
  {
    if (hideCursorCounter <= 0)
    {
      static BOOL bInitialized = FALSE;
      static BOOL bVanish = FALSE;
      if (!bInitialized)
      {
        bInitialized = SystemParametersInfo(SPI_GETMOUSEVANISH, 0, &bVanish, 0);
      }
      if (bVanish)
      {
        ShowCursor(FALSE);
        ++hideCursorCounter;
      }
    }
  }
}
