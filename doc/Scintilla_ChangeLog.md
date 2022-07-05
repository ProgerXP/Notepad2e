﻿Changes in Scintilla's source code:

---
**add AHK lexer**

[Scintilla/include/SciLexer.h]:
```
#define SCLEX_AHK 200
...
#define SCE_AHK_DEFAULT 0
#define SCE_AHK_COMMENTLINE 1
#define SCE_AHK_COMMENTBLOCK 2
#define SCE_AHK_ESCAPE 3
#define SCE_AHK_SYNOPERATOR 4
#define SCE_AHK_EXPOPERATOR 5
#define SCE_AHK_STRING 6
#define SCE_AHK_NUMBER 7
#define SCE_AHK_IDENTIFIER 8
#define SCE_AHK_VARREF 9
#define SCE_AHK_LABEL 10
#define SCE_AHK_WORD_CF 11
#define SCE_AHK_WORD_CMD 12
#define SCE_AHK_WORD_FN 13
#define SCE_AHK_WORD_DIR 14
#define SCE_AHK_WORD_KB 15
#define SCE_AHK_WORD_VAR 16
#define SCE_AHK_WORD_SP 17
#define SCE_AHK_WORD_UD 18
#define SCE_AHK_VARREFKW 19
#define SCE_AHK_ERROR 20
```
Add file:

[Scintilla/lexers/LexAHK.h]

