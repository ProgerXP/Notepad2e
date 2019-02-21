Changes in Scintilla's source code:


[**add AHK lexer**]:
[Scintilla/include/SciLexer.h]:
\#define SCLEX_AHK 200
...
\#define SCE_AHK_DEFAULT 0
\#define SCE_AHK_COMMENTLINE 1
\#define SCE_AHK_COMMENTBLOCK 2
\#define SCE_AHK_ESCAPE 3
\#define SCE_AHK_SYNOPERATOR 4
\#define SCE_AHK_EXPOPERATOR 5
\#define SCE_AHK_STRING 6
\#define SCE_AHK_NUMBER 7
\#define SCE_AHK_IDENTIFIER 8
\#define SCE_AHK_VARREF 9
\#define SCE_AHK_LABEL 10
\#define SCE_AHK_WORD_CF 11
\#define SCE_AHK_WORD_CMD 12
\#define SCE_AHK_WORD_FN 13
\#define SCE_AHK_WORD_DIR 14
\#define SCE_AHK_WORD_KB 15
\#define SCE_AHK_WORD_VAR 16
\#define SCE_AHK_WORD_SP 17
\#define SCE_AHK_WORD_UD 18
\#define SCE_AHK_VARREFKW 19
\#define SCE_AHK_ERROR 20

