#include "stdafx.h"
#include <cassert>
#include "scintilla.h"
#include "ExtSelection.h"
#include "Helpers.h"
#include "Utils.h"

/************************************************************************/
/* when many matches on ALL document  but NOT ALL OF THEM  on screen                                                                   */
/************************************************************************/
#define N2E_SELECT_INDICATOR 9
/************************************************************************/
/* when one match one document                                                                     */
/************************************************************************/
#define N2E_SELECT_INDICATOR_SINGLE 10
/************************************************************************/
/* when many matches on screen                                                                     */
/************************************************************************/
#define N2E_SELECT_INDICATOR_PAGE 11
/************************************************************************/
/* SE mode                                                                     */
/************************************************************************/
#define N2E_SELECT_INDICATOR_EDIT 12

#define N2E_SELECT_MAX_SIZE	0xff
#define N2E_SELECT_MAX_COUNT	0xff

BOOL	bHighlightSelection = TRUE;
BOOL	_n2e_edit_selection = FALSE;
BOOL	_n2e_highlight_all = TRUE;
BOOL	_n2e_se_init = FALSE;
BOOL	_n2e_se_exit = FALSE;

extern int iHighlightLineIfWindowInactive;
extern int iWordNavigationMode;

typedef struct tagHLSEdata
{
  long	pos;
  long	len;
  char*	original;
} SE_DATA, *LPSE_DATA;

enum N2E_SEOpt
{
  SEO_ROLLBACK = 1 << 0,
  SEO_MODIFIED = 1 << 1
};

SE_DATA		_n2e_se_array[N2E_SELECT_MAX_COUNT];
long		_n2e_se_count = 0; // total count   '
struct		Sci_TextRange	_n2e_se_tr;
long		_n2e_se_old_len = 0;
long		_n2e_max_search_range = 2048 * 1024;
BOOL		_n2e_se_mode_whole_word = TRUE;
BOOL		_n2e_se_strict_mode = TRUE;
char		*_n2e_se_orig_word = 0;

char to_lower(char in)
{
  if (in <= 'Z' && in >= 'A')
    return in - ('Z' - 'z');
  if (in <= 'ß' && in >= 'À')
    return in - ('ß' - 'ÿ');
  return in;
}

BOOL case_compare(const char* a, const char* b, BOOL ignore_case)
{
  if (ignore_case)
  {
    return 0 == _stricmp(a, b);
  }
  return 0 == strcmp(a, b);
}

BOOL icase_compare(const char* a, const char* b)
{
  while (*a && *b)
  {
    if (*a != *b)
    {
      char lA = to_lower(*a);
      char lB = to_lower(*b);
      if (lA != lB)
      {
        return FALSE;
      }
    }
    a++;
    b++;
  }
  // Either *a or *b is nul
  return *a == *b;
}

int	n2e_SelectionKeyAction(int key, int msg)
{
  if (_n2e_edit_selection)
  {
    if (VK_RETURN == key && GetKeyState(VK_SHIFT) >= 0)
    {
      if (WM_CHAR == msg)
      {
        n2e_SelectionEditStop(N2E_SE_APPLY);
      }
      return 0;
    }
  }
  return -1;
}

