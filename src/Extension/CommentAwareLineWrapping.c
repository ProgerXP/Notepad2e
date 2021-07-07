#include <assert.h>
#include <wtypes.h>
#include <Shlwapi.h>
#include "Externals.h"
#include "CommentAwareLineWrapping.h"
#include "LexerUtils.h"
#include "SciLexer.h"
#include "StringRecoding.h"


BOOL CALW_IsValidSequence(EncodingData* pED, const int requiredChars)
{
  return TRUE;
}

struct TPrefixData
{
  char data[MAX_PATH];
  BOOL isInitialized;
  BOOL isComment;
};

typedef struct TPrefixData PrefixData;

void PrefixData_SetEmpty(PrefixData *pd)
{
  pd->data[0] = 0;
}

BOOL PrefixData_IsInitialized(PrefixData *pd)
{
  return pd->isInitialized;
}

void PrefixData_SetInitialized(PrefixData *pd, BOOL isInitialized)
{
  pd->isInitialized = isInitialized;
}

BOOL PrefixData_IsComment(PrefixData *pd)
{
  return pd->isComment;
}

void PrefixData_SetComment(PrefixData *pd, BOOL isComment)
{
  pd->isComment = isComment;
}

int PrefixData_GetLength(PrefixData *pd)
{
  return strlen(pd->data);
}

BOOL PrefixData_IsEmpty(PrefixData *pd)
{
  return PrefixData_GetLength(pd) == 0;
}

void PrefixData_PushChar(PrefixData *pd, const unsigned char ch)
{
  const int pos = PrefixData_GetLength(pd);
  pd->data[pos] = ch;
  pd->data[pos + 1] = 0;
}

const unsigned char PrefixData_GetChar(PrefixData *pd, const int pos)
{
  return pd->data[pos];
}

struct TCALWData
{
  int longLineLimit;
  int lexerId;
  int relativeLineIndex;
  int relativeLineIndexPrefixProcessed;
  BOOL previosLineUseMarker;
  
  PrefixData prefixFirstLine;
  PrefixData prefixMarkerLine;

  BOOL initLine;
  BOOL skipNextEOL;
  int iLineOffset;
  int iWordCount;
  int iSingleLineCommentPrefixLength;
  int iEOLMode;
  int iTrailingEOLLength;
};

typedef struct TCALWData CALWData;

static CALWData calwdata = { 0 };

inline BOOL GetTrailingEOLLength()
{
  switch (calwdata.iEOLMode)
  {
  case SC_EOL_CRLF:
    return 1;
  case SC_EOL_LF:
  case SC_EOL_CR:
    return 0;
  }
  assert(0);
  return 0;
}

LPVOID CALW_InitAlgorithmData(const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3)
{
  ZeroMemory(&calwdata, sizeof(calwdata));
  calwdata.longLineLimit = iAdditionalData1;
  calwdata.lexerId = iAdditionalData2;
  calwdata.iSingleLineCommentPrefixLength = n2e_GetSingleLineCommentPrefixLength(calwdata.lexerId);
  calwdata.iEOLMode = iAdditionalData3;
  calwdata.iTrailingEOLLength = GetTrailingEOLLength();
  return (LPVOID)&calwdata;
}

void CALW_ReleaseAlgorithmData(LPVOID pData)
{
}

static unsigned char CHAR_SPACE = ' ';
static unsigned char CHAR_EOL_R = '\r';
static unsigned char CHAR_EOL_N = '\n';
static LPCSTR lpstrWhiteSpaces = " \t";
static LPCSTR lpstrStaticMarkerChars = "#>=?*";
static LPCSTR lpstrDynamicMarkerChars = ":).";
static LPCSTR lpstrDigits = "0123456789";

inline BOOL IsEOLChar(const unsigned char ch)
{
  return (ch == CHAR_EOL_R) || (ch == CHAR_EOL_N);
}

inline BOOL IsEOL(const unsigned char ch)
{
  switch (calwdata.iEOLMode)
  {
  case SC_EOL_CRLF:
    return ch == CHAR_EOL_N;
  case SC_EOL_LF:
    return ch == CHAR_EOL_N;
  case SC_EOL_CR:
    return ch == CHAR_EOL_R;
  }
  assert(0);
  return FALSE;
}

