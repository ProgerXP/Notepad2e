#define _WIN32_WINNT 0x501
#include <windows.h>
#include "scintilla.h"
#include "HLSelection.h"
#include "Helpers.h"
#include <cassert>

/************************************************************************/
/* when many matches on ALL document  but NOT ALL OF THEM  on screen                                                                   */
/************************************************************************/
#define HL_SELECT_INDICATOR 9
/************************************************************************/
/* when one match one document                                                                     */
/************************************************************************/
#define HL_SELECT_INDICATOR_SINGLE 10
/************************************************************************/
/* when many matches on screen                                                                     */
/************************************************************************/
#define HL_SELECT_INDICATOR_PAGE 11   
/************************************************************************/
/* SE mode                                                                     */
/************************************************************************/
#define HL_SELECT_INDICATOR_EDIT 12

#define HL_SELECT_MAX_SIZE	0xff
#define HL_SELECT_MAX_COUNT	0xff


BOOL	b_HL_highlight_selection = TRUE;
BOOL	b_HL_edit_selection = FALSE;
BOOL	_hl_se_init = FALSE;
BOOL	_hl_se_exit = FALSE;
//
//
typedef struct tagHLSEdata {
    UINT pos;
    UINT len;
} SE_DATA, *LPSE_DATA ;

typedef enum HL_SEOpt {
    SEO_ROLLBACK = 1 << 0,
    SEO_MODIFIED = 1 << 1
} ;
//
SE_DATA		_hl_se_array [HL_SELECT_MAX_COUNT];
UINT		_hl_se_count = 0; // total count   '
struct		Sci_TextRange	_hl_se_tr;
UINT		_hl_se_old_len = 0;
UINT		_hl_max_search_range = 2048 * 1024;
BOOL		_hl_se_mode_whole_word = TRUE;
BOOL		_hl_se_strict_mode = TRUE;
char		*_hl_se_orig_word = 0;

char to_lower(char in){
	if (in <= 'Z' && in >= 'A')
		return in - ('Z' - 'z');
	if (in <= 'ß' && in >= 'À')
		return in - ('ß' - 'ÿ');
	return in;
}


BOOL case_compare(const char* a, const char* b){
	return 0 == strcmp(a, b);
}

BOOL icase_compare(const char* a, const char* b){
	while (*a && *b) {
		if (*a != *b) {
			char lA = to_lower(*a);
			char lB = to_lower(*b);
			if (lA != lB){
				return FALSE;
			}
		}
		a++;
		b++;
	}
	// Either *a or *b is nul
	return *a == *b;
}

int	HLS_key_action ( int key , int msg )
{
    if ( b_HL_edit_selection ) {

        if ( VK_RETURN == key && GetKeyState(VK_SHIFT) >= 0 ) {
            if ( WM_CHAR == msg ) {
				HLS_Edit_selection_stop(HL_SE_APPLY);
            }
			return 0;
        }
    }
	return -1;
}


