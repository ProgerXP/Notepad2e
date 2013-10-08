#define _WIN32_WINNT 0x501
#include <windows.h>
#include "scintilla.h"
#include "HLSelection.h"
#include "Helpers.h"


#define HL_SELECT_INDICATOR 9
#define HL_SELECT_INDICATOR_SINGLE 10
#define HL_SELECT_INDICATOR_EDIT 11

#define HL_SELECT_MAX_SIZE	0xff
#define HL_SELECT_MAX_COUNT	0xff
#define HL_SEARCH_WORD_SIZE (64*1024)


BOOL	b_HL_highlight_selection = TRUE;
BOOL	b_HL_edit_selection = FALSE;
BOOL	_hl_se_init = FALSE;
struct	Sci_TextRange	_hl_se_tr;
//
typedef struct tagHLSEdata
{
	UINT pos;
	UINT len;
} SE_DATA, *LPSE_DATA ;

typedef enum HL_SEOpt
{
	SEO_ROLLBACK = 1 << 0,
	SEO_MODIFIED = 1 << 1
} ;
//
SE_DATA		_hl_se_array [HL_SELECT_MAX_COUNT];
UINT		_hl_se_count = 0; // total count   '


int	HLS_key_action ( int key , int msg )
{
	if ( b_HL_edit_selection ) {
		HL_TRACE ( "enter key %d on message %d , edit mode %d " , key , msg , b_HL_edit_selection );
		if ( VK_RETURN == key ) {
			if ( WM_CHAR == msg ) {
				HLS_Edit_selection_stop ( HL_SE_APPLY );
			}
			return 0;
		}
	}
	return -1;
}


void	HLS_init()
{
    SendMessage ( hwndEdit , SCI_INDICSETSTYLE , HL_SELECT_INDICATOR , IniGetInt ( HL_INI_SECTION , L"SelectionType" , 6 ) );
    SendMessage ( hwndEdit , SCI_INDICSETALPHA , HL_SELECT_INDICATOR , IniGetInt ( HL_INI_SECTION , L"SelectionAlpha" , 0 ) );
    SendMessage ( hwndEdit , SCI_INDICSETOUTLINEALPHA , HL_SELECT_INDICATOR , IniGetInt ( HL_INI_SECTION , L"SelectionLineAlpha" , 0 ) );
    SendMessage ( hwndEdit , SCI_INDICSETFORE , HL_SELECT_INDICATOR , IniGetInt ( HL_INI_SECTION , L"SelectionColor" , RGB (	0x00 , 0x00, 0x00 ) ) );
    SendMessage ( hwndEdit , SCI_INDICSETUNDER , HL_SELECT_INDICATOR , IniGetInt ( HL_INI_SECTION , L"SelectionUnder" , 0 ) );
    SendMessage ( hwndEdit , SCI_INDICSETSTYLE , HL_SELECT_INDICATOR_SINGLE , IniGetInt ( HL_INI_SECTION , L"SingleSelectionType" , 6 ) );
    SendMessage ( hwndEdit , SCI_INDICSETALPHA , HL_SELECT_INDICATOR_SINGLE , IniGetInt ( HL_INI_SECTION , L"SingleSelectionAlpha" , 0 ) );
    SendMessage ( hwndEdit , SCI_INDICSETOUTLINEALPHA , HL_SELECT_INDICATOR_SINGLE , IniGetInt ( HL_INI_SECTION , L"SingleSelectionLineAlpha" , 0 ) );
    SendMessage ( hwndEdit , SCI_INDICSETFORE , HL_SELECT_INDICATOR_SINGLE , IniGetInt ( HL_INI_SECTION , L"SingleSelectionColor" , RGB (	0x90 , 0x00,
                  0x00 ) ) );
    SendMessage ( hwndEdit , SCI_INDICSETUNDER , HL_SELECT_INDICATOR_SINGLE , IniGetInt ( HL_INI_SECTION , L"SingleSelectionUnder" , 0 ) );
    //
    SendMessage ( hwndEdit , SCI_INDICSETSTYLE , HL_SELECT_INDICATOR_EDIT , IniGetInt ( HL_INI_SECTION , L"EditSelectionType" , 7 ) );
    SendMessage ( hwndEdit , SCI_INDICSETALPHA , HL_SELECT_INDICATOR_EDIT , IniGetInt ( HL_INI_SECTION , L"EditSelectionAlpha" , 100 ) );
    SendMessage ( hwndEdit , SCI_INDICSETOUTLINEALPHA , HL_SELECT_INDICATOR_EDIT , IniGetInt ( HL_INI_SECTION , L"EditSelectionLineAlpha" , 0 ) );
    SendMessage ( hwndEdit , SCI_INDICSETFORE , HL_SELECT_INDICATOR_EDIT , IniGetInt ( HL_INI_SECTION , L"EditSelectionColor" , RGB (	0xaa , 0xaa,
                  0x00 ) ) );
    SendMessage ( hwndEdit , SCI_INDICSETUNDER , HL_SELECT_INDICATOR_EDIT , IniGetInt ( HL_INI_SECTION , L"EditSelectionUnder" , 0 ) );

	//
	hl_proc_action = HLS_key_action;
	_hl_se_tr.lpstrText = 0;
}


