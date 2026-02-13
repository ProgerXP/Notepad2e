#include "stdafx.h"
#include <assert.h>
#include <iomanip>
#include <Shlwapi.h>
#include "CppUnitTest.h"
#include "../src/Extension/Externals.h"
#include "../src/Extension/StrToHex.h"
#include "../src/Extension/StrToBase64.h"
#include "../src/Extension/StrToQP.h"
#include "../src/Extension/StrToURL.h"
#include "../src/Extension/CommentAwareLineWrapping.h"
#include "../scintilla/include/SciLexer.h"
#include "CppUnitTest.h"
#include "TextEncodingTestCaseData.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

typedef LPCSTR (TWorkingProc)(LPCSTR, const int, const int, const int, const int, const int, const int, int*);

#define MIN_BUFFER_SIZE 8
#define MAX_BUFFER_SIZE 65536
#define BUFFER_TEST_COUNT 3

std::wstring GetCharWString(const char ch)
{
  switch (ch)
  {
  case 10:
    return L"\\n";
  case 13:
    return L"\\r";
  case '\t':
    return L"\\t";
  default:
    {
      std::wstringstream ss;
      if (isascii(ch) && (ch > 0x20))
      {
        ss << ch;
      }
      else
      {
        ss << L"0x" << std::hex << (unsigned char)ch;
      }
      return ss.str();
    }
  }
}

std::wstring GetStringDiff(const std::string& expected, const std::string& actual)
{
  std::wstringstream ss;
  ss << std::endl << L"--- String difference ---" << std::endl;
  if (expected.size() != actual.size())
  {
    ss << L"Expected Length: " << expected.size() << L", Actual Length: " << actual.size() << std::endl;
  }
  else
  {
    ss << L"Correct Length: " << expected.length() << std::endl;
  }

  ss << L"Offset  |   Expected   |   Actual" << std::endl;
  const int sizeDiff = actual.size() - expected.size();
  const int maxLength = (sizeDiff > 0) && (sizeDiff < 10) ? actual.size() : min(expected.size(), actual.size());
  const int maxLines = 50;
  int count = 1;
  for (int i = 0; i < maxLength; ++i)
  {
    const char _expectedChar = (i < (int)expected.size()) ? expected[i] : '-';
    const char _actualChar = (i < (int)actual.size()) ? actual[i] : '-';
    if (_expectedChar != _actualChar)
    {
      ss << std::setw(9) << i << L"   " << std::setw(9) << GetCharWString(_expectedChar) << L"   " << std::setw(9) << GetCharWString(_actualChar) << std::endl;
      ++count;
      if (count >= maxLines)
        break;
    }
  }

  return ss.str();
}

static void DoRecodingTest(TWorkingProc proc, const bool isEncoding, const CTestCaseData* pData, const int count, const bool testBufferSize)
{
  srand(GetTickCount());
  bool continueTesting = true;
  int bufferSize = testBufferSize ? MIN_BUFFER_SIZE : -1;
  int bufferTestCount = 0;
  while (continueTesting)
  {
    for (auto i = 0; i < count; i++)
    {
      const auto info = pData[i];
      const int bufferSizeOld = bufferSize;
      if (!isEncoding && info.IsDecodeOnly() && (bufferSize > 0))
      {
        if (bufferSize < info.GetDecodeOnlyMinBufferSize())
        {
          bufferSize = info.GetDecodeOnlyMinBufferSize();
        }
        assert(bufferSize >= bufferSizeOld);
      }

      std::wstringstream ss;
      ss << std::endl << "Data sample index: " << i << std::endl;
      ss << (isEncoding ? L"Encoding" : L"Decoding") << L"/";
      ss << bufferSize << L"/";
      ss << L"Target encoding: " << info.GetEncodingName() << L"/";

      if (isEncoding && !info.IsDecodeOnly())
      {
        ss << L"Source: " << CPtoUCS2(StringFromVector(info.GetPlainSource()), CP_ACP) << std::endl;

        int resultLength = 0;
        const auto srcData = info.GetSourceText();
        LPCSTR result = proc((LPCSTR)srcData.data(),
                                srcData.size(),
                                info.GetEncoding(),
                                std::get<0>(info.GetAdditionalData()),
                                std::get<1>(info.GetAdditionalData()),
                                std::get<2>(info.GetAdditionalData()),
                                bufferSize,
                                &resultLength);
        if (info.GetExpectedResultText() != VectorFromString(result, resultLength))
        {
          const auto msg = ss.str() + GetStringDiff(StringFromVector(info.GetExpectedResultText()), result);
          Assert::Fail(msg.c_str());
        }
      }
      else if (!isEncoding)
      {
        ss << L"Source: " << CPtoUCS2(StringFromVector(info.GetPlainResult()), CP_ACP) << std::endl;

        int resultLength = 0;
        const auto srcData = info.GetExpectedResultText();
        LPCSTR result = proc((LPCSTR)srcData.data(),
                             srcData.size(),
                             info.GetEncoding(),
                             std::get<0>(info.GetAdditionalData()),
                             std::get<1>(info.GetAdditionalData()),
                             std::get<2>(info.GetAdditionalData()),
                             bufferSize,
                             &resultLength);
        if (info.GetSourceText() != VectorFromString(result, resultLength))
        {
          const auto msg = ss.str() + GetStringDiff(StringFromVector(info.GetSourceText()), result);
          Assert::Fail(msg.c_str());
        }
      }
      bufferSize = bufferSizeOld;
    }
    bufferSize = max(MIN_BUFFER_SIZE, min(MAX_BUFFER_SIZE, rand()));
    ++bufferTestCount;
    if (!testBufferSize || (bufferTestCount >= BUFFER_TEST_COUNT))
    {
      continueTesting = false;
    }
  }
};

