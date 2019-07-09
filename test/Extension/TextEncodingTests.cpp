#include "stdafx.h"
#include <assert.h>
#include <Shlwapi.h>
#include "CppUnitTest.h"
#include "../src/Extension/Externals.h"
#include "../src/Extension/StrToHex.h"
#include "../src/Extension/StrToBase64.h"
#include "../src/Extension/StrToQP.h"
#include "../src/Extension/StrToURL.h"
#include "CppUnitTest.h"
#include "TextEncodingTestCaseData.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

typedef LPCSTR (TWorkingProc)(LPCSTR, const int, const int, const int, int*);

#define MIN_BUFFER_SIZE 8
#define MAX_BUFFER_SIZE 65536
#define BUFFER_TEST_COUNT 3

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
      std::wstring errorMessage(isEncoding ? L"Encoding, " : L"Decoding, ");
      errorMessage += info.GetErrorMessageText();
      errorMessage += L", ";
      errorMessage += L" source: " + CPtoUCS2(StringFromVector(info.GetPlainSource()), CP_ACP);
      errorMessage += L"\r\n";
      if (isEncoding && !info.IsDecodeOnly())
      {
        errorMessage += info.GetErrorMessageText();
        int resultLength = 0;
        LPCSTR result = proc((LPCSTR)info.GetSourceText().data(),
                                info.GetSourceText().size(),
                                info.GetEncoding(),
                                bufferSize,
                                &resultLength);
        if (info.GetExpectedResultText() != VectorFromString(result, resultLength))
        {
          Assert::Fail(errorMessage.c_str(), LINE_INFO());
        }
      }
      else if (!isEncoding)
      {
        int resultLength = 0;
        LPCSTR result = proc((LPCSTR)info.GetExpectedResultText().data(),
                             info.GetExpectedResultText().size(),
                             info.GetEncoding(),
                             bufferSize,
                             &resultLength);
        if (info.GetSourceText() != VectorFromString(result, resultLength))
        {
          Assert::Fail(errorMessage.c_str(), LINE_INFO());
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
        CTestCaseData(true, "TestFile1__src_1251.txt", CPI_DEFAULT/*no need to recode file contents*/, "TestFile1_Hex_1251.txt"),
        CTestCaseData(true, "TestFile1__src_SHIFT-JIS.txt", CPI_DEFAULT/*no need to recode file contents*/, "TestFile1_Hex_SHIFT-JIS.txt"),
        CTestCaseData(true, "TestFile1__src_1250.txt", CPI_DEFAULT/*no need to recode file contents*/, "TestFile1_Hex_1250.txt"),
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
      };
      DoRecodingTest(EncodeStringToURL, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeURLToString, false, &data[0], _countof(data), false);
    }
  };
}