void	HLS_init()
{
	int indi_style = IniGetInt(HL_INI_SECTION, L"SelectionType", 6);
	if (indi_style >= 0) {
		SendMessage(hwndEdit, SCI_INDICSETSTYLE, HL_SELECT_INDICATOR, indi_style);
		SendMessage(hwndEdit, SCI_INDICSETALPHA, HL_SELECT_INDICATOR, IniGetInt(HL_INI_SECTION, L"SelectionAlpha", 0));
		SendMessage(hwndEdit, SCI_INDICSETOUTLINEALPHA, HL_SELECT_INDICATOR, IniGetInt(HL_INI_SECTION, L"SelectionLineAlpha", 0));
		SendMessage(hwndEdit, SCI_INDICSETFORE, HL_SELECT_INDICATOR, IniGetInt(HL_INI_SECTION, L"SelectionColor", RGB(0x00, 0x00, 0x00)));
		SendMessage(hwndEdit, SCI_INDICSETUNDER, HL_SELECT_INDICATOR, IniGetInt(HL_INI_SECTION, L"SelectionUnder", 0));
	}
	//		
	indi_style = IniGetInt(HL_INI_SECTION, L"PageSelectionType", 6);
	if (indi_style >= 0) {
		SendMessage(hwndEdit, SCI_INDICSETSTYLE, HL_SELECT_INDICATOR_PAGE, indi_style);
		SendMessage(hwndEdit, SCI_INDICSETALPHA, HL_SELECT_INDICATOR_PAGE, IniGetInt(HL_INI_SECTION, L"PageSelectionAlpha", 0));
		SendMessage(hwndEdit, SCI_INDICSETOUTLINEALPHA, HL_SELECT_INDICATOR_PAGE, IniGetInt(HL_INI_SECTION, L"PageSelectionLineAlpha", 0));
		SendMessage(hwndEdit, SCI_INDICSETFORE, HL_SELECT_INDICATOR_PAGE, IniGetInt(HL_INI_SECTION, L"PageSelectionColor", RGB(0x00, 0x00,
			0x90)));
		SendMessage(hwndEdit, SCI_INDICSETUNDER, HL_SELECT_INDICATOR_PAGE, IniGetInt(HL_INI_SECTION, L"PageSelectionUnder", 0));
	}
	//	
	indi_style = IniGetInt(HL_INI_SECTION, L"SingleSelectionType", 6);
	if (indi_style >= 0) {
		SendMessage(hwndEdit, SCI_INDICSETSTYLE, HL_SELECT_INDICATOR_SINGLE, indi_style);
		SendMessage ( hwndEdit , SCI_INDICSETALPHA , HL_SELECT_INDICATOR_SINGLE , IniGetInt ( HL_INI_SECTION , L"SingleSelectionAlpha" , 0 ) );
		SendMessage ( hwndEdit , SCI_INDICSETOUTLINEALPHA , HL_SELECT_INDICATOR_SINGLE , IniGetInt ( HL_INI_SECTION , L"SingleSelectionLineAlpha" , 0 ) );
		SendMessage ( hwndEdit , SCI_INDICSETFORE , HL_SELECT_INDICATOR_SINGLE , IniGetInt ( HL_INI_SECTION , L"SingleSelectionColor" , RGB (	0x90 , 0x00,
					  0x00 ) ) );
		SendMessage(hwndEdit, SCI_INDICSETUNDER, HL_SELECT_INDICATOR_SINGLE, IniGetInt(HL_INI_SECTION, L"SingleSelectionUnder", 0));
	}
    //
	indi_style = IniGetInt(HL_INI_SECTION, L"EditSelectionType", 6);
	if (indi_style >= 0) {
		SendMessage(hwndEdit, SCI_INDICSETSTYLE, HL_SELECT_INDICATOR_EDIT, indi_style);
		SendMessage(hwndEdit, SCI_INDICSETALPHA, HL_SELECT_INDICATOR_EDIT, IniGetInt(HL_INI_SECTION, L"EditSelectionAlpha", 100));
		SendMessage(hwndEdit, SCI_INDICSETOUTLINEALPHA, HL_SELECT_INDICATOR_EDIT, IniGetInt(HL_INI_SECTION, L"EditSelectionLineAlpha", 0));
		SendMessage(hwndEdit, SCI_INDICSETFORE, HL_SELECT_INDICATOR_EDIT, IniGetInt(HL_INI_SECTION, L"EditSelectionColor", RGB(0xaa, 0xaa,
			0x00)));
		SendMessage(hwndEdit, SCI_INDICSETUNDER, HL_SELECT_INDICATOR_EDIT, IniGetInt(HL_INI_SECTION, L"EditSelectionUnder", 0));
	}
    //
    hl_proc_action = HLS_key_action;
    _hl_se_tr.lpstrText = 0;
}


void HLS_release()
{
    if ( _hl_se_tr.lpstrText ) {
        HL_Free ( _hl_se_tr.lpstrText );
        _hl_se_tr.lpstrText = 0;
    }
    if ( _hl_se_orig_word ) {
        HL_Free ( _hl_se_orig_word );
        _hl_se_orig_word = 0;
    }
}