/**add AHK lexer**

---
**Implement wheel_action/proc_action**

[Scintilla/include/Scintilla.h]:
```
typedef void ( * wheel_action ) ( void*, int );
typedef int ( * key_action ) ( void*, int , int );
extern wheel_action hl_wheel_action ;
extern key_action hl_proc_action;
```
[Scintilla/win32/ScintillaWin.cxx]
```
wheel_action hl_wheel_action = 0;
key_action hl_proc_action = 0;

...

          if (wParam & MK_CONTROL)
          {
            // Zoom! We play with the font sizes in the styles.
            // Number of steps/line is ignored, we just care if sizing up or down
            if ( hl_wheel_action ) {
                hl_wheel_action ( MainHWND(), linesToScroll );
            } else {
                if ( linesToScroll < 0 ) {
                    KeyCommand ( SCI_ZOOMIN );
                } else {
                    KeyCommand ( SCI_ZOOMOUT );
                }
            }
	  }

...

      case WM_CHAR:
        if (hl_proc_action)
	{

		int ret = hl_proc_action(MainHWND(), wParam, WM_CHAR);

		if (ret >= 0)
		{

            return ret;
		}

	}
...

      case WM_SYSKEYDOWN:
      case WM_KEYDOWN: {
          if (hl_proc_action)

          {

            int ret = hl_proc_action(MainHWND(), wParam, WM_KEYDOWN);

            if (ret >= 0)

            {

              return ret;

            }

          }
```

/**Implement wheel_action/proc_action**

---
**Enable additional Lexers**

[scintilla/src/Catalogue.cxx]
```
LINK_LEXER(lmAsn1);
LINK_LEXER(lmBash);
LINK_LEXER(lmCaml);
LINK_LEXER(lmCoffeeScript);
LINK_LEXER(lmD);
LINK_LEXER(lmLISP);
LINK_LEXER(lmTeX);
```
/**Enable additional Lexers**

---
**13. "No line selection on active selection"-feature**
Remove triple-click handler in Editor::ButtonDown():

[scintilla/src/Editor.cxx]
```
                if ( selectionType == selChar ) {
                    selectionType = selWord;
                    doubleClick = true;
                } else if ( selectionType == selWord ) {
                    // do nothing on *triple* click
                } else {
                    selectionType = selChar;
                    originalAnchorPos = sel.MainCaret();
                }
```
/**13.**


---
**6. "Scroll margin"-feature**

New notification code added:

[scintilla/include/Scintilla.h]
```
#define SCN_CARETMOVED 2031
```

New notification proc added:

[scintilla/src/Editor.h]
```
void NotifyCaretMoved();
```

[scintilla/src/Editor.cxx]
```
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
            ParaUpOrDown ( 1 );
            NotifyCaretMoved();
            break;
        case SCI_PARADOWNEXTEND:
            ParaUpOrDown ( 1, Selection::selStream );
            NotifyCaretMoved();
            break;

...

        case SCI_PARAUP:
            ParaUpOrDown ( -1 );
            NotifyCaretMoved();
            break;
        case SCI_PARAUPEXTEND:
            ParaUpOrDown ( -1, Selection::selStream );
            NotifyCaretMoved();
            break;

...

        case SCI_PAGEUP:
            PageMove ( -1 );
            NotifyCaretMoved();
            break;
        case SCI_PAGEUPEXTEND:
            PageMove ( -1, Selection::selStream );
            NotifyCaretMoved();
            break;
        case SCI_PAGEUPRECTEXTEND:
            PageMove ( -1, Selection::selRectangle );
            NotifyCaretMoved();
            break;
        case SCI_PAGEDOWN:
            PageMove ( 1 );
            NotifyCaretMoved();
            break;
        case SCI_PAGEDOWNEXTEND:
            PageMove ( 1, Selection::selStream );
            NotifyCaretMoved();
            break;
        case SCI_PAGEDOWNRECTEXTEND:
            PageMove ( 1, Selection::selRectangle );
            NotifyCaretMoved();
            break;
```	    
/**6.**

---
**"Update gutter width"-feature**

New notification code added:

[scintilla/include/Scintilla.h]
```
#define SCN_LINECOUNTCHANGED 2032
```

New notification proc added:

[scintilla/src/Editor.h]
```
void NotifyLineCountChanged();
```

[scintilla/src/Editor.cxx]
```
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
        NotifyLineCountChanged();
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
        
    case SCI_LINEDUPLICATE:
        ...
        NotifyLineCountChanged();
        break;
    case SCI_SELECTIONDUPLICATE:
        ...
        NotifyLineCountChanged();
        break;
...

Corresponding calls added to Editor::WndProc():

    case SCI_ADDTEXT: {
        ...
        NotifyLineCountChanged();
        return 0;
    }
...

New code around DropAt()-call:

		position = positionAfterDeletion;

		*const int linesTotal = pdoc->LinesTotal();*
		std::string convertedText = Document::TransformLineEnds(value, lengthValue, pdoc->eolMode);
		
...

		if (rectangular) {
			PasteRectangular(position, convertedText.c_str(), static_cast<int>(convertedText.length()));
			// Should try to select new rectangle but it may not be a rectangle now so just select the drop position
			SetEmptySelection(position);
		} else {
			position = MovePositionOutsideChar(position, sel.MainCaret() - position.Position());
			position = SelectionPosition(InsertSpace(position.Position(), position.VirtualSpace()));
			const int lengthInserted = pdoc->InsertString(
				position.Position(), convertedText.c_str(), static_cast<int>(convertedText.length()));
			if (lengthInserted > 0) {
				SelectionPosition posAfterInsertion = position;
				posAfterInsertion.Add(lengthInserted);
				SetSelection(posAfterInsertion, position);
			}
		}
		*if (pdoc->LinesTotal() != linesTotal) {
			NotifyLineCountChanged();
		}*
```		

/**"Update gutter width"-feature**

---
**Drag & drop improvement #63**

New code around DropAt()-call:

[scintilla/win32/ScintillaWin.cxx]
```
    const bool bIsTrailingLineEnd = (data.size() >= 3) && (data[data.size() - 3] == '\r') && (data[data.size() - 2] == '\n');
    const bool bAddNewLine = (inDragDrop != ddDragging) && (!bIsTrailingLineEnd && pdoc->IsLineStartPosition(movePos.Position()) && pdoc->IsLineEndPosition(movePos.Position()));
    if (bAddNewLine)
    {
      data.insert(data.end() - 1, '\r');
      data.insert(data.end() - 1, '\n');
    }
    DropAt(movePos, &data[0], data.size() - 1, \*pdwEffect == DROPEFFECT_MOVE, hrRectangular == S_OK);
    if (bAddNewLine)
    {
      KeyCommand(SCI_CHARLEFTEXTEND);
    }
```    
/**Drag & drop improvement #63**

---
**Implement Notepad's right click behavior #54**

Add new message SCI_MOVECARETONRCLICK:

[scintilla/include/Scintilla.h]
```
#define SCI_MOVECARETONRCLICK 2369
```

[scintilla/src/ScintillaBase.h]
```
 	enum { maxLenInputIME = 200 };

    bool moveCaretOnRClick;
    bool displayPopupMenu;
```    

[scintilla/src/ScintillaBase.cxx]
```
ScintillaBase::ScintillaBase() {
    *moveCaretOnRClick = true;*
    displayPopupMenu = true;

...

    case SCI_MOVECARETONRCLICK:
        moveCaretOnRClick = wParam != 0;
        break;*

    case SCI_USEPOPUP:
        displayPopupMenu = wParam != 0;
        break;
```

[scintilla/win32/ScintillaWin.cxx]
```
      case WM_RBUTTONDOWN:
        ::SetFocus(MainHWND());
        if ( *moveCaretOnRClick &&* !PointInSelection(Point::FromLong(static_cast<long>(lParam))))
        {
```

/**Implement Notepad's right click behavior #54**

---
**Unindent and tabs #128**

[scintilla/src/Editor.cxx]

Change the code in Editor::Indent(bool forwards): replace condition code block
```
    if (pdoc->GetColumn(caretPosition) <= pdoc->GetLineIndentation(lineCurrentPos) &&
                                                pdoc->tabIndents) {
            int indentation = pdoc->GetLineIndentation(lineCurrentPos);
            int indentationStep = pdoc->IndentSize();
            const int posSelect = pdoc->SetLineIndentation(lineCurrentPos, indentation - indentationStep);
            sel.Range(r) = SelectionRange(posSelect);
    }
```
with:
```
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
    }
```
/**Unindent and tabs #128**

---
**ctrl+arrow behavior toggle #89**

Add new message SCI_SETWORDNAVIGATIONMODE:

[scintilla/include/Scintilla.h]
```
#define SCI_SETWORDNAVIGATIONMODE 2379
```

Add message handler in ScintillaBase::WndProc:

[scintilla/src/ScintillaBase.cxx]
```
	case SCI_SETWORDNAVIGATIONMODE:
		pdoc->SetWordNavigationMode((int)wParam);
		break;
```

Add method declaration/implemention to Document class:

[scintilla/src/Document.h]
```
	double durationStyleOneLine;
	*int wordNavigationMode;*
...
	int BraceMatch(int position, int maxReStyle);
	void SetWordNavigationMode(const int iMode);
```

[scintilla/src/Document.cxx]
```
	durationStyleOneLine = 0.00001;
	*wordNavigationMode = 0;*
...

int Document::NextWordStart(int pos, int delta) {
	if (delta < 0) {
		// [2e]: ctrl+arrow behavior toggle #89
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
			// [2e]: ctrl+arrow behavior toggle #89
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
			// [/2e]
		default:
			// not implemented
			PLATFORM_ASSERT(false);
			break;
		}
	} else {
		// [2e]: ctrl+arrow behavior toggle #89
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
			// [2e]: ctrl+arrow behavior toggle #89
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
	}
	return pos;
}
..
void Document::SetWordNavigationMode(const int iMode)
{
	wordNavigationMode = iMode;
}
```

/**ctrl+arrow behavior toggle #89**

---
**Regexp: confine to single line #90**

Move class RESearchRange declaration/implementation from Document.cxx to Document.h

/**Regexp: confine to single line #90**

---
**Increasingly slow to hex/base64/qp #142**

Add new message SCI_SETSKIPUIUPDATE:

[scintilla/include/Scintilla.h]
```
#define SCI_GETSUBSTYLEBASES 4026
#define SCI_SETSKIPUIUPDATE 9000
```

Add corresponding flag to Editor class:
[scintilla/src/Editor.h]
```
    bool convertPastes;
    bool skipUIUpdate;
```

[scintilla/src/Editor.cxx]
```
    convertPastes = true;
    skipUIUpdate = false;
...
void Editor::RedrawRect(PRectangle rc) {
    //Platform::DebugPrintf("Redraw %0d,%0d - %0d,%0d\n", rc.left, rc.top, rc.right, rc.bottom);
    if (skipUIUpdate) {
        return;
    }
...
void Editor::Redraw() {
    //Platform::DebugPrintf("Redraw all\n");
    if (skipUIUpdate) {
        return;
    }
...
void Editor::InvalidateSelection(SelectionRange newMain, bool invalidateWholeSelection) {
    if (skipUIUpdate) {
        return;
    }
...
void Editor::EnsureCaretVisible(bool useMargin, bool vert, bool horiz) {
    if (skipUIUpdate) {
        return;
    }
...
void Editor::InvalidateCaret() {
    if (skipUIUpdate) {
        return;
    }
...
void Editor::Paint(Surface *surfaceWindow, PRectangle rcArea) {
    if (skipUIUpdate) {
        return;
    }
...
```
Replace the code in Editor::WndProc() for case SCI_REPLACESEL:
```
            SetEmptySelection(sel.MainCaret() + lengthInserted);
            EnsureCaretVisible();
```	    
with
```
            if (!skipUIUpdate) {
                    SetEmptySelection(sel.MainCaret() + lengthInserted);
                    EnsureCaretVisible();
            }
```

Add next handler to Editor::WndProc():
```
    case SCI_SETSKIPUIUPDATE:
        skipUIUpdate = (wParam != 0);
        if (!skipUIUpdate) {
            InvalidateWholeSelection();
            Redraw();
        }
        return skipUIUpdate;
```

/**Increasingly slow to hex/base64/qp #142**

---
**DPI awareness #154**

Add new message SCI_SETDPI:
[scintilla/include/Scintilla.h]
```
#define SCI_SETDPI 9001
```

Add message handler and replace some code:

[scintilla/win32/ScintillaWin.cxx]
```
    drtp.dpiX = DEFAULT_SCREEN_DPI;
    drtp.dpiY = DEFAULT_SCREEN_DPI;

...

        DEFAULT_SCREEN_DPI, DEFAULT_SCREEN_DPI, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT),

...

      case SCI_SETDPI:
        SetDPI(LOWORD(wParam),
               HIWORD(wParam),
               MulDiv(DEFAULT_FONT_DPI, DEFAULT_SCREEN_DPI, GetDpiY()));
        InvalidateStyleData();
        RefreshStyleData();
        return 0;

... 

            drtp.dpiX = DEFAULT_SCREEN_DPI;
            drtp.dpiY = DEFAULT_SCREEN_DPI;
```

Add required subroutines:

[scintilla/win32/PlatWin.h]
```
#define DEFAULT_SCREEN_DPI 96
#define DEFAULT_FONT_DPI 72

void SetDPI(const float _dpiX, const float _dpiY, const int _dpiFont);
float GetDpiX();
float GetDpiY();
int GetDpiFont();
```

[scintilla/win32/PlatWin.cxx]
```
static float dpiX = DEFAULT_SCREEN_DPI;
static float dpiY = DEFAULT_SCREEN_DPI;
static int dpiFont = DEFAULT_FONT_DPI;

void SetDPI(const float _dpiX, const float _dpiY, const int _dpiFont)
{
    dpiX = _dpiX;
    dpiY = _dpiY;
    dpiFont = _dpiFont;
}

float GetDpiX()
{
    return dpiX;
}

float GetDpiY()
{
    return dpiY;
}

int GetDpiFont()
{
    return dpiFont;
}

...

int SurfaceGDI::LogPixelsY() {
    return GetDpiY();
}

int SurfaceGDI::DeviceHeightFont(int points) {
    return ::MulDiv(points, LogPixelsY(), DEFAULT_FONT_DPI);
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
```

/**DPI awareness #154**

---
**Lua LPeg Lexers #251**

Add ifdef-code:

[scintilla/lua/src/lauxlib.c]
```
#define LUA_LIB

#ifdef _WIN32
#include <Windows.h>
#endif

...

#ifdef _WIN32
    wchar_t wfilename[MAX_PATH] = { 0 };
    MultiByteToWideChar(CP_UTF8, 0, filename, strlen(filename), wfilename, sizeof(wfilename) / sizeof(wchar_t));
    lf.f = _wfopen(wfilename, L"r");
#else
    lf.f = fopen(filename, "r");
#endif

...

#ifdef _WIN32
    wchar_t wfilename[MAX_PATH] = { 0 };
    MultiByteToWideChar(CP_UTF8, 0, filename, strlen(filename), wfilename, sizeof(wfilename) / sizeof(wchar_t));
    lf.f = _wfreopen(wfilename, L"rb", lf.f);  /* reopen in binary mode */