void	n2e_SelectionInit()
{
  SendMessage(hwndEdit, SCI_SETCARETLINEVISIBLEALWAYS, iHighlightLineIfWindowInactive, 0);
  SendMessage(hwndEdit, SCI_SETWORDNAVIGATIONMODE, iWordNavigationMode, 0);
  int indi_style = IniGetInt(N2E_INI_SECTION, L"SelectionType", 6);
  if (indi_style >= 0)
  {
    SendMessage(hwndEdit, SCI_INDICSETSTYLE, N2E_SELECT_INDICATOR, indi_style);
    SendMessage(hwndEdit, SCI_INDICSETALPHA, N2E_SELECT_INDICATOR, IniGetInt(N2E_INI_SECTION, L"SelectionAlpha", 0));
    SendMessage(hwndEdit, SCI_INDICSETOUTLINEALPHA, N2E_SELECT_INDICATOR, IniGetInt(N2E_INI_SECTION, L"SelectionLineAlpha", 0));
    SendMessage(hwndEdit, SCI_INDICSETFORE, N2E_SELECT_INDICATOR, IniGetInt(N2E_INI_SECTION, L"SelectionColor", RGB(0x00, 0x00, 0x00)));
    SendMessage(hwndEdit, SCI_INDICSETUNDER, N2E_SELECT_INDICATOR, IniGetInt(N2E_INI_SECTION, L"SelectionUnder", 0));
  }
  indi_style = IniGetInt(N2E_INI_SECTION, L"PageSelectionType", 6);
  if (indi_style >= 0)
  {
    SendMessage(hwndEdit, SCI_INDICSETSTYLE, N2E_SELECT_INDICATOR_PAGE, indi_style);
    SendMessage(hwndEdit, SCI_INDICSETALPHA, N2E_SELECT_INDICATOR_PAGE, IniGetInt(N2E_INI_SECTION, L"PageSelectionAlpha", 0));
    SendMessage(hwndEdit, SCI_INDICSETOUTLINEALPHA, N2E_SELECT_INDICATOR_PAGE, IniGetInt(N2E_INI_SECTION, L"PageSelectionLineAlpha", 0));
    SendMessage(hwndEdit, SCI_INDICSETFORE, N2E_SELECT_INDICATOR_PAGE, IniGetInt(N2E_INI_SECTION, L"PageSelectionColor", RGB(0x00, 0x00,
                                                                                                                           0x90)));
    SendMessage(hwndEdit, SCI_INDICSETUNDER, N2E_SELECT_INDICATOR_PAGE, IniGetInt(N2E_INI_SECTION, L"PageSelectionUnder", 0));
  }
  //
  indi_style = IniGetInt(N2E_INI_SECTION, L"SingleSelectionType", 6);
  if (indi_style >= 0)
  {
    SendMessage(hwndEdit, SCI_INDICSETSTYLE, N2E_SELECT_INDICATOR_SINGLE, indi_style);
    SendMessage(hwndEdit, SCI_INDICSETALPHA, N2E_SELECT_INDICATOR_SINGLE, IniGetInt(N2E_INI_SECTION, L"SingleSelectionAlpha", 0));
    SendMessage(hwndEdit, SCI_INDICSETOUTLINEALPHA, N2E_SELECT_INDICATOR_SINGLE, IniGetInt(N2E_INI_SECTION, L"SingleSelectionLineAlpha", 0));
    SendMessage(hwndEdit, SCI_INDICSETFORE, N2E_SELECT_INDICATOR_SINGLE, IniGetInt(N2E_INI_SECTION, L"SingleSelectionColor", RGB(0x90, 0x00,
                                                                                                                               0x00)));
    SendMessage(hwndEdit, SCI_INDICSETUNDER, N2E_SELECT_INDICATOR_SINGLE, IniGetInt(N2E_INI_SECTION, L"SingleSelectionUnder", 0));
  }
  //
  indi_style = IniGetInt(N2E_INI_SECTION, L"EditSelectionType", 6);
  if (indi_style >= 0)
  {
    SendMessage(hwndEdit, SCI_INDICSETSTYLE, N2E_SELECT_INDICATOR_EDIT, indi_style);
    SendMessage(hwndEdit, SCI_INDICSETALPHA, N2E_SELECT_INDICATOR_EDIT, IniGetInt(N2E_INI_SECTION, L"EditSelectionAlpha", 100));
    SendMessage(hwndEdit, SCI_INDICSETOUTLINEALPHA, N2E_SELECT_INDICATOR_EDIT, IniGetInt(N2E_INI_SECTION, L"EditSelectionLineAlpha", 0));
    SendMessage(hwndEdit, SCI_INDICSETFORE, N2E_SELECT_INDICATOR_EDIT, IniGetInt(N2E_INI_SECTION, L"EditSelectionColor", RGB(0xaa, 0xaa,
                                                                                                                           0x00)));
    SendMessage(hwndEdit, SCI_INDICSETUNDER, N2E_SELECT_INDICATOR_EDIT, IniGetInt(N2E_INI_SECTION, L"EditSelectionUnder", 0));
  }
  n2e_proc_action = n2e_SelectionKeyAction;
  _n2e_se_tr.lpstrText = 0;
}

void n2e_SelectionRelease()
{
  int k = 0;
  if (_n2e_se_tr.lpstrText)
  {
    n2e_Free(_n2e_se_tr.lpstrText);
    _n2e_se_tr.lpstrText = 0;
  }
  if (_n2e_se_orig_word)
  {
    n2e_Free(_n2e_se_orig_word);
    _n2e_se_orig_word = 0;
  }
  for (k = 0; k < COUNTOF(_n2e_se_array); ++k)
  {
    SE_DATA* se = &_n2e_se_array[k];
    if (se->original)
    {
      n2e_Free(se->original);
      se->original = NULL;
    }
  }
}

