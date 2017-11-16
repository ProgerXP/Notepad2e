#pragma once
#include <wtypes.h>

#define MAX_TEST_STRING_LENGTH 1200000

extern int RECODING_BUFFER_SIZE;
extern int RECODING_BUFFER_SIZE_MAX;
extern BOOL bBreakOnError;

struct TTextBuffer
{
  int m_iSize;
  LPSTR m_ptr;
  int m_iMaxPos;
  int m_iPos;
};
typedef struct TTextBuffer TextBuffer;

struct TTextRange
{
  HWND m_hwnd;
  long m_iSelStart;
  long m_iSelEnd;
  long m_iPositionStart;
  long m_iPositionCurrent;
  long m_iExpectedProcessedChars;
};
typedef struct TTextRange TextRange;

struct TEncodingData
{
  struct TTextRange m_tr;
  struct TTextBuffer m_tb;
  struct TTextBuffer m_tbRes;
  struct TTextBuffer m_tbTmp;
  BOOL m_bIsEncoding;
};
typedef struct TEncodingData EncodingData;

struct TStringSource
{
  HWND hwnd;
  char text[MAX_TEST_STRING_LENGTH];
  char result[MAX_TEST_STRING_LENGTH];
  int iTextLength;
  int iProcessedChars;
};
typedef struct TStringSource StringSource;

typedef BOOL(*IsValidStrSequence)(EncodingData* pED, const int requiredChars);
typedef BOOL(*RecodeMethod)(LPVOID pRA, EncodingData* pED, long* piCharsProcessed);

struct TRecodingAlgorythm
{
  enum ERecodingType recodingType;
  BOOL isEncoding;
  int iRequiredCharsForEncode;
  int iRequiredCharsForDecode;
  IsValidStrSequence pIsValidStrSequence;
  RecodeMethod pEncodeMethod;
  RecodeMethod pEncodeTailMethod;
  RecodeMethod pDecodeMethod;
  wchar_t statusText[MAX_PATH];
  LPVOID data;
};
typedef struct TRecodingAlgorythm RecodingAlgorythm;

BOOL IsUnicodeEncodingMode();
BOOL Is8BitEncodingMode();
BOOL IsReverseUnicodeEncodingMode();

void TextBuffer_ResetPos(TextBuffer* pTB, const int iMaxPos);
void TextBuffer_Clear(TextBuffer* pTB);
BOOL TextBuffer_Init(TextBuffer* pTB, const int iSize);
BOOL TextBuffer_Free(TextBuffer* pTB);
BOOL TextBuffer_Update(TextBuffer* pTB, LPSTR ptr, const int iSize);
BOOL TextBuffer_GetTailLength(TextBuffer* pTB);
BOOL TextBuffer_IsPosOKImpl(TextBuffer* pTB, const int requiredChars);
BOOL TextBuffer_IsPosOK(TextBuffer* pTB, RecodingAlgorythm* pRA);
void TextBuffer_IncPos(TextBuffer* pTB);
void TextBuffer_DecPos(TextBuffer* pTB);
char TextBuffer_GetChar(TextBuffer* pTB);
char TextBuffer_PopChar(TextBuffer* pTB);
BOOL TextBuffer_PushChar(TextBuffer* pTB, const char ch);
BOOL TextBuffer_PushNonZeroChar(TextBuffer* pTB, const char ch);

typedef enum
{
  ERT_HEX,
  ERT_BASE64
} ERecodingType;

BOOL RecodingAlgorythm_Init(RecodingAlgorythm* pRA, const ERecodingType rt, const BOOL isEncoding);
BOOL RecodingAlgorythm_Release(RecodingAlgorythm* pRA);

void StringSource_Init(StringSource* pSS, LPCSTR text, const HWND hwnd);
long StringSource_GetSelectionStart(const StringSource* pSS);
long StringSource_GetSelectionEnd(const StringSource* pSS);
long StringSource_GetLength(const StringSource* pSS);
long StringSource_IsDataPortionAvailable(const StringSource* pSS, EncodingData* pED);
BOOL StringSource_GetText(StringSource* pSS, LPSTR pText, const long iStart, const long iEnd);

void Recode_Run(RecodingAlgorythm* pRA, StringSource* pSS, const int bufferSize);
BOOL Recode_ProcessDataPortion(RecodingAlgorythm* pRA, StringSource* pSS, EncodingData* pED);
