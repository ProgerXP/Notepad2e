#include <wtypes.h>
#include "Externals.h"
#include "StrToQP.h"
#include "StringRecoding.h"

BOOL QP_IsValidSequence(EncodingData* pED, const int requiredChars)
{
  return TRUE;
}

struct TQPData
{
  int linePosition;
};

typedef struct TQPData QPData;

static QPData qpdata = { 0 };

LPVOID QP_InitAlgorythmData(const BOOL isEncoding)
{
  if (isEncoding)
  {
    qpdata.linePosition = 0;
    return (LPVOID)&qpdata;
  }
  else
  {
    return NULL;
  }
}

void QP_ReleaseAlgorythmData(LPVOID pData)
{
}

#define QP_MAX_ENCODED_LINE_LENGTH  76

BOOL QP_LineBreakRequired(TextBuffer* pTB, const int charsRequired)
{
  return (qpdata.linePosition + charsRequired >= QP_MAX_ENCODED_LINE_LENGTH);
}

BOOL IsQPChar(TextBuffer* pTB, const unsigned char ch)
{
  return ((ch >= 33) && (ch <= 60))
         || ((ch >= 62) && (ch <= 126))
         || (((ch == '\t') || (ch == ' ')) && (TextBuffer_GetTailLength(pTB) > 0));
}

void QP_UpdateLinePosition(EncodingData* pED, const int charsRequired)
{
  if (QP_LineBreakRequired(&pED->m_tb, charsRequired))
  {
    TextBuffer_PushChar(&pED->m_tbRes, '=');  // soft line break
    TextBuffer_PushChar(&pED->m_tbRes, '\r');
    TextBuffer_PushChar(&pED->m_tbRes, '\n');
    qpdata.linePosition = 0;
  }
  qpdata.linePosition += charsRequired;
}

const unsigned char QP_GetNextChar(TextBuffer* pTB)
{
  return (pTB->m_iPos < pTB->m_iMaxPos) ? TextBuffer_GetChar(pTB) : ' ';
}

BOOL QP_Encode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  const unsigned char ch = TextBuffer_PopChar(&pED->m_tb);
  
  int iCharsProcessed = 0;
  if (IsQPChar(&pED->m_tb, ch))
  {
    QP_UpdateLinePosition(pED, 1);
    TextBuffer_PushChar(&pED->m_tbRes, ch);
    ++iCharsProcessed;
  }
  else
  {
    QP_UpdateLinePosition(pED, 3);
    TextBuffer_PushChar(&pED->m_tbRes, '=');
    TextBuffer_PushHexChar(pED, ch);
    ++iCharsProcessed;
  }
  if (piCharsProcessed)
  {
    (*piCharsProcessed) += iCharsProcessed;
  }
  return TRUE;
}

BOOL IsLineEnd(unsigned char ch)
{
  return (ch == '\r') || (ch == '\n');
}

BOOL QP_Decode(RecodingAlgorythm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  int iCharsProcessed = 0;
  const char ch = TextBuffer_PopChar(&pED->m_tb);
  unsigned char chNext = TextBuffer_GetChar(&pED->m_tb);
  if (ch == '=')
  {
    ++iCharsProcessed;
    const char chEncoded1 = TextBuffer_PopChar(&pED->m_tb);
    const char chEncoded2 = TextBuffer_PopChar(&pED->m_tb);

    if (IsLineEnd(chEncoded1))  // soft line break
    {
      ++iCharsProcessed;
      if (IsLineEnd(chEncoded2))
      {
        ++iCharsProcessed;
      }
      else
      {
        TextBuffer_DecPos(&pED->m_tb);
      }
      if (piCharsProcessed)
      {
        (*piCharsProcessed) += iCharsProcessed;
      }
      return TRUE;
    }

    iCharsProcessed += 2;
    char chDecoded = 0;
    if (DecodeHexDigits(chEncoded1, chEncoded2, &chDecoded))
    {
      TextBuffer_PushChar(&pED->m_tbRes, chDecoded);
    }
    else
    {
      TextBuffer_PushChar(&pED->m_tbRes, ch);
      TextBuffer_PushChar(&pED->m_tbRes, chEncoded1);
      TextBuffer_PushChar(&pED->m_tbRes, chEncoded2);
    }
    if (piCharsProcessed)
    {
      (*piCharsProcessed) += 3;
    }
    return TRUE;
  }
  else if (IsQPChar(&pED->m_tb, ch) || IsLineEnd(ch)/*hack to decode broken QP from RFC-incompatible encoders*/)
  {
    TextBuffer_PushChar(&pED->m_tbRes, ch);
    if (piCharsProcessed)
    {
      (*piCharsProcessed) += 1;
    }
    return TRUE;
  }
  return FALSE;
}

static StringSource ss = { 0 };
static RecodingAlgorythm ra = { 0 };

LPCSTR EncodeStringToQP(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorythm_Init(&ra, ERT_QP, TRUE);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorythm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

LPCSTR DecodeQPToString(LPCSTR text, const int textLength, const int encoding, const int bufferSize, int* pResultLength)
{
  iEncoding = encoding;
  RecodingAlgorythm_Init(&ra, ERT_QP, FALSE);
  StringSource_InitFromString(&ss, text, textLength);
  Recode_Run(&ra, &ss, bufferSize);
  RecodingAlgorythm_Release(&ra);
  *pResultLength = ss.iResultLength;
  return ss.result;
}

void EncodeStrToQP(const HWND hwnd)
{
  RecodingAlgorythm_Init(&ra, ERT_QP, TRUE);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorythm_Release(&ra);
}

void DecodeQPToStr(const HWND hwnd)
{
  RecodingAlgorythm_Init(&ra, ERT_QP, FALSE);
  StringSource_InitFromHWND(&ss, hwnd);
  Recode_Run(&ra, &ss, -1);
  RecodingAlgorythm_Release(&ra);
}
