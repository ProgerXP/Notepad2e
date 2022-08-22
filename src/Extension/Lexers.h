#pragma once
#include <WTypes.h>
#include "SciLexer.h"
#include "Scintilla.h"

#define MULTI_STYLE(a,b,c,d) ((a)|(b<<8)|(c<<16)|(d<<24))
#define MULTI_STYLE_STYLE1(s) (s & 0xFF)
#define MULTI_STYLE_STYLE2(s) ((s >> 8) & 0xFF)
#define MULTI_STYLE_STYLE3(s) ((s >> 16) & 0xFF)
#define MULTI_STYLE_STYLE4(s) ((s >> 24) & 0xFF)

#define NULL_COMMENT   FALSE, "", "", "", L"", L"", L""
#define ASM_COMMENT    TRUE, ";", "", "", L";", L"", L""
#define BASH_COMMENT   TRUE, "#", "", "", L"#", L"", L""
#define BATCH_COMMENT  TRUE, "rem ", "", "", L"rem ", L"", L""
// using of C-style comments for HTML/XML/CSS lexers
// was implemented as a part of "CSS syntax scheme improvements #4"
#define C_COMMENT      FALSE, "//", "/*", "*/", L"//", L"/*", L"*/"
#define DIFF_COMMENT   TRUE, " ", "", "", L" ", L"", L""
#define LUA_COMMENT    TRUE, "--", "--[[", "]]", L"--", L"--[[", L"]]"
#define PASCAL_COMMENT FALSE, "//", "{", "}", L"//", L"{", L"}"
#define SQL_COMMENT    TRUE, "--", "", "", L"--", L"", L""
#define VB_COMMENT     FALSE, "'", "", "", L"'", L"", L""

#define COMMENT_INFO(info, style) { info, style }
#define NULL_COMMENT_INFO COMMENT_INFO(NULL_COMMENT, -1)

typedef struct _editstyle
{
  union
  {
    INT32 iStyle;
    UINT8 iStyle8[4];
  };
  int rid;
  WCHAR* pszName;
  WCHAR* pszDefault;
  WCHAR  szValue[128];

} EDITSTYLE, *PEDITSTYLE;


typedef struct _keywordlist
{
  char *pszKeyWords[9];

} KEYWORDLIST, *PKEYWORDLIST;

typedef struct _commentinfo
{
  BOOL    bInsertLineCommentAtLineStart;
  LPCSTR  pszLineComment;
  LPCSTR  pszStreamStart;
  LPCSTR  pszStreamEnd;
  LPCWSTR pszLineCommentW;
  LPCWSTR pszStreamStartW;
  LPCWSTR pszStreamEndW;
  int     iLineStyle;      // lexer style id for single line comment
} COMMENTINFO;

#pragma warning( push )
#pragma warning( disable : 4200 ) // nonstandard extension used: zero-sized array in struct/union

typedef struct _editlexer
{
  int iLexer;
  int rid;
  WCHAR* pszName;
  WCHAR* pszDefExt;
  WCHAR  szExtensions[128];
  PKEYWORDLIST pKeyWords;
  const COMMENTINFO commentInfo;
  EDITSTYLE    Styles[];

} EDITLEXER, *PEDITLEXER;

#pragma warning( pop )

// Number of Lexers in pLexArray
#ifdef LPEG_LEXER
#define NUMLEXERS 38
#else
#define NUMLEXERS 37
#endif

extern EDITLEXER lexDefault;
extern EDITLEXER lexHTML;
extern EDITLEXER lexPL;
extern EDITLEXER lexPY;
extern EDITLEXER lexMAK;
extern EDITLEXER lexXML;
extern PEDITLEXER pLexArray[NUMLEXERS];
extern PEDITLEXER pLexCurrent;
