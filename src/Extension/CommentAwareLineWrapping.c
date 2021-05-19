#include <wtypes.h>
#include <Shlwapi.h>
#include "Externals.h"
#include "CommentAwareLineWrapping.h"
#include "StringRecoding.h"


BOOL CALW_IsValidSequence(EncodingData* pED, const int requiredChars)
{
  return TRUE;
}

struct TCALWData
{
  int longLineLimit;
  int relativeLineIndex;
  int relativeLineIndexPrefixProcessed;
  BOOL firstLineOffsetCalcDone;
  int firstLineOffset;
  char firstLinePrefix[MAX_PATH];
  char commentLinePrefix[MAX_PATH];
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

// remove EOLs/front spaces
BOOL CALW_Encode_Pass1(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  BOOL bSkipChars = FALSE;
  const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
  int iCharCount = 1 + TextBuffer_GetCharSequenceLength(&pED->m_tb, ch, 0);
  int iCharsProcessed = 0;
  if ((calwdata.relativeLineIndex == 0) && !calwdata.firstLineOffsetCalcDone)
  {
    if (ch == ' ')
    {
      const int res = TextBuffer_Find(&pED->m_tb, "//", 0);
      if (res >= 0)
      {
        StrCpyA(calwdata.commentLinePrefix, "//");
        calwdata.firstLineOffset = 1 + 2 + res + 1/*additional spacing*/;
        calwdata.firstLinePrefix[0] = ch;
        for (int i = 1; i < calwdata.firstLineOffset; ++i)
        {
          calwdata.firstLinePrefix[i] = TextBuffer_PopChar(&pED->m_tb);
        }
        iCharCount = calwdata.firstLineOffset;
        bSkipChars = TRUE;
      }
      else
      {
        if (iCharCount > 1)
        {
          TextBuffer_OffsetPos(&pED->m_tb, iCharCount - 1);
        }
        calwdata.firstLineOffset = (ch == ' ') ? iCharCount : 0;
        bSkipChars = TRUE;
      }
    }
    calwdata.firstLineOffsetCalcDone = TRUE;
  }
  else if ((calwdata.relativeLineIndex > 0) 
    && (calwdata.relativeLineIndex > calwdata.relativeLineIndexPrefixProcessed))
  {
    if (strlen(calwdata.commentLinePrefix) > 0)
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
    else if ((calwdata.firstLineOffset > 0) && (ch == ' ') && (iCharCount >= calwdata.firstLineOffset))
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
        (((calwdata.firstLineOffset == 0)&& !isWhiteSpaceLine)
            || (isEOLAtPosition && (spacesAfterPosition != calwdata.firstLineOffset)));

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
        const int bulletOffset = 1 + spacesAfterPosition + (commentAtPosition ? 2 : 0);
        if (isEOLAtPosition
          && TextBuffer_IsCharAtPos_IgnoreSpecial(&pED->m_tb, '*', " \t", bulletOffset))
        {
          TextBuffer_PushChar(&pED->m_tbRes, ch);
          TextBuffer_PushChar(&pED->m_tbRes, TextBuffer_GetChar(&pED->m_tb));
          iCharCount += bulletOffset;
          TextBuffer_OffsetPos(&pED->m_tb, bulletOffset);
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
    calwdata.initLine &= (ch == ' ') && (iCharCount != calwdata.firstLineOffset);
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
  const auto prefixLength = strlen(calwdata.firstLinePrefix);
  if ((calwdata.iLineOffset == 0) && ((prefixLength > 0) || (calwdata.firstLineOffset > 0)))
  {
    if (prefixLength > 0)
    {
      for (int i = 0; i < prefixLength; ++i)
      {
        TextBuffer_PushChar(&pED->m_tbRes, calwdata.firstLinePrefix[i]);
        ++calwdata.iLineOffset;
      }
    }
    else
    {
      for (int i = 0; i < calwdata.firstLineOffset; ++i)
      {
        TextBuffer_PushChar(&pED->m_tbRes, ' ');
        ++calwdata.iLineOffset;
      }
    }
  }
  else if ((pED->m_tb.m_iPos != 0) && (calwdata.iLineOffset == 0))
  {
    for (int i = 0; i < calwdata.firstLineOffset; ++i)
    {
      TextBuffer_PushChar(&pED->m_tbRes, ' ');
      ++calwdata.iLineOffset;
    }
  }

  int iCharsProcessed = 0;
  int iWordLength = max(1, TextBuffer_GetWordLength(&pED->m_tb));
  if (calwdata.iLineOffset + iWordLength <= prefixLength + calwdata.longLineLimit)
  {
    for (int i = 0; i < iWordLength; ++i)
    {
      const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
      TextBuffer_PushChar(&pED->m_tbRes, ch);
      ++iCharsProcessed;
      ++calwdata.iLineOffset;
      if (ch == '\n')
      {
        calwdata.iLineOffset = 0;
      }
    }
  }
  else
  {
    calwdata.iLineOffset = prefixLength + calwdata.longLineLimit;
  }

  if ((calwdata.iLineOffset >= prefixLength + calwdata.longLineLimit) && (TextBuffer_GetTailLength(&pED->m_tb) > 0))
  {
    calwdata.iLineOffset = 0;
    TextBuffer_PushChar(&pED->m_tbRes, '\r');
    TextBuffer_PushChar(&pED->m_tbRes, '\n');
    iCharsProcessed += 2;

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