#else
    lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
#endif
```

[scintilla/lua/src/loadlib.c]
```
#ifdef _WIN32
  wchar_t wfilename[MAX_PATH] = { 0 };
  MultiByteToWideChar(CP_UTF8, 0, filename, strlen(filename), wfilename, sizeof(wfilename)/sizeof(wchar_t));
  FILE *f = _wfopen(wfilename, L"r");  /* try to open file */
#else
  FILE *f = fopen(filename, "r");  /* try to open file */
#endif
```

/**Lua LPeg Lexers #251**

---

**Inconsistent behaviour in navigation on a line containing Japanese symbols #247**

Replace code:

[scintilla/src/EditView.cxx]
```
posRet = ll->LineStart(subLine + 1) + posLineStart - 1;
```
->
```
posRet = model.pdoc->MovePositionOutsideChar(ll->LineStart(subLine + 1) + posLineStart - 1, -1, false);
```

/**Inconsistent behaviour in navigation on a line containing Japanese symbols #247**

---
**Treat quotes as braces #287**

[/scintilla/src/Document.cxx]

Replace ``BraceOpposite()`` and ``BraceMatch()`` using the following code:

```
static char BraceOpposite(char ch, bool treatQuotesAsBraces) noexcept {
	switch (ch) {
	case '(':
		return ')';
	case ')':
		return '(';
	case '[':
		return ']';
	case ']':
		return '[';
	case '{':
		return '}';
	case '}':
		return '{';
	case '<':
		return '>';
	case '>':
		return '<';
	default:
		// [2e]: Treat quotes as braces #287
		if (treatQuotesAsBraces && (ch == '\'' || ch == '"' || ch == '`'))
			return ch;

		return '\0';
	}
}

