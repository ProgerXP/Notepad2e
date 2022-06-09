#include "Lexers.h"
#include "LexerUtils.h"
#include "Externals.h"
#include "SciLexer.h"

const COMMENTINFO COMMENT_INFO_NULL = COMMENT_INFO(NULL_COMMENT, -1);

const COMMENTINFO* n2e_GetCommentInfo(const int iLexer)
{
  for (int i = 0; i < NUMLEXERS; i++)
  {
    if (pLexArray[i]->iLexer == iLexer)
    {
      return &pLexArray[i]->commentInfo;
    }
  }
  return &COMMENT_INFO_NULL;
}

LPCSTR n2e_GetSingleLineCommentPrefix(const int iLexer)
{
  return n2e_GetCommentInfo(iLexer)->pszLineComment;
}

LPCSTR n2e_GetCurrentSingleLineCommentPrefix()
{
  return n2e_GetCommentInfo(pLexCurrent->iLexer)->pszLineComment;
}

int n2e_GetSingleLineCommentPrefixLength(const int iLexer)
{
  return strlen(n2e_GetSingleLineCommentPrefix(iLexer));
}

BOOL n2e_IsSingleLineCommentStyle(const int iLexer, const int iStyle)
{
  const int iLineStyle = n2e_GetCommentInfo(iLexer)->iLineStyle;
  return (iLineStyle == iStyle)
    || (MULTI_STYLE_STYLE1(iLineStyle) == iStyle)
    || (MULTI_STYLE_STYLE2(iLineStyle) == iStyle)
    || (MULTI_STYLE_STYLE3(iLineStyle) == iStyle)
    || (MULTI_STYLE_STYLE4(iLineStyle) == iStyle);
}
