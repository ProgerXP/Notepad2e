#include "stdafx.h"
#include <Windows.h>
#include <functional>
#include "BoostRegexSearch.h"

extern HWND hwndTopLevel;

#include "../src/Extension/Externals.h"
#include "../src/Extension/SciCall.h"

namespace SciTests
{

HWND hwnd();
void runTest(const std::function<void()>& funcTest);

} // namespace SciTests