int HLS_get_wraps ( int beg , int end )
{
    int k = 0;
    int out = 0;
	int len = SendMessage(hwndEdit, SCI_GETLINECOUNT, 0, 0);
    for ( k = beg ; k < end && k + beg < len ; ++ k ) {
        out += SendMessage ( hwndEdit , SCI_WRAPCOUNT , beg + k   , 0 ) - 1;
    }
    return out;
}



VOID HLS_Highlight_word ( LPCSTR  word )
{
    int res  = 0;
    int cnt = 0;
    int lstart , lwrap , lrange , len , curr;
    int old;
    struct Sci_TextToFind ttf;
    struct Sci_TextToFind ttf1;
    //
    //
    lstart = SendMessage ( hwndEdit , SCI_GETFIRSTVISIBLELINE , 0 , 0 );
    lstart = ( int ) SendMessage ( hwndEdit, SCI_DOCLINEFROMVISIBLE, lstart , 0 );
    lrange = min ( SendMessage ( hwndEdit , SCI_LINESONSCREEN , 0 , 0 ) , SendMessage ( hwndEdit , SCI_GETLINECOUNT , 0 , 0 ) );
    ttf.chrg.cpMin  = SendMessage ( hwndEdit , SCI_POSITIONFROMLINE , lstart  , 0 );
    len = SendMessage ( hwndEdit , SCI_GETTEXTLENGTH , 0 , 0 );
	curr = SendMessage(hwndEdit, SCI_GETCURRENTPOS, 0, 0);
    ttf.chrg.cpMax  = SendMessage ( hwndEdit , SCI_GETLINEENDPOSITION , lstart + lrange, 0 ) + 1  ;
    old = SendMessage ( hwndEdit , SCI_GETINDICATORCURRENT , 0 , 0 );
    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR , 0 );
    SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , 0 , len );
    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR_EDIT , 0 );
    SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , 0 , len );
	SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, HL_SELECT_INDICATOR_PAGE, 0);
	SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, 0, len);
	SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, HL_SELECT_INDICATOR_SINGLE, 0);
	SendMessage(hwndEdit, SCI_INDICATORCLEARRANGE, 0, len);
    if ( word ) {
        int	search_opt = SCFIND_WHOLEWORD;
        int wlen = strlen ( word );
		int	curr_indi = HL_SELECT_INDICATOR_SINGLE;
		BOOL	is_visible = FALSE;
        if ( _hl_se_init ) {
            _hl_se_count = 0;
            //    strcpy ( _hl_sel_edit_prev , word );
            //    strcpy ( _hl_sel_edit_orig , word );
            if ( _hl_se_mode_whole_word ) {
                //search_opt |= SCFIND_MATCHCASE;
            } else {
                search_opt = SCFIND_MATCHCASE;
            }
            //
            if ( _hl_se_orig_word ){
				if (strlen(_hl_se_orig_word) != wlen + 1) {
					_hl_se_orig_word = HL_Realloc(_hl_se_orig_word, wlen + 1);
				}
            } else {
                _hl_se_orig_word = HL_Alloc ( wlen + 1 );
            }
            strcpy ( _hl_se_orig_word , word );
        }
        // 2 first words
#if 0
		ttf1.chrg.cpMin = 0;
		ttf1.chrg.cpMax = len;
#else
		{
			ttf1.chrg.cpMin = max(0, ttf.chrg.cpMin - _hl_max_search_range);
			ttf1.chrg.cpMax = min(len, ttf.chrg.cpMin + _hl_max_search_range);
		}
#endif
		//
		HL_TRACE(L"HL WORD RANGES %d-%d %d-%d", ttf.chrg.cpMin, ttf.chrg.cpMax, ttf1.chrg.cpMin, ttf1.chrg.cpMax);
        ttf1.lpstrText = ( LPSTR ) word;
        res =   SendMessage ( hwndEdit , SCI_FINDTEXT , search_opt , ( LPARAM ) &ttf1 );
		is_visible = ttf1.chrgText.cpMin >= ttf.chrg.cpMin && ttf1.chrgText.cpMin < ttf.chrg.cpMax;
        while ( 1 ) {
            ttf1.chrg.cpMin = ttf1.chrgText.cpMax;
            res =   SendMessage ( hwndEdit , SCI_FINDTEXT , search_opt , ( LPARAM ) &ttf1 );
			if (-1 != res) {
				/************************************************************************/
				/* current match is visible                                                                     */
				/************************************************************************/
				if (
					ttf1.chrgText.cpMin >= ttf.chrg.cpMin &&
					ttf1.chrgText.cpMin < ttf.chrg.cpMax
					) {
					/************************************************************************/
					/* if previous match was visible                                                                     */
					/************************************************************************/
					if (is_visible) {
						if (_hl_se_init) {
							curr_indi = HL_SELECT_INDICATOR_EDIT;
							b_HL_edit_selection = TRUE;
							_hl_se_old_len = wlen;
							break;
						}
						else {
							curr_indi = HL_SELECT_INDICATOR_PAGE;
						}
					}
					else{
						curr_indi = HL_SELECT_INDICATOR;
						
						/*
						 previous match was invisible and this is visible
						 then we must don't check next matches
						 Anyhow HL_SELECT_INDICATOR must be there ?!?!
						 **/
						if (!_hl_se_init)
						{
							break;
						}
					}
					is_visible = TRUE;
				}
				else {
					curr_indi = HL_SELECT_INDICATOR;
				}
				//
				if (ttf1.chrgText.cpMin >= ttf.chrg.cpMax && HL_SELECT_INDICATOR == curr_indi) {
					break;
				}
				ttf1.chrg.cpMin = ttf1.chrgText.cpMax;
			}
			else {
				break;
			}
        }
		//
		HL_TRACE("INDICATOR OF WORD '%s' is %d", word, curr_indi);
		SendMessage(hwndEdit, SCI_SETINDICATORCURRENT, curr_indi, 0);
        //
        if ( _hl_se_init && !b_HL_edit_selection ) {
            _hl_se_init = FALSE;
        }
        lwrap = 0;
        if ( _hl_se_init ) {
            lwrap = HLS_get_wraps ( lstart , lstart + lrange );
        }
        //
        ttf.lpstrText = ( LPSTR ) word;
        while ( 1 ) {
            res =   SendMessage ( hwndEdit , SCI_FINDTEXT , search_opt , ( LPARAM ) &ttf );
            if ( -1 != res ) {
                if ( _hl_se_init ) {
                    int line = SendMessage ( hwndEdit , SCI_LINEFROMPOSITION , ttf.chrgText.cpMax  , 0 );
                    //line = SendMessage ( hwndEdit , SCI_VISIBLEFROMDOCLINE , line , 0 );
                    if ( ttf.chrgText.cpMin < _hl_se_tr.chrg.cpMin && ttf.chrgText.cpMax > _hl_se_tr.chrg.cpMin ) {
						HL_TRACE ( "SKIP collision SELECTION" );
						ttf.chrg.cpMin = ttf.chrgText.cpMax;
                        continue;
                    }
                    //HL_TRACE ( " line %d ", line );
					HL_TRACE("[%d] line__ %d (%d , %d , %d) ", ttf.chrgText.cpMin, line, lwrap, lstart, lrange);
                    if ( line /* + lwrap */ <= lrange + lstart ) {
                        LPSE_DATA dt = &_hl_se_array[_hl_se_count++];
                        dt->pos = ttf.chrgText.cpMin;
                        dt->len = wlen;
					}
					else {
						HL_TRACE("out of loop");
                        break;
                    }
                }
                SendMessage ( hwndEdit , SCI_INDICATORFILLRANGE , ttf.chrgText.cpMin , ttf.chrgText.cpMax - ttf.chrgText.cpMin );
                cnt++;
                ttf.chrg.cpMin = ttf.chrgText.cpMax;
            } else {
                break;
            }
        }
    }
    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , old , 0 );
    //
}