inline BOOL IsTrailingEOL(const unsigned char ch, TextBuffer* pTB)
{
  switch (calwdata.iEOLMode)
  {
  case SC_EOL_CRLF:
    return (ch == CHAR_EOL_R) && IsEOL(TextBuffer_GetChar(pTB));
  case SC_EOL_LF:
    return ch == CHAR_EOL_N;
  case SC_EOL_CR:
    return ch == CHAR_EOL_R;
  }
  assert(0);
  return FALSE;
}

inline BOOL isMarker(const unsigned char ch, EncodingData* pED)
{
  const BOOL isStaticMarker = strchr(lpstrStaticMarkerChars, ch) != NULL;
  const BOOL isDynamicMarker = TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, 0);
  return isStaticMarker || isDynamicMarker;
}

// remove EOLs/front spaces
BOOL CALW_Encode_Pass1(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  BOOL bSkipChars = FALSE;
  BOOL bSkipInitLineCheck = FALSE;
  const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
  int iCharCount = 1 + (!IsTrailingEOL(ch, &pED->m_tb) ? TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0) : 0);
  int iCharsProcessed = 0;
  if ((calwdata.relativeLineIndex == 0) && !PrefixData_IsInitialized(&calwdata.prefixFirstLine))
  {
    const BOOL isWhiteSpace = (strchr(lpstrWhiteSpaces, ch) != NULL);
    const int iCommentOffset = iCharCount - 1 + (isWhiteSpace ? calwdata.iSingleLineCommentPrefixLength : 0);
    const BOOL isSingleLineComment = n2e_IsSingleLineCommentStyleAtPos(NULL, calwdata.lexerId, iCommentOffset, pED);
    PrefixData_SetComment(&calwdata.prefixFirstLine, isSingleLineComment);
    if (isSingleLineComment)
    {
      const int iWhiteSpacesAfterComment = TextBuffer_CountWhiteSpaces(&pED->m_tb, iCommentOffset);
      PrefixData_SetEmpty(&calwdata.prefixFirstLine);
      PrefixData_PushChar(&calwdata.prefixFirstLine, ch);
      for (int i = 0; i < iCommentOffset + iWhiteSpacesAfterComment; ++i)
      {
        PrefixData_PushChar(&calwdata.prefixFirstLine, TextBuffer_PopChar(&pED->m_tb));
      }
      iCharCount = PrefixData_GetLength(&calwdata.prefixFirstLine);
      bSkipChars = TRUE;
    }
    else if (isWhiteSpace)
    {
      for (int i = 0; i < iCharCount; ++i)
      {
        PrefixData_PushChar(&calwdata.prefixFirstLine, ch);
      }
      TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
      bSkipChars = TRUE;
    }
    PrefixData_SetInitialized(&calwdata.prefixFirstLine, TRUE);
  }
  else if ((calwdata.relativeLineIndex > 0) 
    && (calwdata.relativeLineIndex > calwdata.relativeLineIndexPrefixProcessed))
  {
    if (PrefixData_IsComment(&calwdata.prefixFirstLine))
    {
      if (calwdata.iTrailingEOLLength == 0)
      {
        TextBuffer_OffsetPos(&pED->m_tb, -1);
        --(*piCharsProcessed);
      }
      const int iWhiteSpaces = TextBuffer_CountWhiteSpaces(&pED->m_tb, 0);
      const BOOL isSingleLineComment = 
        n2e_IsSingleLineCommentStyleAtPos(NULL, calwdata.lexerId, iWhiteSpaces + calwdata.iSingleLineCommentPrefixLength, pED);
      if (isSingleLineComment)
      {
        iCharCount = iWhiteSpaces + 1 + calwdata.iSingleLineCommentPrefixLength;
        TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
        bSkipChars = TRUE;
        bSkipInitLineCheck = TRUE;
        calwdata.relativeLineIndexPrefixProcessed = calwdata.relativeLineIndex;
      }
    }
    else if ((PrefixData_GetLength(&calwdata.prefixFirstLine) > 0)
      && (ch == CHAR_SPACE) && (iCharCount >= PrefixData_GetLength(&calwdata.prefixFirstLine)))
    {
      TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
      bSkipChars = TRUE;
    }
  }

  if (IsEOLChar(ch) || bSkipInitLineCheck)
  {
    if (!calwdata.initLine)
    {
      ++calwdata.relativeLineIndex;

      const BOOL isEOLAtPosition = (calwdata.iTrailingEOLLength == 0) ? TRUE : IsEOLChar(TextBuffer_GetChar(&pED->m_tb));
      const int spacesAfterPosition = TextBuffer_GetCharSequenceLength(&pED->m_tb, CHAR_SPACE, calwdata.iTrailingEOLLength);
      int iLineLength = 0;
      const BOOL isWhiteSpaceLine = TextBuffer_IsWhiteSpaceLine(&pED->m_tb, calwdata.iTrailingEOLLength, &iLineLength);

      calwdata.initLine = !calwdata.skipNextEOL &&
        (((PrefixData_GetLength(&calwdata.prefixFirstLine) == 0) && !isWhiteSpaceLine) || isEOLAtPosition);

      if (isWhiteSpaceLine)
      {
        TextBuffer_PushChar(&pED->m_tbRes, ch); 
        int count = 0;
        if (calwdata.iEOLMode == SC_EOL_CRLF)
        {
          TextBuffer_PushChar(&pED->m_tbRes, TextBuffer_GetChar(&pED->m_tb));
          count += 1;
        }
        count += iLineLength;
        TextBuffer_OffsetPos(&pED->m_tb, count);
        iCharCount += count;
        calwdata.initLine = FALSE;
        calwdata.skipNextEOL = TRUE;
        bSkipChars = TRUE;
      }
      else if (calwdata.initLine)
      {
        const BOOL commentAtPosition = n2e_IsSingleLineCommentStyleAtPos(NULL, calwdata.lexerId, calwdata.iTrailingEOLLength + spacesAfterPosition + calwdata.iSingleLineCommentPrefixLength, pED);
        const int markerOffset = calwdata.iTrailingEOLLength + spacesAfterPosition + (commentAtPosition ? calwdata.iSingleLineCommentPrefixLength : 0);
        if (isEOLAtPosition
          && TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrStaticMarkerChars, lpstrWhiteSpaces, markerOffset))
        {
          TextBuffer_PushChar(&pED->m_tbRes, ch);
          if (calwdata.iEOLMode == SC_EOL_CRLF)
          {
            TextBuffer_PushChar(&pED->m_tbRes, TextBuffer_GetChar(&pED->m_tb));
          }
          calwdata.relativeLineIndexPrefixProcessed = calwdata.relativeLineIndex;
          calwdata.initLine = FALSE;
          if (!calwdata.previosLineUseMarker && !commentAtPosition)
          {
            calwdata.previosLineUseMarker = TRUE;
            iCharCount += calwdata.iTrailingEOLLength;
            TextBuffer_OffsetPos(&pED->m_tb, calwdata.iTrailingEOLLength);
          }
          else
          {
            iCharCount += markerOffset;
            TextBuffer_OffsetPos(&pED->m_tb, markerOffset);
          }
          bSkipChars = TRUE;
        }
        else if ((pED->m_tbRes.m_iPos > 0)
          && (TextBuffer_GetCharAt(&pED->m_tbRes, -calwdata.iTrailingEOLLength) != CHAR_SPACE))
        {
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_SPACE);
        }
      }
      if (!isWhiteSpaceLine && IsEOL(ch))
      {
        calwdata.skipNextEOL = FALSE;
        calwdata.previosLineUseMarker = FALSE;
      }
    }
  }
  else if (calwdata.initLine)
  {
    calwdata.initLine &= (ch == CHAR_SPACE);
  }
  else if (isMarker(ch, pED))
  {
    calwdata.previosLineUseMarker = TRUE;
  }
  
  if (!bSkipChars)
  {
    for (int i = 0; i < iCharCount; ++i)
    {
      if (!calwdata.initLine)
      {
        TextBuffer_PushChar(&pED->m_tbRes, ch);
      }
    }
    TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
  }
  iCharsProcessed += iCharCount;
  if (piCharsProcessed)
  {
    (*piCharsProcessed) += iCharsProcessed;
  }
  return TRUE;
}

