#include "stdafx.h"
#include "CppUnitTest.h"
#include "../src/Extension/StrToHex.h"

#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Notepad2eTests
{		
	TEST_CLASS(CStrToHex)
	{
	public:

    std::string UCS2toUTF8(const std::wstring &str)
    {
      if (str.empty())
      {
        return "";
      }
      std::vector<char> res;
      const int requiredSize = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
      if (requiredSize == 0)
      {
        return "";
      }
      res.resize(requiredSize);
      WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &res[0], requiredSize, NULL, NULL);
      return &res[0];
    }

    struct TestCaseData
    {
      std::string strSrc;
      int iEncoding;
      std::string strExpectedRes;
      std::wstring strMessage;
    };
		
		TEST_METHOD(Test1)
		{
      TestCaseData data[] = {
        { "test", CPI_DEFAULT, "74657374", L"Default encoding" },
        { "test", CPI_UNICODE, "0074006500730074", L"Unicode encoding" },
        { UCS2toUTF8(L"тестовая строка"), CPI_UTF8, "D182D0B5D181D182D0BED0B2D0B0D18F20D181D182D180D0BED0BAD0B0", L"UTF-8 encoding"}
      };
      for (auto i=0; i < _countof(data); i++)
      {
        auto info = data[i];
        Assert::AreEqual(EncodeStringToHex(info.strSrc.c_str(), info.iEncoding),
                         info.strExpectedRes.c_str(), info.strMessage.c_str(), LINE_INFO());
      }
		}

	};
}