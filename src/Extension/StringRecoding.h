#pragma once
#include <wtypes.h>

#define MAX_TEST_STRING_LENGTH 1500000  // must be greater than the size of the largest file test\data\Extension\TestFile*.txt

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
  int iResultLength;
};
typedef struct TStringSource StringSource;

typedef int(*ExpectedResultLengthMethod)(const BOOL isEncoding, const int originalLength);
typedef BOOL(*IsValidStrSequence)(EncodingData* pED, const int requiredChars);
typedef BOOL(*RecodeMethod)(LPVOID pRA, EncodingData* pED, long* piCharsProcessed);

struct TRecodingAlgorithm
{
  enum ERecodingType recodingType;
  BOOL isEncoding;
  int iRequiredCharsForEncode;
  int iRequiredCharsForDecode;
  IsValidStrSequence pIsValidStrSequence;
  RecodeMethod pEncodeMethod;
  RecodeMethod pEncodeTailMethod;
  RecodeMethod pDecodeMethod;
  RecodeMethod pDecodeTailMethod;
  wchar_t statusText[MAX_PATH];
  LPVOID data;
};
typedef struct TRecodingAlgorithm RecodingAlgorithm;

BOOL n2e_IsUnicodeEncodingMode();
BOOL n2e_IsUTF8EncodingMode();
BOOL Is8BitEncodingMode();
BOOL IsReverseUnicodeEncodingMode();
BOOL IsHexDigit(const unsigned char ch);
int IntByHexDigit(const unsigned char ch);
BOOL DecodeHexDigits(const unsigned char chEncoded1, const unsigned char chEncoded2, unsigned char* pchDecoded);

void TextBuffer_ResetPos(TextBuffer* pTB, const int iMaxPos);
void TextBuffer_Clear(TextBuffer* pTB);
BOOL TextBuffer_Init(TextBuffer* pTB, const int iSize);
BOOL TextBuffer_Free(TextBuffer* pTB);
BOOL TextBuffer_Update(TextBuffer* pTB, LPSTR ptr, const int iSize);
BOOL TextBuffer_GetTailLength(TextBuffer* pTB);
BOOL TextBuffer_IsPosOKImpl(TextBuffer* pTB, const int requiredChars);
BOOL TextBuffer_IsPosOK(TextBuffer* pTB, RecodingAlgorithm* pRA);
void TextBuffer_IncPos(TextBuffer* pTB);
void TextBuffer_DecPos(TextBuffer* pTB);
char TextBuffer_GetChar(TextBuffer* pTB);
char TextBuffer_PopChar(TextBuffer* pTB);
BOOL TextBuffer_PushChar(TextBuffer* pTB, const char ch);
BOOL TextBuffer_PushHexChar(EncodingData* pED, const unsigned char ch);
BOOL TextBuffer_PushNonZeroChar(TextBuffer* pTB, const char ch);
BOOL TextBuffer_GetLiteralChar(TextBuffer* pTB, char* pCh, long* piCharsProcessed);

typedef enum
{
  ERT_HEX,
  ERT_BASE64,
  ERT_QP,
  ERT_URL
} ERecodingType;

BOOL RecodingAlgorithm_Init(RecodingAlgorithm* pRA, const ERecodingType rt, const BOOL isEncoding);
BOOL RecodingAlgorithm_Release(RecodingAlgorithm* pRA);

void StringSource_InitFromString(StringSource* pSS, LPCSTR text, const int textLength);
void StringSource_InitFromHWND(StringSource* pSS, const HWND hwnd);
long StringSource_GetSelectionStart(const StringSource* pSS);
long StringSource_GetSelectionEnd(const StringSource* pSS);
long StringSource_GetLength(const StringSource* pSS);
long StringSource_IsDataPortionAvailable(const StringSource* pSS, EncodingData* pED);
BOOL StringSource_GetText(StringSource* pSS, LPSTR pText, const long iStart, const long iEnd);

void Recode_Run(RecodingAlgorithm* pRA, StringSource* pSS, const int bufferSize);
BOOL Recode_ProcessDataPortion(RecodingAlgorithm* pRA, StringSource* pSS, EncodingData* pED);