VOID	HLS_Get_word()
{
    int sel_len = 0 , cpos = 0  ;
    //
    if ( _hl_se_tr.lpstrText ) {
        HL_Free ( _hl_se_tr.lpstrText );
        _hl_se_tr.lpstrText = 0;
    }
    //
    cpos = SendMessage ( hwndEdit , SCI_GETCURRENTPOS , 0 , 0 );
    //
    if ( _hl_se_init ) {
        _hl_se_tr.chrg.cpMin = SendMessage ( hwndEdit , SCI_GETSELECTIONSTART , 0 , 0 );
        _hl_se_tr.chrg.cpMax = SendMessage ( hwndEdit , SCI_GETSELECTIONEND , 0 , 0 );
        sel_len = _hl_se_tr.chrg.cpMax - _hl_se_tr.chrg.cpMin;
        _hl_se_mode_whole_word = FALSE;
        //
        if ( sel_len < 1 ) {
            sel_len = 0;
        }
    }
    if ( 0 == sel_len ) {
        _hl_se_tr.chrg.cpMin = SendMessage ( hwndEdit , SCI_WORDSTARTPOSITION , cpos , TRUE );
        _hl_se_tr.chrg.cpMax = SendMessage ( hwndEdit , SCI_WORDENDPOSITION , cpos , TRUE );
        sel_len = _hl_se_tr.chrg.cpMax - _hl_se_tr.chrg.cpMin;
        _hl_se_mode_whole_word = TRUE;
    }
    //
    if ( sel_len > ( !_hl_se_init || _hl_se_mode_whole_word ) ? 1 : 0 ) {
        _hl_se_tr.lpstrText = HL_Alloc ( sel_len + 1 );
        SendMessage ( hwndEdit, SCI_GETTEXTRANGE , 0 , ( LPARAM ) &_hl_se_tr );
		HL_TRACE_TR(_hl_se_tr);
    } else {
        _hl_se_tr.chrg.cpMin = 0;
        _hl_se_tr.chrg.cpMax = 0;
    }
}

