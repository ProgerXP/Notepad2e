// Scintilla source code edit control
/** @file ScintillaWin.h
 ** Define functions from ScintillaWin.cxx that can be called from ScintillaDLL.cxx.
 **/
// Copyright 1998-2018 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#if defined(SCI_DLL_EXPORT)
class ScintillaWin;

namespace Scintilla {

int ResourcesRelease(bool fromDllMain);
sptr_t DirectFunction(ScintillaWin *sci, UINT iMessage, uptr_t wParam, sptr_t lParam);

}

#elif defined(SCI_DLL_IMPORT)
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllimport)
LRESULT Scintilla_DirectFunction(HANDLE hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#elif defined(SCI_STATIC)
#ifdef __cplusplus
extern "C"
#endif
LRESULT Scintilla_DirectFunction(HANDLE hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
#endif