namespace Notepad2eTests
{
  TEST_CLASS(CStringToHex)
  {
  public:
    TEST_METHOD(TestHex_StringSamples)
    {
      const CTestCaseData data[] = {
          CTestCaseData(false, "test", CPI_DEFAULT, "74657374"),
          CTestCaseData(false, "test", CPI_UNICODE, "0074006500730074"),
          CTestCaseData(false, L"тестовая строка", CPI_UTF8, "D182 D0B  5D181D1\r\n82D0BED  0B2D 0B0  D18F20\rD1\n   81D \r182D1  80D0BED    0BAD 0 B 0", true/*test decode only*/, MIN_BUFFER_SIZE*10),
          CTestCaseData(false, L"тестовая строка", CPI_UTF8, "D182D0B5D181D182D0BED0B2D0B0D18F20D181D182D180D0BED0BAD0B0"),
          CTestCaseData(false, UCS2toCP(L"тестовая строка", CP_WINDOWS_1251), CPI_DEFAULT, "F2E5F1F2EEE2E0FF20F1F2F0EEEAE0"),
          CTestCaseData(false, UCS2toCP(L"test string", CP_WINDOWS_1250), CPI_DEFAULT, "7465737420737472696E67"),
          CTestCaseData(false, UCS2toCP(L"ハローワールド", CP_SHIFT_JIS), CPI_DEFAULT, "836E838D815B838F815B838B8368")
      };
      DoRecodingTest(EncodeStringToHex, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeHexToString, false, &data[0], _countof(data), false);
    }

    TEST_METHOD(TestHex_FileSamples)
    {
      const CTestCaseData data[] = {
        CTestCaseData(true, "TestFile1__src_UTF8.txt", CPI_UTF8, "TestFile1_Hex_UTF8.txt"),
        CTestCaseData(true, "TestFile1__src_UTF8.txt", CPI_UNICODE, "TestFile1_Hex_UnicodeLE.txt"),
        CTestCaseData(true, "TestFile1__src_UTF8.txt", CPI_UNICODE, "TestFile1_Hex_UnicodeLE_impure.txt", true, MIN_BUFFER_SIZE*30),
        CTestCaseData(true, "TestFile1__src_UTF8_big.txt", CPI_UNICODE, "TestFile1_Hex_UnicodeLE_big.txt"),
        CTestCaseData(true, "TestFile1__src_SHIFT-JIS.txt", CPI_DEFAULT, "TestFile1_Hex_SHIFT-JIS.txt"),
        CTestCaseData(true, "TestFile1__src_1251.txt", CPI_DEFAULT, "TestFile1_Hex_1251.txt"),
        CTestCaseData(true, "TestFile1__src_1250.txt", CPI_DEFAULT, "TestFile1_Hex_1250.txt"),
      };
      DoRecodingTest(EncodeStringToHex, true, &data[0], _countof(data), true/*heavy testing, use random buffer size*/);
      DoRecodingTest(DecodeHexToString, false, &data[0], _countof(data), true);
    }
  };

  TEST_CLASS(CStringToBase64)
  {
  public:
    TEST_METHOD(TestBase64_StringSamples)
    {
      const CTestCaseData data[] = {
        CTestCaseData(false, "test", CPI_DEFAULT, "dGVzdA=="),
        CTestCaseData(false, "test", CPI_UTF8, "dGVzdA=="),
        CTestCaseData(false, VectorFromString("t\0e\0s\0t\0s\0\0\0t\0r\0i\0n\0g\0", 22), CPI_DEFAULT, "dABlAHMAdABzAAAAdAByAGkAbgBnAA=="),
        CTestCaseData(false, L"тестовая строка", CPI_UTF8, "0YLQtdGB0YLQvtCy0LDRjyDRgdGC0YDQvtC60LA="),
        CTestCaseData(false, L"тестовая строка", CPI_UTF8, "0   Y     L    Q\r\n  t   d    GB 0   YL   Q   \rvtC\n\ny0L\tDRj   yDRgdGC0YDQvtC60LA=", true/*decodeOnly*/),
        CTestCaseData(false, L"тестовая строка кириллица-1251", CPI_WINDOWS_1251, "8uXx8u7i4P8g8fLw7urgIOro8Ojr6+j24C0xMjUx"),
        CTestCaseData(false, L"тестовая строка кириллица-KOI8-R", CPI_WINDOWS_KOI8_R, "1MXT1M/XwdEg09TSz8vBIMvJ0snMzMnDwS1LT0k4LVI="),
        CTestCaseData(false, "Base64 is a generic term for a number of similar encoding schemes that encode binary data by treating it numerically and translating it into a base 64 representation. The Base64 term originates from a specific MIME content transfer encoding.",
                      CPI_DEFAULT,
                      "QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciBhIG51bWJlciBvZiBzaW1pbGFyIGVuY29kaW5nIHNjaGVtZXMgdGhhdCBlbmNvZGUgYmluYXJ5IGRhdGEgYnkgdHJlYXRpbmcgaXQgbnVtZXJpY2FsbHkgYW5kIHRyYW5zbGF0aW5nIGl0IGludG8gYSBiYXNlIDY0IHJlcHJlc2VudGF0aW9uLiBUaGUgQmFzZTY0IHRlcm0gb3JpZ2luYXRlcyBmcm9tIGEgc3BlY2lmaWMgTUlNRSBjb250ZW50IHRyYW5zZmVyIGVuY29kaW5nLg==")
      };
      DoRecodingTest(EncodeStringToBase64, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeBase64ToString, false, &data[0], _countof(data), false);
    }

    TEST_METHOD(TestBase64_FileSamples)
    {
      const CTestCaseData data[] = {
        CTestCaseData(true, "TestFile1__src_UTF8.txt", CPI_UTF8, "TestFile1_Base64_UTF8.txt"),
        CTestCaseData(true, "TestFile1__src_UTF8_big.txt", CPI_UTF8, "TestFile1_Base64_UTF8_big.txt"),
        CTestCaseData(true, "TestFile1__src_SHIFT-JIS.txt", CPI_DEFAULT, "TestFile1_Base64_SHIFT-JIS.txt"),
        CTestCaseData(true, "TestFile1__src_1251.txt", CPI_DEFAULT, "TestFile1_Base64_1251.txt"),
        CTestCaseData(true, "TestFile1__src_1250.txt", CPI_DEFAULT, "TestFile1_Base64_1250.txt"),
      };
      DoRecodingTest(EncodeStringToBase64, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeBase64ToString, false, &data[0], _countof(data), false);
    }
  };