VOID HLS_Highlight_turn ( )
{
    if ( b_HL_highlight_selection ) {
        //
        HLS_Get_word();
        //    HL_TRACE_S ( _hl_se_tr.lpstrText );
        HLS_Highlight_word ( _hl_se_tr.lpstrText );
    } else {
        int old;
        old = SendMessage ( hwndEdit , SCI_GETINDICATORCURRENT , 0 , 0 );
        //
        SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR , 0 );
        SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , 0 ,
                      SendMessage ( hwndEdit , SCI_GETTEXTLENGTH , 0 , 0 ) );
        //
        SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR_SINGLE , 0 );
        SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , 0 ,
                      SendMessage ( hwndEdit , SCI_GETTEXTLENGTH , 0 , 0 ) );
        SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , old , 0 );
        //
        SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR_EDIT , 0 );
        SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , 0 ,
                      SendMessage ( hwndEdit , SCI_GETTEXTLENGTH , 0 , 0 ) );
        SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , old , 0 );
    }
}





BOOL HLS_process_changes ( UINT opt )
{
    int		old_ind;
    int		new_len = 0;
    int		k = 0;
    int		doc_len = SendMessage ( hwndEdit , SCI_GETTEXTLENGTH, 0, 0 );
    BOOL	out = TRUE;
    BOOL	work = TRUE;
    BOOL	cur_se = FALSE;
    BOOL	rollback = opt & SEO_ROLLBACK ;
    char 	*old_word = 0;
    struct	Sci_TextRange	tr;
    int		cur_pos = SendMessage ( hwndEdit , SCI_GETCURRENTPOS , 0 , 0 );
    int		delta_len = 0;
    //
    tr.lpstrText = 0;
    old_ind = SendMessage ( hwndEdit , SCI_GETINDICATORCURRENT , 0 , 0 );
    //
    if ( cur_pos < _hl_se_tr.chrg.cpMin || cur_pos > _hl_se_tr.chrg.cpMax ) {
        HL_TRACE ( "OUT OF BOUND  SE exit (applied) %d %d %d" , cur_pos , _hl_se_tr.chrg.cpMin , _hl_se_tr.chrg.cpMax );
        out = FALSE;
        goto _EXIT;
    }
    //
    if ( _hl_se_tr.chrg.cpMax < _hl_se_tr.chrg.cpMin ) {
        HL_TRACE ( "critical SE exit" );
        out = FALSE;
        goto _EXIT;
    }
    //
    new_len = _hl_se_tr.chrg.cpMax - _hl_se_tr.chrg.cpMin;
    old_word = HL_Alloc ( _hl_se_old_len + 1 );
    tr.lpstrText = HL_Alloc ( _hl_se_old_len + 1 );
    //
    /*
    SET EDIT INDOCATOR
    */
    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR_EDIT , 0 );
    //
    assert ( _hl_se_tr.lpstrText );
    strcpy ( old_word , _hl_se_tr.lpstrText );
    if ( rollback ) {
        if ( 0 == _hl_se_orig_word ) {
            //
            HL_TRACE ( "NO orginal word ????????????????" );
            goto _EXIT;
        }
        HL_TRACE ( "ROLLBACK to TR '%s' (%d - %d) " , _hl_se_orig_word , _hl_se_tr.chrg.cpMin , _hl_se_tr.chrg.cpMax );
        new_len = strlen ( _hl_se_orig_word );
        if ( _hl_se_tr.lpstrText && new_len != strlen ( _hl_se_tr.lpstrText ) ) {
            _hl_se_tr.lpstrText = HL_Realloc ( _hl_se_tr.lpstrText , strlen ( _hl_se_orig_word ) + 1 );
        }
        strcpy ( _hl_se_tr.lpstrText , _hl_se_orig_word );
    } else {
        if ( _hl_se_tr.lpstrText && ( _hl_se_tr.chrg.cpMax - _hl_se_tr.chrg.cpMin ) != _hl_se_old_len ) {
            _hl_se_tr.lpstrText = HL_Realloc ( _hl_se_tr.lpstrText , _hl_se_tr.chrg.cpMax - _hl_se_tr.chrg.cpMin + 1 );
        }
        SendMessage ( hwndEdit , SCI_GETTEXTRANGE , 0 , ( LPARAM ) &_hl_se_tr );
        //
		if (case_compare(old_word, _hl_se_tr.lpstrText)) {
            goto _EXIT;
        }
    }
