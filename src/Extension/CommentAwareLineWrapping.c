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
      calwdata.firstLineOffset = iCharCount;
    }
    calwdata.firstLineOffsetCalcDone = TRUE;
  }
  else if ((calwdata.relativeLineIndex > 0) 
    && (calwdata.relativeLineIndex > calwdata.relativeLineIndexPrefixProcessed)
    && strlen(calwdata.commentLinePrefix) > 0)
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

  if (IsEOLChar(ch))
  {
    if (!calwdata.initLine)
    {
      ++calwdata.relativeLineIndex;
      calwdata.initLine = (calwdata.firstLineOffset == 0)
        || (IsEOLChar(TextBuffer_GetChar(&pED->m_tb)) && (TextBuffer_GetCharSequenceLength(&pED->m_tb, ' ', 1) != calwdata.firstLineOffset));
      if (calwdata.initLine && (TextBuffer_GetCharAt(&pED->m_tbRes, -1) != ' '))
      {
        TextBuffer_PushChar(&pED->m_tbRes, ' ');
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
  if ((calwdata.iLineOffset == 0) && prefixLength)
  {
    for (int i = 0; i < prefixLength; ++i)
    {
      TextBuffer_PushChar(&pED->m_tbRes, calwdata.firstLinePrefix[i]);
      ++calwdata.iLineOffset;
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
