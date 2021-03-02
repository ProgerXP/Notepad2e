/******************************************************************************
*
*
* Notepad2
*
* version.h
*   Notepad2 version information
*
* See Readme.txt for more information about this source code.
* Please send me your comments to this work.
*
* See License.txt for details about distribution and modification.
*
*                                              (c) Florian Balmer 1996-2011
*                                                  florian.balmer@gmail.com
*                                               http://www.flos-freeware.ch
*
*
******************************************************************************/


#define STRINGIFY(x) L#x
#define TOSTRING(x) STRINGIFY(x)

#define VERSION_FILEVERSION_BUILD    0
#define VERSION_FILEVERSION_BUILD_STRING  TOSTRING(VERSION_FILEVERSION_BUILD)
#define VERSION_FILEVERSION_NUM      1,VERSION_FILEVERSION_BUILD
#define VERSION_FILEVERSION_SHORT    L"1." VERSION_FILEVERSION_BUILD_STRING
#define VERSION_LEGALCOPYRIGHT_SHORT L"Copyright © 2004-2011"
#define VERSION_LEGALCOPYRIGHT_LONG  L"© Florian Balmer and contributors"
#if defined(ICU_BUILD) && defined(LPEG_LEXER)
#define VERSION_FILEDESCRIPTION_BASE L"Notepad 2e-ICU-LPeg"
#elif ICU_BUILD
#define VERSION_FILEDESCRIPTION_BASE L"Notepad 2e-ICU"
#elif LPEG_LEXER
#define VERSION_FILEDESCRIPTION_BASE L"Notepad 2e-LPeg"
#else
#define VERSION_FILEDESCRIPTION_BASE L"Notepad 2e"
#endif
#ifdef _WIN64
#define VERSION_FILEDESCRIPTION      VERSION_FILEDESCRIPTION_BASE L" x64"
#else
#define VERSION_FILEDESCRIPTION      VERSION_FILEDESCRIPTION_BASE
#endif
#if defined(ICU_BUILD) && defined(LPEG_LEXER)
#define VERSION_INTERNALNAME         L"Notepad2e-ICU-LPeg"
#define VERSION_ORIGINALFILENAME     L"Notepad2eil.exe"
#elif ICU_BUILD
#define VERSION_INTERNALNAME         L"Notepad2e-ICU"
#define VERSION_ORIGINALFILENAME     L"Notepad2ei.exe"
#elif LPEG_LEXER
#define VERSION_INTERNALNAME         L"Notepad2e-LPeg"
#define VERSION_ORIGINALFILENAME     L"Notepad2el.exe"
#else
#define VERSION_INTERNALNAME         L"Notepad2e"
#define VERSION_ORIGINALFILENAME     L"Notepad2e.exe"
#endif
#define VERSION_AUTHORNAME           L"Florian Balmer"
#define VERSION_WEBPAGEDISPLAY       L"flo's freeware - http://www.flos-freeware.ch"
#define VERSION_EMAILDISPLAY         L"florian.balmer@gmail.com"
#define VERSION_EXT_VERSION          L"Extended Edition © 2013-2021"
#define VERSION_EXT_BY               L"By Proger_XP and contributors"
#define VERSION_EXT_PAGE             L"https://github.com/ProgerXP/Notepad2e"
#define VERSION_COMMIT               L"6c3f5ac"