#ifdef _DEBUG
    HL_TRACE ( "current TR '%s' (%d - %d) " , _hl_se_tr.lpstrText , _hl_se_tr.chrg.cpMin , _hl_se_tr.chrg.cpMax );
    for ( k = 0 ; k < _hl_se_count; ++k ) {
        HL_TRACE ( "pos %d = %d (%d)" , k , _hl_se_array[k].pos , _hl_se_array[k].len );
    }
#endif
    /*
    clear cur edit
    */
    //SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , _hl_se_tr.chrg.cpMin , _hl_se_old_len );
    //	_hl_se_notif_block = TRUE;
    SendMessage ( hwndEdit, SCI_SETMODEVENTMASK, HLS_Sci_event_mask ( FALSE ), 0 );
    tr.lpstrText = HL_Alloc ( _hl_se_old_len + 1 );
    for ( k = 0 ; k < _hl_se_count; ++ k ) {
        LPSE_DATA se = &_hl_se_array[k];
        // shifting
        HL_TRACE ( "start shift: pos:%d cur:%d delta:%d" , se->pos , _hl_se_tr.chrg.cpMin , delta_len );
        se->pos += delta_len;
        if ( !rollback && se->pos > _hl_se_tr.chrg.cpMin ) {
            se->pos += ( new_len - _hl_se_old_len );
        }
        // check collisions
        if (
            //	( se->pos < _hl_se_tr.chrg.cpMin && ( se->pos + new_len ) >= _hl_se_tr.chrg.cpMin )
            //	||
            ( _hl_se_tr.chrg.cpMax > se->pos && se->pos > _hl_se_tr.chrg.cpMin )
        ) {
            HL_TRACE ( "critical SE exit" );
            out = FALSE;
            goto _EXIT;
        }
        //
        SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , se->pos , se->len );
        se->len = new_len;
        //
        if ( _hl_se_strict_mode ) {
            /*
            edited item
            */
            work = FALSE;
            cur_se = ( se->pos == _hl_se_tr.chrg.cpMin && !rollback );
            HL_TRACE ( "start check: pos:%d cur:%d delta:%d" , se->pos , _hl_se_tr.chrg.cpMin , delta_len );
            if ( !cur_se ) {
                tr.chrg.cpMin = se->pos;
                tr.chrg.cpMax = se->pos + _hl_se_old_len;
                doc_len = SendMessage ( hwndEdit , SCI_GETTEXTLENGTH, 0, 0 );
                if ( tr.chrg.cpMax > doc_len ) {
                    HL_TRACE ( "!!!SE item last pos out of document (cur pos %d len %d doclen %d) . " ,  se->pos , _hl_se_old_len , doc_len );
                    break;
                }
                SendMessage ( hwndEdit , SCI_GETTEXTRANGE , 0 , ( LPARAM ) &tr );
				work = case_compare(tr.lpstrText, old_word);
            } else {
                work = FALSE;
                HL_TRACE ( "cur pos!" )
            }
        } else {
            work = TRUE;
        }
        if ( work ) {
            // SendMessage ( hwndEdit , SCI_DELETERANGE , se->pos , ( LPARAM ) _hl_se_old_len );
            SendMessage ( hwndEdit , SCI_SETTARGETSTART , se->pos , 0 );
            SendMessage ( hwndEdit , SCI_SETTARGETEND , se->pos + _hl_se_old_len , 0 );
            SendMessage ( hwndEdit , SCI_REPLACETARGET , -1 , ( LPARAM ) _hl_se_tr.lpstrText );
            delta_len += ( new_len - _hl_se_old_len );
            if ( se->pos </*=*/ _hl_se_tr.chrg.cpMax ) {
                _hl_se_tr.chrg.cpMin += ( new_len - _hl_se_old_len );
                _hl_se_tr.chrg.cpMax += ( new_len - _hl_se_old_len );
            }
        } else if ( !cur_se ) {
            HL_TRACE ( "!!!SE mismatch error at idx %d pos %d expect %s but got %s then skip item" , k , se->pos , old_word , tr.lpstrText );
        }
        //
        SendMessage ( hwndEdit , SCI_INDICATORFILLRANGE , se->pos , new_len );
        HL_TRACE ( "new se pos %d = %d (%d). delta %d" , k , se->pos , se->len , delta_len );
    }
    // exit
