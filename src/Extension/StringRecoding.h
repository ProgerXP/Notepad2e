#pragma once
#include <wtypes.h>

#define MAX_TEST_STRING_LENGTH 1500000  // must be greater than the size of the largest file test\data\Extension\TestFile*.txt

extern int RECODING_BUFFER_SIZE;
extern int RECODING_BUFFER_SIZE_MAX;
extern BOOL bBreakOnError;

static unsigned char CHAR_SPACE = ' ';
static unsigned char CHAR_EOL_R = '\r';
static unsigned char CHAR_EOL_N = '\n';
static unsigned char CHAR_FORCE_EOL = '\a';
static unsigned char CHAR_FORCE_EOL_PROCESSED = '\b';
static unsigned char CHAR_NEXT_PARAGRAPH = '\f';

static LPCSTR lpstrWhiteSpaces = " \t";
static LPCSTR lpstrWhiteSpacesAndEOLs = " \t\r\n";
static LPCSTR lpstrStaticMarkerChars = ">=?*#";
static LPCSTR lpstrDynamicMarkerChars = ":).";
static LPCSTR lpstrDigits = "0123456789";

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
  long m_iSelEndOriginal;
  long m_iPositionStart;
  long m_iPositionCurrent;
  long m_iExpectedProcessedChars;
  BOOL m_emptyOriginalSelection;
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
typedef void(*InitPassMethod)(LPVOID pRA);

struct TRecodingAlgorithm
{
  enum _ERecodingType recodingType;
  BOOL isEncoding;
  int iPassCount;
  int iPassIndex;
  int iRequiredCharsForEncode;
  int iRequiredCharsForDecode;
  int iAdditionalData1;
  int iAdditionalData2;
  int iAdditionalData3;
  IsValidStrSequence pIsValidStrSequence;
  RecodeMethod pEncodeMethod;
  RecodeMethod pEncodeTailMethod;
  RecodeMethod pDecodeMethod;
  RecodeMethod pDecodeTailMethod;
  InitPassMethod pInitPassMethod;
  wchar_t statusText[MAX_PATH];
  LPVOID data;
  int iResultStart;
  int iResultEnd;
  int iResultEndBackup;
  int iResultSelEnd;
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
int TextBuffer_GetHeadLength(TextBuffer* pTB);
int TextBuffer_GetLineHeadLength(TextBuffer* pTB);
int TextBuffer_GetTailLength(TextBuffer* pTB);
int TextBuffer_GetLineTailLength(TextBuffer* pTB);
int TextBuffer_GetWordLength(TextBuffer* pTB, const int _iEncoding, int *piByteCount);
int TextBuffer_GetWordRLength(TextBuffer* pTB, const int _iEncoding, int *piByteCount);
int TextBuffer_GetCharSequenceLength(TextBuffer* pTB, const char ch, const int iOffsetFrom);
int TextBuffer_Find(TextBuffer* pTB, const LPCSTR lpstr, const int iOffsetFrom);
int TextBuffer_CountWhiteSpaces(TextBuffer* pTB, const int iOffsetFrom);
BOOL TextBuffer_IsWhiteSpaceLine(TextBuffer* pTB, const int iOffsetFrom, int* piLineLength);
BOOL TextBuffer_IsTextAtPos(TextBuffer* pTB, const LPCSTR lpstr, const int iOffsetFrom);
BOOL TextBuffer_IsAnyCharAtPos_RequireSpecial(TextBuffer* pTB, LPCSTR lpChars, LPCSTR lpstrSpecial, const int iOffsetFrom);
BOOL TextBuffer_IsPosOKImpl(TextBuffer* pTB, const int requiredChars);
BOOL TextBuffer_IsPosOK(TextBuffer* pTB, RecodingAlgorithm* pRA);
void TextBuffer_IncPos(TextBuffer* pTB);
void TextBuffer_DecPos(TextBuffer* pTB);
void TextBuffer_OffsetPos(TextBuffer* pTB, const int iOffset);
int TextBuffer_CountTrailingWhiteSpaces(TextBuffer* pTB, const int offset);
char TextBuffer_GetChar(TextBuffer* pTB);
char TextBuffer_GetCharAt(TextBuffer* pTB, const int iOffset);
char TextBuffer_PopChar(TextBuffer* pTB);
BOOL TextBuffer_PushChar(TextBuffer* pTB, const char ch);
BOOL TextBuffer_PushHexChar(EncodingData* pED, const unsigned char ch);
BOOL TextBuffer_PushNonZeroChar(TextBuffer* pTB, const char ch);
BOOL TextBuffer_GetLiteralChar(TextBuffer* pTB, char* pCh, long* piCharsProcessed);
BOOL TextBuffer_IsEOL(TextBuffer* pTB, const int iEOLMode);
void TextBuffer_AddEOL(TextBuffer* pTB, const int iEOLMode);

typedef enum _ERecodingType
{
  ERT_HEX,
  ERT_BASE64,
  ERT_QP,
  ERT_URL,
  ERT_CALW
} ERecodingType;

BOOL RecodingAlgorithm_ShouldBreak(const RecodingAlgorithm* pRA);
BOOL RecodingAlgorithm_ShouldBreakEncoding(const RecodingAlgorithm* pRA);
BOOL RecodingAlgorithm_CanUseHWNDForReading(const RecodingAlgorithm* pRA);
BOOL RecodingAlgorithm_CanUseHWNDForWriting(const RecodingAlgorithm* pRA);
BOOL RecodingAlgorithm_Init(RecodingAlgorithm* pRA, const ERecodingType rt,
  const BOOL isEncoding, const int iAdditionalData1, const int iAdditionalData2, const int iAdditionalData3);
BOOL RecodingAlgorithm_Release(RecodingAlgorithm* pRA);

void StringSource_InitFromString(StringSource* pSS, LPCSTR text, const int textLength);
void StringSource_InitFromHWND(StringSource* pSS, const HWND hwnd);
long StringSource_GetSelectionStart(const StringSource* pSS);
long StringSource_GetSelectionEnd(const StringSource* pSS);
long StringSource_GetLineStart(const StringSource* pSS, const int iPos);
long StringSource_GetLineEnd(const StringSource* pSS, const int iPos);
long StringSource_GetLength(const StringSource* pSS);
char StringSource_GetCharAt(const StringSource* pSS, const RecodingAlgorithm* pRA, const int iPos);
long StringSource_IsDataPortionAvailable(const StringSource* pSS, const RecodingAlgorithm* pRA, EncodingData* pED);
BOOL StringSource_GetText(StringSource* pSS, const RecodingAlgorithm* pRA, LPSTR pText, const long iStart, const long iEnd);

void Recode_Run(RecodingAlgorithm* pRA, StringSource* pSS, const int bufferSize);
BOOL Recode_ProcessDataPortion(RecodingAlgorithm* pRA, StringSource* pSS, EncodingData* pED);

BOOL IsCharFromString(LPCSTR lpstrSample, const unsigned char ch);
BOOL IsEOLChar(const unsigned char ch);
BOOL IsTrailingEOL(const int eolMode, const unsigned char ch, TextBuffer* pTB);
BOOL GetTrailingEOLLength(const int eolMode);
