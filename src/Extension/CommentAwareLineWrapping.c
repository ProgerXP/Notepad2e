#include <assert.h>
#include <wtypes.h>
#include <Shlwapi.h>
#include "Externals.h"
#include "CommentAwareLineWrapping.h"
#include "LexerUtils.h"
#include "SciLexer.h"
#include "StringRecoding.h"

struct TPrefixData
{
  char data[MAX_PATH];
  BOOL isInitialized;
  BOOL isComment;
};

typedef struct TPrefixData PrefixData;

struct TCALWData
{
  int longLineLimit;
  int lexerId;
  int iEOLMode;
  int iTrailingEOLLength;

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
};

typedef struct TCALWData CALWData;

static CALWData calwdata = { 0 };

void PrefixData_SetEmpty(PrefixData* pd)
{
  pd->data[0] = 0;
}

BOOL PrefixData_IsInitialized(PrefixData* pd)
{
  return pd->isInitialized;
}

void PrefixData_SetInitialized(PrefixData* pd, BOOL isInitialized)
{
  pd->isInitialized = isInitialized;
}

BOOL PrefixData_IsComment(PrefixData* pd)
{
  return pd->isComment;
}

void PrefixData_SetComment(PrefixData* pd, BOOL isComment)
{
  pd->isComment = isComment;
}

int PrefixData_GetLength(PrefixData* pd)
{
  return strlen(pd->data);
}

BOOL PrefixData_IsEmpty(PrefixData* pd)
{
  return PrefixData_GetLength(pd) == 0;
}

void PrefixData_PushChar(PrefixData* pd, const unsigned char ch)
{
  const int pos = PrefixData_GetLength(pd);
  if (pos < _countof(pd->data) - 2)
  {
    pd->data[pos] = ch;
    pd->data[pos + 1] = 0;
  }
}

const unsigned char PrefixData_GetChar(PrefixData* pd, const int pos)
{
  return pd->data[pos];
}

BOOL CALW_IsValidSequence(EncodingData* pED, const int requiredChars)
{
  return TRUE;
}

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
  calwdata.iEOLMode = iAdditionalData3;
  calwdata.iTrailingEOLLength = GetTrailingEOLLength();
  calwdata.iSingleLineCommentPrefixLength = n2e_GetSingleLineCommentPrefixLength(calwdata.lexerId);
  return (LPVOID)&calwdata;
}

void CALW_ReleaseAlgorithmData(LPVOID pData)
{
}

static unsigned char CHAR_SPACE = ' ';
static unsigned char CHAR_EOL_R = '\r';
static unsigned char CHAR_EOL_N = '\n';
static unsigned char CHAR_FORCE_EOL = '\a';
static unsigned char CHAR_FORCE_EOL_PROCESSED = '\b';
static LPCSTR lpstrWhiteSpaces = " \t";
static LPCSTR lpstrWhiteSpacesAndEOLs = " \t\r\n";
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

BOOL IsTrailingEOL(const unsigned char ch, TextBuffer* pTB)
{
  switch (calwdata.iEOLMode)
  {
  case SC_EOL_CRLF:
    return (ch == CHAR_EOL_R) && (TextBuffer_GetChar(pTB) == CHAR_EOL_N);
  case SC_EOL_LF:
    return ch == CHAR_EOL_N;
  case SC_EOL_CR:
    return ch == CHAR_EOL_R;
  }
  assert(0);
  return FALSE;
}

BOOL isCharFromString(LPCSTR lpstrSample, const unsigned char ch)
{
  return strchr(lpstrSample, ch) != NULL;
}

BOOL isMarker(const unsigned char ch, EncodingData* pED)
{
  const BOOL isStaticMarker = isCharFromString(lpstrStaticMarkerChars, ch);
  const BOOL isDynamicMarker = TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, 0);
  return isStaticMarker || isDynamicMarker;
}