int n2e_SelectionGetWraps(int beg, int end)
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

VOID n2e_HighlightWord(LPCSTR  word)
{
  int res = 0;
  int cnt = 0;
  int lstart, lwrap, lrange, len, curr;
  int old;
  struct Sci_TextToFind ttf;
  struct Sci_TextToFind ttf1;
  len = SendMessage(hwndEdit, SCI_GETTEXTLENGTH, 0, 0);
  curr = SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
  if (_n2e_highlight_all)
  {
    lstart = SendMessage(hwndEdit, SCI_GETFIRSTVISIBLELINE, 0, 0);
    lstart = (int)SendMessage(hwndEdit, SCI_DOCLINEFROMVISIBLE, lstart, 0);
  }
  else
  {
    lstart = SendMessage(hwndEdit, SCI_LINEFROMPOSITION, curr, 0);
  }
  lrange = _n2e_highlight_all
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
    int	search_opt = SCFIND_WHOLEWORD;
    int wlen = strlen(word);
    int	curr_indi = N2E_SELECT_INDICATOR_SINGLE;
    BOOL	is_visible = FALSE;
    if (_n2e_se_init)
    {
      _n2e_se_count = 0;
      if (!_n2e_se_mode_whole_word)
      {
        search_opt = SCFIND_MATCHCASE;
      }
      if (_n2e_se_orig_word)
      {
        if (strlen(_n2e_se_orig_word) != wlen + 1)
        {
          _n2e_se_orig_word = n2e_Realloc(_n2e_se_orig_word, wlen + 1);
        }
      }
      else
      {
        _n2e_se_orig_word = n2e_Alloc(wlen + 1);
      }
      strcpy(_n2e_se_orig_word, word);
    }
    // 2 first words
    {
      ttf1.chrg.cpMin = max(0, ttf.chrg.cpMin - _n2e_max_search_range);
      ttf1.chrg.cpMax = min(len, ttf.chrg.cpMin + _n2e_max_search_range);
    }
    ttf1.lpstrText = (LPSTR)word;
    res = SendMessage(hwndEdit, SCI_FINDTEXT, search_opt, (LPARAM)&ttf1);
    is_visible = ttf1.chrgText.cpMin >= ttf.chrg.cpMin && ttf1.chrgText.cpMin < ttf.chrg.cpMax;
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
          // if previous match was visible
          if (is_visible)
          {
            if (_n2e_se_init)
            {
              curr_indi = N2E_SELECT_INDICATOR_EDIT;
              _n2e_edit_selection = TRUE;
              _n2e_se_old_len = wlen;
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
            /*
             previous match was invisible and this is visible
             then we must don't check next matches
             Anyhow HL_SELECT_INDICATOR must be there ?!?!
             **/
            if (!_n2e_se_init)
            {
              break;
            }
          }
          is_visible = TRUE;
        }
        else
        {
          curr_indi = N2E_SELECT_INDICATOR;
        }
        if (ttf1.chrgText.cpMin >= ttf.chrg.cpMax && N2E_SELECT_INDICATOR == curr_indi)
        {
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
    if (_n2e_se_init && !_n2e_edit_selection)
    {
      _n2e_se_init = FALSE;
    }
    lwrap = 0;
    if (_n2e_se_init)
    {
      lwrap = n2e_SelectionGetWraps(lstart, lstart + lrange);
    }
    ttf.lpstrText = (LPSTR)word;
    while (1)
    {
      res = SendMessage(hwndEdit, SCI_FINDTEXT, search_opt, (LPARAM)&ttf);
      if (-1 != res)
      {
        if (_n2e_se_init)
        {
          int line = SendMessage(hwndEdit, SCI_LINEFROMPOSITION, ttf.chrgText.cpMax, 0);
          if (ttf.chrgText.cpMin < _n2e_se_tr.chrg.cpMin && ttf.chrgText.cpMax > _n2e_se_tr.chrg.cpMin)
          {
            N2E_TRACE("SKIP collision SELECTION");
            ttf.chrg.cpMin = ttf.chrgText.cpMax;
            continue;
          }
          N2E_TRACE("[%d] line__ %d (%d , %d , %d) ", ttf.chrgText.cpMin, line, lwrap, lstart, lrange);
          if (line <= lrange + lstart)
          {
            LPSE_DATA dt = &_n2e_se_array[_n2e_se_count++];
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
}

VOID	n2e_SelectionGetWord()
{
  int sel_len = 0, cpos = 0;
  if (_n2e_se_tr.lpstrText)
  {
    n2e_Free(_n2e_se_tr.lpstrText);
    _n2e_se_tr.lpstrText = 0;
  }
  cpos = SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
  if (_n2e_se_init)
  {
    _n2e_se_tr.chrg.cpMin = SendMessage(hwndEdit, SCI_GETSELECTIONSTART, 0, 0);
    _n2e_se_tr.chrg.cpMax = SendMessage(hwndEdit, SCI_GETSELECTIONEND, 0, 0);
    sel_len = _n2e_se_tr.chrg.cpMax - _n2e_se_tr.chrg.cpMin;
    _n2e_se_mode_whole_word = FALSE;
    if (sel_len < 1)
    {
      sel_len = 0;
    }
  }
  if (0 == sel_len)
  {
    _n2e_se_tr.chrg.cpMin = SendMessage(hwndEdit, SCI_WORDSTARTPOSITION, cpos, TRUE);
    _n2e_se_tr.chrg.cpMax = SendMessage(hwndEdit, SCI_WORDENDPOSITION, cpos, TRUE);
    sel_len = _n2e_se_tr.chrg.cpMax - _n2e_se_tr.chrg.cpMin;
    _n2e_se_mode_whole_word = TRUE;
  }
  if (sel_len > (!_n2e_se_init || _n2e_se_mode_whole_word) ? 1 : 0)
  {
    _n2e_se_tr.lpstrText = n2e_Alloc(sel_len + 1);
    SendMessage(hwndEdit, SCI_GETTEXTRANGE, 0, (LPARAM)&_n2e_se_tr);
  }
  else
  {
    _n2e_se_tr.chrg.cpMin = 0;
    _n2e_se_tr.chrg.cpMax = 0;
  }
}

VOID n2e_SelectionHighlightTurn()
{
  if (bHighlightSelection)
  {
    n2e_SelectionGetWord();
    n2e_HighlightWord(_n2e_se_tr.lpstrText);
  }
  else
  {
    int old;
    old = SendMessage(hwndEdit, SCI_GETINDICATORCURRENT, 0, 0);
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

BOOL n2e_SelectionProcessChanges(UINT opt)
{
  int		old_ind;
  int		new_len = 0;
  int		k = 0;
  int		doc_len = SendMessage(hwndEdit, SCI_GETTEXTLENGTH, 0, 0);
  BOOL	out = TRUE;
  BOOL	work = TRUE;
  BOOL	cur_se = FALSE;
  BOOL	rollback = opt & SEO_ROLLBACK;
  char 	*old_word = 0;
  struct	Sci_TextRange	tr;
  int		cur_pos = SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
  int		delta_len = 0;
  tr.lpstrText = 0;
  old_ind = SendMessage(hwndEdit, SCI_GETINDICATORCURRENT, 0, 0);
  if (cur_pos < _n2e_se_tr.chrg.cpMin || cur_pos > _n2e_se_tr.chrg.cpMax)
  {
    N2E_TRACE("OUT OF BOUND  SE exit (applied) %d %d %d", cur_pos, _n2e_se_tr.chrg.cpMin, _n2e_se_tr.chrg.cpMax);
    out = FALSE;
    goto _EXIT;
  }
  if (_n2e_se_tr.chrg.cpMax < _n2e_se_tr.chrg.cpMin)
  {
    N2E_TRACE("critical SE exit");
    out = FALSE;
    goto _EXIT;
  }
  new_len = _n2e_se_tr.chrg.cpMax - _n2e_se_tr.chrg.cpMin;
  old_word = n2e_Alloc(_n2e_se_old_len + 1);
  tr.lpstrText = n2e_Alloc(_n2e_se_old_len + 1);
  SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, N2E_SELECT_INDICATOR_EDIT, 0);
  assert(_n2e_se_tr.lpstrText);
  strcpy(old_word, _n2e_se_tr.lpstrText);
  if (rollback)
  {
    if (0 == _n2e_se_orig_word)
    {
      N2E_TRACE("NO original word ????????????????");
      goto _EXIT;
    }
    N2E_TRACE("ROLLBACK to TR '%s' (%d - %d) ", _n2e_se_orig_word, _n2e_se_tr.chrg.cpMin, _n2e_se_tr.chrg.cpMax);
    new_len = strlen(_n2e_se_orig_word);
    if (_n2e_se_tr.lpstrText && new_len != strlen(_n2e_se_tr.lpstrText))
    {
      _n2e_se_tr.lpstrText = n2e_Realloc(_n2e_se_tr.lpstrText, strlen(_n2e_se_orig_word) + 1);
    }
  }
  else
  {
    if (_n2e_se_tr.lpstrText && (_n2e_se_tr.chrg.cpMax - _n2e_se_tr.chrg.cpMin) != _n2e_se_old_len)
    {
      _n2e_se_tr.lpstrText = n2e_Realloc(_n2e_se_tr.lpstrText, _n2e_se_tr.chrg.cpMax - _n2e_se_tr.chrg.cpMin + 1);
    }
    SendMessage(hwndEdit, SCI_GETTEXTRANGE, 0, (LPARAM)&_n2e_se_tr);
    if (case_compare(old_word, _n2e_se_tr.lpstrText, 0/*_hl_se_mode_whole_word*/))
    {
      N2E_TRACE("case (%d) compare exit  ????????????????", _n2e_se_mode_whole_word);
      goto _EXIT;
    }
  }
  /*
  clear cur edit
  */
  SendMessage(hwndEdit, SCI_SETMODEVENTMASK, n2e_SelectionGetSciEventMask(FALSE), 0);
  for (k = 0; k < _n2e_se_count; ++k)
  {
    LPSE_DATA se = &_n2e_se_array[k];
    // shifting
    N2E_TRACE("start shift: pos:%d cur:%d delta:%d", se->pos, _n2e_se_tr.chrg.cpMin, delta_len);
    se->pos += delta_len;
    if (!rollback && se->pos > _n2e_se_tr.chrg.cpMin)
    {
      se->pos += (new_len - _n2e_se_old_len);
    }
    // check collisions
    if (_n2e_se_tr.chrg.cpMax > se->pos && se->pos > _n2e_se_tr.chrg.cpMin)
    {
      N2E_TRACE("critical SE exit");
      out = FALSE;
      goto _EXIT;
    }
    SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, se->pos, se->len);
    se->len = new_len;
    if (_n2e_se_strict_mode)
    {
      /*
      edited item
      */
      work = FALSE;
      cur_se = (se->pos == _n2e_se_tr.chrg.cpMin && !rollback);
      N2E_TRACE("start check: pos:%d cur:%d delta:%d", se->pos, _n2e_se_tr.chrg.cpMin, delta_len);
      if (!cur_se)
      {
        tr.chrg.cpMin = se->pos;
        tr.chrg.cpMax = se->pos + _n2e_se_old_len;
        doc_len = SendMessage(hwndEdit, SCI_GETTEXTLENGTH, 0, 0);
        if (tr.chrg.cpMax > doc_len)
        {
          N2E_TRACE("!!!SE item last pos out of document (cur pos %d len %d doclen %d) . ", se->pos, _n2e_se_old_len, doc_len);
          break;
        }
        SendMessage(hwndEdit, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
        work = case_compare(tr.lpstrText, old_word, _n2e_se_mode_whole_word);
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
      SendMessage(hwndEdit, SCI_SETTARGETEND, se->pos + _n2e_se_old_len, 0);
      if (rollback)
      {
        assert(case_compare(_n2e_se_orig_word, se->original, TRUE));
        SendMessage(hwndEdit, SCI_REPLACETARGET, -1, (LPARAM)se->original);
      }
      else
      {
        SendMessage(hwndEdit, SCI_REPLACETARGET, -1, (LPARAM)_n2e_se_tr.lpstrText);
      }
      delta_len += (new_len - _n2e_se_old_len);
      if (se->pos < _n2e_se_tr.chrg.cpMax)
      {
        _n2e_se_tr.chrg.cpMin += (new_len - _n2e_se_old_len);
        _n2e_se_tr.chrg.cpMax += (new_len - _n2e_se_old_len);
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
  _n2e_se_old_len = new_len;
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
  N2E_TRACE("new range is %d : %d . curpos is %d", _n2e_se_tr.chrg.cpMin, _n2e_se_tr.chrg.cpMax, cur_pos);
  SendMessage(hwndEdit, SCI_SETMODEVENTMASK, n2e_SelectionGetSciEventMask(TRUE), 0);
  return out;
}

VOID n2e_SelectionEditStart(const BOOL highlightAll)
{
  _n2e_highlight_all = highlightAll;
  // if mode already ON - then turn it OFF
  if (_n2e_edit_selection)
  {
    n2e_SelectionEditStop(N2E_SE_APPLY);
    return;
  }
  _n2e_se_init = TRUE;
  _n2e_se_count = 0;
  n2e_SelectionHighlightTurn();
  _n2e_se_init = FALSE;
  if (_n2e_edit_selection)
  {
    SendMessage(hwndEdit, SCI_SETSEL, _n2e_se_tr.chrg.cpMin, _n2e_se_tr.chrg.cpMax);
    SendMessage(hwndEdit, SCI_BEGINUNDOACTION, 0, 0);
    _n2e_se_exit = FALSE;
  }
}

BOOL n2e_SelectionEditStop(UINT mode)
{
  _n2e_se_init = FALSE;
  if (_n2e_edit_selection)
  {
    if (mode & N2E_SE_REJECT)
    {
      n2e_SelectionProcessChanges(SEO_ROLLBACK);
    }
    /*
     * skip any selection
     */
    const int pos = SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
    SendMessage(hwndEdit, SCI_SETANCHOR, pos, 0);
    _n2e_edit_selection = FALSE;
    _n2e_highlight_all = TRUE;

    //
    n2e_SelectionHighlightTurn();
    SendMessage(hwndEdit, SCI_ENDUNDOACTION, 0, 0);
    return TRUE;
  }
  return FALSE;
}

void n2e_SelectionUpdate(UINT place)
{
  if (_n2e_edit_selection)
  {
    UINT opt = 0;
    if (_n2e_se_exit)
    {
      n2e_SelectionEditStop(N2E_SE_APPLY);
      return;
    }
    if (SH_MODIF == place)
    {
      opt |= SEO_MODIFIED;
    }
    else
    {
      if (!n2e_SelectionProcessChanges(opt))
      {
        n2e_SelectionEditStop(N2E_SE_APPLY);
      }
    }
  }
  else
  {
    n2e_SelectionHighlightTurn();
  }
}

BOOL _check_se_mode(struct SCNotification *scn)
{
  if (scn->position >= _n2e_se_tr.chrg.cpMin && scn->position < _n2e_se_tr.chrg.cpMin + _n2e_se_old_len)
  {
    return TRUE;
  }
  _n2e_se_exit = TRUE;
  return FALSE;
}

void nn2e_SelectionNotificationHandler(int code, struct SCNotification *scn)
{
  switch (code)
  {
    case SCN_UPDATEUI:
      if (bHighlightSelection)
      {
        n2e_SelectionUpdate(SH_UPDATE);
      }
      break;
    case SCN_MODIFIED:
      if ((scn->modificationType & (SC_MOD_CONTAINER|SC_PERFORMED_UNDO)) == (SC_MOD_CONTAINER|SC_PERFORMED_UNDO))
      {
        PostMessage(hwndEdit, SCI_GOTOPOS, (WPARAM)scn->token, 0);
      }
      else if (bHighlightSelection)
      {
        if (_n2e_edit_selection)
        {
          if (scn->modificationType & SC_MOD_INSERTTEXT)
          {
            N2E_TRACE("MODIF INSERT pos:%d len%d lines:%d text:%s", scn->position, scn->length, scn->linesAdded, scn->text);
            _n2e_se_tr.chrg.cpMax += scn->length;
          }
          else if (scn->modificationType & SC_MOD_DELETETEXT)
          {
            N2E_TRACE("MODIF DELETE pos:%d len%d lines:%d text:%s", scn->position, scn->length, scn->linesAdded, scn->text);
            _n2e_se_tr.chrg.cpMax -= scn->length;
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
      n2e_SelectionEditStop(N2E_SE_APPLY);
      break;
  }
}

UINT n2e_SelectionGetSciEventMask(BOOL range_not)
{
  UINT out = SC_PERFORMED_UNDO | SC_PERFORMED_REDO;
  if (range_not)
  {
    out |= SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT;
  }
  return out;
}