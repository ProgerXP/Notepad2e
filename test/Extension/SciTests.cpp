#include "stdafx.h"
#include <Windows.h>
#include "SciTests.h"
#include "BoostRegexSearch.h"

HWND hwndTopLevel = NULL;

#include "../src/Extension/Externals.h"
#include "../src/Extension/SciCall.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SciTests
{

void setupScintillaInstance()
{
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
}

void releaseScintillaInstance()
{
  DestroyWindow(g_hwndActiveEdit);
  DestroyWindow(hwndTopLevel);
}

HWND hwnd()
{
  return g_hwndActiveEdit;
}

void runTest(const std::function<void()>& funcTest)
{
  setupScintillaInstance();
  funcTest();
  releaseScintillaInstance();
}

TEST_CLASS(SampleTests)
{
public:
  TEST_METHOD(Basic)
  {
    SciTests::runTest([&] {
      const std::string data = "0123456789ABCDEF";
      SciCall_SetSel(0, SciCall_GetLength());
      SciCall_ReplaceSel(0, data.c_str());

      const auto length = data.length();
      Assert::IsTrue(SciCall_GetLength() == length, L"Wrong length 1");
      Assert::IsTrue(SciCall_GetSelStart() == length, L"Wrong selection start 1");
      Assert::IsTrue(SciCall_GetSelEnd() == length, L"Wrong selection end 1");

      SciCall_SetSel(0, length - 1);
      Assert::IsTrue(SciCall_GetSelStart() == 0, L"Wrong selection start 2");
      Assert::IsTrue(SciCall_GetSelEnd() == length - 1, L"Wrong selection end 2");

      SciCall_DeleteRange(0, length - 1);
      Assert::IsTrue(SciCall_GetLength() == 1, L"Wrong length 2");
    });
  }
};

} // namespace SciTests
