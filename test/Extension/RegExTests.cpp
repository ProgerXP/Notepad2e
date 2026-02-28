#include "stdafx.h"
#include "SciTests.h"
#include "../src/Shared/SharedEditHelper.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

DWORD regexFlags()
{
  return SCFIND_REGEXP | SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH | SCFIND_REGEXP_EMPTYMATCH_ALLOWATSTART | SCFIND_REGEXP_SKIPCRLFASONE;
}

std::wstring formatErrorText(const std::wstring& prefix, const int index)
{
  std::wostringstream ss;
  ss << prefix << " " << index;
  return ss.str();
}

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
      SciTests::runTest([&] {

        const CRegExTestData data[] = {
          CRegExTestData("aa\nbb", "a", "b", "bb\nbb"),
          CRegExTestData("aa\nbb", "$", "\\n", "aa\n\nbb\n"),
          CRegExTestData("aa\nbb", "$", "\\n\\1\\1\\n", "aa\n\n\nbb\n\n"),
          CRegExTestData("aa\nbb", "(.*)$", "\\n\\1\\1\\n", "\naaaa\n\n\nbbbb\n"),
          CRegExTestData("ab\nab", "b|$", "$0z", "abzz\nabzz"),
          CRegExTestData("aa\n\naa\n", "$", "z", "aaz\nz\naaz\nz"),

          //FAILED: CRegExTestData("a@b@c\na@b@c", "^[^@]+@", "", "b@c\nb@c"),
          //FAILED: CRegExTestData("a@b@c\n1\nabc@b@c\n2", "^[^@]+@.*$", "\\n", "\n\n1\n\n2"),
          //FAILED: CRegExTestData("a@b@c\na@b@c", "^([^@]+@.*)$", "\\1X\\n\\1", "a@b@cX\na@b@c\n1\nabc@b@cX\nabc@b@c\n2"),
          //FAILED: CRegExTestData("aa bb\naa bb", ".*", "z", "z\nz"),
          //FAILED: CRegExTestData("aa\n\naa", "$", "z", "aaz\nz\naaz"),
        };

        #define BUFFER_LENGTH MAX_PATH * 100
        char actualResult[BUFFER_LENGTH];

        EDITFINDREPLACE efr = { 0 };
        efr.fuFlags = SCFIND_REGEXP;

        Sci_TextToFind ttf = {};
        for (auto i = 0; i < _countof(data); i++)
        {
          const auto info = data[i];
          SciCall_SetSel(0, SciCall_GetLength());
          SciCall_ReplaceSel(0, info.source.c_str());

          lstrcpyA(efr.szFind, info.regexFrom.c_str());
          lstrcpyA(efr.szFindUTF8, info.regexFrom.c_str());
          lstrcpyA(efr.szReplace, info.regexTo.c_str());
          lstrcpyA(efr.szReplaceUTF8, info.regexTo.c_str());

          n2e_EditReplaceAll(SciTests::hwnd(), &efr, FALSE);

          if (SciCall_GetText(SciCall_GetLength() + 1, &actualResult[0]))
            Assert::IsTrue(info.result == std::string(actualResult), formatErrorText(L"Incorrect result", i).c_str());
          else
            Assert::Fail(L"Failed to retrieve text");
        }
      });
    }
  };

}
