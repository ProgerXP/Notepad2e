#include "stdafx.h"
#include <Windows.h>

HWND hwndTopLevel = NULL;

#include "../src/Extension/Externals.h"
#include "../src/Extension/SciCall.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

class CRegExTestData
{
public:
  std::string source;
  std::string regexFrom;
  std::string regexTo;
  std::string result;

  CRegExTestData(const std::string& src, const std::string& from, const std::string& to, const std::string& res)
    : source(src), regexFrom(from), regexTo(to), result(res)
  {
  }
};

namespace Notepad2eTests
{
  TEST_CLASS(CRegEx)
  {
  public:
    TEST_METHOD(RegExTests)
    {
      const CRegExTestData data[] = {
        CRegExTestData("aa\nbb", "$", "\n", "aa\n\nbb\n\n")
      };

      const auto hInst = GetModuleHandle(NULL);
      hwndTopLevel = CreateWindow(L"Edit", L"", 0, 0, 0, 0, 0, 0, 0, hInst, 0);

      g_hwndActiveEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        L"Scintilla",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 0, 0,
        hwndTopLevel,
        (HMENU)0,
        hInst,
        NULL);
      InitScintillaHandle(g_hwndActiveEdit);

      #define BUFFER_LENGTH MAX_PATH * 100
      char actualResult[BUFFER_LENGTH];

      for (auto i = 0; i < _countof(data); i++)
      {
        const auto info = data[i];
        SciCall_SetSel(0, SciCall_GetLength());
        SciCall_ReplaceSel(0, info.source.c_str());
        
        SciCall_SetSel(0, SciCall_GetLength());
        SciCall_ReplaceSel(0, info.result.c_str());

        if (SciCall_GetText(SciCall_GetLength() + 1, &actualResult[0]))
        {
          if (info.result != std::string(actualResult))
            Assert::Fail(L"Wrong result");
        }
        else
        {
          Assert::Fail(L"Invalid expression");
        }
      }
    }
  };

}