// [2e]: Treat quotes as braces #287
int Document::FindBrace(Sci::Position position, const int direction, const char chBrace, const char chSeek, const int styBrace, bool* separatorFound) const noexcept {
	int depth = 1;
	position = NextPosition(position, direction);
	while ((position >= 0) && (position < LengthNoExcept())) {
		const char chAtPos = CharAt(position);
		const int styAtPos = StyleIndexAt(position);

		if (separatorFound && ((chAtPos == '\r') || (chAtPos == '\n') || (chAtPos == '=')))
			*separatorFound = true;

		if ((position > GetEndStyled()) || (styAtPos == styBrace)) {
			if ((chBrace != chSeek) && (chAtPos == chBrace))
				depth++;
			if (chAtPos == chSeek)
				depth--;
			if (depth == 0)
				return position;
		}
		const Sci::Position positionBeforeMove = position;
		position = NextPosition(position, direction);
		if (position == positionBeforeMove)
			break;
	}
	return -1;
}
// [/2e]

// TODO: should be able to extend styled region to find matching brace
Sci::Position Document::BraceMatch(Sci::Position position, bool treatQuotesAsBraces) noexcept {
	const char chBrace = CharAt(position);
	const char chSeek = BraceOpposite(chBrace, treatQuotesAsBraces);
	if (chSeek == '\0')
		return - 1;
	const int styBrace = StyleIndexAt(position);
	int direction = -1;
	if (chBrace == '(' || chBrace == '[' || chBrace == '{' || chBrace == '<')
		direction = 1;
	// [2e]: Treat quotes as braces #287
	else if (treatQuotesAsBraces && (chBrace == '\'' || chBrace == '"' || chBrace == '`'))
	{
		bool separatorForPrev = false;
		bool separatorForNext = false;
		const auto posPrev = FindBrace(position, -1, chBrace, chSeek, styBrace, &separatorForPrev);
		const auto posNext = FindBrace(position, 1, chBrace, chSeek, styBrace, &separatorForNext);
		if (posPrev >= 0 && posNext < 0)
		{
			return posPrev;
		}
		else if (posNext >= 0 && posPrev < 0)
		{
			return posNext;
		}
		else if (posPrev >= 0 && posNext >= 0)
		{
			Sci::Position posNearest, posAlternative;
			bool separatorForNearest = false;
			if (position - posPrev < posNext - position - 1)
			{
				posNearest = posPrev;
				separatorForNearest = separatorForPrev;
				posAlternative = posNext;
			}
			else
			{
				posNearest = posNext;
				separatorForNearest = separatorForNext;
				posAlternative = posPrev;
			}
			return !separatorForNearest ? posNearest : posAlternative;
		}
		return -1;
	}
	return FindBrace(position, direction, chBrace, chSeek, styBrace, nullptr);
	// [/2e]
}
```
**/Treat quotes as braces #287**

---

**Alt+Arrow to invert accelerated mode for single navigation #323**

Add new messages:

[scintilla/include/Scintilla.h]
```
#define SCI_ALTWORDLEFT 9002
#define SCI_ALTWORDLEFTEXTEND 9003
#define SCI_ALTWORDRIGHT 9004
#define SCI_ALTWORDRIGHTEXTEND 9005
```

Add changes:

[scintilla/src/Document.h]
```
    Sci::Position NextWordStart(Sci::Position pos, int delta, bool useAlternativeNavigation) const;