_EXIT:
    _hl_se_old_len = new_len;
    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , old_ind , 0 );
    if ( old_word ) {
        HL_Free ( old_word );
        old_word = 0;
    }
    if ( tr.lpstrText ) {
        HL_Free ( tr.lpstrText );
        tr.lpstrText = 0;
    }
    HL_TRACE ( "new range is %d : %d . curpos is %d" , _hl_se_tr.chrg.cpMin , _hl_se_tr.chrg.cpMax , cur_pos );
    //   SendMessage ( hwndEdit , SCI_SETCURRENTPOS , cur_pos , 0 );
    SendMessage ( hwndEdit, SCI_SETMODEVENTMASK, HLS_Sci_event_mask ( TRUE ), 0 );
    return out;
}



VOID HLS_Edit_selection_start()
{
    // if mode already ON - then turn it OFF
    if ( b_HL_edit_selection ) {
        HLS_Edit_selection_stop ( HL_SE_APPLY );
        return;
    }
    _hl_se_init = TRUE;
    _hl_se_count = 0;
    HLS_Highlight_turn ( );
    _hl_se_init = FALSE;
    if ( b_HL_edit_selection ) {
        SendMessage ( hwndEdit , SCI_SETSEL , _hl_se_tr.chrg.cpMin , _hl_se_tr.chrg.cpMax );
        SendMessage ( hwndEdit , SCI_BEGINUNDOACTION , 0, 0 );
        _hl_se_exit = FALSE;
    }
}