  TEST_CLASS(CStringToQP)
  {
  public:
    TEST_METHOD(TestQP_StringSamples)
    {
      const CTestCaseData data[] = {
        CTestCaseData(false, "test string", CPI_DEFAULT, "test string"),
        CTestCaseData(false, L"тестовая строка", CPI_UTF8, "=D1=82=D0=B5=D1=81=D1=82=D0=BE=D0=B2=D0=B0=D1=8F =D1=81=D1=82=D1=80=D0=BE=\r\n"
                                                           "=D0=BA=D0=B0"),
        CTestCaseData(false, "trailing space test  \r\nwith new    line    test    ", CPI_DEFAULT, "trailing space test  =0D=0Awith new    line    test   =20"),
        CTestCaseData(false, VectorFromString("t\0e\0s\0t\0s\0\0\0t\0r\0i\0n\0g\0", 22), CPI_DEFAULT, "t=00e=00s=00t=00s=00=00=00t=00r=00i=00n=00g=00"),
      };
      DoRecodingTest(EncodeStringToQP, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeQPToString, false, &data[0], _countof(data), false);
    }

    TEST_METHOD(TestQP_FileSamples)
    {
      const CTestCaseData data[] = {
        CTestCaseData(true, "TestFile1__src_UTF8.txt", CPI_UTF8, "TestFile1_QP_UTF8.txt"),
        CTestCaseData(true, "TestFile1__src_UTF8_big.txt", CPI_UTF8, "TestFile1_QP_UTF8_big.txt"),
        CTestCaseData(true, "TestFile1__src_SHIFT-JIS.txt", CPI_DEFAULT, "TestFile1_QP_SHIFT-JIS.txt"),
        CTestCaseData(true, "TestFile1__src_1251.txt", CPI_DEFAULT, "TestFile1_QP_1251.txt"),
        CTestCaseData(true, "TestFile1__src_1250.txt", CPI_DEFAULT, "TestFile1_QP_1250.txt"),
      };
      DoRecodingTest(EncodeStringToQP, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeQPToString, false, &data[0], _countof(data), false);
    }
  };

  TEST_CLASS(CStringToURL)
  {
  public:
    TEST_METHOD(TestURL_StringSamples)
    {
      const CTestCaseData data[] = {
        CTestCaseData(false, "http://site.com/?foo=bar", CPI_DEFAULT, "http%3A%2F%2Fsite.com%2F%3Ffoo%3Dbar"),
        CTestCaseData(false, "test string", CPI_DEFAULT, "test%20string"),
        CTestCaseData(false, L"тестовая строка", CPI_UTF8, "%D1%82%D0%B5%D1%81%D1%82%D0%BE%D0%B2%D0%B0%D1%8F%20%D1%81%D1%82%D1%80%D0%BE%D0%BA%D0%B0"),
        CTestCaseData(false, "trailing space test  \r\nwith new    line    test    ", CPI_DEFAULT, "trailing%20space%20test%20%20%0D%0Awith%20new%20%20%20%20line%20%20%20%20test%20%20%20%20"),
        CTestCaseData(false, VectorFromString("t\0e\0s\0t\0s\0\0\0t\0r\0i\0n\0g\0", 22), CPI_DEFAULT, "t%00e%00s%00t%00s%00%00%00t%00r%00i%00n%00g%00")
      };
      DoRecodingTest(EncodeStringToURL, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeURLToString, false, &data[0], _countof(data), false);
    }

    TEST_METHOD(TestURL_FileSamples)
    {
      const CTestCaseData data[] = {
        CTestCaseData(true, "TestFile1__src_UTF8.txt", CPI_UTF8, "TestFile1_URL_UTF8.txt"),
        CTestCaseData(true, "TestFile1__src_UTF8_big.txt", CPI_UTF8, "TestFile1_URL_UTF8_big.txt"),
        CTestCaseData(true, "TestFile1__src_SHIFT-JIS.txt", CPI_DEFAULT, "TestFile1_URL_SHIFT-JIS.txt"),
        CTestCaseData(true, "TestFile1__src_1251.txt", CPI_DEFAULT, "TestFile1_URL_1251.txt"),
        CTestCaseData(true, "TestFile1__src_1250.txt", CPI_DEFAULT, "TestFile1_URL_1250.txt"),
      };
      DoRecodingTest(EncodeStringToURL, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeURLToString, false, &data[0], _countof(data), false);
    }
  };

#define ENABLE_ALL_TESTS

#ifdef ENABLE_ALL_TESTS
#define ENABLE_UNICODE_TEST
#define ENABLE_LONG_TESTS
#define ENABLE_SHORT_TESTS
#define ENABLE_COMPOSITE_TESTS
#endif

//  #define ENABLE_SHORT_TESTS
//  #define ENABLE_COMPOSITE_TESTS

//#define ENABLE_NEW_TEST

