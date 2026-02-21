// Scintilla source code edit control
/** @file PlatWin.h
 ** Implementation of platform facilities on Windows.
 **/
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef PLATWIN_H
#define PLATWIN_H

namespace Scintilla {

#define DEFAULT_SCREEN_DPI 96
#define DEFAULT_FONT_DPI 72

void SetDPI(const float _dpiX, const float _dpiY, const int _dpiFont);
float GetDpiX();
float GetDpiY();
int GetDpiFont();

extern void Platform_Initialise(void *hInstance);
extern void Platform_Finalise(bool fromDllMain);

RECT RectFromPRectangle(PRectangle prc) noexcept;

#if defined(USE_D2D)
extern bool LoadD2D();
extern ID2D1Factory *pD2DFactory;
extern IDWriteFactory *pIDWriteFactory;
#endif

class MouseWheelDelta {
	int wheelDelta = 0;
public:
	bool Accumulate(WPARAM wParam) noexcept {
		wheelDelta -= GET_WHEEL_DELTA_WPARAM(wParam);
		return std::abs(wheelDelta) >= WHEEL_DELTA;
	}
	int Actions() noexcept {
		const int actions = wheelDelta / WHEEL_DELTA;
		wheelDelta = wheelDelta % WHEEL_DELTA;
		return actions;
	}
};

}

#endif