// remove EOLs/front spaces
BOOL CALW_Encode_Pass1(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  BOOL skipChars = FALSE;
  BOOL skipInitLineCheck = FALSE;
  const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
  int iCharCount = 1 + (!IsTrailingEOL(ch, &pED->m_tb) ? TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0) : 0);
  int iCharsProcessed = 0;
  if ((calwdata.relativeLineIndex == 0) && !PrefixData_IsInitialized(&calwdata.prefixFirstLine))
  {
    const BOOL isWhiteSpace = isCharFromString(lpstrWhiteSpaces, ch);
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
      skipChars = TRUE;
    }
    else if (isWhiteSpace)
    {
      for (int i = 0; i < iCharCount; ++i)
      {
        PrefixData_PushChar(&calwdata.prefixFirstLine, ch);
      }
      TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
      skipChars = TRUE;
    }
    PrefixData_SetInitialized(&calwdata.prefixFirstLine, TRUE);
  }
  else if ((calwdata.relativeLineIndex > 0) 
    && (calwdata.relativeLineIndex > calwdata.relativeLineIndexPrefixProcessed))
  {
    if (PrefixData_IsComment(&calwdata.prefixFirstLine))
    {
      const int iWhiteSpaces = TextBuffer_CountWhiteSpaces(&pED->m_tb, 0);
      const BOOL isSingleLineComment = 
        n2e_IsSingleLineCommentStyleAtPos(NULL, calwdata.lexerId,
          (isCharFromString(lpstrWhiteSpacesAndEOLs, ch) ? 0 : -1) + iWhiteSpaces + calwdata.iSingleLineCommentPrefixLength,
          pED);
      if (isSingleLineComment)
      {
        if ((calwdata.iLineOffset == 0) && (iWhiteSpaces == 0) && !IsEOLChar(TextBuffer_GetCharAt(&pED->m_tb, -1)))
        {
          TextBuffer_DecPos(&pED->m_tb);
          --(*piCharsProcessed);
        }
        iCharCount = iWhiteSpaces + 1 + calwdata.iSingleLineCommentPrefixLength;
        TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);

        skipChars = TRUE;
        skipInitLineCheck = TRUE;
        calwdata.relativeLineIndexPrefixProcessed = calwdata.relativeLineIndex;

        int iLineLength = 0;
        const BOOL isWhiteSpaceLine = TextBuffer_IsWhiteSpaceLine(&pED->m_tb, 0, &iLineLength);
        if (isWhiteSpaceLine)
        {
          if (TextBuffer_GetCharAt(&pED->m_tbRes, -1) == CHAR_SPACE)
          {
            TextBuffer_OffsetPos(&pED->m_tbRes, -1);
          }
          TextBuffer_PushChar(&pED->m_tbRes, CHAR_FORCE_EOL);
          if (TextBuffer_GetTailLength(&pED->m_tb) > 0)
          {
            const int offset = 1 + min(GetTrailingEOLLength(), TextBuffer_GetTailLength(&pED->m_tb));
            TextBuffer_OffsetPos(&pED->m_tb, offset);
            iCharsProcessed += offset;
          }
          calwdata.initLine = TRUE;
          ++calwdata.relativeLineIndex;
        }
      }
    }
    else if ((PrefixData_GetLength(&calwdata.prefixFirstLine) > 0)
      && (ch == CHAR_SPACE) && (iCharCount >= PrefixData_GetLength(&calwdata.prefixFirstLine)))
    {
      TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
      skipChars = TRUE;
    }
  }

  if (IsEOLChar(ch) || skipInitLineCheck)
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
        skipChars = TRUE;
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
          skipChars = TRUE;
        }
        else if ((pED->m_tbRes.m_iPos > 0)
          && (TextBuffer_GetCharAt(&pED->m_tbRes, -1) != CHAR_SPACE))
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
  
  if (!skipChars)
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
  BOOL isPrefixInitialized = FALSE;
  auto prefixLength = PrefixData_GetLength(&calwdata.prefixFirstLine);
  if ((calwdata.iLineOffset == 0) && (prefixLength > 0) && !PrefixData_IsInitialized(&calwdata.prefixMarkerLine))
  {
    for (int i = 0; i < prefixLength; ++i)
    {
      TextBuffer_PushChar(&pED->m_tbRes, PrefixData_GetChar(&calwdata.prefixFirstLine, i));
      ++calwdata.iLineOffset;
    }
  }
  prefixLength = PrefixData_IsInitialized(&calwdata.prefixMarkerLine) ? PrefixData_GetLength(&calwdata.prefixMarkerLine) : 0;

  int iCharsProcessed = 0;
  int iWordByteCount = 0;
  const int iWordLength = TextBuffer_GetWordLength(&pED->m_tb, iEncoding, &iWordByteCount);
  if ((calwdata.iWordCount == 0)
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
    BOOL isMarker = FALSE;
    BOOL isWhiteSpace = FALSE;

    if ((iWordByteCount == 1)
      && ((TextBuffer_GetChar(&pED->m_tb) == CHAR_FORCE_EOL)
          || (TextBuffer_GetChar(&pED->m_tb) == CHAR_FORCE_EOL_PROCESSED)))
    {
      calwdata.iLineOffset = 0;
      calwdata.iWordCount = 0;
      if (IsEOL(TextBuffer_GetCharAt(&pED->m_tbRes, -(PrefixData_GetLength(&calwdata.prefixFirstLine) + 1))))
      {
        TextBuffer_PushChar(&pED->m_tb, CHAR_FORCE_EOL_PROCESSED);
        TextBuffer_DecPos(&pED->m_tb);
      }
      if (TextBuffer_GetChar(&pED->m_tb) != CHAR_FORCE_EOL_PROCESSED)
      {
        TextBuffer_PushChar(&pED->m_tb, CHAR_FORCE_EOL_PROCESSED);
        TextBuffer_DecPos(&pED->m_tb);
        TextBuffer_AddEOL(&pED->m_tbRes, calwdata.iEOLMode);
      }
      else 
      {
        TextBuffer_IncPos(&pED->m_tb);
        iCharsProcessed += 1;
        if (TextBuffer_GetTailLength(&pED->m_tb) > 0)
        {
          TextBuffer_AddEOL(&pED->m_tbRes, calwdata.iEOLMode);
        }
      }
    }
    else
    {
      for (int i = 1; i <= iWordByteCount; ++i)
      {
        const BOOL isDynamicMarker = TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrDynamicMarkerChars, lpstrDigits, 0);
        const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
        const BOOL isStaticMarker = isCharFromString(lpstrStaticMarkerChars, ch);
        isMarker = isStaticMarker || isDynamicMarker;
        if ((iWordLength >= 1)
          && isMarker
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
            while (isCharFromString(lpstrDigits, TextBuffer_GetChar(&pED->m_tb))
                  || isCharFromString(lpstrDynamicMarkerChars, TextBuffer_GetChar(&pED->m_tb)))
            {
              const unsigned char ch1 = TextBuffer_PopChar(&pED->m_tb);
              TextBuffer_PushChar(&pED->m_tbRes, ch1);
              PrefixData_PushChar(&calwdata.prefixMarkerLine, CHAR_SPACE); // space chars under NNN)
              ++iCharsProcessed;
              ++calwdata.iLineOffset;
            }
          }

          const unsigned char chNext = TextBuffer_GetChar(&pED->m_tb);
          if (isCharFromString(lpstrWhiteSpaces, chNext))
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
          isPrefixInitialized = TRUE;
          break;
        }
        isWhiteSpace = isCharFromString(lpstrWhiteSpaces, ch);
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
    }
    if (!isMarker && !isWhiteSpace)
    {
      calwdata.iWordCount = (calwdata.iLineOffset == 0) ? 0 : calwdata.iWordCount + 1;
    }
    if (!isPrefixInitialized && (iWordByteCount != iWordLength))
    {
      calwdata.iLineOffset -= iWordByteCount - iWordLength;
    }
  }
  else
  {
    calwdata.iLineOffset = prefixLength + calwdata.longLineLimit;
  }

  if (!isPrefixInitialized
      && (calwdata.iLineOffset >= prefixLength + calwdata.longLineLimit)
      && (TextBuffer_GetTailLength(&pED->m_tb) > 0))
  {
    calwdata.iLineOffset = 0;
    calwdata.iWordCount = 0;
    if (!IsEOLChar(TextBuffer_GetChar(&pED->m_tb))
      || PrefixData_IsComment(&calwdata.prefixFirstLine))
    {
      if (TextBuffer_GetCharAt(&pED->m_tbRes, -1) == CHAR_SPACE)
      {
        TextBuffer_DecPos(&pED->m_tbRes);
      }
      TextBuffer_AddEOL(&pED->m_tbRes, calwdata.iEOLMode);
      if (TextBuffer_IsEOL(&pED->m_tb, calwdata.iEOLMode))
      {
        TextBuffer_OffsetPos(&pED->m_tb, 1 + GetTrailingEOLLength());
        iCharsProcessed += 1 + GetTrailingEOLLength();
      }
      // skip trailing space
      else if (TextBuffer_GetChar(&pED->m_tb) == CHAR_SPACE)
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