  TEST_CLASS(CCommentAwareLineWrapping)
  {
  public:
    TEST_METHOD(TestCALW_StringSamples)
    {
      const CTestCaseData data[] = {

#ifdef ENABLE_UNICODE_TEST
        CTestCaseData(false, UCS2toCP(L"   𝕷𝖔𝖗𝖊𝖒 𝖎𝖕𝖘𝖚𝖒 𝖉𝖔𝖑𝖔𝖗 𝖘𝖎𝖙 𝖆𝖒𝖊𝖙, 𝖈𝖔𝖓𝖘𝖊𝖈𝖙𝖊𝖙𝖚𝖗 𝖆𝖉𝖎𝖕𝖎𝖘𝖈𝖎𝖓𝖌\r\n"
                                      L"   𝖊𝖑𝖎𝖙, 𝖘𝖊𝖉 𝖉𝖔 𝖊𝖎𝖚𝖘𝖒𝖔𝖉 𝖙𝖊𝖒𝖕𝖔𝖗 𝖎𝖓𝖈𝖎𝖉𝖎𝖉𝖚𝖓𝖙 𝖚𝖙 𝖑𝖆𝖇𝖔𝖗𝖊\r\n"
                                      L"   𝖊𝖙 𝖉𝖔𝖑𝖔𝖗𝖊 𝖒𝖆𝖌𝖓𝖆 𝖆𝖑𝖎𝖖𝖚𝖆. 𝖀𝖙 𝖊𝖓𝖎𝖒 𝖆𝖉 𝖒𝖎𝖓𝖎𝖒 𝖛𝖊𝖓𝖎𝖆𝖒,", CP_UTF8),
                CPI_UTF8,
                             UCS2toCP(L"   𝕷𝖔𝖗𝖊𝖒 𝖎𝖕𝖘𝖚𝖒 𝖉𝖔𝖑𝖔𝖗 𝖘𝖎𝖙 𝖆𝖒𝖊𝖙,\r\n"
                                      L"   𝖈𝖔𝖓𝖘𝖊𝖈𝖙𝖊𝖙𝖚𝖗 𝖆𝖉𝖎𝖕𝖎𝖘𝖈𝖎𝖓𝖌 𝖊𝖑𝖎𝖙, 𝖘𝖊𝖉 𝖉𝖔\r\n"
                                      L"   𝖊𝖎𝖚𝖘𝖒𝖔𝖉 𝖙𝖊𝖒𝖕𝖔𝖗 𝖎𝖓𝖈𝖎𝖉𝖎𝖉𝖚𝖓𝖙 𝖚𝖙 𝖑𝖆𝖇𝖔𝖗𝖊\r\n"
                                      L"   𝖊𝖙 𝖉𝖔𝖑𝖔𝖗𝖊 𝖒𝖆𝖌𝖓𝖆 𝖆𝖑𝖎𝖖𝖚𝖆. 𝖀𝖙 𝖊𝖓𝖎𝖒 𝖆𝖉\r\n"
                                      L"   𝖒𝖎𝖓𝖎𝖒 𝖛𝖊𝖓𝖎𝖆𝖒,", CP_UTF8),
				false, 0, { 40, SCLEX_NULL, SC_EOL_CRLF }),
#endif

#ifdef ENABLE_LONG_TESTS

        CTestCaseData(false, "    // aa aa\r\n"
                             "    // aa aa\r\n"
                             "    //\r\n"
                             "    //\r\n"
                             "    //       \r\n"
                             "    // aa aa",
                CPI_DEFAULT,
                             "    // aa aa aa\r\n"
                             "    // aa\r\n"
                             "    //\r\n"
                             "    //\r\n"
                             "    //\r\n"
                             "    // aa aa",
                false, 0, { 15, SCLEX_CPP, SC_EOL_CRLF }),
        
        CTestCaseData(false, "  // aa aa aa\r\n"
                             "  // aa aa aa\r\n"
                             "  //      \r\n"
                             "  // aa aa aa",
                CPI_DEFAULT,
                             "  // aa aa\r\n"
                             "  // aa aa\r\n"
                             "  // aa aa\r\n"
                             "  //\r\n"
                             "  // aa aa\r\n"
                             "  // aa",
                false, 0, { 10, SCLEX_CPP, SC_EOL_CRLF }),
       
        CTestCaseData(false, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
                             "sed do eiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmod",
                CPI_DEFAULT,
                             "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do\r\n"
                             "eiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmodeiusmod",
                false, 0, { 80, SCLEX_NULL, SC_EOL_CRLF }),

        CTestCaseData(false, "    Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "    elit, sed do eiusmod tempor incididunt ut labore\r\n"
                             "    et dolore magna aliqua. Ut enim ad minim veniam,",
                CPI_DEFAULT,
                             "    Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "    elit, sed do eiusmod tempor incididunt ut labore et\r\n"
                             "    dolore magna aliqua. Ut enim ad minim veniam,",
                false, 0, { 55, SCLEX_NULL, SC_EOL_CRLF }),

        CTestCaseData(false, "    Lorem ipsum dolor sit amet, consectetur adipiscing\r"
                             "    elit, sed do eiusmod tempor incididunt ut labore\r"
                             "    et dolore magna aliqua. Ut enim ad minim veniam,",
                CPI_DEFAULT,
                             "    Lorem ipsum dolor sit amet, consectetur adipiscing\r"
                             "    elit, sed do eiusmod tempor incididunt ut labore et\r"
                             "    dolore magna aliqua. Ut enim ad minim veniam,",
                false, 0, { 55, SCLEX_NULL, SC_EOL_CR }),

        CTestCaseData(false, "   Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "   elit, sed do eiusmod tempor incididunt ut labore\r\n"
                             "   et dolore magna aliqua. Ut enim ad minim veniam,",
                CPI_DEFAULT,
                             "   Lorem ipsum dolor sit amet, consectetur adipiscing elit,\r\n"
                             "   sed do eiusmod tempor incididunt ut labore et dolore\r\n"
                             "   magna aliqua. Ut enim ad minim veniam,",
                false, 0, { 60, SCLEX_NULL, SC_EOL_CRLF }),

        CTestCaseData(false, "  Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "  elit, sed do eiusmod tempor incididunt ut labore\r\n"
                             "  et dolore magna aliqua. Ut enim ad minim veniam,",
                CPI_DEFAULT,
                             "  Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod\r\n"
                             "  tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim\r\n"
                             "  veniam,",
                false, 0, { 75, SCLEX_NULL, SC_EOL_CRLF }),

        CTestCaseData(false, "    // Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "   // elit, sed do eiusmod tempor incididunt ut labore\r\n"
                             "// et dolore magna aliqua. Ut enim ad minim veniam,",
                CPI_DEFAULT,
                             "    // Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "    // elit, sed do eiusmod tempor incididunt ut labore et\r\n"
                             "    // dolore magna aliqua. Ut enim ad minim veniam,",
                false, 0, { 58, SCLEX_CPP, SC_EOL_CRLF }),
                
        CTestCaseData(false, "//Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "   //elit, sed do eiusmod tempor incididunt ut labore\r\n"
                             " //et dolore magna aliqua. Ut enim ad minim veniam,",
                CPI_DEFAULT,
                             "//Lorem ipsum dolor sit amet, consectetur\r\n"
                             "//adipiscing elit, sed do eiusmod tempor\r\n"
                             "//incididunt ut labore et dolore magna\r\n"
                             "//aliqua. Ut enim ad minim veniam,",
                false, 0, { 41, SCLEX_CPP, SC_EOL_CRLF }),

        CTestCaseData(false, "//Lorem ipsum dolor sit amet, consectetur adipiscing\n"
                             "   //elit, sed do eiusmod tempor incididunt ut labore\n"
                             "//et dolore magna aliqua. Ut enim ad minim veniam,",
                CPI_DEFAULT,
                             "//Lorem ipsum dolor sit amet, consectetur\n"
                             "//adipiscing elit, sed do eiusmod tempor\n"
                             "//incididunt ut labore et dolore magna\n"
                             "//aliqua. Ut enim ad minim veniam,",
                false, 0, { 41, SCLEX_CPP, SC_EOL_LF }),

        CTestCaseData(false, "    # Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "   # elit, sed do eiusmod tempor incididunt ut labore\r\n"
                             "# et dolore magna aliqua. Ut enim ad minim veniam,",
                CPI_DEFAULT,
                             "    # Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "    # elit, sed do eiusmod tempor incididunt ut labore et\r\n"
                             "    # dolore magna aliqua. Ut enim ad minim veniam,",
                false, 0, { 57, SCLEX_PERL, SC_EOL_CRLF }),

        CTestCaseData(false, "  // Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "      // elit, sed do eiusmod tempor incididunt ut labore et\r\n"
                             "// dolore magna aliqua. Ut enim ad minim veniam,",
                CPI_DEFAULT,
                             "  // Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod\r\n"
                             "  // tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim\r\n"
                             "  // veniam,",
                false, 0, { 80, SCLEX_CPP, SC_EOL_CRLF }),

        CTestCaseData(false, "  // Lorem ipsum dolor sit amet, consectetur adipiscing\r\n"
                             "      // elit, sed do eiusmod tempor incididunt ut labore et\r\n"
                             "// dolore magna aliqua. Ut enim ad minim veniam,",
                CPI_DEFAULT,
                             "  // Lorem ipsum dolor sit amet, consectetur\r\n"
                             "  // adipiscing elit, sed do eiusmod tempor\r\n"
                             "  // incididunt ut labore et dolore magna aliqua.\r\n"
                             "  // Ut enim ad minim veniam,",
                false, 0, { 50, SCLEX_CPP, SC_EOL_CRLF }),
#endif

#ifdef ENABLE_SHORT_TESTS
/**/
        CTestCaseData(false, "// a:\n"
                             "// 1.",
              CPI_DEFAULT,
                             "// a:\n"
                             "// 1.",
              false, 0, { 50, SCLEX_CPP, SC_EOL_LF }),

        CTestCaseData(false, "* it\n"
                             "  em",
              CPI_DEFAULT,
                             "* it em",
              false, 0, { 50, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "//* item\n"
                               "//  List",
              CPI_DEFAULT,
                               "//* item List",
              false, 0, { 50, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "//* item\n"
                               "  //  List",
              CPI_DEFAULT,
                               "//* item List",
              false, 0, { 50, SCLEX_CPP, SC_EOL_LF }),

        CTestCaseData(false, "// * Lorem Ipsum",
                CPI_DEFAULT,
                             "// * Lorem\n"
                             "//   Ipsum",
                false, 0, { 10, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false,  "  // \r\n"
                                "  // abc",
              CPI_DEFAULT,
                              "  // abc",
              false, 0, { 15, SCLEX_CPP, SC_EOL_CRLF }),

          CTestCaseData(false,  "  //\n"
                                "  // abc",
              CPI_DEFAULT,
                                "  //\n"
                                "  // abc",
              false, 0, { 15, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false,  "  // > aaa\n"
                                "  //   \t\t\n"
                                "  //  ",
              CPI_DEFAULT,
                              "  // > aaa\n"
                              "  //\n"
                              "  //",
              false, 0, { 15, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "  // > aaa\n"
                               "  //\t\n"
                               " //   \n"
                               "  //",
              CPI_DEFAULT,
                               "  // > aaa\n"
                               "  //\n"
                               "  //\n"
                               "  //",
              false, 0, { 15, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "  //\n"
                               "  //       \n"
                               "// \t\t\n",
                CPI_DEFAULT,
                               "  //\n"
                               "  //\n"
                               "  //\n",
                false, 0, { 20, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, " //\r"
                               "  //\t\t\r"
                               "//",
                CPI_DEFAULT,
                               " //\r"
                               " //\r"
                               " //",
                false, 0, { 20, SCLEX_CPP, SC_EOL_CR }),

          CTestCaseData(false, "  // aa bb\n"
                               "  //\n"
                               "// cc dd",
                CPI_DEFAULT,
                               "  // aa bb\n"
                               "  //\n"
                               "  // cc dd",
                false, 0, { 20, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, " //\r\n"
                               "  // aa bb\r\n"
                               " // cc dd",
                CPI_DEFAULT,
                               " //\r\n"
                               " // aa bb cc dd",
                false, 0, { 50, SCLEX_CPP, SC_EOL_CRLF }),

          CTestCaseData(false, "// e.g. foo bar",
                CPI_DEFAULT,
                               "// e.g.\r\n"
                               "// foo\r\n"
                               "// bar",
                false, 0, { 5, SCLEX_CPP, SC_EOL_CRLF }),

          CTestCaseData(false, "// aa\r\n"
                               "// passing",
                CPI_DEFAULT,
                               "// aa passing",
                false, 0, { 50, SCLEX_CPP, SC_EOL_CRLF }),

        CTestCaseData(false, "  // aa\r\n"
                             "  //\r\n"
                             "//s",
                CPI_DEFAULT,
                             "  // aa\r\n"
                             "  //\r\n"
                             "  //s",
                false, 0, { 50, SCLEX_CPP, SC_EOL_CRLF }),

        CTestCaseData(false, "  // aa\r"
                             "  // aa\r"
                             "  //\r"
                             "  //\r"
                             "//",
                CPI_DEFAULT,
                             "  // aa aa\r"
                             "  //\r"
                             "  //\r"
                             "  //",
                false, 0, { 50, SCLEX_CPP, SC_EOL_CR }),

        CTestCaseData(false, "    Lorem\r"
                             "    \r"
                             "    Ipsum",
                CPI_DEFAULT,
                             "    Lorem\r"
                             "\r"
                             "    Ipsum",
                false, 0, { 50, SCLEX_NULL, SC_EOL_CR }),

        CTestCaseData(false, "    Lorem\r\n"
                             "    \r\n"
                             "    Ipsum",
                CPI_DEFAULT,
                             "    Lorem\r\n"
                             "\r\n"
                             "    Ipsum",
                false, 0, { 50, SCLEX_NULL, SC_EOL_CRLF }),

        CTestCaseData(false, "    Lorem\r\n"
                             "\r\n"
                             "    Ipsum",
                CPI_DEFAULT,
                             "    Lorem\r\n"
                             "\r\n"
                             "    Ipsum",
                false, 0, { 50, SCLEX_NULL, SC_EOL_CRLF }),
                                 /**/
        CTestCaseData(false, "  * Lorem\r\n"
                             "* Ipsum",
                CPI_DEFAULT,
                             "  * Lorem\r\n"
                             "* Ipsum",
                false, 0, { 50, SCLEX_NULL, SC_EOL_CRLF }),

        CTestCaseData(false, "    * Lorem\r\n"
                             "         * Ipsum\r\n"
                             "      *  Dolor",
                CPI_DEFAULT,
                             "    * Lorem\r\n"
                             "         * Ipsum\r\n"
                             "    * Dolor",
                false, 0, { 50, SCLEX_NULL, SC_EOL_CRLF }),

        CTestCaseData(false, "    // Lorem\r\n"
                             "  //*Ipsum",
                CPI_DEFAULT,
                             "    // Lorem\r\n"
                             "    //*Ipsum",
                false, 0, { 50, SCLEX_CPP, SC_EOL_CRLF }),

        CTestCaseData(false, "    // Lorem\n"
                             "      //*Ipsum",
                CPI_DEFAULT,
                             "    // Lorem\n"
                             "    //*Ipsum",
                false, 0, { 50, SCLEX_CPP, SC_EOL_LF }),

        CTestCaseData(false, "    // Lorem\r\n"
                             "      //*Ipsum",
                CPI_DEFAULT,
                             "    // Lorem\r\n"
                             "    //*Ipsum",
                false, 0, { 50, SCLEX_CPP, SC_EOL_CRLF }),

		    CTestCaseData(false, "    ; Lorem\r\n"
                             "      ;*Ipsum",
                CPI_DEFAULT,
                             "    ; Lorem\r\n"
                             "    ;*Ipsum",
    				    false, 0, { 50, SCLEX_PROPERTIES, SC_EOL_CRLF }),
          /**/
        CTestCaseData(false, "aa\r\n"
                             "bb\r\n"
                             "* cc\r\n"
                             "  dd\r\n"
                             "\r\n"
                             "ee",
                CPI_DEFAULT,
                             "aa bb\r\n"
                             "* cc dd\r\n"
                             "\r\n"
                             "ee",
                false, 0, { 50, SCLEX_NULL, SC_EOL_CRLF }),
          /**/
        CTestCaseData(false, "*  Lorem ipsum",
                CPI_DEFAULT,
                             "*  Lorem\r\n"
                             "   ipsum",
                false, 0, { 5, SCLEX_NULL, SC_EOL_CRLF }),
  /**/
        CTestCaseData(false, "*\t\tLorem ipsum",
                CPI_DEFAULT,
                             "*\t\tLorem\r\n"
                             " \t\tipsum",
                false, 0, { 5, SCLEX_NULL, SC_EOL_CRLF }),

        CTestCaseData(false, "*Lorem ipsum",
                CPI_DEFAULT,
                             "*Lorem\r\n"
                             " ipsum",
                false, 0, { 5, SCLEX_NULL, SC_EOL_CRLF }),
                
          CTestCaseData(false, ">Lorem ipsum",
                CPI_DEFAULT,
                               ">Lorem\r\n"
                               " ipsum",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, " # Lorem ipsum",
                CPI_DEFAULT,
                               " # Lorem\r\n"
                               "   ipsum",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, ">   Lorem ipsum",
                CPI_DEFAULT,
                               ">   Lorem\r\n"
                               "    ipsum",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),
                
          CTestCaseData(false, "  >\t\tLorem ipsum dolor sit amet, consectetur",
                CPI_DEFAULT,
                               "  >\t\tLorem ipsum\r\n"
                               "   \t\tdolor sit amet,\r\n"
                               "   \t\tconsectetur",
                false, 0, { 20, SCLEX_NULL, SC_EOL_CRLF }),
          /**/
          CTestCaseData(false, "12.  Lorem ipsum",
                CPI_DEFAULT,
                             "12.  Lorem\r\n"
                             "     ipsum",
                false, 0, { 5, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, "123.   Lorem ipsum, dolor sit amet, consectetur",
                CPI_DEFAULT,
                               "123.   Lorem ipsum,\r\n"
                               "       dolor sit amet,\r\n"
                               "       consectetur",
                false, 0, { 23, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, "123)   Lorem ipsum, dolor sit amet, consectetur",
                CPI_DEFAULT,
                               "123)   Lorem ipsum,\n"
                               "       dolor sit amet,\n"
                               "       consectetur",
                false, 0, { 22, SCLEX_NULL, SC_EOL_LF }),
            /**/
          CTestCaseData(false, "   1234: Lorem ipsum",
                CPI_DEFAULT,
                               "   1234: Lorem\r\n"
                               "         ipsum",
                false, 0, { 5, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, "   1234: Lorem ipsum",
                CPI_DEFAULT,
                               "   1234: Lorem\r"
                               "         ipsum",
                false, 0, { 5, SCLEX_NULL, SC_EOL_CR }),

          CTestCaseData(false, "  123)   Lorem ipsum, dolor sit amet, consectetur",
                CPI_DEFAULT,
                               "  123)   Lorem\r\n"
                               "         ipsum,\r\n"
                               "         dolor\r\n"
                               "         sit\r\n"                          
                               "         amet,\r\n"
                               "         consectetur",
                false, 0, { 5, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, "  * aa aa",
                CPI_DEFAULT,
                               "  * aa\r\n"
                               "    aa",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),
            /**/
          CTestCaseData(false, "aaa aa\r\n"
                               "aaa",
                CPI_DEFAULT,
                               "aaa\r\n"
                               "aa\r\n"
                               "aaa",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, "aaa aa\r\n"
                               "aaaaa",
                CPI_DEFAULT,
                               "aaa\r\n"
                               "aa\r\n"
                               "aaaaa",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),
            /**/
          CTestCaseData(false, "// aa \r"
                               "// aa",
                CPI_DEFAULT,
                               "// aa aa",
                false, 0, { 10, SCLEX_CPP, SC_EOL_CR }),

          CTestCaseData(false, "* item 1\n"
                               "  * sub item\n"
                               "* item 2",
              CPI_DEFAULT,
                               "* item 1\n"
                               "  * sub\n"
                               "    item\n"
                               "* item 2",
              false, 0, { 8, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "  * item 1\n"
                               "    * sub item\n"
                               " * item 2",
              CPI_DEFAULT,
                               "  * item 1\n"
                               "    * sub item\n"
                               "  * item 2",
              false, 0, { 18, SCLEX_NULL, SC_EOL_LF }),

          CTestCaseData(false, "* item 1\n"
                               "  * sub sub sub sub item\n"
                               "* item 2",
              CPI_DEFAULT,
                               "* item 1\n"
                               "  * sub sub sub sub item\n"
                               "* item 2",
              false, 0, { 24, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "* item 1\n"
                               "  * sub sub sub sub sub item\n"
                               "* item 2",
              CPI_DEFAULT,
                               "* item 1\n"
                               "  * sub sub sub sub sub\n"
                               "    item\n"
                               "* item 2",
              false, 0, { 24, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "// * item 1\n"
                               "//   * sub sub sub sub sub item\n"
                               "// * item 2",
              CPI_DEFAULT,
                               "// * item 1\n"
                               "//   * sub sub sub sub\n"
                               "//     sub item\n"
                               "// * item 2",
              false, 0, { 24, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "* item 1\n"
                               "  * sub item\n"
                               "    * sub sub item 2",
              CPI_DEFAULT,
                               "* item 1\n"
                               "  * sub item\n"
                               "    * sub sub item 2",
              false, 0, { 24, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "// * item 1\n"
                               "//   * sub item\n"
                               "//     * sub sub item 2",
              CPI_DEFAULT,
                               "// * item 1\n"
                               "//   * sub item\n"
                               "//     * sub sub item 2",
              false, 0, { 24, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "  * item 1\n"
                               "    * sub item\n"
                               "   * item 2",
              CPI_DEFAULT,
                               "  * item 1\n"
                               "    * sub item\n"
                               "  * item 2",
              false, 0, { 24, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "  * item 1\n"
                               "    * sub item\n"
                               "* item 2",
              CPI_DEFAULT,
                               "  * item 1\n"
                               "    * sub item\n"
                               "  * item 2",
              false, 0, { 24, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "* *a *b",
              CPI_DEFAULT,
                               "* *a *b",
              false, 0, { 5, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "* *a *b",
              CPI_DEFAULT,
                               "* *a *b",
              false, 0, { 3, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "* a *b",
              CPI_DEFAULT,
                               "* a *b",
              false, 0, { 5, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "* a b *c",
              CPI_DEFAULT,
                               "* a b *c",
              false, 0, { 5, SCLEX_CPP, SC_EOL_CRLF }),

          CTestCaseData(false, "* a b *c",
              CPI_DEFAULT,
                               "* a\n"
                               "  b *c",
              false, 0, { 4, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "// 1. test string\n"
                               "// 2. test string\n",
              CPI_DEFAULT,
                               "// 1. test\n"
                               "//    string\n"
                               "// 2. test\n"
                               "//    string\n",
              false, 0, { 10, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, " // 1. foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo",
              CPI_DEFAULT,
                               " // 1. foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo\r\n"
                               " //    foo foo",
              false, 0, { 80, SCLEX_CPP, SC_EOL_CRLF }),

          CTestCaseData(false, " // 1. foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo 2. foo",
              CPI_DEFAULT,
                               " // 1. foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo 2.\n"
                               " //    foo",
              false, 0, { 70, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, " // . foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo 2. foo",
              CPI_DEFAULT,
                               " // . foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo 2.\n"
                               " // foo",
              false, 0, { 70, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, " // 1. foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo . foo",
              CPI_DEFAULT,
                               " // 1. foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo\n"
                               " //    . foo",
              false, 0, { 70, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, " // 1. foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo , foo",
              CPI_DEFAULT,
                               " // 1. foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo\n"
                               " //    , foo",
              false, 0, { 70, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "## b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b",
              CPI_DEFAULT,
                               "## b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b\r\n"
                               "#  b b b",
              false, 0, { 70, SCLEX_CONF, SC_EOL_CRLF }),

          CTestCaseData(false, "# # abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc",
              CPI_DEFAULT,
                               "# # abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc\n"
                               "#   abc abc abc abc",
              false, 0, { 70, SCLEX_CONF, SC_EOL_LF }),

          CTestCaseData(false, "  # # abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc",
              CPI_DEFAULT,
                               "  # # abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc\n"
                               "  #   abc abc abc abc",
              false, 0, { 70, SCLEX_CONF, SC_EOL_LF }),

          CTestCaseData(false, "  #  1.  abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc",
              CPI_DEFAULT,
                               "  #  1.  abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc\n"
                               "  #      abc abc abc abc abc",
              false, 0, { 70, SCLEX_CONF, SC_EOL_LF }),

          CTestCaseData(false, "rem foo foo\r\n"
                               "rem\r\n"
                               "rem xxxxxxxxxxxxxxxxxxx",
              CPI_DEFAULT,
                               "rem foo foo\r\n"
                               "rem\r\n"
                               "rem xxxxxxxxxxxxxxxxxxx",
              false, 0, { 50, SCLEX_BATCH, SC_EOL_CRLF }),

          CTestCaseData(false, "rem foo foo foo foo\r\n"
                               "rem123\r\n"
                               "rem xxxxxxxxxxxxxxxxxxx",
              CPI_DEFAULT,
                               "rem foo foo\r\n"
                               "rem foo foo\r\n"
                               "rem123\r\n"
                               "rem xxxxxxxxxxxxxxxxxxx",
              false, 0, { 12, SCLEX_BATCH, SC_EOL_CRLF }),

          CTestCaseData(false, "rem foo foo foo foo\r\n"
                            "rem 123\r\n"
                            "rem xxxxxxxxxxxxxxxxxxx",
              CPI_DEFAULT,
                            "rem foo foo foo\r\n"
                            "rem foo 123\r\n"
                            "rem xxxxxxxxxxxxxxxxxxx",
              false, 0, { 15, SCLEX_BATCH, SC_EOL_CRLF }),

          CTestCaseData(false, "rem foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo \n"
                            "rem\n"
                            "rem xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
                            "rem\n"
                            "rem foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo foo\n"
                            "rem foo",
              CPI_DEFAULT,
                            "rem foo foo foo foo foo foo foo foo foo\n"
                            "rem foo foo foo foo foo foo foo foo foo\n"
                            "rem foo foo foo foo foo foo foo foo foo\n"
                            "rem foo foo foo foo foo foo foo\n"
                            "rem\n"
                            "rem xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
                            "rem\n"
                            "rem foo foo foo foo foo foo foo foo foo\n"
                            "rem foo foo foo foo foo foo foo foo foo\n"
                            "rem foo foo foo foo foo foo foo foo foo\n"
                            "rem foo foo foo foo foo foo foo foo",
              false, 0, { 40, SCLEX_BATCH, SC_EOL_LF }),

          CTestCaseData(false, "; foo\r\n"
                            ";\r\n"
                            "; xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
              CPI_DEFAULT,
                            "; foo\r\n"
                            ";\r\n"
                            "; xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
              false, 0, { 50, SCLEX_PROPERTIES, SC_EOL_CRLF }),

          CTestCaseData(false, "# XXXXXXX XXXXXX\n"
                              "# 1. XX\n"
                              "",
              CPI_DEFAULT,
                              "# XXXXXXX XXXXXX\n"
                              "# 1. XX\n"
                              "",
              false, 0, { 50, SCLEX_CONF, SC_EOL_LF }),

          CTestCaseData(false, "# XXXXXXX XXXXXX\n"
                               "# 1. XX\n"
                               "# yyy zzz",
              CPI_DEFAULT,
                              "# XXXXXXX XXXXXX\n"
                              "# 1. XX yyy zzz",
              false, 0, { 50, SCLEX_CONF, SC_EOL_LF }),
#endif

#ifdef ENABLE_COMPOSITE_TESTS
          CTestCaseData(false, "aaa\r\n"
                               "  * aa aa",
                CPI_DEFAULT,
                               "aaa\r\n"
                               "  * aa\r\n"
                               "    aa",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, "aaa\r\n"
                               "  * aa aa\r\n"
                               "\r\n"
                               " bbb",
                CPI_DEFAULT,
                               "aaa\r\n"
                               "  * aa\r\n"
                               "    aa\r\n"
                               "\r\n"
                               " bbb",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, "  * aa aa\r\n"
                               "      * aa aa",
                CPI_DEFAULT,
                               "  * aa\r\n"
                               "    aa\r\n"
                               "      * aa\r\n"
                               "        aa",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, "* aa aa\r\n"
                               "\r\n"
                               "  * bb bb",
                CPI_DEFAULT,
                               "* aa\r\n"
                               "  aa\r\n"
                               "\r\n"
                               "  * bb\r\n"
                               "    bb",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, "aaa aa\r\n"
                               "\r\n"
                               "aaa\r\n"
                               "* aa aa\r\n"
                               "* aa aa\r\n"
                               "\r\n"
                               "aaaaa",
                CPI_DEFAULT,
                               "aaa\r\n"
                               "aa\r\n"
                               "\r\n"
                               "aaa\r\n"
                               "* aa\r\n"
                               "  aa\r\n"
                               "* aa\r\n"
                               "  aa\r\n"
                               "\r\n"
                               "aaaaa",
                false, 0, { 3, SCLEX_NULL, SC_EOL_CRLF }),

          CTestCaseData(false, "  // List:\n"
                               "  //* aa\n"
                               "//* bb",
                CPI_DEFAULT,
                               "  // List:\n"
                               "  //* aa\n"
                               "  //* bb",
                false, 0, { 20, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "  // Test string\n"
                               "     //     test string\n"
                               "  //  * aa\n"
                               "//* bb",
                CPI_DEFAULT,
                               "  // Test string\n"
                               "  //     test\n"
                               "  //     string\n"
                               "  //  * aa\n"
                               "  //* bb",
                false, 0, { 17, SCLEX_CPP, SC_EOL_LF }),
                
          CTestCaseData(false, " * item 1\n"
                               "  * sub item\n"
                               "    * sub sub item\n"
                               "   * sub item 2\n"
                               "* item 2\n",
              CPI_DEFAULT,
                               " * item 1\n"
                               "  * sub item\n"
                               "    * sub sub item\n"
                               "  * sub item 2\n"
                               " * item 2\n",
              false, 0, { 25, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "// * item 1\n"
                               "//  * sub item\n"
                               "//    * sub sub item\n"
                               "//   * sub item 2\n"
                               "//* item 2\n",
              CPI_DEFAULT,
                               "// * item 1\n"
                               "//  * sub item\n"
                               "//    * sub sub item\n"
                               "//  * sub item 2\n"
                               "//* item 2\n",
              false, 0, { 25, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, " line 1  \n"
                               "   2   3   \n"
                               " 4",
              CPI_DEFAULT,
                               " line 1\n"
                               "   2 3\n"
                               " 4",
              false, 0, { 18, SCLEX_NULL, SC_EOL_LF }),

          CTestCaseData(false, "//  line 1  \n"
                               "//   2   3   \n"
                               "//4",
              CPI_DEFAULT,
                               "//  line 1\n"
                               "//   2 3\n"
                               "//4",
              false, 0, { 18, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "line 1  \n"
                               "   line 2   ",
              CPI_DEFAULT,
                               "line 1\n"
                               "   line 2",
              false, 0, { 18, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, " line 1  \n"
                               "   line 2   ",
              CPI_DEFAULT,
                               " line 1\n"
                               "   line 2",
              false, 0, { 18, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "line 1  2   3   ",
              CPI_DEFAULT,
                               "line 1 2 3",
              false, 0, { 18, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, " line 1  \n"
                               "   2   3   \n"
                               "   4",
              CPI_DEFAULT,
                               " line 1\n"
                               "   2 3 4",
              false, 0, { 18, SCLEX_NULL, SC_EOL_LF }),

          CTestCaseData(false, "//  line 1  \n"
                               "//   2   3   \n"
                               "//4",
              CPI_DEFAULT,
                               "//  line 1\n"
                               "//   2 3\n"
                               "//4",
              false, 0, { 18, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "  // line 1  \n"
                               "   line 2   ",
              CPI_DEFAULT,
                               "  // line 1\n"
                               "   line 2",
              false, 0, { 18, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "List:\n"
                               "* item",
              CPI_DEFAULT,
                               "List:\n"
                               "* item",
              false, 0, { 8, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "* item\n"
                               "List:",
              CPI_DEFAULT,
                               "* item\n"
                               "List:",
              false, 0, { 8, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "* item\n"
                               "  List:",
              CPI_DEFAULT,
                               "* item List:",
              false, 0, { 15, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "* item\n"
                               " List:",
              CPI_DEFAULT,
                               "* item\n"
                               " List:",
              false, 0, { 15, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, " * item\n"
                               "List:",
              CPI_DEFAULT,
                               " * item\n"
                               "List:",
              false, 0, { 15, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "  test\n"
                               "   string",
              CPI_DEFAULT,
                               "  test\n"
                               "   string",
              false, 0, { 50, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "  test\n"
                               "  string",
              CPI_DEFAULT,
                               "  test string",
              false, 0, { 50, SCLEX_CPP, SC_EOL_LF }),

          CTestCaseData(false, "//   if (foo) {\n"
                               "//     var bar = 123;\n"
                               "//   }",
              CPI_DEFAULT,
                               "//   if (foo) {\n"
                               "//     var bar = 123;\n"
                               "//   }",
              false, 0, { 21, SCLEX_CPP, SC_EOL_LF }),
#endif

#ifdef ENABLE_NEW_TEST
          CTestCaseData(false, "# XXXXXXX XXXXXX\n"
                              "# 1. XX\n"
                              "",
              CPI_DEFAULT,
                              "# XXXXXXX XXXXXX\n"
                              "# 1. XX\n"
                              "",
              false, 0, { 50, SCLEX_CONF, SC_EOL_LF }),
#endif 
      };
      DoRecodingTest(EncodeStringWithCALW, true, &data[0], _countof(data), false);
    }
  };
}
