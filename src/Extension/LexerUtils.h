#pragma once
#include "Lexers.h"

const COMMENTINFO* n2e_GetCommentInfo();
const COMMENTINFO* n2e_GetCommentInfo(const int iLexer);
LPCSTR n2e_GetSingleLineCommentPrefix(const int iLexer);
LPCSTR n2e_GetCurrentSingleLineCommentPrefix();
int n2e_GetSingleLineCommentPrefixLength(const int iLexer);
BOOL n2e_IsSingleLineCommentStyle(const int iLexer, const int iStyle);