BOOL CALW_Encode_Pass2(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  BOOL bPrefixInitialized = FALSE;
  auto prefixLength = PrefixData_GetLength(&calwdata.prefixFirstLine);
  if ((calwdata.iLineOffset == 0) && (prefixLength > 0) && !PrefixData_IsInitialized(&calwdata.prefixMarkerLine))
  {
    for (int i = 0; i < prefixLength; ++i)
    {
      TextBuffer_PushChar(&pED->m_tbRes, PrefixData_GetChar(&calwdata.prefixFirstLine, i));
      ++calwdata.iLineOffset;
    }
  }
  prefixLength = PrefixData_IsComment(&calwdata.prefixFirstLine) ? prefixLength : 0;
  prefixLength = PrefixData_IsInitialized(&calwdata.prefixMarkerLine) ? PrefixData_GetLength(&calwdata.prefixMarkerLine) : prefixLength;

  int iCharsProcessed = 0;
  int iWordByteCount = 0;
  int iWordLength = TextBuffer_GetWordLength(&pED->m_tb, iEncoding, &iWordByteCount);
  if ((iWordLength >= calwdata.longLineLimit)
      || (calwdata.iWordCount == 0)
      || (calwdata.iLineOffset + iWordLength <= prefixLength + calwdata.longLineLimit))
  {
    if (PrefixData_IsInitialized(&calwdata.prefixMarkerLine)
      && ((pED->m_tbRes.m_iPos > 0) && IsEOL(TextBuffer_GetCharAt(&pED->m_tbRes, -1))))
    {
      const int markerPrefixLength = PrefixData_GetLength(&calwdata.prefixMarkerLine);
      if (markerPrefixLength > 0)
      {
        for (int i = 0; i < markerPrefixLength; ++i)
        {
          TextBuffer_PushChar(&pED->m_tbRes, PrefixData_GetChar(&calwdata.prefixMarkerLine, i));
          ++calwdata.iLineOffset;
        }
      }
    }
    for (int i = 1; i <= iWordByteCount; ++i)
    {
      const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
      const BOOL isStaticMarker = strchr(lpstrStaticMarkerChars, ch) != NULL;
      const BOOL isDynamicMarker = TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, 0);
      if ((iWordLength >= 1)
        && (isStaticMarker || isDynamicMarker)
        && ((PrefixData_GetLength(&calwdata.prefixFirstLine) == 0)
          || (calwdata.iLineOffset == PrefixData_GetLength(&calwdata.prefixFirstLine))))
      {
        TextBuffer_PushChar(&pED->m_tbRes, ch);
        ++iCharsProcessed;
        ++calwdata.iLineOffset;

        PrefixData_SetEmpty(&calwdata.prefixMarkerLine);
        if (PrefixData_GetLength(&calwdata.prefixFirstLine) > 0)
        {
          for (int j = 0; j < PrefixData_GetLength(&calwdata.prefixFirstLine); ++j)
          {
            PrefixData_PushChar(&calwdata.prefixMarkerLine, PrefixData_GetChar(&calwdata.prefixFirstLine, j));
          }
        }
        else
        {
          for (int j = 0; j < calwdata.iLineOffset - 1; ++j)
          {
            PrefixData_PushChar(&calwdata.prefixMarkerLine, TextBuffer_GetCharAt(&pED->m_tb, j - calwdata.iLineOffset));
          }
        }
        PrefixData_PushChar(&calwdata.prefixMarkerLine, CHAR_SPACE);

        if (isDynamicMarker)
        {
          while (strchr(lpstrDigits, TextBuffer_GetChar(&pED->m_tb))
                || strchr(lpstrDynamicMarkerChars, TextBuffer_GetChar(&pED->m_tb)))
          {
            const unsigned char ch1 = TextBuffer_PopChar(&pED->m_tb);
            TextBuffer_PushChar(&pED->m_tbRes, ch1);
            PrefixData_PushChar(&calwdata.prefixMarkerLine, CHAR_SPACE); // space chars under NNN)
            ++iCharsProcessed;
            ++calwdata.iLineOffset;
          }
        }

        const unsigned char chNext = TextBuffer_GetChar(&pED->m_tb);
        if (strchr(lpstrWhiteSpaces, chNext) != 0)
        {
          const int markerPostfixLength = TextBuffer_GetCharSequenceLength(&pED->m_tb, chNext, 0);
          for (int i = 0; i < markerPostfixLength; ++i)
          {
            PrefixData_PushChar(&calwdata.prefixMarkerLine, chNext);
            TextBuffer_PushChar(&pED->m_tbRes, chNext);
            ++iCharsProcessed;
            ++calwdata.iLineOffset;
          }
          TextBuffer_OffsetPos(&pED->m_tb, markerPostfixLength);
        }
        PrefixData_SetInitialized(&calwdata.prefixMarkerLine, TRUE);
        bPrefixInitialized = TRUE;
        break;
      }
      TextBuffer_PushChar(&pED->m_tbRes, ch);
      ++iCharsProcessed;
      ++calwdata.iLineOffset;
      if (IsEOLChar(ch))
      {
        if (IsTrailingEOL(ch, &pED->m_tb))
        {
          if (calwdata.iEOLMode == SC_EOL_CRLF)
          {
            TextBuffer_PushChar(&pED->m_tbRes, TextBuffer_PopChar(&pED->m_tb));
            ++iCharsProcessed;
          }
        }

        calwdata.iLineOffset = 0;
        calwdata.iWordCount = 0;
        PrefixData_SetInitialized(&calwdata.prefixMarkerLine, FALSE);
      }
    }
    calwdata.iWordCount = (calwdata.iLineOffset == 0) ? 0 : calwdata.iWordCount + 1;
    if (!bPrefixInitialized && (iWordByteCount != iWordLength))
    {
      calwdata.iLineOffset -= iWordByteCount - iWordLength;
    }
  }
  else
  {
    calwdata.iLineOffset = prefixLength + calwdata.longLineLimit;
  }

  if (!bPrefixInitialized
      && (calwdata.iLineOffset >= prefixLength + calwdata.longLineLimit)
      && (TextBuffer_GetTailLength(&pED->m_tb) > 0))
  {
    calwdata.iLineOffset = 0;
    calwdata.iWordCount = 0;
    if (!IsEOLChar(TextBuffer_GetChar(&pED->m_tb)))
    {
      TextBuffer_AddEOL(&pED->m_tbRes, calwdata.iEOLMode);
      // skip trailing space
      if (TextBuffer_GetChar(&pED->m_tb) == CHAR_SPACE)
      {
        TextBuffer_PopChar(&pED->m_tb);
        ++iCharsProcessed;
      }
    }
  }

  if (piCharsProcessed)
  {
    (*piCharsProcessed) += iCharsProcessed;
  }
  return TRUE;
}

BOOL CALW_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  switch (pRA->iPassIndex)
  {
  case 0:
    return CALW_Encode_Pass1(pRA, pED, piCharsProcessed);
  case 1:
    return CALW_Encode_Pass2(pRA, pED, piCharsProcessed);
  }

  return TRUE;
}

BOOL CALW_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  return FALSE;
}

static StringSource ss = { 0 };
static RecodingAlgorithm ra = { 0 };

LPCSTR EncodeStringWithCALW(LPCSTR text, const int textLength, const int encoding,
  const int additionalData1, const int additionalData2, const int additionalData3, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorithm_Init(&ra, ERT_CALW, TRUE, additionalData1, additionalData2, additionalData3);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorithm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

void EncodeStrWithCALW(const HWND hwnd)
{
  RecodingAlgorithm_Init(&ra, ERT_CALW, TRUE, iLongLinesLimit, pLexCurrent->iLexer, iEOLMode);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorithm_Release(&ra);
}