**Add file:**
[Scintilla/lexers/LexAHK.h]
[/**add AHK lexer**]


[**Implement wheel_action/proc_action**]:
[Scintilla/include/Scintilla.h]:
typedef void (*wheel_action) (int);
typedef int (*key_action)(int , int);
extern wheel_action n2e_wheel_action;
extern key_action n2e_proc_action;
[/Scintilla/include/Scintilla.h]:

[Scintilla/win32/ScintillaWin.cxx]
wheel_action n2e_wheel_action = 0;
key_action n2e_proc_action = 0;

...

				if (wParam & MK_CONTROL) {
					// Zoom! We play with the font sizes in the styles.
					// Number of steps/line is ignored, we just care if sizing up or down
					if (n2e_wheel_action) {
						n2e_wheel_action(linesToScroll);
					} else {
						if (linesToScroll < 0) {
							KeyCommand(SCI_ZOOMIN);
						} else {
							KeyCommand(SCI_ZOOMOUT);
						}
					}
				} else {

...

		case WM_CHAR:
			if (n2e_proc_action) {
				int ret = n2e_proc_action(wParam, WM_CHAR);
				if (ret >= 0) {
					return ret;
				}
			}
			if (((wParam >= 128) || !iscntrl(static_cast<int>(wParam))) || !lastKeyDownConsumed) {

...

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN: {
			//Platform::DebugPrintf("S keydown %d %x %x %x %x\n",iMessage, wParam, lParam, ::IsKeyDown(VK_SHIFT), ::IsKeyDown(VK_CONTROL));
				if (n2e_proc_action) {
					int ret = n2e_proc_action(wParam, WM_KEYDOWN);
					if (ret >= 0) {

						return ret;
					}
				}
				lastKeyDownConsumed = false;
[/Scintilla/win32/ScintillaWin.cxx]
[/**Implement wheel_action/proc_action**]


[**Enable additional Lexers**]:
[scintilla/src/Catalogue.cxx]
	LINK_LEXER(lmAsn1);
	LINK_LEXER(lmBash);
	LINK_LEXER(lmCaml);
	LINK_LEXER(lmCoffeeScript);
	LINK_LEXER(lmD);
	LINK_LEXER(lmLISP);
	LINK_LEXER(lmTeX);
[/scintilla/src/Catalogue.cxx]

The script lexlink.js is adjusted accordingly.

[/**Enable additional Lexers**]


[**13. "No line selection on active selection"-feature**]:
Remove triple-click handler in Editor::ButtonDown():
[scintilla/src/Editor.cxx]
				if (selectionType == selChar) {
					selectionType = selWord;
					doubleClick = true;
				} else if (selectionType == selWord) {
					// [n2e]: "No line selection on active selection"-feature: do nothing on *triple* click
				} else {
					selectionType = selChar;
					originalAnchorPos = sel.MainCaret();
				}
[/scintilla/src/Editor.cxx]
[/**13.**]


[**6. "Scroll margin"-feature**]:
New notification code added:
[scintilla/include/Scintilla.h]
\#define SCN_CARETMOVED 2031
[/scintilla/include/Scintilla.h]

New notification proc added:
[scintilla/src/Editor.h]
void NotifyCaretMoved();
[/scintilla/src/Editor.h]

[scintilla/src/Editor.cxx]
void Editor::NotifyCaretMoved()
{
	// Send notification
	SCNotification scn = { 0 };
	scn.nmhdr.code = SCN_CARETMOVED;
	NotifyParent(scn);
}

...

Corresponding calls added to Editor::KeyCommand():

	case SCI_PARADOWN:
		ParaUpOrDown(1, Selection::noSel);
		NotifyCaretMoved(); // [n2e]: "Scroll margin"-feature
		break;
	case SCI_PARADOWNEXTEND:
		ParaUpOrDown(1, Selection::selStream);
		NotifyCaretMoved(); // [n2e]: "Scroll margin"-feature
		break;
	case SCI_LINESCROLLDOWN:

...

	case SCI_PARAUP:
		ParaUpOrDown(-1, Selection::noSel);
		NotifyCaretMoved(); // [n2e]: "Scroll margin"-feature
		break;
	case SCI_PARAUPEXTEND:
		ParaUpOrDown(-1, Selection::selStream);
		NotifyCaretMoved(); // [n2e]: "Scroll margin"-feature
		break;
	case SCI_LINESCROLLUP:

...

	case SCI_PAGEUP:
		PageMove(-1);
		NotifyCaretMoved(); // [n2e]: "Scroll margin"-feature
		break;
	case SCI_PAGEUPEXTEND:
		PageMove(-1, Selection::selStream);
		NotifyCaretMoved(); // [n2e]: "Scroll margin"-feature
		break;
	case SCI_PAGEUPRECTEXTEND:
		PageMove(-1, Selection::selRectangle);
		NotifyCaretMoved(); // [n2e]: "Scroll margin"-feature
		break;
	case SCI_PAGEDOWN:
		PageMove(1);
		NotifyCaretMoved(); // [n2e]: "Scroll margin"-feature
		break;
	case SCI_PAGEDOWNEXTEND:
		PageMove(1, Selection::selStream);
		NotifyCaretMoved(); // [n2e]: "Scroll margin"-feature
		break;
	case SCI_PAGEDOWNRECTEXTEND:
		PageMove(1, Selection::selRectangle);
		NotifyCaretMoved(); // [n2e]: "Scroll margin"-feature
		break;
	case SCI_EDITTOGGLEOVERTYPE:
[/scintilla/src/Editor.cxx]
[/**6.**]


[**"Update gutter width"-feature**]:
New notification code added:
[scintilla/include/Scintilla.h]
\#define SCN_LINECOUNTCHANGED 2032
[/scintilla/include/Scintilla.h]

New notification proc added:
[scintilla/src/Editor.h]
void NotifyLineCountChanged();
[/scintilla/src/Editor.h]

[scintilla/src/Editor.cxx]
void Editor::NotifyLineCountChanged()
{
  // Send notification
  SCNotification scn = { 0 };
  scn.nmhdr.code = SCN_LINECOUNTCHANGED;
  NotifyParent(scn);
}

...

Corresponding calls added to Editor::KeyCommand():

	case SCI_NEWLINE:
		...
		NotifyLineCountChanged(); // [n2e]: "Update gutter width"-feature
		break;
	case SCI_LINEDELETE:
		...
		NotifyLineCountChanged();
		break;
	case SCI_CUT:
		...
		NotifyLineCountChanged();
		break;
	case SCI_PASTE:
		...
		NotifyLineCountChanged();
		break;
	case SCI_CLEAR:
		...
		NotifyLineCountChanged();
		break;
	case SCI_UNDO:
		...
		NotifyLineCountChanged();
		break;
	case SCI_REDO:
		...
		NotifyLineCountChanged();
		break;
[/scintilla/src/Editor.cxx]
[/**"Update gutter width"-feature**]

[**Drag & drop improvement #63**]
New code around DropAt()-call:
[scintilla/win32/ScintillaWin.cxx]
		const bool bIsTrailingLineEnd = (data.size() >= 3) && (data[data.size() - 3] == '\r') && (data[data.size() - 2] == '\n');
		const bool bAddNewLine = (inDragDrop != ddDragging) && (!bIsTrailingLineEnd && pdoc->IsLineStartPosition(movePos.Position()) && pdoc->IsLineEndPosition(movePos.Position()));
		if (bAddNewLine)
		{
			data.insert(data.end() - 1, '\r');
			data.insert(data.end() - 1, '\n');
		}
		DropAt(movePos, &data[0], data.size() - 1, *pdwEffect == DROPEFFECT_MOVE, hrRectangular == S_OK);
		if (bAddNewLine) {
			KeyCommand(SCI_CHARRIGHT);
		}
[/scintilla/win32/ScintillaWin.cxx]
[/**Drag & drop improvement #63**]

[**Implement Notepad's right click behavior #54**]
Add new message SCI_MOVECARETONRCLICK:

[scintilla/include/Scintilla.h]
\#define SCI_MOVECARETONRCLICK 2369
[/scintilla/include/Scintilla.h]

[scintilla/src/ScintillaBase.h]
	enum { maxLenInputIME = 200 };

	*bool n2e_moveCaretOnRClick;*
	bool displayPopupMenu;
[/scintilla/src/ScintillaBase.h]

[scintilla/src/ScintillaBase.cxx]
ScintillaBase::ScintillaBase() {
	*n2e_moveCaretOnRClick = true;*
	displayPopupMenu = true;

...

	case SCI_MOVECARETONRCLICK:
		n2e_moveCaretOnRClick = wParam != 0;
		break;*

	case SCI_USEPOPUP:
[/scintilla/src/ScintillaBase.cxx]

[scintilla/win32/ScintillaWin.cxx]
		case WM_RBUTTONDOWN:
			::SetFocus(MainHWND());
			if (*n2e_moveCaretOnRClick* && !PointInSelection(Point::FromLong(static_cast<long>(lParam)))) {
				CancelModes();
[/scintilla/win32/ScintillaWin.cxx]
[/**Implement Notepad's right click behavior #54**]

[**Unindent and tabs #128**]
[scintilla/src/Editor.cxx]
Change the code in Editor::Indent(bool forwards): replace condition code block

				if (pdoc->GetColumn(caretPosition) <= pdoc->GetLineIndentation(lineCurrentPos) &&
						pdoc->tabIndents) {
					int indentation = pdoc->GetLineIndentation(lineCurrentPos);
					int indentationStep = pdoc->IndentSize();
					const int posSelect = pdoc->SetLineIndentation(lineCurrentPos, indentation - indentationStep);
					sel.Range(r) = SelectionRange(posSelect);
				} else {
with:

				if (pdoc->tabIndents) {
					SelectionPosition posCaret(sel.Range(r).caret.Position());
					SelectionPosition posAnchor(sel.Range(r).anchor.Position());
					const int indentation = pdoc->GetLineIndentation(lineCurrentPos);
					const int indentationStep = pdoc->IndentSize();
					bool adjustCaretPosition = false;
					int tabsCount = 0;
					if (pdoc->CharAt(pdoc->LineStart(lineCurrentPos)) == '\t') {
						adjustCaretPosition = true;
						for (int i = pdoc->LineStart(lineCurrentPos); i < posCaret.Position(); i++) {
							if (pdoc->CharAt(i) != '\t')
								break;
							++tabsCount;
						}
					}
					pdoc->SetLineIndentation(lineCurrentPos, indentation - indentationStep);
					if (adjustCaretPosition) {
						const int offset = (pdoc->useTabs ? std::max(1, tabsCount - 1) : tabsCount) * (indentationStep - 1);
						posCaret.Add(offset);
						posAnchor.Add(offset);
					}
					if ((posCaret.Position() - pdoc->LineStart(lineCurrentPos) >= indentationStep) && (indentation >= indentationStep)) {
						posCaret.Add(-indentationStep);
						posAnchor.Add(-indentationStep);
						sel.Range(r) = SelectionRange(posCaret, posAnchor);
					}
				} else {
[/scintilla/src/Editor.cxx]
[/**Unindent and tabs #128**]

[**ctrl+arrow behavior toggle #89**]
Add new message SCI_SETWORDNAVIGATIONMODE:
[scintilla/include/Scintilla.h]
\#define SCI_SETWORDNAVIGATIONMODE 2379
[/scintilla/include/Scintilla.h]

Add message handler in ScintillaBase::WndProc:
[scintilla/src/ScintillaBase.cxx]
	case SCI_SETWORDNAVIGATIONMODE:
		pdoc->SetWordNavigationMode((int)wParam);
		break;

	case SCI_USEPOPUP:
[/scintilla/src/ScintillaBase.cxx]

Add method declaration/implemention to Document class:
[scintilla/src/Document.h]
	double durationStyleOneLine;
	*int wordNavigationMode;*
...
	int BraceMatch(int position, int maxReStyle);
	*void SetWordNavigationMode(const int iMode);*
[/scintilla/src/Document.h]

[scintilla/src/Document.cxx]
	durationStyleOneLine = 0.00001;
	*wordNavigationMode = 0;*
...

int Document::NextWordStart(int pos, int delta) {
	if (delta < 0) {
		// [n2e]: ctrl+arrow behavior toggle #89
		switch (wordNavigationMode)
		{
		case 0:
			// standard navigation
			while (pos > 0 && (WordCharClass(cb.CharAt(pos - 1)) == CharClassify::ccSpace))
				pos--;
			if (pos > 0)
			{
				CharClassify::cc ccStart = WordCharClass(cb.CharAt(pos - 1));
				while (pos > 0 && (WordCharClass(cb.CharAt(pos - 1)) == ccStart))
					pos--;
			}
			break;
		case 1:
			// accelerated navigation
			{
				if (pos > 0)
				{
					pos--;
				}
				bool stopAtCurrentNewLine = false;
				while ((pos >= 0) && (WordCharClass(cb.CharAt(pos)) == CharClassify::ccNewLine))
				{
					pos--;
					stopAtCurrentNewLine = true;
				}
				if (stopAtCurrentNewLine)
				{
					pos++;
				}
				else
				{
					CharClassify::cc ccCurrent = WordCharClass(cb.CharAt(pos));
					while (pos > 0)
					{
						CharClassify::cc ccPrev = WordCharClass(cb.CharAt(pos - 1));
						if ((ccPrev == CharClassify::ccNewLine)
							|| ((ccPrev == CharClassify::ccSpace) && (ccCurrent != CharClassify::ccSpace)))
							break;
						pos--;
						ccCurrent = ccPrev;
					}
				}
			}
			break;
		default:
			// not implemented
			PLATFORM_ASSERT(false);
			break;
		}
	} else {
		switch (wordNavigationMode)
		{
		case 0:
			// standard navigation
			{
				CharClassify::cc ccStart = WordCharClass(cb.CharAt(pos));
				while (pos < (Length()) && (WordCharClass(cb.CharAt(pos)) == ccStart))
					pos++;
				while (pos < (Length()) && (WordCharClass(cb.CharAt(pos)) == CharClassify::ccSpace))
					pos++;
			}
			break;
		case 1:
			// accelerated navigation
			{
				bool stopAtCurrentNewLine = false;
				while ((pos < Length()) && (WordCharClass(cb.CharAt(pos)) == CharClassify::ccNewLine))
				{
					pos++;
					stopAtCurrentNewLine = true;
				}
				if (!stopAtCurrentNewLine)
				{
					pos++;
					assert(pos > 0);
					CharClassify::cc ccPrev = WordCharClass(cb.CharAt(pos - 1));
					while (pos < Length())
					{
						CharClassify::cc ccCurrent = WordCharClass(cb.CharAt(pos));
						if ((ccCurrent == CharClassify::ccNewLine)
							|| ((ccPrev == CharClassify::ccSpace) && (ccCurrent != CharClassify::ccSpace)))
							break;
						pos++;
						ccPrev = ccCurrent;
					}
				}
			}
			break;
			// [/2e]
		default:
			// not implemented
			PLATFORM_ASSERT(false);
			break;
		}
	// [/n2e]
	}
	return pos;
}
..
void Document::SetWordNavigationMode(const int iMode)
{
	wordNavigationMode = iMode;
}
[/scintilla/src/Document.cxx]
[/**ctrl+arrow behavior toggle #89**]

[**Regexp: confine to single line #90**]
Move class RESearchRange declaration/implementation from Document.cxx to Document.h

[/**Regexp: confine to single line #90**]


[**Increasingly slow to hex/base64/qp #142**]

Add new message SCI_SETSKIPUIUPDATE:
[scintilla/include/Scintilla.h]
#define SCI_GETSUBSTYLEBASES 4026
*#define SCI_SETSKIPUIUPDATE 9000*
[/scintilla/include/Scintilla.h]

Add corresponding flag to Editor class:
[scintilla/src/Editor.h]
	bool convertPastes;
	*bool n2e_skipUIUpdate;*
[/scintilla/src/Editor.h]

[scintilla/src/Editor.cxx]
	convertPastes = true;
	*n2e_skipUIUpdate = false;*
...
void Editor::RedrawRect(PRectangle rc) {
	//Platform::DebugPrintf("Redraw %0d,%0d - %0d,%0d\n", rc.left, rc.top, rc.right, rc.bottom);
	*if (n2e_skipUIUpdate) {
		return;
	}*
...
void Editor::Redraw() {
	//Platform::DebugPrintf("Redraw all\n");
	*if (n2e_skipUIUpdate) {
		return;
	}*
...
void Editor::InvalidateSelection(SelectionRange newMain, bool invalidateWholeSelection) {
	*if (n2e_skipUIUpdate) {
		return;
	}*
...
void Editor::EnsureCaretVisible(bool useMargin, bool vert, bool horiz) {
	*if (n2e_skipUIUpdate) {
		return;
	}*
...
void Editor::InvalidateCaret() {
	*if (n2e_skipUIUpdate) {
		return;
	}*
...
void Editor::Paint(Surface *surfaceWindow, PRectangle rcArea) {
	*if (n2e_skipUIUpdate) {
		return;
	}*
...
Replace the code in Editor::WndProc() for case SCI_REPLACESEL:
			SetEmptySelection(sel.MainCaret() + lengthInserted);
			EnsureCaretVisible();
with
			*if (!n2e_skipUIUpdate) {
					SetEmptySelection(sel.MainCaret() + lengthInserted);
					EnsureCaretVisible();
			}*
...
Add next handler to Editor::WndProc():
	case SCI_SETSKIPUIUPDATE:
		n2e_skipUIUpdate = (wParam != 0);
		if (!n2e_skipUIUpdate) {
			InvalidateWholeSelection();
			Redraw();
		}
		return n2e_skipUIUpdate;
[/scintilla/src/Editor.cxx]
[/**Increasingly slow to hex/base64/qp #142**]


[**DPI awareness #154**]

Add new message SCI_SETDPI:
[scintilla/include/Scintilla.h]
#define SCI_SETDPI 9001
[/scintilla/include/Scintilla.h]

Add message handler and replace some code:
[scintilla/win32/ScintillaWin.cxx]
	drtp.dpiX = 96.0;
	drtp.dpiY = 96.0;
>>
	drtp.dpiX = GetDpiX();
	drtp.dpiY = GetDpiY();

...

		96.0f, 96.0f, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT),
>>
		GetDpiSystemScaleFactorX(), GetDpiSystemScaleFactorY(), D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT),

...

	  case SCI_SETDPI:
		SetDPI(LOWORD(wParam),
			HIWORD(wParam),
			MulDiv(N2E_DEFAULT_FONT_DPI, N2E_DEFAULT_SCREEN_DPI, GetDpiY()));
		InvalidateStyleData();
		RefreshStyleData();
		return 0;

... 

			drtp.dpiX = 96.0;
			drtp.dpiY = 96.0;
>>
			drtp.dpiX = GetDpiX();
			drtp.dpiY = GetDpiY();
[/scintilla/win32/ScintillaWin.cxx]

Add required subroutines:
[scintilla/win32/PlatWin.h]
#define N2E_DEFAULT_SCREEN_DPI 96
#define N2E_DEFAULT_FONT_DPI 72

void SetDPI(const float _dpiX, const float _dpiY, const int _dpiFont);
float GetDpiX();
float GetDpiY();
int GetDpiFont();
[/scintilla/win32/PlatWin.h]

[scintilla/win32/PlatWin.cxx]
static float n2e_dpiX = N2E_DEFAULT_SCREEN_DPI;
static float n2e_dpiY = N2E_DEFAULT_SCREEN_DPI;
static int n2e_dpiFont = N2E_DEFAULT_FONT_DPI;

void SetDPI(const float _dpiX, const float _dpiY, const int _dpiFont)
{
	n2e_dpiX = _dpiX;
	n2e_dpiY = _dpiY;
	n2e_dpiFont = _dpiFont;
}

float GetDpiX()
{
	return n2e_dpiX;
}

float GetDpiY()
{
	return n2e_dpiY;
}

int GetDpiFont()
{
	return n2e_dpiFont;
}

...

In SurfaceD2D::SurfaceD2D() replace
logPixelsY = 72;
with
logPixelsY = N2E_DEFAULT_FONT_DPI;

int SurfaceGDI::LogPixelsY() {
	return GetDpiY();
}

int SurfaceGDI::DeviceHeightFont(int points) {
	return ::MulDiv(points, LogPixelsY(), N2E_DEFAULT_FONT_DPI);
}

void SurfaceD2D::SetScale() {
	HDC hdcMeasure = ::CreateCompatibleDC(NULL);
	logPixelsY = ::GetDeviceCaps(hdcMeasure, LOGPIXELSY);
	dpiScaleX = ::GetDeviceCaps(hdcMeasure, LOGPIXELSX) / GetDpiX();
	dpiScaleY = logPixelsY / GetDpiY();
	::DeleteDC(hdcMeasure);
}

int SurfaceD2D::DeviceHeightFont(int points) {
	return ::MulDiv(points, LogPixelsY(), GetDpiFont());
}

[/scintilla/win32/PlatWin.cxx]

[/**DPI awareness #154**]

[**EscapeHTML**]
Remove const from lpstrText.

[scintilla/include/scintilla.h]
struct Sci_TextToFind {
	struct Sci_CharacterRange chrg;
	char *lpstrText;
	struct Sci_CharacterRange chrgText;
};
[/scintilla/include/scintilla.h]
[/**EscapeHTML**]

[**repaint issue when using Ctrl+Shift+Backspace/Del #116**]
[scintilla/src/Editor.cxx]
Replace

		if (currentLine >= wrapPending.start)
			WrapLines(wsAll);

with

		if (currentLine >= wrapPending.start) {
			if (WrapLines(wsAll)) {
				Redraw();
			}
		}
[/scintilla/src/Editor.cxx]
[/**repaint issue when using Ctrl+Shift+Backspace/Del #116**]

[**#145 Allow NULL character substitution when using regexp-replace (\x00)**]
[scintilla/src/UniConversion.cxx]
unsigned int UTF8Length(const wchar_t *uptr, unsigned int tlen) {
>>
unsigned int UTF8Length(const wchar_t *uptr, unsigned int tlen, const bool processNULL) {

...

for (unsigned int i = 0; i < tlen && uptr[i];) {
>>
for (unsigned int i = 0; i < tlen && (uptr[i] || processNULL);) {

...

void UTF8FromUTF16(const wchar_t *uptr, unsigned int tlen, char *putf, unsigned int len) {
>>
void UTF8FromUTF16(const wchar_t *uptr, unsigned int tlen, char *putf, unsigned int len, const bool processNULL) {

...

for (unsigned int i = 0; i < tlen && uptr[i];) {
>>
for (unsigned int i = 0; i < tlen && (uptr[i] || processNULL);) {

[/scintilla/src/UniConversion.cxx]

[scintilla/src/UniConversion.h]
unsigned int UTF8Length(const wchar_t *uptr, unsigned int tlen);
void UTF8FromUTF16(const wchar_t *uptr, unsigned int tlen, char *putf, unsigned int len);
>>
unsigned int UTF8Length(const wchar_t *uptr, unsigned int tlen, const bool processNULL = false);
void UTF8FromUTF16(const wchar_t *uptr, unsigned int tlen, char *putf, unsigned int len, const bool processNULL = false);
[/scintilla/src/UniConversion.h]
