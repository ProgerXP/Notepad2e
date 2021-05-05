#include <wtypes.h>
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
  int firstLineOffset;
  BOOL firstLineOffsetCalcDone;
  BOOL initLine;
  int iLineOffset;
};

typedef struct TCALWData CALWData;

static CALWData calwdata = { 0 };

LPVOID CALW_InitAlgorithmData(const int iAdditionalData)
{
  calwdata.longLineLimit = iAdditionalData;
  calwdata.firstLineOffset = 0;
  calwdata.firstLineOffsetCalcDone = FALSE;
  calwdata.relativeLineIndex = 0;
  calwdata.initLine = FALSE;
  calwdata.iLineOffset = 0;
  return (LPVOID)&calwdata;
}

void CALW_ReleaseAlgorithmData(LPVOID pData)
{
}

inline BOOL IsEOLChar(const char ch)
{
  return (ch == '\r') || (ch == '\n');
}

BOOL CALW_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
  int iCharsProcessed = 0;
  if ((calwdata.relativeLineIndex == 0) && !calwdata.firstLineOffsetCalcDone)
  {
    calwdata.firstLineOffsetCalcDone = (ch != ' ');
    if (!calwdata.firstLineOffsetCalcDone)
      ++calwdata.firstLineOffset;
  }
  
  if (IsEOLChar(ch))
  {
    if (!calwdata.initLine)
    {
      ++calwdata.relativeLineIndex;
      calwdata.initLine = TRUE;
      TextBuffer_PushChar(&pED->m_tbRes, ' ');
    }
    // else: ignore
  }
  else if (calwdata.initLine)
  {
    calwdata.initLine &= (ch == '\t') || (ch == ' ');
  }
  
  if (!calwdata.initLine)
  {
    TextBuffer_PushChar(&pED->m_tbRes, ch);
  }
  ++iCharsProcessed;
  if (piCharsProcessed)
  {
    (*piCharsProcessed) += iCharsProcessed;
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
  RecodingAlgorithm_Init(&ra, ERT_CALW, TRUE, 0);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorithm_Release(&ra);
}
