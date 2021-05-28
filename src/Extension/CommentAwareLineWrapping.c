#include <wtypes.h>
#include <Shlwapi.h>
#include "Externals.h"
#include "CommentAwareLineWrapping.h"
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
  int relativeLineIndex;
  int relativeLineIndexPrefixProcessed;
  
  PrefixData prefixFirstLine;
  PrefixData prefixMarkerLine;

  BOOL initLine;
  BOOL skipNextEOL;
  int iLineOffset;
};

typedef struct TCALWData CALWData;

static CALWData calwdata = { 0 };

LPVOID CALW_InitAlgorithmData(const int iAdditionalData)
{
  ZeroMemory(&calwdata, sizeof(calwdata));
  calwdata.longLineLimit = iAdditionalData;
  return (LPVOID)&calwdata;
}

void CALW_ReleaseAlgorithmData(LPVOID pData)
{
}

inline BOOL IsEOLChar(const char ch)
{
  return (ch == '\r') || (ch == '\n');
}

static LPCSTR lpstrWhiteSpaces = " \t";
static LPCSTR lpstrMarkerChars = "#>=?*";

// remove EOLs/front spaces
BOOL CALW_Encode_Pass1(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  BOOL bSkipChars = FALSE;
  const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
  int iCharCount = 1 + TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0);
  int iCharsProcessed = 0;
  if ((calwdata.relativeLineIndex == 0) && !PrefixData_IsInitialized(&calwdata.prefixFirstLine))
  {
    if (strchr(lpstrWhiteSpaces, ch))
    {
      const int res = TextBuffer_Find(&pED->m_tb, "//", 0);
      PrefixData_SetComment(&calwdata.prefixFirstLine, res >= 0);
      if (res >= 0)
      {
        PrefixData_SetEmpty(&calwdata.prefixFirstLine);
        PrefixData_PushChar(&calwdata.prefixFirstLine, ch);
        for (int i = 1; i < 1 + 2 + res + 1; ++i)
        {
          PrefixData_PushChar(&calwdata.prefixFirstLine, TextBuffer_PopChar(&pED->m_tb));
        }
        iCharCount = PrefixData_GetLength(&calwdata.prefixFirstLine);
        bSkipChars = TRUE;
      }
      else
      {
        for (int i = 0; i < iCharCount; ++i)
        {
          PrefixData_PushChar(&calwdata.prefixFirstLine, ch);
        }
        TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
        bSkipChars = TRUE;
      }
    }
    PrefixData_SetInitialized(&calwdata.prefixFirstLine, TRUE);
  }
  else if ((calwdata.relativeLineIndex > 0) 
    && (calwdata.relativeLineIndex > calwdata.relativeLineIndexPrefixProcessed))
  {
    if (PrefixData_IsComment(&calwdata.prefixFirstLine))
    {
      const int res = TextBuffer_Find(&pED->m_tb, "//", -1);
      if (res >= 0)
      {
        iCharCount = res + 2;
        TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
        bSkipChars = TRUE;
        calwdata.relativeLineIndexPrefixProcessed = calwdata.relativeLineIndex;
      }
    }
    else if ((PrefixData_GetLength(&calwdata.prefixFirstLine) > 0)
      && (ch == ' ') && (iCharCount >= PrefixData_GetLength(&calwdata.prefixFirstLine)))
    {
      TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
      bSkipChars = TRUE;
    }
  }

  if (IsEOLChar(ch))
  {
    if (!calwdata.initLine)
    {
      ++calwdata.relativeLineIndex;

      const BOOL isEOLAtPosition = IsEOLChar(TextBuffer_GetChar(&pED->m_tb));
      const int spacesAfterPosition = TextBuffer_GetCharSequenceLength(&pED->m_tb, ' ', 1);
      int iLineLength = 0;
      const BOOL isWhiteSpaceLine = TextBuffer_IsWhiteSpaceLine(&pED->m_tb, 1, &iLineLength);

      calwdata.initLine = !calwdata.skipNextEOL &&
        (((PrefixData_GetLength(&calwdata.prefixFirstLine) == 0)&& !isWhiteSpaceLine)
            || (isEOLAtPosition && (spacesAfterPosition != PrefixData_GetLength(&calwdata.prefixFirstLine))));

      if (isWhiteSpaceLine)
      {
        TextBuffer_PushChar(&pED->m_tbRes, ch);
        TextBuffer_PushChar(&pED->m_tbRes, TextBuffer_GetChar(&pED->m_tb));
        iCharCount += 1 + iLineLength;
        TextBuffer_OffsetPos(&pED->m_tb, 1 + iLineLength);
        calwdata.initLine = FALSE;
        calwdata.skipNextEOL = TRUE;
        bSkipChars = TRUE;
      }
      else
      if (calwdata.initLine)
      {
        const BOOL commentAtPosition = TextBuffer_IsTextAtPos(&pED->m_tb, "//", 1 + spacesAfterPosition);
        const int markerOffset = 1 + spacesAfterPosition + (commentAtPosition ? 2 : 0);
        if (isEOLAtPosition
          && TextBuffer_IsAnyCharAtPos_IgnoreSpecial(&pED->m_tb, lpstrMarkerChars, lpstrWhiteSpaces, markerOffset))
        {
          TextBuffer_PushChar(&pED->m_tbRes, ch);
          TextBuffer_PushChar(&pED->m_tbRes, TextBuffer_GetChar(&pED->m_tb));
          iCharCount += markerOffset;
          TextBuffer_OffsetPos(&pED->m_tb, markerOffset);
          bSkipChars = TRUE;
        }
        else if ((pED->m_tbRes.m_iPos > 0)
          && (TextBuffer_GetCharAt(&pED->m_tbRes, -1) != ' '))
        {
          TextBuffer_PushChar(&pED->m_tbRes, ' ');
        }
      }
      if (!isWhiteSpaceLine && (ch == '\n'))
      {
        calwdata.skipNextEOL = FALSE;
      }
    }
    // else: ignore
  }
  else if (calwdata.initLine)
  {
    calwdata.initLine &= (ch == ' ') && (iCharCount != PrefixData_GetLength(&calwdata.prefixFirstLine));
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
  if ((calwdata.iLineOffset == 0) && (prefixLength > 0))
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
  int iWordLength = max(1, TextBuffer_GetWordLength(&pED->m_tb));
  if ((iWordLength > calwdata.longLineLimit)
    || (calwdata.iLineOffset + iWordLength <= prefixLength + calwdata.longLineLimit))
  {
    if (PrefixData_IsInitialized(&calwdata.prefixMarkerLine)
      && ((pED->m_tbRes.m_iPos > 0) && TextBuffer_GetCharAt(&pED->m_tbRes, -1) == '\n'))
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
    for (int i = 1; i <= iWordLength; ++i)
    {
      const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
      if ((iWordLength >= 1) && strchr(lpstrMarkerChars, ch) && (calwdata.iLineOffset == PrefixData_GetLength(&calwdata.prefixFirstLine)))
      {
        TextBuffer_PushChar(&pED->m_tbRes, ch);
        ++iCharsProcessed;
        ++calwdata.iLineOffset;

        const auto chNext = TextBuffer_GetChar(&pED->m_tb);
        PrefixData_SetEmpty(&calwdata.prefixMarkerLine);
        PrefixData_PushChar(&calwdata.prefixMarkerLine, ' '); // space char under *
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
      if (ch == '\n')
      {
        calwdata.iLineOffset = 0;
        PrefixData_SetInitialized(&calwdata.prefixMarkerLine, FALSE);
      }
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
    TextBuffer_PushChar(&pED->m_tbRes, '\r');
    TextBuffer_PushChar(&pED->m_tbRes, '\n');

    // skip trailing space
    if (TextBuffer_GetChar(&pED->m_tb) == ' ')
    {
      TextBuffer_PopChar(&pED->m_tb);
      ++iCharsProcessed;
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

LPCSTR EncodeStringWithCALW(LPCSTR text, const int textLength, const int encoding, const int additionalData, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorithm_Init(&ra, ERT_CALW, TRUE, additionalData);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorithm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

void EncodeStrWithCALW(const HWND hwnd)
{
  RecodingAlgorithm_Init(&ra, ERT_CALW, TRUE, iLongLinesLimit);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorithm_Release(&ra);
}
