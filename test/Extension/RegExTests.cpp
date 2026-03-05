#include "stdafx.h"
#include "SciTests.h"
#include "../src/Shared/SharedEditHelper.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

DWORD regexFlags()
{
  return SCFIND_REGEXP | SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH | SCFIND_REGEXP_EMPTYMATCH_ALLOWATSTART | SCFIND_REGEXP_SKIPCRLFASONE;
}

const std::wstring c_errorResult = L"Incorrect result";
const std::wstring c_errorSelStart = L"Incorrect selStart";
const std::wstring c_errorSelEnd = L"Incorrect selEnd";

std::wstring formatErrorText(const std::wstring& prefix, const int index)
{
  std::wostringstream ss;
  ss << prefix << L" " << index;
  return ss.str();
}

class CRegExTestData
{
public:
  std::string source;
  std::string regexFrom;
  std::string regexTo;
  std::string result;
  int selectionFrom = -1;
  int selectionTo = -1;
  int expectedSelectionFrom = -1;
  int expectedSelectionTo = -1;

  CRegExTestData(const std::string& src, const std::string& from, const std::string& to, const std::string& res)
    : source(src), regexFrom(from), regexTo(to), result(res){}
  CRegExTestData(const std::string& src, const std::string& from, const std::string& to, const std::string& res,
    const int selFrom, const int selTo, const int expectedSelFrom, const int expectedSelTo)
    : source(src), regexFrom(from), regexTo(to), result(res),
    selectionFrom(selFrom), selectionTo(selTo), expectedSelectionFrom(expectedSelFrom), expectedSelectionTo(expectedSelTo){}
};

void generalTest(const CRegExTestData* dataset, const int datasetCount,
  const std::function<void(LPCEDITFINDREPLACE, const CRegExTestData&, const int, char* buffer)>& funcTestDataItem)
{
  SciTests::runTest([&] {
    char buffer[MAX_PATH * 100];
    EDITFINDREPLACE efr = { 0 };
    efr.fuFlags = SCFIND_REGEXP;
    for (auto i = 0; i < datasetCount; i++)
    {
      const auto data = dataset[i];
      lstrcpyA(efr.szFind, data.regexFrom.c_str());
      lstrcpyA(efr.szFindUTF8, data.regexFrom.c_str());
      lstrcpyA(efr.szReplace, data.regexTo.c_str());
      lstrcpyA(efr.szReplaceUTF8, data.regexTo.c_str());
      funcTestDataItem(&efr, data, i, buffer);
    }
  });
}

namespace Notepad2eTests
{
  TEST_CLASS(CRegEx)
  {
  public:
    TEST_METHOD(ReplaceAll)
    {
      const CRegExTestData data[] = {
        CRegExTestData("aa\nbb", "a", "b", "bb\nbb"),
        CRegExTestData("aa\nbb", "$", "\\n", "aa\n\nbb\n"),
        CRegExTestData("aa\nbb", "$", "\\n\\1\\1\\n", "aa\n\n\nbb\n\n"),
        CRegExTestData("aa\nbb", "(.*)$", "\\n\\1\\1\\n", "\naaaa\n\n\nbbbb\n"),
        CRegExTestData("ab\nab", "b|$", "$0z", "abzz\nabzz"),
        CRegExTestData("ab\nab\n", "b|$", "$0z", "abzz\nabzz\nz"),
        CRegExTestData("ab\nab\n\n", "b|$", "$0z", "abzz\nabzz\nz\nz"),
        CRegExTestData("aa\n\naa\n", "$", "z", "aaz\nz\naaz\nz"),
        CRegExTestData("a@b@c\na@b@c", "^[^@]+@", "", "b@c\nb@c"),
        CRegExTestData("a@b@c\n1\nabc@b@c\n2", "^[^@]+@.*$", "\\n", "\n\n1\n\n\n2"),
        CRegExTestData("a@b@c\n1\nabc@b@c\n2", "^[^@]+@.*$", "zzz", "zzz\nzzz\n2"),
        CRegExTestData("a@b@c\n1\nabc@b@c\n2", "^[\\l]+@.*$", "zzz", "zzz\n1\nzzz\n2"),
        CRegExTestData("a@b@c\na@b@c", "^([^@]+@.*)$", "\\1X\\n\\1", "a@b@cX\na@b@c\na@b@cX\na@b@c"),
        CRegExTestData("aa bb\naa bb", ".*", "z", "z\nz"),
        CRegExTestData("aa\n\naa", "$", "z", "aaz\nz\naaz"),
        CRegExTestData("aa", "$", "z", "aaz"),
      };

      generalTest(&data[0], _countof(data), [&](LPCEDITFINDREPLACE lpefr, const CRegExTestData& data, const int index, char* buffer) {
        SciCall_SetSel(0, SciCall_GetLength());
        SciCall_ReplaceSel(0, data.source.c_str());
        EditReplaceAll(SciTests::hwnd(), lpefr, FALSE);
        if (SciCall_GetText(SciCall_GetLength() + 1, buffer))
          Assert::IsTrue(data.result == std::string(buffer), formatErrorText(c_errorResult, index).c_str());
        else
          Assert::Fail(L"Failed to retrieve text");
      });
    }

    TEST_METHOD(ReplaceInSelection)
    {
      const CRegExTestData data[] = {
        CRegExTestData("aa\nab\nb", "a", "b", "ab\nbb\nb", 1, 4, 1, 4),
        CRegExTestData("aa\nab\nb", "a", "bz", "abz\nbzb\nb", 1, 4, 1, 6),
        CRegExTestData("cbc\nxbc", "c|$", "z", "cbzz\nxbc", 1, 6, 1, 7),
      };

      generalTest(&data[0], _countof(data), [&](LPCEDITFINDREPLACE lpefr, const CRegExTestData& data, const int index, char* buffer) {
        SciCall_SetSel(0, SciCall_GetLength());
        SciCall_ReplaceSel(0, data.source.c_str());
        SciCall_SetSel(data.selectionFrom, data.selectionTo);
        EditReplaceAllInSelection(SciTests::hwnd(), lpefr, FALSE);

        Assert::IsTrue(SciCall_GetSelStart() == data.expectedSelectionFrom, formatErrorText(c_errorSelStart, index).c_str());
        Assert::IsTrue(SciCall_GetSelEnd() == data.expectedSelectionTo, formatErrorText(c_errorSelEnd, index).c_str());
        if (SciCall_GetText(SciCall_GetLength() + 1, buffer))
          Assert::IsTrue(data.result == std::string(buffer), formatErrorText(c_errorResult, index).c_str());
        else
          Assert::Fail(L"Failed to retrieve text");
      });
    }
  };

}
