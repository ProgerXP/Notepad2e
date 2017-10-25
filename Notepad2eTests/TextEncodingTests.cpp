#include "stdafx.h"
#include <assert.h>
#include <Shlwapi.h>
#include "CppUnitTest.h"
#include "../src/Extension/StrToHex.h"
#include "CppUnitTest.h"
#include "TextEncodingTestCaseData.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

typedef LPCSTR (TWorkingProc)(LPCSTR, const int, const int);

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
      std::wstring errorMessage(isEncoding ? L"Encoding, " : L"Decoding, ");
      errorMessage += info.GetErrorMessageText();
      errorMessage += L", ";
      errorMessage += L" source: " + CPtoUCS2(info.GetPlainSource(), CP_ACP);
      errorMessage += L"\r\n";
      if (isEncoding)
      {
        errorMessage += info.GetErrorMessageText();
        if (!info.IsFile())
        {
          Assert::AreEqual(info.GetExpectedResultText(),
                           proc(info.GetSourceText(), info.GetEncoding(), bufferSize),
                           UCS2toCP(errorMessage, CP_ACP).c_str(), LINE_INFO());
        }
        else
        {
          if (StrCmpA(info.GetExpectedResultText(), proc(info.GetSourceText(), info.GetEncoding(), bufferSize)) != 0)
          {
            Assert::Fail(errorMessage.c_str(), LINE_INFO());
          }
        }
      }
      else
      {
        if (!info.IsFile())
        {
          Assert::AreEqual(info.GetSourceText(),
                           proc(info.GetExpectedResultText(), info.GetEncoding(), bufferSize),
                           UCS2toCP(errorMessage, CP_ACP).c_str(), LINE_INFO());
        }
        else if (StrCmpA(info.GetSourceText(), proc(info.GetExpectedResultText(), info.GetEncoding(), bufferSize)) != 0)
        {
          Assert::Fail(errorMessage.c_str(), LINE_INFO());
        }
      }
    }
    bufferSize = max(MIN_BUFFER_SIZE, min(MAX_BUFFER_SIZE, rand()));
    ++bufferTestCount;
    if (!testBufferSize || (bufferTestCount >= BUFFER_TEST_COUNT))
    {
      continueTesting = false;
    }
  }
};

LPCSTR EncodeStringToBase64(LPCSTR, const int, const int)
{
  Assert::Fail(L"not implemented");
  return NULL;
}

LPCSTR DecodeBase64ToString(LPCSTR, const int, const int)
{
  Assert::Fail(L"not implemented");
  return NULL;
}

LPCSTR EncodeStringToQP(LPCSTR, const int, const int)
{
  Assert::Fail(L"not implemented");
  return NULL;
}

LPCSTR DecodeQPToString(LPCSTR, const int, const int)
{
  Assert::Fail(L"not implemented");
  return NULL;
}

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
      CTestCaseData data[] = {
        CTestCaseData(true, "StrToHex\\TestFile1__src_UTF8.txt", CPI_UTF8, "StrToHex\\TestFile1_Hex_UTF8.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_UTF8.txt", CPI_UNICODE, "StrToHex\\TestFile1_Hex_UnicodeLE.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_UTF8_big.txt", CPI_UNICODE, "StrToHex\\TestFile1_Hex_UnicodeLE_big.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_1251.txt", CPI_DEFAULT/*no need to recode file contents*/, "StrToHex\\TestFile1_Hex_1251.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_SHIFT-JIS.txt", CPI_DEFAULT/*no need to recode file contents*/, "StrToHex\\TestFile1_Hex_SHIFT-JIS.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_1250.txt", CPI_DEFAULT/*no need to recode file contents*/, "StrToHex\\TestFile1_Hex_1250.txt"),
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
        CTestCaseData(false, L"тестовая строка", CPI_UTF8, "0YLQtdGB0YLQvtCy0LDRjyDRgdGC0YDQvtC60LA=")
      };
      DoRecodingTest(EncodeStringToBase64, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeBase64ToString, false, &data[0], _countof(data), false);
    }

    TEST_METHOD(TestBase64_FileSamples)
    {
      CTestCaseData data[] = {
        CTestCaseData(true, "StrToHex\\TestFile1__src_UTF8.txt", CPI_UTF8, "StrToHex\\TestFile1_Base64_UTF8.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_UTF8_big.txt", CPI_UTF8, "StrToHex\\TestFile1_Base64_UTF8_big.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_SHIFT-JIS.txt", CPI_DEFAULT, "StrToHex\\TestFile1_Base64_SHIFT-JIS.txt"),
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
        CTestCaseData(false, L"тестовая строка", CPI_UTF8, "=D1=82=D0=B5=D1=81=D1=82=D0=BE=D0=B2=D0=B0=D1=8F=D1=81=D1=82=D1=80=D0=BE=D0=BA=D0=B0")
      };
      DoRecodingTest(EncodeStringToQP, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeQPToString, false, &data[0], _countof(data), false);
    }

    TEST_METHOD(TestQP_FileSamples)
    {
      CTestCaseData data[] = {
        CTestCaseData(true, "StrToHex\\TestFile1__src_UTF8.txt", CPI_UTF8, "StrToHex\\TestFile1_QP_UTF8.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_UTF8_big.txt", CPI_UTF8, "StrToHex\\TestFile1_QP_UTF8_big.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_SHIFT-JIS.txt", CPI_DEFAULT, "StrToHex\\TestFile1_QP_SHIFT-JIS.txt"),
      };
      DoRecodingTest(EncodeStringToQP, true, &data[0], _countof(data), false);
      DoRecodingTest(DecodeQPToString, false, &data[0], _countof(data), false);
    }
  };
}