...
    void SetWordNavigationMode(const int iMode);
    int CalcWordNavigationMode(const bool invertMode) const;
```

[scintilla/src/Document.cxx]
```
Sci::Position Document::NextWordStart(Sci::Position pos, int delta, bool useAlternativeNavigation) const {
    if (delta < 0) {
        // [2e]: ctrl+arrow behavior toggle #89
        switch (CalcWordNavigationMode(useAlternativeNavigation))
...

// [2e]: Alt + Arrow to invert accelerated mode for single navigation #323
int Document::CalcWordNavigationMode(const bool invertMode) const
{
    if (!invertMode)
        return wordNavigationMode;

    return (wordNavigationMode != 0) ? 0 : 1;
}
```

[scintilla/src/Editor.cxx]
```
static bool IsAltWordMessage(unsigned int iMessage) noexcept {
    switch (iMessage) {
    case SCI_ALTWORDLEFT:
    case SCI_ALTWORDLEFTEXTEND:
    case SCI_ALTWORDRIGHT:
    case SCI_ALTWORDRIGHTEXTEND:
        return true;
    default:
        return false;
    }
}

int Editor::HorizontalMove(unsigned int iMessage) {
...
            case SCI_WORDLEFT:
            case SCI_WORDLEFTEXTEND:
            case SCI_ALTWORDLEFT:
            case SCI_ALTWORDLEFTEXTEND:
                spCaret = SelectionPosition(pdoc->NextWordStart(spCaret.Position(), -1, IsAltWordMessage(iMessage)));
                break;
            case SCI_WORDRIGHT:
            case SCI_WORDRIGHTEXTEND:
            case SCI_ALTWORDRIGHT:
            case SCI_ALTWORDRIGHTEXTEND:
                spCaret = SelectionPosition(pdoc->NextWordStart(spCaret.Position(), 1, IsAltWordMessage(iMessage)));
                break;
...

            case SCI_WORDLEFT:
            case SCI_ALTWORDLEFT:
            case SCI_WORDRIGHT:
            case SCI_ALTWORDRIGHT:
...
            case SCI_WORDLEFTEXTEND:
            case SCI_ALTWORDLEFTEXTEND:
            case SCI_WORDRIGHTEXTEND:
            case SCI_ALTWORDRIGHTEXTEND:
...

        case SCI_DELWORDLEFT:
            rangeDelete = Range(
                pdoc->NextWordStart(sel.Range(r).caret.Position(), -1, false),
                sel.Range(r).caret.Position());
            break;
        case SCI_DELWORDRIGHT:
            rangeDelete = Range(
                sel.Range(r).caret.Position(),
                pdoc->NextWordStart(sel.Range(r).caret.Position(), 1, false));
            break;
...
```

/**Alt+Arrow to invert accelerated mode for single navigation #323**

---

**Ctrl+Shift+D - affect selected lines #354**

Replace SCI_LINEDELETE handler:

[scintilla/src/Editor.cxx]
```
	case SCI_LINECUT: {
			const Sci::Line lineStart = pdoc->SciLineFromPosition(SelectionStart().Position());
			const Sci::Line lineEnd = pdoc->SciLineFromPosition(SelectionEnd().Position());
			const Sci::Position start = pdoc->LineStart(lineStart);
			const Sci::Position end = pdoc->LineStart(lineEnd + 1);
			SetSelection(start, end);
			Cut();
			SetLastXChosen();
		}
		break;
```

/**Ctrl+Shift+D - affect selected lines #354**

---
**Focus target split view after drag & drop between views #406**

New notification code added:

[scintilla/include/Scintilla.h]
```
#define SCN_DROPCOMPLETED 2035
```

New notification proc added:

[scintilla/src/Editor.h]
```
void NotifyDropCompleted();
```

[scintilla/src/Editor.cxx]
```
void Editor::NotifyDropCompleted()
{
  SCNotification scn = { 0 };
  scn.nmhdr.code = SCN_DROPCOMPLETED;
  NotifyParent(scn);
}
```

Corresponding call added to ```Editor::DropAt()```:
```
	if ((inDragDrop != ddDragging) || !(positionWasInSelection) ||
	        (positionOnEdgeOfSelection && !moving)) {
		...
		NotifyDropCompleted();
		...

	} else if (inDragDrop == ddDragging) {
		SetEmptySelection(position);
	}
```

/**Focus target split view after drag & drop between views #406**

---

**Graphical artifacts in Indentation Guides #396**

Add new method to class ``Surface``:

[scintilla/include/Platform.h]
```
    virtual void InitPixMap(int width, int height, Surface *surface_, WindowID wid)=0;
    virtual void EndDraw()=0;
```

Add implementation for class ``SurfaceGDI``:

[scintilla/win32/PlatWin.cxx]
```
	void InitPixMap(int width, int height, Surface *surface_, WindowID wid) override;
	void EndDraw() override;

...

void SurfaceGDI::EndDraw() {
}
```

Add implementation for class ``SurfaceD2D``:

[scintilla/win32/PlatWin.cxx]
```
	void InitPixMap(int width, int height, Surface *surface_, WindowID wid) override;
	void EndDraw() override;

...

void SurfaceD2D::EndDraw() {
	if (pRenderTarget)
		pRenderTarget->EndDraw();
}
```

Add the following code to ``EditView::RefreshPixMaps``:

[scintilla/src/EditView.cxx]
```
void EditView::RefreshPixMaps(Surface *surfaceWindow, WindowID wid, const ViewStyle &vsDraw) {
	if (!pixmapIndentGuide->Initialised()) {

		...

		pixmapIndentGuide->EndDraw();
		pixmapIndentGuideHighlight->EndDraw();
	}
}
```

Add the following code to ``MarginView::RefreshPixMaps``:

[scintilla/src/MarginView.cxx]
```
void MarginView::RefreshPixMaps(Surface *surfaceWindow, WindowID wid, const ViewStyle &vsDraw) {
	if (!pixmapSelPattern->Initialised()) {

		...

		pixmapSelPattern->EndDraw();
		pixmapSelPatternOffset1->EndDraw();
	}
}
```

/**Graphical artifacts in Indentation Guides #396**

---