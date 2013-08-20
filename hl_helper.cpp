#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <string>
#include <Shlobj.h>
#include <algorithm>


extern "C"
{
    extern UINT		_hl_ctx_menu_type ;

#if 0
    BOOL HL_Explorer_cxt_menu ( LPCWSTR path, void *parentWindow )
    {
        ITEMIDLIST *id = 0;
        std::wstring windowsPath = path;
        std::replace ( windowsPath.begin(), windowsPath.end(), '/', '\\' );
        HRESULT result = SHParseDisplayName ( windowsPath.c_str(), 0, &id, 0, 0 );
        if ( !SUCCEEDED ( result ) || !id ) {
            return false;
        }
        bool out = true;
        IShellFolder *ifolder = 0;
        LPCITEMIDLIST idChild = 0;
        result = SHBindToParent ( id, IID_IShellFolder, ( void ** ) &ifolder, &idChild );
        if ( !SUCCEEDED ( result ) || !ifolder ) {
            out = false;
        }
        if ( out ) {
            IContextMenu2 *imenu = 0;
            result = ifolder->GetUIObjectOf ( ( HWND ) parentWindow
                                              , 1
                                              , ( const ITEMIDLIST ** ) &idChild
                                              , IID_IContextMenu
                                              , 0
                                              , ( void ** ) &imenu );
            if ( !SUCCEEDED ( result ) || !ifolder ) {
                out = false;
            }
            if ( out ) {
                HMENU hMenu = CreatePopupMenu();
                if ( !hMenu ) {
                    out = false;
                }
                if ( out && SUCCEEDED ( imenu->QueryContextMenu ( hMenu, 0, 1, 0x7FFF, _hl_ctx_menu_type	) ) ) {
                    POINT pt ;
                    GetCursorPos ( &pt );
                    int iCmd = TrackPopupMenuEx ( hMenu, TPM_RETURNCMD, pt.x, pt.y, ( HWND ) parentWindow, NULL );
                    if ( iCmd > 0 ) {
                        CMINVOKECOMMANDINFOEX info = { 0 };
                        info.cbSize = sizeof ( info );
                        info.fMask = CMIC_MASK_UNICODE;
                        info.hwnd = ( HWND ) parentWindow;
                        info.lpVerb  = MAKEINTRESOURCEA ( iCmd - 1 );
                        info.lpVerbW = MAKEINTRESOURCEW ( iCmd - 1 );
                        info.nShow = SW_SHOWNORMAL;
                        imenu->InvokeCommand ( ( LPCMINVOKECOMMANDINFO ) &info );
                    }
                }
                if ( hMenu ) {
                    DestroyMenu ( hMenu );
                }
            }
            if ( imenu ) {
                imenu->Release();
            }
        }
        if ( ifolder ) {
            ifolder->Release();
        }
        return out;
    }
#else
    LPCONTEXTMENU2	g_IContext2 = NULL;
    LPCONTEXTMENU3	g_IContext3 = NULL;
    LRESULT CALLBACK HookWndProc ( HWND hWnd, UINT message,
                                   WPARAM wParam, LPARAM lParam )
    {
        switch ( message ) {
            case WM_MENUCHAR:	// only supported by IContextMenu3
                if ( g_IContext3 ) {
                    LRESULT lResult = 0;
                    g_IContext3->HandleMenuMsg2 ( message, wParam, lParam, &lResult );
                    return ( lResult );
                }
                break;
            case WM_DRAWITEM:
            case WM_MEASUREITEM:
                if ( wParam ) {
                    break;    // if wParam != 0 then the message is not menu-related
                }
            case WM_INITMENUPOPUP:
                if ( g_IContext2 ) {
                    g_IContext2->HandleMenuMsg ( message, wParam, lParam );
                } else {	// version 3
                    g_IContext3->HandleMenuMsg ( message, wParam, lParam );
                }
                return ( message == WM_INITMENUPOPUP ? 0 : TRUE ); // inform caller that
                // we handled WM_INITPOPUPMENU by ourself
                break;
            default:
                break;
        }
        // call original WndProc of window to prevent undefined bevhaviour
        // of window
        return ::CallWindowProc ( ( WNDPROC ) GetProp ( hWnd, TEXT ( "OldWndProc" ) ),
                                  hWnd, message, wParam, lParam );
    }
    BOOL GetContextMenu ( LPCWSTR path, void **ppContextMenu, int &iMenuType )
    {
        *ppContextMenu = NULL;
        LPCONTEXTMENU icm1 = NULL;
        // first we retrieve the normal IContextMenu
        // interface (every object should have it)
        LPCITEMIDLIST idChild = 0;
        IShellFolder *ifolder = 0;
        ITEMIDLIST *id = 0;
        std::wstring windowsPath = path;
        std::replace ( windowsPath.begin(), windowsPath.end(), '/', '\\' );
        HRESULT result = SHParseDisplayName ( windowsPath.c_str(), 0, &id, 0, 0 );
        if ( !SUCCEEDED ( result ) || !id ) {
            return false;
        }
        result = SHBindToParent ( id, IID_IShellFolder, ( void ** ) &ifolder, &idChild );
        ifolder->GetUIObjectOf ( NULL,
                                 1,
                                 ( LPCITEMIDLIST * ) &idChild,
                                 IID_IContextMenu,
                                 NULL,
                                 ( void ** ) &icm1 );
        if ( icm1 ) {
            // since we got an IContextMenu interface we can
            // now obtain the higher version interfaces via that
            if ( icm1->QueryInterface ( IID_IContextMenu3, ppContextMenu ) == NOERROR ) {
                iMenuType = 3;
            } else if ( icm1->QueryInterface ( IID_IContextMenu2,
                                               ppContextMenu ) == NOERROR ) {
                iMenuType = 2;
            }
            if ( *ppContextMenu ) {
                icm1->Release();    // we can now release version 1 interface,
            }
            // cause we got a higher one
            else {
                iMenuType = 1;
                *ppContextMenu = icm1;    // since no higher versions were found
            }  // redirect ppContextMenu to version 1 interface
        } else {
            return ( FALSE );    // something went wrong
        }
        return ( TRUE ); // success
    }

    //UINT ShowContextMenu(CWnd *pWnd, CPoint pt)
    BOOL HL_Explorer_cxt_menu ( LPCWSTR path, void *parentWindow )
    {
        int iMenuType = 0;
        // to know which version of IContextMenu is supported
        LPCONTEXTMENU pContextMenu;
        // common pointer to IContextMenu and higher version interface
        if ( !GetContextMenu ( path , ( void ** ) &pContextMenu, iMenuType ) ) {
            return FALSE;    // something went wrong
        }
        //
        HMENU	h_menu = CreatePopupMenu();
        // lets fill the our popupmenu
        pContextMenu->QueryContextMenu ( h_menu ,
                                         0, 1, 0x7FFF, _hl_ctx_menu_type );
        WNDPROC OldWndProc = NULL;
#if WINVER==0x0501
        if ( iMenuType > 1 ) { // only version 2 and 3 supports menu messages
            OldWndProc = ( WNDPROC ) SetWindowLong ((HWND) parentWindow,
                         GWL_WNDPROC, ( DWORD ) HookWndProc );
            if ( iMenuType == 2 ) {
                g_IContext2 = ( LPCONTEXTMENU2 ) pContextMenu;
            } else {	// version 3
                g_IContext3 = ( LPCONTEXTMENU3 ) pContextMenu;
            }
        } else {
            OldWndProc = NULL;
        }
#endif
        POINT pt ;
        GetCursorPos ( &pt );
        int iCmd = TrackPopupMenuEx ( h_menu, TPM_RETURNCMD, pt.x, pt.y, ( HWND ) parentWindow, NULL );
        if ( OldWndProc ) {
            SetWindowLong ((HWND) parentWindow, GWL_WNDPROC, ( DWORD ) OldWndProc );
        }
        pContextMenu->Release();
        return ( TRUE );
    }
#endif
}