VOID HLS_Edit_selection_stop ( UINT mode )
{
    int pos;
    if ( b_HL_edit_selection ) {
        if ( mode & HL_SE_REJECT ) {
            HLS_process_changes ( SEO_ROLLBACK );
        }
        /*
         * skip any selection
         */
        pos = SendMessage ( hwndEdit , SCI_GETCURRENTPOS , 0 , 0 );
        SendMessage ( hwndEdit , SCI_SETANCHOR , pos , 0 );
        b_HL_edit_selection = FALSE;
        //
		HLS_Highlight_turn();
        SendMessage ( hwndEdit , SCI_ENDUNDOACTION , 0, 0 );
    }
    _hl_se_init = FALSE;
}

void HLS_Update_selection ( UINT place )
{
    if ( b_HL_edit_selection ) {
        UINT opt = 0;
        if ( _hl_se_exit ) {
            HLS_Edit_selection_stop ( HL_SE_APPLY );
            return;
        }
        if ( SH_MODIF == place ) {
            opt |= SEO_MODIFIED;
        } else {
            if ( !HLS_process_changes ( opt ) ) {
                HLS_Edit_selection_stop ( HL_SE_APPLY );
            }
        }
    } else {
        HLS_Highlight_turn();
    }
}

BOOL _check_se_mode ( struct SCNotification *scn )
{
    if ( scn->position >= _hl_se_tr.chrg.cpMin && scn->position < _hl_se_tr.chrg.cpMin + _hl_se_old_len ) {
        return TRUE;
    }
    _hl_se_exit = TRUE;
    return FALSE;
}

void HLS_on_notification ( int code , struct SCNotification *scn )
{
    if ( SCN_PAINTED != code ) {
//        HL_TRACE_I ( code );
    }
    //
    switch ( code ) {
        case SCN_UPDATEUI:
            if ( b_HL_highlight_selection ) {
                HLS_Update_selection ( SH_UPDATE );
            }
            break;
        case SCN_MODIFIED:
            if ( b_HL_highlight_selection ) {
                //
#if 1
                if ( b_HL_edit_selection )
#endif
                {
                    if ( scn->modificationType & SC_MOD_INSERTTEXT ) {
                        HL_TRACE ( "MODIF INSERT pos:%d len%d lines:%d text:%s" , scn->position , scn->length , scn->linesAdded  , scn->text );
                        //
                        _hl_se_tr.chrg.cpMax += scn->length;
                        //
                    } else if ( scn->modificationType & SC_MOD_DELETETEXT ) {
                        HL_TRACE ( "MODIF DELETE pos:%d len%d lines:%d text:%s" , scn->position , scn->length  , scn->linesAdded, scn->text );
                        //
                        _hl_se_tr.chrg.cpMax -= scn->length;
                    } else if ( scn->modificationType & SC_PERFORMED_USER ) {
                        HL_TRACE ( "MODIF PERFORMED USER" );
                    } else if ( scn->modificationType & SC_PERFORMED_UNDO ) {
                        HL_TRACE ( "MODIF PERFORMED UNDO" );
                    } else if ( scn->modificationType & SC_PERFORMED_REDO ) {
                        HL_TRACE ( "MODIF PERFORMED REDO" );
                    } else if ( scn->modificationType & SC_MOD_BEFOREINSERT ) {
                        HL_TRACE ( "MODIF BEFORE INSERT pos:%d len%d " , scn->position , scn->length );
                    } else if ( scn->modificationType & SC_MOD_BEFOREDELETE ) {
                        HL_TRACE ( "MODIF BEFORE DELETE pos:%d len%d " , scn->position , scn->length );
                    } else if ( scn->modificationType & SC_MULTILINEUNDOREDO ) {
                        HL_TRACE ( "MODIF MULTILINE UNDO" );
                    } else if ( scn->modificationType & SC_STARTACTION ) {
                        HL_TRACE ( "MODIF START ACTION" );
                    }
                }
     //           HLS_Update_selection ( SH_MODIF );
            }
            break;
        case SCN_SAVEPOINTREACHED:
        case SCEN_KILLFOCUS:
            HLS_Edit_selection_stop ( HL_SE_APPLY );
            break;
    }
}

UINT HLS_Sci_event_mask ( BOOL range_not )
{
    UINT out = SC_PERFORMED_UNDO | SC_PERFORMED_REDO;
    if ( range_not ) {
        out |= SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT;
    }
    return out;
}


 