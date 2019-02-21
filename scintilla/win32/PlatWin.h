// Scintilla source code edit control
/** @file PlatWin.h
 ** Implementation of platform facilities on Windows.
 **/
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef PLATWIN_H
#define PLATWIN_H

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif


// [n2e]: DPI awareness #154
#define N2E_DEFAULT_SCREEN_DPI 96
#define N2E_DEFAULT_FONT_DPI 72

void n2e_SetDPI(const float _dpiX, const float _dpiY, const int _dpiFont);
float n2e_GetDpiX();
float n2e_GetDpiY();
int n2e_GetDpiFont();
// [/n2e]

extern void Platform_Initialise(void *hInstance);
extern void Platform_Finalise(bool fromDllMain);

#if defined(USE_D2D)
extern bool LoadD2D();
extern ID2D1Factory *pD2DFactory;
extern IDWriteFactory *pIDWriteFactory;
#endif

#ifdef SCI_NAMESPACE
}
#endif

#endif
