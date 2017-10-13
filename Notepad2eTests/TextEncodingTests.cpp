#include "stdafx.h"
#include <assert.h>
#include "CppUnitTest.h"
#include "../src/Extension/StrToHex.h"
#include "CppUnitTest.h"
#include "TextEncodingTestCaseData.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

typedef LPCSTR (TWorkingProc)(LPCSTR, const int);

static void DoRecodingTest(TWorkingProc proc, const bool isEncoding, const CTestCaseData* pData, const int count)
{
  for (auto i = 0; i < count; i++)
  {
    const auto info = pData[i];
    std::string errorMessage(std::string("Source text: \"") + info.GetPlainSource() + "\"> ");
    errorMessage += info.GetErrorMessageText();
    if (isEncoding)
    {
      Assert::AreEqual(info.GetExpectedResultText(),
                       proc(info.GetSourceText(), info.GetEncoding()),
                       errorMessage.c_str(), LINE_INFO());
    }
    else
    {
      Assert::AreEqual(info.GetSourceText(),
                       proc(info.GetExpectedResultText(), info.GetEncoding()),
                       errorMessage.c_str(), LINE_INFO());
    }
  }
};

LPCSTR EncodeStringToBase64(LPCSTR, const int)
{
  Assert::Fail(L"not implemented");
  return NULL;
}

LPCSTR DecodeBase64ToString(LPCSTR, const int)
{
  Assert::Fail(L"not implemented");
  return NULL;
}

LPCSTR EncodeStringToQP(LPCSTR, const int)
{
  Assert::Fail(L"not implemented");
  return NULL;
}

LPCSTR DecodeQPToString(LPCSTR, const int)
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
        CTestCaseData(false, L"тестовая строка", CPI_UTF8, "D182D0B5D181D182D0BED0B2D0B0D18F20D181D182D180D0BED0BAD0B0")
      };
      DoRecodingTest(EncodeStringToHex, true, &data[0], _countof(data));
      DoRecodingTest(DecodeHexToString, false, &data[0], _countof(data));
		}

    TEST_METHOD(TestHex_FileSamples)
    {
      CTestCaseData data[] = {
        CTestCaseData(true, "StrToHex\\TestFile1__src_UTF8.txt", CPI_DEFAULT, "StrToHex\\TestFile1_Hex_UTF8.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_UTF8.txt", CPI_UNICODE, "StrToHex\\TestFile1_Hex_UnicodeLE.txt"),
        CTestCaseData(true, "StrToHex\\TestFile1__src_UTF8.txt", CPI_UNICODE, "StrToHex\\TestFile1_Hex_UnicodeLE.txt"),
      };
      DoRecodingTest(EncodeStringToHex, true, &data[0], _countof(data));
      DoRecodingTest(DecodeHexToString, false, &data[0], _countof(data));
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
      DoRecodingTest(EncodeStringToBase64, true, &data[0], _countof(data));
      DoRecodingTest(DecodeBase64ToString, false, &data[0], _countof(data));
    }

    TEST_METHOD(TestBase64_FileSamples)
    {
      CTestCaseData data[] = {
        CTestCaseData(true, "TODO", CPI_DEFAULT, "TODO"),
      };
      DoRecodingTest(EncodeStringToBase64, true, &data[0], _countof(data));
      DoRecodingTest(DecodeBase64ToString, false, &data[0], _countof(data));
    }
  };

  TEST_CLASS(CStringToQP)
  {
  public:
    TEST_METHOD(TestQP_StringSamples)
    {
      const CTestCaseData data[] = {
        CTestCaseData(false, "TODO", CPI_DEFAULT, "TODO"),
      };
      DoRecodingTest(EncodeStringToQP, true, &data[0], _countof(data));
      DoRecodingTest(DecodeQPToString, false, &data[0], _countof(data));
    }

    TEST_METHOD(TestQP_FileSamples)
    {
      CTestCaseData data[] = {
        CTestCaseData(true, "TODO", CPI_DEFAULT, "TODO"),
      };
      DoRecodingTest(EncodeStringToQP, true, &data[0], _countof(data));
      DoRecodingTest(DecodeQPToString, false, &data[0], _countof(data));
    }
  };
}