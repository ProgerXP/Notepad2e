#include <assert.h>
#include <wtypes.h>
#include <Shlwapi.h>
#include "Externals.h"
#include "CALWInterface.h"
#include "CommonUtils.h"
#include "CommentAwareLineWrapping.h"
#include "LexerUtils.h"
#include "SciLexer.h"
#include "StringRecoding.h"

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

static HCALWData hcalwdata;

void PrefixData_SetEmpty(PrefixData* pd)
{
  pd->data[0] = 0;
}

BOOL PrefixData_IsInitialized(PrefixData* pd)
{
  return pd->isInitialized;
}

void PrefixData_SetInitialized(PrefixData* pd, const BOOL isInitialized)
{
  pd->isInitialized = isInitialized;
}

BOOL PrefixData_IsComment(PrefixData* pd)
{
  return pd->isComment;
}

BOOL PrefixData_IsEmptyLineComment(PrefixData* pd)
{
  return pd->isEmptyLineComment;
}

void PrefixData_SetComment(PrefixData* pd, const BOOL isComment, const BOOL isEmptyLineComment)
{
  pd->isComment = isComment;
  pd->isEmptyLineComment = isEmptyLineComment;
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

const int PrefixData_CountTrailingWhiteSpaces(PrefixData* pd)
{
  int res = 0;
  int pos = PrefixData_GetLength(pd) - 1;
  while (pos > 0)
  {
    if (!IsCharFromString(lpstrWhiteSpaces, pd->data[pos]))
    {
      break;
    }
    --pos;
    ++res;
  }
  return res;
}

BOOL CALW_IsValidSequence(EncodingData* pED, const int requiredChars)
{
  return TRUE;
}

LPVOID CALW_InitAlgorithmData(const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3)
{
  hcalwdata = CALW_Create(iAdditionalData1, iAdditionalData2, iAdditionalData3);
  return hcalwdata;
}

void CALW_ReleaseAlgorithmData(LPVOID pData)
{
}

BOOL CALW_Encode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  return CALW_Run(hcalwdata, pRA, pED, piCharsProcessed);
}

BOOL CALW_Decode(RecodingAlgorithm* pRA, EncodingData* pED, long* piCharsProcessed)
{
  return FALSE;
}

void CALW_InitPass(RecodingAlgorithm* pRA, const int iPassIndex)
{
  CALW_InitPassImpl(hcalwdata, pRA, iPassIndex);
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
