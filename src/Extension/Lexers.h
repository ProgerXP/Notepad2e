#pragma once
#include <WTypes.h>
#include "SciLexer.h"
#include "Scintilla.h"

#define MULTI_STYLE(a,b,c,d) ((a)|(b<<8)|(c<<16)|(d<<24)|(ULONG64)0)
#define MULTI_STYLE2(a,b,c,d,e,f,g,h) ((a)|(b<<8)|(c<<16)|(d<<24)|((ULONG64)e<<32)|((ULONG64)f<<40)|((ULONG64)g<<48)|((ULONG64)h<<56))
#define MULTI_STYLE_STYLEX(s,x) ((s >> x) & 0xFF)
#define MULTI_STYLE_STYLE1(s) MULTI_STYLE_STYLEX(s,0)
#define MULTI_STYLE_STYLE2(s) MULTI_STYLE_STYLEX(s,8)
#define MULTI_STYLE_STYLE3(s) MULTI_STYLE_STYLEX(s,16)
#define MULTI_STYLE_STYLE4(s) MULTI_STYLE_STYLEX(s,24)
#define MULTI_STYLE_STYLE5(s) MULTI_STYLE_STYLEX(s,32)
#define MULTI_STYLE_STYLE6(s) MULTI_STYLE_STYLEX(s,40)
#define MULTI_STYLE_STYLE7(s) MULTI_STYLE_STYLEX(s,48)
#define MULTI_STYLE_STYLE8(s) MULTI_STYLE_STYLEX(s,56)

#define NULL_COMMENT   FALSE, "", "", "", L"", L"", L""
#define ASM_COMMENT    TRUE, ";", "", "", L";", L"", L""
#define BASH_COMMENT   TRUE, "#", "", "", L"#", L"", L""
#define BATCH_COMMENT  TRUE, "rem", "", "", L"rem", L"", L""
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

typedef enum
{
  DLO_MARGIN_AND_LINE_COLOR = 1,
  DLO_CURRENT_LINE_BACKGROUND = 8,
  DLO_CURRENT_LINE_BACKGROUND_INACTIVE = 9,
  DLO_CARET_COLOR = 10,
  DLO_LONG_LINE_MARKER = 11,
  DLO_FIND_MARKER = 12,
  DLO_FIND_MARKER_FIRST_LAST = 13,
  DLO_EXTRA_LINE_SPACING = 14,
  DLO_SPLITTER_COLOR = 15,
  DLO_2ND_DEFAULT_STYLE = 16
} EDefaultLexerOptions;

typedef struct _editstyle
{
  union
  {
    INT64 i64Style;
    UINT8 iStyle8[8];
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