void HLS_release()
{
	if (_hl_se_tr.lpstrText)
	{
		free(_hl_se_tr.lpstrText);
		_hl_se_tr.lpstrText = 0;
	}
}

int HLS_get_wraps ( int beg , int end )
{
	int k = 0;
	int out = 0;
	for ( k = beg ; k < end ; ++ k ) {
		out += SendMessage ( hwndEdit , SCI_WRAPCOUNT , beg + k  , 0 ) - 1;
	}
	return out;
}



VOID HLS_Highlight_word ( LPCSTR  word )
{
    int res  = 0;
    int cnt = 0;
    int lstart , lwrap , lrange , len;
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
    ttf.chrg.cpMax  = SendMessage ( hwndEdit , SCI_GETLINEENDPOSITION , lstart + lrange, 0 ) + 1  ;
    old = SendMessage ( hwndEdit , SCI_GETINDICATORCURRENT , 0 , 0 );
    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR , 0 );
    SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , 0 , len );
    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR_EDIT , 0 );
    SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , 0 , len );
    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR_SINGLE , 0 );
    SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , 0 , len );
    if ( word ) {
        int	search_opt = SCFIND_WHOLEWORD;
		int wlen = strlen(word);
        if ( _hl_se_init ) {
            _hl_se_count = 0;
        //    strcpy ( _hl_sel_edit_prev , word );
        //    strcpy ( _hl_sel_edit_orig , word );
            search_opt = SCFIND_MATCHCASE;
        }
        // 2 first words
        ttf1.chrg.cpMin = max ( ttf.chrg.cpMin - HL_SEARCH_WORD_SIZE , 0 );
        ttf1.chrg.cpMax = min ( ttf.chrg.cpMin + HL_SEARCH_WORD_SIZE 
			, SendMessage ( hwndEdit , SCI_GETTEXTLENGTH , 0 , 0 ) );
        ttf1.lpstrText = ( LPSTR ) word;
        res =   SendMessage ( hwndEdit , SCI_FINDTEXT , search_opt , ( LPARAM ) &ttf1 );
        if ( -1 != res ) {
            ttf1.chrg.cpMin = ttf1.chrgText.cpMax;
            res =   SendMessage ( hwndEdit , SCI_FINDTEXT , search_opt , ( LPARAM ) &ttf1 );
            if ( -1 != res ) {
                if ( _hl_se_init ) {
                    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR_EDIT , 0 );
                    b_HL_edit_selection = TRUE;
                } else {
                    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR , 0 );
                }
            }
        }
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
                    //HL_TRACE ( " line %d ", line );
                    HL_TRACE ( " line__ %d (%d , %d , %d) ", line , lwrap , lstart ,  lrange );
                    if ( line + lwrap <= lrange + lstart ) {
							LPSE_DATA dt = &_hl_se_array[_hl_se_count++];
							dt->pos = ttf.chrgText.cpMin;
							dt->len = wlen;
                    } else {
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

VOID	HLS_Get_word(){
	int sel_len = 0 , cpos = 0  ;
	//
	if (_hl_se_tr.lpstrText)
	{
		free(_hl_se_tr.lpstrText);
		_hl_se_tr.lpstrText = 0;
	}
	//
	cpos = SendMessage(hwndEdit , SCI_GETCURRENTPOS , 0 , 0);
	//
	if( _hl_se_init ){
		_hl_se_tr.chrg.cpMin = SendMessage(hwndEdit , SCI_GETSELECTIONSTART , 0 , 0);
		_hl_se_tr.chrg.cpMax = SendMessage(hwndEdit , SCI_GETSELECTIONEND , 0 , 0);
		sel_len = _hl_se_tr.chrg.cpMax - _hl_se_tr.chrg.cpMin;
		//
		if(sel_len < 1){
			sel_len = 0;
		}

	}
	if(0 == sel_len){
		_hl_se_tr.chrg.cpMin = SendMessage(hwndEdit , SCI_WORDSTARTPOSITION , cpos , TRUE);
		_hl_se_tr.chrg.cpMax = SendMessage(hwndEdit , SCI_WORDENDPOSITION , cpos , TRUE);
		sel_len = _hl_se_tr.chrg.cpMax - _hl_se_tr.chrg.cpMin;
	}
	//
	if( sel_len > 1 ){
		_hl_se_tr.lpstrText = malloc(sel_len+1);
		SendMessage(hwndEdit,SCI_GETTEXTRANGE , 0 , (LPARAM)&_hl_se_tr);
	}else
	{
		_hl_se_tr.chrg.cpMin =0;
		_hl_se_tr.chrg.cpMax = 0;
	}


}

VOID HLS_Highlight_turn (  )
{
	if ( b_HL_highlight_selection ) {
        //
            HLS_Get_word();
			HL_TRACE_S(_hl_se_tr.lpstrText);
            if ( TRUE ) {
                HLS_Highlight_word ( _hl_se_tr.lpstrText );
            }
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


VOID HLS_process_changes ( UINT opt )
{
	
    struct	Sci_TextRange	tr;
    int	wlen = strlen ( _hl_se_tr.lpstrText );
    int	srchlen = 0;
    int k;
    char	*nword = 0;
    int	cpos = 0;
    int	nlen = 0;
    int	diflen = 0;
    int	delta = 0;
    BOOL	replace = FALSE;
    BOOL	need_replace ;
    BOOL	current_pos_passed = 0;
    int old_ind;
    int pos;
    int len = SendMessage ( hwndEdit , SCI_GETTEXTLENGTH , 0 , 0 );
    tr.lpstrText = 0;
    //
    pos = ( int ) SendMessage ( hwndEdit, SCI_GETCURRENTPOS, 0, 0 );
    //
    if ( opt & SEO_ROLLBACK ) {
      //  nword = _hl_sel_edit_orig;
        nlen = strlen ( nword );
    } else {
        //tr.chrg.cpMin = ;
        tr.chrg.cpMax = pos;
      //  nlen =  tr.chrg.cpMax - tr.chrg.cpMin;
      //  tr.lpstrText = malloc ( nlen + 1 );
    //    nlen = SendMessage ( hwndEdit , SCI_GETTEXTRANGE , 0 , ( LPARAM ) &tr ) ;
     //   nword = tr.lpstrText;
    }
    diflen = wlen - nlen;
    //
#ifdef _DEBUG
	HL_TRACE( "current TR '%s' (%d - %d)" , _hl_se_tr.lpstrText , _hl_se_tr.chrg.cpMin , _hl_se_tr.chrg.cpMax		);
    for ( k = 0 ; k < _hl_se_count; ++k ) {
        HL_TRACE ( "pos %d = %d (%d)" , k , _hl_se_array[k].pos , _hl_se_array[k].len );
    }
#endif
	/*
    //
    HL_TRACE ( "Process SELEDIT changes: count '%d' , old word '%s' , new word '%s' , cur pos %d , doc len %d , rollback? %d"
               , _hl_se_count
               , ( _hl_sel_edit_prev )
               , nword
               , _hl_se_index
               , len
               , rollback
             );
    old_ind = SendMessage ( hwndEdit , SCI_GETINDICATORCURRENT , 0 , 0 );
    SendMessage ( hwndEdit , SCI_SETINDICATORCURRENT , HL_SELECT_INDICATOR_EDIT , 0 );
    SendMessage ( hwndEdit , SCI_INDICATORCLEARRANGE , 0 , len );
    if ( nword ) {
        need_replace = strcmp ( _hl_sel_edit_prev , nword );
        for ( k = 0 ; k < _hl_se_count; ++k ) {
            //
            replace = need_replace;
            //
            tr.chrg.cpMin = _hl_se_array[k] + delta;
            HL_TRACE ( "MIN %d" , tr.chrg.cpMin );
            if ( !current_pos_passed
                    && tr.chrg.cpMin == _hl_se_index + delta ) { // at word
                replace = need_replace && rollback;
                _hl_se_index += delta;
                HL_TRACE ( " new Current word pos %d" , _hl_se_index );
                if ( replace ) {
                    tr.chrg.cpMax = tr.chrg.cpMin + wlen;
                    tr.lpstrText = malloc ( wlen + 1 );
                } else {
                    tr.chrg.cpMax = tr.chrg.cpMin + nlen;
                    delta -= ( diflen );
                }
                current_pos_passed = 1;
            } else {
                if ( 0 && tr.chrg.cpMin > _hl_se_index + delta && !rollback ) { // after word
                    HL_TRACE ( "correct after curpos - old: %d diff:%d" , tr.chrg.cpMin , diflen );
                    tr.chrg.cpMin -= diflen;
                }
                tr.chrg.cpMax = tr.chrg.cpMin + wlen;
                tr.lpstrText = malloc ( wlen + 1 );
            }
            //
            //
            if ( need_replace ) {
                _hl_se_array[k] = tr.chrg.cpMin;
                HL_TRACE ( " delta %d & pos %d & doc %d" , delta , _hl_se_array[k] , len );
            }
            //
            if ( replace ) {
                if ( tr.chrg.cpMax > len ) {
                    HL_TRACE ( "WARN ABORT sel edit at range (%d/%d) doc len %d  index %d", tr.chrg.cpMin , tr.chrg.cpMax , len , k );
                    break;
                }
                srchlen = SendMessage ( hwndEdit , SCI_GETTEXTRANGE , 0 , ( LPARAM ) &tr ) ;
                if ( tr.chrg.cpMin == _hl_se_index && !rollback ) { // at word
                    replace = ( 0 == strcmp ( tr.lpstrText , nword ) );
                } else {
                    replace = ( 0 == strcmp ( tr.lpstrText , _hl_sel_edit_prev ) );
                }
                HL_TRACE ( "found text at index '%d' pos '%d' : '%s' , replace: %d" , k , tr.chrg.cpMin , tr.lpstrText , replace );
            }
            //
            if ( replace ) {
                SendMessage ( hwndEdit , SCI_DELETERANGE , tr.chrg.cpMin , tr.chrg.cpMax - tr.chrg.cpMin );
                SendMessage ( hwndEdit , SCI_INSERTTEXT , tr.chrg.cpMin , ( LPARAM ) nword );
                SendMessage ( hwndEdit , SCI_INDICATORFILLRANGE , tr.chrg.cpMin , strlen ( nword ) );
                len = SendMessage ( hwndEdit , SCI_GETTEXTLENGTH , 0 , 0 );
                delta -= ( diflen );
                //
            } else {
                HL_TRACE ( "WARN - SKIP SELEDIT work at %d pos %d " , k , tr.chrg.cpMin );
                SendMessage ( hwndEdit , SCI_INDICATORFILLRANGE , tr.chrg.cpMin , tr.chrg.cpMax - tr.chrg.cpMin );
            }
            if ( tr.lpstrText ) {
                free ( tr.lpstrText );
                tr.lpstrText = 0;
            }
        }
        if ( tr.lpstrText ) {
            free ( tr.lpstrText );
            tr.lpstrText = 0;
        }
        HL_Trace ( " seledit complete	doc %d" , len );
        strcpy ( _hl_sel_edit_prev , nword );
    }
    if ( nword && !rollback ) {
        free ( nword );
    }
	*/

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
    HLS_Highlight_turn (  );
    _hl_se_init = FALSE;
    if ( b_HL_edit_selection ) {
		SendMessage( hwndEdit , SCI_SETSEL , _hl_se_tr.chrg.cpMin , _hl_se_tr.chrg.cpMax );
		SendMessage ( hwndEdit , SCI_BEGINUNDOACTION , 0, 0 );
    }
}


VOID HLS_Edit_selection_stop ( UINT mode )
{
	int pos;
    if ( b_HL_edit_selection ) {
        if ( mode & HL_SE_REJECT ) {
			HLS_process_changes(SEO_ROLLBACK);
        }
        /*
         * skip any selection
		 */
        pos = SendMessage ( hwndEdit , SCI_GETCURRENTPOS , 0 , 0 );
        SendMessage ( hwndEdit , SCI_SETANCHOR , pos , 0);
        b_HL_edit_selection = FALSE;
        //
        HLS_Highlight_turn (  );
        SendMessage ( hwndEdit , SCI_ENDUNDOACTION , 0, 0 );
    }
    _hl_se_init = FALSE;
}

void HLS_Update_selection( UINT place )
{
	HL_TRACE( "reason (%d) .init SE %d , SE %d , HS %d" , place
		, _hl_se_init  ,b_HL_edit_selection , b_HL_highlight_selection );
	if(b_HL_edit_selection){
		UINT opt = 0;
		if( SH_MODIF == place ){
			opt |= SEO_MODIFIED;
		}
		HLS_process_changes(opt);
	}else{
		HLS_Highlight_turn();
	}
}
