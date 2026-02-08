Changes in Scintilla's source code:

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

Declare ``IsLineEnd`` function:

[scintilla/src/Document.h]
```
constexpr bool IsLineEndChar(char c) noexcept;

```

Modify ``IsLineEnd`` function:

[scintilla/src/Document.cpp]
```
static constexpr bool IsLineEndChar(char c) noexcept {

->

constexpr bool IsLineEndChar(char c) noexcept {

```


New code around DropAt()-call:

[scintilla/win32/ScintillaWin.cxx]
```
    const bool bIsTrailingLineEnd = (data.size() >= 2) && IsLineEndChar(*++data.rbegin());
    const bool bAddNewLine = (inDragDrop != ddDragging) && (!bIsTrailingLineEnd && pdoc->IsLineStartPosition(movePos.Position()) && pdoc->IsLineEndPosition(movePos.Position()));
    if (bAddNewLine)
    {
        const std::string eol(StringFromEOLMode(pdoc->eolMode));
        data.insert(data.cend() - 1, eol.cbegin(), eol.cend());
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
				while ((pos > 0)
					&& (WordCharacterClass(cb.CharAt(pos)) == CharClassify::ccNewLine))
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
					CharClassify::cc ccCurrent = WordCharacterClass(cb.CharAt(pos));
					while (pos > 0)
					{
						CharClassify::cc ccPrev = WordCharacterClass(cb.CharAt(pos - 1));
						if ((ccCurrent == CharClassify::ccNewLine)
								|| (((ccPrev == CharClassify::ccSpace) || (ccPrev == CharClassify::ccNewLine))
										&& (ccCurrent != CharClassify::ccSpace))
							)
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
				while ((pos < Length()) && (WordCharacterClass(cb.CharAt(pos)) == CharClassify::ccNewLine))
				{
					pos++;
					stopAtCurrentNewLine = true;
				}
				if (stopAtCurrentNewLine)
				{
					while ((pos < Length())
						&& (WordCharacterClass(cb.CharAt(pos)) != CharClassify::ccWord)
						&& (WordCharacterClass(cb.CharAt(pos)) != CharClassify::ccNewLine))
					{
						pos++;
					}
				}
				else
				{
					pos++;
					assert(pos > 0);
					CharClassify::cc ccPrev = WordCharacterClass(cb.CharAt(pos - 1));
					while (pos < Length())
					{
						CharClassify::cc ccCurrent = WordCharacterClass(cb.CharAt(pos));
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

[/scintilla/src/Document.h]

Replace ``FindBrace`` and ``BraceMatch`` declaration:

```
	int FindBrace(Sci::Position position, const int direction, const char chBrace, const char chSeek, const int styBrace, const bool respectStyle) const noexcept;
	Sci::Position BraceMatch(Sci::Position position, bool treatQuotesAsBraces) noexcept;
```

[/scintilla/src/Document.cxx]

Replace ``BraceOpposite``, ``FindBrace`` and ``BraceMatch`` using the following code:

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
int Document::FindBrace(Sci::Position position, const int direction, const char chBrace, const char chSeek, const int styBrace, const bool respectStyle) const noexcept {
	int depth = 1;
	position = NextPosition(position, direction);
	while ((position >= 0) && (position < LengthNoExcept())) {
		const char chAtPos = CharAt(position);
		const int styAtPos = StyleIndexAt(position);

		if ((position > GetEndStyled()) || (!respectStyle || (styAtPos == styBrace))) {
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
		const auto lineIndex = LineFromPosition(position);
		const auto lineStartPos = LineStart(lineIndex);
		const auto lineEndPos = LineEnd(lineIndex);
		int braceCount = 0;
		int escapedBraceCount = 0;
		int bracePosition = -1;
		auto i = lineStartPos - 1;

		if (CharAt(NextPosition(position, -1)) == '\\')
			return -1;

		while (i < position)
		{
			bracePosition = FindBrace(i, 1, chBrace, chSeek, styBrace, false);
			if (bracePosition < 0)
				break;
			if ((bracePosition != lineStartPos) && (CharAt(NextPosition(bracePosition, -1)) == '\\'))
				++escapedBraceCount;
			else
			{
				if (bracePosition >= position)
					break;
				++braceCount;
			}
			i = bracePosition;
		}

		bracePosition = -1;
		const auto direction = (braceCount % 2 == 0) ? 1 : -1;
		i = position;
		do 
		{
			bracePosition = FindBrace(i, direction, chBrace, chSeek, styBrace, false);
			if (CharAt(NextPosition(bracePosition, -1)) == '\\')
			{
				i = bracePosition;
				--escapedBraceCount;
				bracePosition = -1;
			}
			else if ((bracePosition >= lineEndPos) || (i >= lineEndPos))
			{
				bracePosition = -1;
				break;
			}
			else if (bracePosition >= 0)
				break;
			else
				i = NextPosition(i, direction);
		} while ((bracePosition < 0) || (escapedBraceCount > 0));

		return bracePosition;
	}
	return FindBrace(position, direction, chBrace, chSeek, styBrace, true);
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
#define SCI_ALTDELWORDLEFT 9006
#define SCI_ALTDELWORDRIGHT 9007
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
    case SCI_ALTDELWORDLEFT:
    case SCI_ALTDELWORDRIGHT:
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


int Editor::DelWordOrLine(unsigned int iMessage) {

...
	const bool leftwards = (iMessage == SCI_DELWORDLEFT) || (iMessage == SCI_DELLINELEFT) || (iMessage == SCI_ALTDELWORDLEFT);
...
			if (iMessage != SCI_ALTDELWORDRIGHT)
				// Delete to the right so first realise the virtual space.
				sel.Range(r) = SelectionRange(
					RealizeVirtualSpace(sel.Range(r).caret));
...

		case SCI_DELWORDLEFT:
			rangeDelete = Range(
				pdoc->NextWordStart(sel.Range(r).caret.Position(), -1, false),
				sel.Range(r).caret.Position());
			break;
		case SCI_ALTDELWORDLEFT:
			if (sel.Range(r).anchor.Position() != sel.Range(r).caret.Position())
				rangeDelete = Range(
					std::min(sel.Range(r).anchor.Position(), sel.Range(r).caret.Position()),
					std::max(sel.Range(r).anchor.Position(), sel.Range(r).caret.Position()));
			else
				rangeDelete = Range(
					pdoc->NextWordStart(sel.Range(r).caret.Position(), -1, true),
					sel.Range(r).caret.Position());
			break;
		case SCI_DELWORDRIGHT:
			rangeDelete = Range(
				sel.Range(r).caret.Position(),
				pdoc->NextWordStart(sel.Range(r).caret.Position(), 1, false));
			break;
		case SCI_ALTDELWORDRIGHT:
			if (sel.Range(r).anchor.Position() != sel.Range(r).caret.Position())
				rangeDelete = Range(
					std::min(sel.Range(r).anchor.Position(), sel.Range(r).caret.Position()),
					std::max(sel.Range(r).anchor.Position(), sel.Range(r).caret.Position()));
			else
				rangeDelete = Range(
					sel.Range(r).caret.Position(),
					pdoc->NextWordStart(sel.Range(r).caret.Position(), 1, true));
			break;

...

    case SCI_DELWORDLEFT:
    case SCI_ALTDELWORDLEFT:
    case SCI_DELWORDRIGHT:
    case SCI_ALTDELWORDRIGHT:

...

    case SCI_DELWORDLEFT:
    case SCI_ALTDELWORDLEFT:
    case SCI_DELWORDRIGHT:
    case SCI_ALTDELWORDRIGHT:
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

**View > St&arting Line Number... #342**

Add new message SCI_SETSTARTINGLINENUMBER:

[scintilla/include/Scintilla.h]
```
#define SCI_SETSTARTINGLINENUMBER 2383
```

Add message handler in ScintillaBase::WndProc:

[scintilla/src/ScintillaBase.cxx]
```
	case SCI_SETSTARTINGLINENUMBER:
		pdoc->SetStartingLineNumber((int)wParam);
		break;
```

Add method declaration/implemention to Document class:

[scintilla/src/Document.h]
```
	double durationStyleOneLine;
	int wordNavigationMode;
	*int startingLineNumber*
...
	void SetStartingLineNumber(const int lineNumber);
	int GetStartingLineNumber() const;
```

[scintilla/src/Document.cxx]
```
	durationStyleOneLine = 0.00001;
	wordNavigationMode = 0;
	*startingLineNumber = 0;*
...

void Document::SetStartingLineNumber(const int lineNumber)
{
	startingLineNumber = lineNumber;
}

int Document::GetStartingLineNumber() const
{
	return startingLineNumber;
}
```

[scintilla/src/MarginView.cxx]

Replace code in MarginView::PaintMargin:
```
                        if (lineDoc >= 0) {
                            sNumber = std::to_string(lineDoc + 1);
                        }
```

->

```
                        if (lineDoc >= 0) {
                            sNumber = std::to_string(lineDoc + model.pdoc->GetStartingLineNumber());
                        }
```

/**View > St&arting Line Number... #342**

---
**Extremely slow Replace when changing line count #363**

Add ``if (!skipUIUpdate)`` conditions into ``Editor::NotifyModified``:

[scintilla/src/Editor.cxx]

```
			// Avoid scrolling of display if change before current display
			if (mh.position < posTopLine && !CanDeferToLastStep(mh)) {
				const Sci::Line newTop = Sci::clamp(topLine + mh.linesAdded, static_cast<Sci::Line>(0), MaxScrollPos());
				if (newTop != topLine) {
					SetTopLine(newTop);
					if (!skipUIUpdate)
  						SetVerticalScrollPos();
				}
			}
```

```
	if (mh.linesAdded != 0 && !CanDeferToLastStep(mh)) {
		if (!skipUIUpdate)
			SetScrollBars();
	}
```

/**Extremely slow Replace when changing line count #363**

---
**Join Lines/Paragraphs - Alt modifier to not add space #451**

Add ``noSpaceDelimiter`` option to ``Editor``:

[scintilla/src/Editor.h]
```
    void LinesJoin(const bool noSpaceDelimiter);
```

[scintilla/src/Editor.cxx]
```
void Editor::LinesJoin(const bool noSpaceDelimiter) {

...

			if (pdoc->IsPositionInLineEnd(pos)) {
				const int lenChar = pdoc->LenChar(pos);
				targetRange.end.Add(-lenChar);
				pdoc->DelChar(pos);
				if (prevNonWS && (!noSpaceDelimiter || pdoc->IsPositionInLineEnd(pos))) {
					// Ensure at least one space separating previous lines
					const Sci::Position lengthInserted = pdoc->InsertString(pos, " ", 1);
					targetRange.end.Add(lengthInserted);
				}
				else if (noSpaceDelimiter && (lenChar > 1)) {
					--pos;
				}
			} else {

...

    case SCI_LINESJOIN:
        LinesJoin();
        LinesJoin(static_cast<bool>(wParam));
        break;

```


/**Join Lines/Paragraphs - Alt modifier to not add space #451**

---
**Shift/Ctrl+Alt+Left/Right to navigate word start/end #436**

Add new commands:

[Scintilla/include/Scintilla.h]:
```
#define SCI_ALTWORDLEFT2 9008
#define SCI_ALTWORDLEFTEXTEND2 9009
#define SCI_ALTWORDRIGHT2 9010
#define SCI_ALTWORDRIGHTEXTEND2 9011
#define SCI_ALTDELWORDLEFT2 9012
#define SCI_ALTDELWORDRIGHT2 9013
```
[/Scintilla/include/Scintilla.h]

[Scintilla/src/Document.h]:

Update methods declaration:
```
    Sci::Position NextWordStart(Sci::Position pos, int delta, int alternativeNavigationMode) const;
...
    int CalcWordNavigationMode(const int navigationMode) const;
```
[Scintilla/src/Document.h]

[Scintilla/src/Document.cxx]:

Update ``Document::NextWordStart`` implementation:
```
Sci::Position Document::NextWordStart(Sci::Position pos, int delta, int alternativeNavigationMode) const {
	if (delta < 0) {
		// [2e]: ctrl+arrow behavior toggle #89
		switch (CalcWordNavigationMode(alternativeNavigationMode))
...
        case 2:
            // [2e]: Shift/Ctrl+Alt+Left/Right to navigate word start/end #436
            {
                if (pos > 0)
                {
                    pos--;
                }
                CharClassify::cc ccCurrent = WordCharacterClass(cb.CharAt(pos));
                while (pos > 0)
                {
                    CharClassify::cc ccPrev = WordCharacterClass(cb.CharAt(pos - 1));
                    if (((ccPrev == CharClassify::ccWord) || (ccPrev == CharClassify::ccPunctuation))
                        && ((ccCurrent != CharClassify::ccWord) && (ccCurrent != CharClassify::ccPunctuation)))                        
                        break;
                    pos--;
                    ccCurrent = ccPrev;
                }
            }
            break;
...
    } else {
        // [2e]: ctrl+arrow behavior toggle #89
        switch (CalcWordNavigationMode(alternativeNavigationMode))
...
        case 2:
            // [2e]: Shift/Ctrl+Alt+Left/Right to navigate word start/end #436
            {
                pos++;
                assert(pos > 0);
                CharClassify::cc ccPrev = WordCharacterClass(cb.CharAt(pos - 1));
                while (pos < Length())
                {
                    CharClassify::cc ccCurrent = WordCharacterClass(cb.CharAt(pos));
                    if ((ccCurrent != CharClassify::ccWord) && (ccCurrent != CharClassify::ccPunctuation)
                        && ((ccPrev == CharClassify::ccWord) || (ccPrev == CharClassify::ccPunctuation)))
                        break;
                    pos++;
                    ccPrev = ccCurrent;
                }
            }
            break;
```
Update ``Document::CalcWordNavigationMode``:
```
int Document::CalcWordNavigationMode(const int navigationMode) const
{
	if (navigationMode == 0)
		return wordNavigationMode;

	return navigationMode;
}
```

[/Scintilla/src/Document.cxx]:

[Scintilla/src/Editor.cxx]:
Replace ``IsAltWordMessage()`` with ``AltWordNavigationMode()``:
```
static int AltWordNavigationMode(unsigned int iMessage) noexcept {
	switch (iMessage) {
	case SCI_ALTWORDLEFT:
	case SCI_ALTWORDLEFTEXTEND:
	case SCI_ALTWORDRIGHT:
	case SCI_ALTWORDRIGHTEXTEND:
	case SCI_ALTDELWORDLEFT:
	case SCI_ALTDELWORDRIGHT:
		return 1;
	case SCI_ALTWORDLEFT2:
	case SCI_ALTWORDLEFTEXTEND2:
	case SCI_ALTDELWORDLEFT2:
	case SCI_ALTWORDRIGHT2:
	case SCI_ALTWORDRIGHTEXTEND2:
	case SCI_ALTDELWORDRIGHT2:
		return 2;
	default:
		return 0;
	}
}
```


Update  ``int Editor::HorizontalMove(unsigned int iMessage)``:
```
			case SCI_WORDLEFT:
			case SCI_WORDLEFTEXTEND:
			case SCI_ALTWORDLEFT:
			case SCI_ALTWORDLEFT2:
			case SCI_ALTWORDLEFTEXTEND:
			case SCI_ALTWORDLEFTEXTEND2:
				spCaret = SelectionPosition(pdoc->NextWordStart(spCaret.Position(), -1, AltWordNavigationMode(iMessage)));
				break;
			case SCI_WORDRIGHT:
			case SCI_WORDRIGHTEXTEND:
			case SCI_ALTWORDRIGHT:
			case SCI_ALTWORDRIGHT2:
			case SCI_ALTWORDRIGHTEXTEND:
			case SCI_ALTWORDRIGHTEXTEND2:
				spCaret = SelectionPosition(pdoc->NextWordStart(spCaret.Position(), 1, AltWordNavigationMode(iMessage)));
				break;
...

			case SCI_WORDLEFT:
			case SCI_ALTWORDLEFT:
			case SCI_ALTWORDLEFT2:
			case SCI_WORDRIGHT:
			case SCI_ALTWORDRIGHT:
			case SCI_ALTWORDRIGHT2:
...

			case SCI_CHARLEFTEXTEND:
			case SCI_CHARRIGHTEXTEND:
			case SCI_WORDLEFTEXTEND:
			case SCI_ALTWORDLEFTEXTEND:
			case SCI_ALTWORDLEFTEXTEND2:
			case SCI_WORDRIGHTEXTEND:
			case SCI_ALTWORDRIGHTEXTEND:
			case SCI_ALTWORDRIGHTEXTEND2:

```

Update ``int Editor::DelWordOrLine(unsigned int iMessage)``:
```
	const bool leftwards = (iMessage == SCI_DELWORDLEFT) || (iMessage == SCI_DELLINELEFT) || (iMessage == SCI_ALTDELWORDLEFT) || (iMessage == SCI_ALTDELWORDLEFT2);

	if (!additionalSelectionTyping) {
		InvalidateWholeSelection();
		sel.DropAdditionalRanges();
	}

	UndoGroup ug0(pdoc, (sel.Count() > 1) || !leftwards);

	for (size_t r = 0; r < sel.Count(); r++) {
		if (leftwards) {
			// Delete to the left so first clear the virtual space.
			sel.Range(r).ClearVirtualSpace();
		} else {
			if ((iMessage != SCI_ALTDELWORDRIGHT) && (iMessage != SCI_ALTDELWORDRIGHT2))
				// Delete to the right so first realise the virtual space.
				sel.Range(r) = SelectionRange(
					RealizeVirtualSpace(sel.Range(r).caret));
		}

		Range rangeDelete;
		switch (iMessage) {
		case SCI_DELWORDLEFT:
			rangeDelete = Range(
				pdoc->NextWordStart(sel.Range(r).caret.Position(), -1, AltWordNavigationMode(iMessage)),
				sel.Range(r).caret.Position());
			break;
		case SCI_ALTDELWORDLEFT:
		case SCI_ALTDELWORDLEFT2:
			if (sel.Range(r).anchor.Position() != sel.Range(r).caret.Position())
				rangeDelete = Range(
					std::min(sel.Range(r).anchor.Position(), sel.Range(r).caret.Position()),
					std::max(sel.Range(r).anchor.Position(), sel.Range(r).caret.Position()));
			else
				rangeDelete = Range(
					pdoc->NextWordStart(sel.Range(r).caret.Position(), -1, AltWordNavigationMode(iMessage)),
					sel.Range(r).caret.Position());
			break;
		case SCI_DELWORDRIGHT:
			rangeDelete = Range(
				sel.Range(r).caret.Position(),
				pdoc->NextWordStart(sel.Range(r).caret.Position(), 1, AltWordNavigationMode(iMessage)));
			break;
		case SCI_ALTDELWORDRIGHT:
		case SCI_ALTDELWORDRIGHT2:
			if (sel.Range(r).anchor.Position() != sel.Range(r).caret.Position())
				rangeDelete = Range(
					std::min(sel.Range(r).anchor.Position(), sel.Range(r).caret.Position()),
					std::max(sel.Range(r).anchor.Position(), sel.Range(r).caret.Position()));
			else
				rangeDelete = Range(
					sel.Range(r).caret.Position(),
					pdoc->NextWordStart(sel.Range(r).caret.Position(), 1, AltWordNavigationMode(iMessage)));
			break;
		case SCI_DELWORDRIGHTEND:
			rangeDelete = Range(
				sel.Range(r).caret.Position(),
				pdoc->NextWordEnd(sel.Range(r).caret.Position(), AltWordNavigationMode(iMessage)));
			break;

```
Update ``int Editor::KeyCommand(unsigned int iMessage)``:
```
	case SCI_ALTWORDLEFT:
	case SCI_ALTWORDLEFT2:
	case SCI_WORDLEFTEXTEND:
	case SCI_ALTWORDLEFTEXTEND:
	case SCI_ALTWORDLEFTEXTEND2:
	case SCI_WORDRIGHT:
	case SCI_ALTWORDRIGHT:
	case SCI_ALTWORDRIGHT2:
	case SCI_WORDRIGHTEXTEND:
	case SCI_ALTWORDRIGHTEXTEND:
	case SCI_ALTWORDRIGHTEXTEND2:
...
	case SCI_DELWORDLEFT:
	case SCI_ALTDELWORDLEFT:
	case SCI_ALTDELWORDLEFT2:
	case SCI_DELWORDRIGHT:
	case SCI_ALTDELWORDRIGHT:
	case SCI_ALTDELWORDRIGHT2:
```

Update ``sptr_t Editor::WndProc()``:
```
...
	case SCI_ALTWORDLEFT:
	case SCI_ALTWORDLEFT2:
	case SCI_WORDLEFTEXTEND:
	case SCI_ALTWORDLEFTEXTEND:
	case SCI_ALTWORDLEFTEXTEND2:
	case SCI_WORDRIGHT:
	case SCI_ALTWORDRIGHT:
	case SCI_ALTWORDRIGHT2:
	case SCI_WORDRIGHTEXTEND:
	case SCI_ALTWORDRIGHTEXTEND:
	case SCI_ALTWORDRIGHTEXTEND2:
...
	case SCI_ALTDELWORDLEFT:
	case SCI_ALTDELWORDLEFT2:
	case SCI_DELWORDRIGHT:
	case SCI_ALTDELWORDRIGHT:
	case SCI_ALTDELWORDRIGHT2:
```

[/Scintilla/src/Editor.cxx]:


/**Shift/Ctrl+Alt+Left/Right to navigate word start/end #436**

---

**Back/Forward caret navigation hotkeys (Ctrl+Alt/Shift+O) #360**

Add new commands:

[Scintilla/include/Scintilla.h]:
```
#define SCI_UNDOPOSITION 2800
#define SCI_REDOPOSITION 2801
```
[/Scintilla/include/Scintilla.h]

Update editor module:

[Scintilla/src/Editor.h]:

```
#ifndef EDITOR_H
#define EDITOR_H

// [2e]: Back/Forward caret navigation hotkeys (Ctrl+Alt/Shift+O) #360
#include "PositionHistory.h"
...
    bool convertPastes;
    bool skipUIUpdate;
    PositionHistory ph;    // [2e]: Back/Forward caret navigation hotkeys (Ctrl+Alt/Shift+O) #360
...
    virtual void ClaimSelection() = 0;

    virtual void UndoPosition();
    virtual void RedoPosition();
...
```
[Scintilla/src/Editor.h]

[Scintilla/src/Editor.cxx]:

Add new statement after every ``    sel.RangeMain() = ...``-entry, i.e:
```
...
    sel.RangeMain() = rangeNew;
    ph.PushPosition(sel.RangeMain());
...
```
Add new statement:
```
...
    sel.Clear();
    ph.Clear();
...
```
Add new methods implementation:
```
void Editor::UndoPosition() {
    if (ph.CanUndo()) {
        InvalidateCaret();
        sel.RangeMain() = ph.UndoPosition();
        if (sel.IsRectangular())
            sel.DropAdditionalRanges();
        InvalidateSelection(sel.RangeMain(), true);
        EnsureCaretVisible();
    }
}

void Editor::RedoPosition() {
    if (ph.CanRedo()) {
        InvalidateCaret();
        sel.RangeMain() = ph.RedoPosition();
        if (sel.IsRectangular())
            sel.DropAdditionalRanges();
        InvalidateSelection(sel.RangeMain(), true);
        EnsureCaretVisible();
    }
}
```

Add new commands:
```
...
    case SCI_CANUNDO:
        return (pdoc->CanUndo() && !pdoc->IsReadOnly()) ? 1 : 0;

    case SCI_UNDOPOSITION:
        UndoPosition();
        break;

    case SCI_REDOPOSITION:
        RedoPosition();
        break;
...
```
[/Scintilla/src/Editor.cxx]:

[Scintilla/src/KeyMap.cxx]:

Update ``KeyMap::MapDefault`` with new commands:
```
...
#else
    {'Y',             SCI_CTRL,    SCI_REDO},
#endif
    {'O',             SCI_CSHIFT,    SCI_UNDOPOSITION},
    {'O',             SCI_CTRL | SCI_ALT,    SCI_REDOPOSITION},
    {'X',             SCI_CTRL,    SCI_CUT},
...
```

[/Scintilla/src/KeyMap.cxx]:

Add ``PositionHistory`` module:

[Scintilla/src/PositionHistory.h]:
```
// [2e]: Back/Forward caret navigation hotkeys (Ctrl+Alt/Shift+O) #360
#ifndef POSITIONHISTORY_H
#define POSITIONHISTORY_H

namespace Scintilla {

	class PositionHistory {
		std::vector<SelectionRange> positions;
		std::vector<SelectionRange> redoPositions;

	public:
		void Clear() noexcept {
			positions.clear();
			redoPositions.clear();
		}
		bool CanUndo() const noexcept {
			return positions.size() > 0;
		}
		bool CanRedo() const noexcept {
			return redoPositions.size() > 0;
		}
		void PushPosition(const SelectionRange& sr) noexcept {
			if (!positions.empty()
				&& (positions.back().Length() > 0)
				&& (positions.back().Contains(sr.anchor) || positions.back().Contains(sr.caret)))
				positions.pop_back();
			if (positions.empty() || !(positions.back() == sr))
				positions.push_back(sr);
			redoPositions.clear();
		}
		SelectionRange UndoPosition() noexcept {
			if (!CanUndo())
				return {};
			redoPositions.push_back(positions.back());
			positions.pop_back();
			return (positions.size() > 0) ? positions.back() : redoPositions.back();
		}
		SelectionRange RedoPosition() noexcept {
			if (!CanRedo())
				return {};
			positions.push_back(redoPositions.back());
			redoPositions.pop_back();
			return positions.back();
		}
	};

}

#endif

```

[/Scintilla/src/PositionHistory.h]:


/**Back/Forward caret navigation hotkeys (Ctrl+Alt/Shift+O) #360**

---

**Selection start changes if replacing leading substring #468**

Replace implementation for ``SelectionRange::MoveForInsertDelete``:

[Scintilla/src/Selection.cxx]:
```
void SelectionRange::MoveForInsertDelete(bool insertion, Sci::Position startChange, Sci::Position length) noexcept {
	caret.MoveForInsertDelete(insertion, startChange, length, false);
	anchor.MoveForInsertDelete(insertion, startChange, length, false);
}
```
[/Scintilla/src/Selection.h]

/**Selection start changes if replacing leading substring #468**

---

**Paste clipboard file(s) as list of paths #471**

Change condition in ``bool ScintillaWin::CanPaste()``:

[Scintilla/win32/ScintillaWin.cxx]:
```
    if (!Editor::CanPaste())
        return false;
    if (::IsClipboardFormatAvailable(CF_TEXT) || ::IsClipboardFormatAvailable(CF_HDROP))
        return true;
    if (IsUnicodeMode())
        ...
```
[/Scintilla/win32/ScintillaWin.cxx]

Add ``else`` block to ``void ScintillaWin::Paste()``:

[Scintilla/win32/ScintillaWin.cxx]:
```
		GlobalMemory memSelection(::GetClipboardData(CF_TEXT));
		if (memSelection) {
			...
		}
		else if (HDROP hDropFiles = (HDROP)::GetClipboardData(CF_HDROP))
		{
			const UINT count = DragQueryFileA(hDropFiles, -1, NULL, 0);
			for (UINT i = 0; i < count; ++i)
			{
				UINT size = DragQueryFile(hDropFiles, i, NULL, 0);
				if (size > 0)
				{
					size_t len = size + 1;
					std::vector<wchar_t> filename;
					filename.resize(len);
					DragQueryFile(hDropFiles, i, &filename[0], len);
					if (i < count - 1)
					{
						filename.pop_back();
						const std::string eol(StringFromEOLMode(pdoc->eolMode));
						filename.insert(filename.cend(), eol.cbegin(), eol.cend());
						len = filename.size();
					}
					const size_t mlen = UTF8Length(&filename[0], len);
					std::vector<char> putf(mlen + 1);
					UTF8FromUTF16(&filename[0], len, &putf[0], mlen);
					InsertPasteShape(&putf[0], mlen, pasteShape);
				}
			}
		}
```
[/Scintilla/win32/ScintillaWin.cxx]

/**Paste clipboard file(s) as list of paths #471**

---

**Find first/last match indication #388**

Add new styles and message to Scintilla.h:

[scintilla/include/Scintilla.h]:
```
#define STYLE_LINEINDICATOR 40
#define STYLE_LINEINDICATOR_FIRST_LAST 41
..
#define SCI_SETINDICATEDLINES 9014
```
[/scintilla/include/Scintilla.h]

Append code to EditModel-class:

[scintilla/src/EditModel.h]:
```
	// [2e]: Find first/last match indication #388
	Sci::Line firstIndicatedLine = -1, lastIndicatedLine = -1;
	Sci::Line beforeIndicatedLine = -1, afterIndicatedLine = -1;
	int topIndicatedLine() const { return std::max(firstIndicatedLine, beforeIndicatedLine); }
	int bottomIndicatedLine() const { return std::max(lastIndicatedLine, afterIndicatedLine); }
	virtual void SetIndicatedLines(
		const Sci::Line line1, const bool isFirst,
		const Sci::Line line2, const bool isLast) {
			firstIndicatedLine = isFirst ? line1 : -1;
			beforeIndicatedLine = isFirst ? -1 : line1;
			lastIndicatedLine = isLast ? line2 : -1;
			afterIndicatedLine = isLast ? -1 : line2;
	}
    // [/2e]
```
[/scintilla/src/EditModel.h]

Add new code to Editor-class:

[scintilla/src/Editor.h]:
```
...
    void ClearAll();
    // [2e]: Find first / last match indication #388
    void ClearIndicatedLines();
    bool ClearIndicatedLinesIfRequired(const Sci::Position pos);
    // [/[2e]
...
```
[/scintilla/src/Editor.h]

[scintilla/src/Editor.cxx]:
```
...
void Editor::InsertPasteShape(const char *text, Sci::Position len, PasteShape shape) {
    ClearIndicatedLinesIfRequired(sel.MainCaret());
...
                sel.Range(r) = SelectionRange(sel.Range(r).Start());
                ClearIndicatedLinesIfRequired(sel.Range(r).Start().Position());
...
    sel.Clear();
    ph.Clear();
    ClearIndicatedLines();
    SetTopLine(0);
...
void Editor::ClearIndicatedLines()
{
    firstIndicatedLine = lastIndicatedLine = beforeIndicatedLine = afterIndicatedLine = -1;
}

bool Editor::ClearIndicatedLinesIfRequired(const Sci::Position pos)
{
    if (((firstIndicatedLine >= 0) || (beforeIndicatedLine >= 0)) && ((lastIndicatedLine >= 0) || (afterIndicatedLine >= 0)))
    {
        const Range rangeIndicated(
            pdoc->LineStart(firstIndicatedLine >= 0 ? firstIndicatedLine : beforeIndicatedLine),
            pdoc->LineEnd(lastIndicatedLine >= 0 ? lastIndicatedLine : afterIndicatedLine));
        if (rangeIndicated.Contains(pos))
        {
            ClearIndicatedLines();
            return true;
        }
    }
    return false;
}
...
                    pdoc->DelChar(sel.Range(r).caret.Position());
                    ClearIndicatedLinesIfRequired(sel.Range(r).caret.Position());
...
                        } else {
                            pdoc->DelCharBack(sel.Range(r).caret.Position());
                            ClearIndicatedLinesIfRequired(sel.Range(r).caret.Position());
                        }
...
        if (insertLength > 0) {
            sel.Range(r) = SelectionRange(positionInsert + insertLength);
            countInsertions++;
            ClearIndicatedLinesIfRequired(positionInsert);
        }
...
            const Sci::Position end = pdoc->LineStart(lineEnd + 1);
            pdoc->DeleteChars(start, end - start);
            ClearIndicatedLines();
            NotifyLineCountChanged();
...
```
[/scintilla/src/Editor.cxx]

Update EditView module:

[scintilla/src/EditView.cxx]:
```
...
static void DrawLineIndicator(Surface *surface, const ViewStyle &vsDraw, const Sci::Line line,
    const bool showIndicatorFirst, const bool showIndicatorLast,
    const bool showIndicatorBefore, const bool showIndicatorAfter, PRectangle rcLine) {
    if (showIndicatorFirst || showIndicatorBefore) {
        PRectangle rcIndicatorLine = rcLine;
        rcIndicatorLine.bottom = rcIndicatorLine.top + 1;
        surface->FillRectangle(rcIndicatorLine, vsDraw.styles[showIndicatorFirst ? STYLE_LINEINDICATOR_FIRST_LAST : STYLE_LINEINDICATOR].fore);
    }
    if (showIndicatorLast || showIndicatorAfter) {
        PRectangle rcIndicatorLine = rcLine;
        rcIndicatorLine.top = rcIndicatorLine.bottom - 1;
        surface->FillRectangle(rcIndicatorLine, vsDraw.styles[showIndicatorLast ? STYLE_LINEINDICATOR_FIRST_LAST : STYLE_LINEINDICATOR].fore);
    }
}
...
    if (phase & drawIndicatorsFore) {
        DrawIndicators(surface, model, vsDraw, ll, line, xStart, rcLine, subLine, lineRangeIncludingEnd.end, false, model.hoverIndicatorPos);
        DrawLineIndicator(surface, vsDraw, line,
            (line == model.firstIndicatedLine) && !subLine, (line == model.lastIndicatedLine) && (subLine == ll->lines - 1),
            (line == model.beforeIndicatedLine) && !subLine, (line == model.afterIndicatedLine) && (subLine == ll->lines - 1), rcLine);
    }
...
```
[/scintilla/src/EditView.cxx]

Add new message handler to ScintillaWin:

[scintilla/win32/ScintillaWin.cxx]:
```
...
		case SCI_SETDPI:
			SetDPI(LOWORD(wParam),
				HIWORD(wParam),
				MulDiv(DEFAULT_FONT_DPI, DEFAULT_SCREEN_DPI, GetDpiY()));
			InvalidateStyleData();
			RefreshStyleData();
			return 0;

		case SCI_SETINDICATEDLINES:
			if ((topIndicatedLine() >= 0) || (bottomIndicatedLine() >= 0))
			{
				const Sci::Line topLine = topIndicatedLine();
				const Sci::Line bottomLine = bottomIndicatedLine();
				if (bottomLine < 0)
					InvalidateRange(pdoc->LineStart(topLine), pdoc->LineEnd(topLine));
				else if (topLine < 0)
					InvalidateRange(pdoc->LineStart(bottomLine), pdoc->LineEnd(bottomLine));
				else
					InvalidateRange(pdoc->LineStart(topLine), pdoc->LineEnd(bottomLine));
			}
			Editor::SetIndicatedLines(LOWORD(wParam), HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
			return 0;

...
```
[/scintilla/win32/ScintillaWin.cxx]

/**Find first/last match indication #388**

---

**Scale highlight border and caret size according to DPI #472**

Add forward declaration:

[scintilla/include/Platform.h]:

```
inline int dsf();
```
[/scintilla/include/Platform.h]

Add ``dsf()``-implementation and update methods:

[scintilla/win32/PlatWin.cxx]
```
static int dpiFont = DEFAULT_FONT_DPI;
// [2e]: Scale highlight border and caret size according to DPI #472
static int dpiScalingFactor = 1;
...
void SetDPI(const float _dpiX, const float _dpiY, const int _dpiFont)
{
	dpiX = _dpiX;
	dpiY = _dpiY;
	dpiFont = _dpiFont;
	dpiScalingFactor = (int)floor(0.5 + _dpiX * 1.0 / DEFAULT_SCREEN_DPI);
}
...
int GetDpiFont()
{
	return dpiFont;
}

inline int dsf()
{
	return dpiScalingFactor;
}
...
void SurfaceGDI::PenColour(ColourDesired fore) {
	if (pen) {
		::SelectObject(hdc, penOld);
		::DeleteObject(pen);
		pen = 0;
		penOld = 0;
	}
	pen = ::CreatePen(0,dsf(),fore.AsInteger());
	penOld = SelectPen(hdc, pen);
}
...
void SurfaceGDI::RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) {
	PenColour(fore);
	BrushColour(back);
	const RECT rcw = RectFromPRectangle(rc);
	::RoundRect(hdc,
		rcw.left + dsf(), rcw.top,
		rcw.right - dsf(), rcw.bottom,
		8, 8);
}
...
void SurfaceGDI::AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
		ColourDesired outline, int alphaOutline, int /* flags*/ ) {
	const RECT rcw = RectFromPRectangle(rc);
	if (rc.Width() > 0) {
		HDC hMemDC = ::CreateCompatibleDC(hdc);
		const int width = rcw.right - rcw.left;
		const int height = rcw.bottom - rcw.top;
		// Ensure not distorted too much by corners when small
		cornerSize = std::min(cornerSize, (std::min(width, height) / 2) - 2);
		const BITMAPINFO bpih = {{sizeof(BITMAPINFOHEADER), width, height, 1, 32, BI_RGB, 0, 0, 0, 0, 0},
			{{0, 0, 0, 0}}};
		void *image = nullptr;
		HBITMAP hbmMem = CreateDIBSection(hMemDC, &bpih,
			DIB_RGB_COLORS, &image, NULL, 0);

		if (hbmMem) {
			HBITMAP hbmOld = SelectBitmap(hMemDC, hbmMem);

			const DWORD valEmpty = dwordFromBGRA(0,0,0,0);
			const DWORD valFill = dwordMultiplied(fill, alphaFill);
			const DWORD valOutline = dwordMultiplied(outline, alphaOutline);

			DWORD *pixels = static_cast<DWORD *>(image);
			if (cornerSize > 1)
			{
				HRGN hrgnRoundRectAll = CreateRoundRectRgn(0, 0, width, height, cornerSize, cornerSize);
				HRGN hrgnRoundRectInner = CreateRoundRectRgn(cornerSize, cornerSize, width - cornerSize, height - cornerSize, cornerSize, cornerSize);
				HRGN hrgnRoundRectOutline = CreateRectRgn(0, 0, width, height);
				CombineRgn(hrgnRoundRectOutline, hrgnRoundRectAll, hrgnRoundRectInner, RGN_DIFF);
				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						if (PtInRegion(hrgnRoundRectOutline, x, y))
							pixels[y*width + x] = valOutline;
						else if (PtInRegion(hrgnRoundRectInner, x, y))
							pixels[y*width + x] = valFill;
						else
							pixels[y*width + x] = valEmpty;
					}
				}
				DeleteObject(hrgnRoundRectAll);
				DeleteObject(hrgnRoundRectOutline);
				DeleteObject(hrgnRoundRectInner);
			}
			else {
				for (int y=0; y<height; y++) {
					for (int x=0; x<width; x++) {
						if ((x==0) || (x==width-1) || (y == 0) || (y == height-1)) {
							pixels[y*width+x] = valOutline;
						} else {
							pixels[y*width+x] = valFill;
						}
					}
				}
				for (int c=0; c<cornerSize; c++) {
					for (int x=0; x<c+1; x++) {
						AllFour(pixels, width, height, x, c-x, valEmpty);
					}
				}
				for (int x=1; x<cornerSize; x++) {
					AllFour(pixels, width, height, x, cornerSize-x, valOutline);
				}
			}

			const BLENDFUNCTION merge = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

			AlphaBlend(hdc, rcw.left, rcw.top, width, height, hMemDC, 0, 0, width, height, merge);

			SelectBitmap(hMemDC, hbmOld);
			::DeleteObject(hbmMem);
		}
		::DeleteDC(hMemDC);
	} else {
		BrushColour(outline);
		FrameRect(hdc, &rcw, brush);
	}
}
...
void SurfaceD2D::LineTo(int x_, int y_) {
	if (pRenderTarget) {
		const int xDiff = x_ - x;
		const int xDelta = Delta(xDiff);
		const int yDiff = y_ - y;
		const int yDelta = Delta(yDiff);
		if ((xDiff == 0) || (yDiff == 0)) {
			// Horizontal or vertical lines can be more precisely drawn as a filled rectangle
			const int xEnd = x_ - xDelta;
			const int left = std::min(x, xEnd);
			const int width = std::abs(x - xEnd) + dsf();
			const int yEnd = y_ - yDelta;
			const int top = std::min(y, yEnd);
			const int height = std::abs(y - yEnd) + dsf();
			const D2D1_RECT_F rectangle1 = D2D1::RectF(static_cast<float>(left), static_cast<float>(top),
				static_cast<float>(left+width), static_cast<float>(top+height));
			pRenderTarget->FillRectangle(&rectangle1, pBrush);
		} else if ((std::abs(xDiff) == std::abs(yDiff))) {
			// 45 degree slope
			pRenderTarget->DrawLine(D2D1::Point2F(x + 0.5f, y + 0.5f),
				D2D1::Point2F(x_ + 0.5f - xDelta, y_ + 0.5f - yDelta), pBrush, dsf());
		} else {
			// Line has a different slope so difficult to avoid last pixel
			pRenderTarget->DrawLine(D2D1::Point2F(x + 0.5f, y + 0.5f),
				D2D1::Point2F(x_ + 0.5f, y_ + 0.5f), pBrush, dsf());
		}
		x = x_;
		y = y_;
	}
}
...
void SurfaceD2D::AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
		ColourDesired outline, int alphaOutline, int /* flags*/ ) {
	if (pRenderTarget) {
		if (cornerSize == 0) {
			// When corner size is zero, draw square rectangle to prevent blurry pixels at corners
			const D2D1_RECT_F rectFill = D2D1::RectF(std::round(rc.left) + 1.0f * dsf(), rc.top + 1.0f * dsf(), std::round(rc.right) - 1.0f * dsf(), rc.bottom - 1.0f * dsf());
			D2DPenColour(fill, alphaFill);
			pRenderTarget->FillRectangle(rectFill, pBrush);

			const D2D1_RECT_F rectOutline = D2D1::RectF(std::round(rc.left) + 0.5f * dsf(), rc.top + 0.5f * dsf(), std::round(rc.right) - 0.5f * dsf(), rc.bottom - 0.5f * dsf());
			D2DPenColour(outline, alphaOutline);
			pRenderTarget->DrawRectangle(rectOutline, pBrush);
		} else {
			const float cornerSizeF = static_cast<float>(cornerSize);
			D2D1_ROUNDED_RECT roundedRectFill = {
				D2D1::RectF(std::round(rc.left) + 1.0f * dsf(), rc.top + 1.0f * dsf(), std::round(rc.right) - 1.0f * dsf(), rc.bottom - 1.0f * dsf()),
				cornerSizeF - 1.0f * dsf(), cornerSizeF - 1.0f * dsf() };
			D2DPenColour(fill, alphaFill);
			pRenderTarget->FillRoundedRectangle(roundedRectFill, pBrush);

			D2D1_ROUNDED_RECT roundedRect = {
				D2D1::RectF(std::round(rc.left) + 0.5f * dsf(), rc.top + 0.5f * dsf(), std::round(rc.right) - 0.5f * dsf(), rc.bottom - 0.5f * dsf()),
				cornerSizeF, cornerSizeF};
			D2DPenColour(outline, alphaOutline);
			pRenderTarget->DrawRoundedRectangle(roundedRect, pBrush);
		}
	}
}

```
[/scintilla/win32/PlatWin.cxx]

Update ``ScintillaWin::CreateSystemCaret()``:

[scintilla/win32/ScintillaWin.cxx]
```
BOOL ScintillaWin::CreateSystemCaret() {
	sysCaretWidth = vs.caretWidth * dsf();
	if (0 == sysCaretWidth) {
...
```
[/scintilla/win32/ScintillaWin.cxx]

[scintilla/src/EditView.cxx]
```
void EditView::RefreshPixMaps(Surface *surfaceWindow, WindowID wid, const ViewStyle &vsDraw) {
	if (!pixmapIndentGuide->Initialised()) {
		// 1 extra pixel in height so can handle odd/even positions and so produce a continuous line
		pixmapIndentGuide->InitPixMap(1*dsf(), vsDraw.lineHeight + 1*dsf(), surfaceWindow, wid);
		pixmapIndentGuideHighlight->InitPixMap(1*dsf(), vsDraw.lineHeight + 1*dsf(), surfaceWindow, wid);
		const PRectangle rcIG = PRectangle::FromInts(0, 0, 1*dsf(), vsDraw.lineHeight);
		pixmapIndentGuide->FillRectangle(rcIG, vsDraw.styles[STYLE_INDENTGUIDE].back);
		pixmapIndentGuide->PenColour(vsDraw.styles[STYLE_INDENTGUIDE].fore);
		pixmapIndentGuideHighlight->FillRectangle(rcIG, vsDraw.styles[STYLE_BRACELIGHT].back);
		pixmapIndentGuideHighlight->PenColour(vsDraw.styles[STYLE_BRACELIGHT].fore);
		for (int stripe = 1*dsf(); stripe < vsDraw.lineHeight + 1; stripe += 2*dsf()) {
			const PRectangle rcPixel = PRectangle::FromInts(0, stripe, 1*dsf(), stripe + 1*dsf());
			pixmapIndentGuide->FillRectangle(rcPixel, vsDraw.styles[STYLE_INDENTGUIDE].fore);
			pixmapIndentGuideHighlight->FillRectangle(rcPixel, vsDraw.styles[STYLE_BRACELIGHT].fore);
		}
		pixmapIndentGuide->EndDraw();
		pixmapIndentGuideHighlight->EndDraw();
	}
}
...
void EditView::DrawIndentGuide(Surface *surface, Sci::Line lineVisible, int lineHeight, XYPOSITION start, PRectangle rcSegment, bool highlight) {
	const Point from = Point::FromInts(0, ((lineVisible & 1) && (lineHeight & 1)) ? 1 : 0);
	const PRectangle rcCopyArea(start + 1*dsf(), rcSegment.top,
		start + 2*dsf(), rcSegment.bottom);
	surface->Copy(rcCopyArea, from,
		highlight ? *pixmapIndentGuideHighlight : *pixmapIndentGuide);
}
```

Modify block in ``EditView::DrawCaret()``:
```
				if (drawDrag) {
					/* Dragging text, use a line caret */
					rcCaret.left = round(xposCaret - caretWidthOffset);
					rcCaret.right = rcCaret.left + vsDraw.caretWidth * dsf();
				} else if ((caretShape == ViewStyle::CaretShape::bar) && drawOverstrikeCaret) {
					/* Overstrike (insert mode), use a modified bar caret */
					rcCaret.top = rcCaret.bottom - 2;
					rcCaret.left = xposCaret + 1;
					rcCaret.right = rcCaret.left + widthOverstrikeCaret - 1;
				} else if ((caretShape == ViewStyle::CaretShape::block) || imeCaretBlockOverride) {
					/* Block caret */
					rcCaret.left = xposCaret;
					if (!caretAtEOL && !caretAtEOF && (ll->chars[offset] != '\t') && !(IsControlCharacter(ll->chars[offset]))) {
						drawBlockCaret = true;
						rcCaret.right = xposCaret + widthOverstrikeCaret;
					} else {
						rcCaret.right = xposCaret + vsDraw.aveCharWidth;
					}
				} else {
					/* Line caret */
					rcCaret.left = round(xposCaret - caretWidthOffset);
					rcCaret.right = rcCaret.left + vsDraw.caretWidth * dsf();
				}
```

Modify block in ``EditView::DrawForeground()``:
```
						const int halfDotWidth = vsDraw.whitespaceSize / 2;
						PRectangle rcDot(xmid + xStart - halfDotWidth - static_cast<XYPOSITION>(subLineStart),
							rcSegment.top + vsDraw.lineHeight / 2, 0.0f, 0.0f);
						rcDot.right = rcDot.left + vsDraw.whitespaceSize;
						rcDot.bottom = rcDot.top + vsDraw.whitespaceSize;
						surface->FillRectangle(rcDot, textFore);
```
[/scintilla/src/EditView.cxx]

Replace ``Indicator::Draw()`` implementation:

[scintilla/src/Indicator.cxx]
```
void Indicator::Draw(Surface *surface, const PRectangle &rc, const PRectangle &rcLine, const PRectangle &rcCharacter, DrawState drawState, int value) const {
	StyleAndColour sacDraw = sacNormal;
	if (Flags() & SC_INDICFLAG_VALUEFORE) {
		sacDraw.fore = ColourDesired(value & SC_INDICVALUEMASK);
	}
	if (drawState == drawHover) {
		sacDraw = sacHover;
	}
	const IntegerRectangle irc(rc);
	surface->PenColour(sacDraw.fore);
	const int ymid = (irc.bottom + irc.top) / 2;
	if (sacDraw.style == INDIC_SQUIGGLE) {
		const IntegerRectangle ircSquiggle(PixelGridAlign(rc));
		int x = ircSquiggle.left;
		const int xLast = ircSquiggle.right;
		int y = 0;
		surface->MoveTo(x, irc.top + y);
		while (x < xLast) {
			if ((x + 2) > xLast) {
				y = 1;
				x = xLast;
			} else {
				x += 2;
				y = 2 - y;
			}
			surface->LineTo(x, irc.top + y);
		}
	} else if (sacDraw.style == INDIC_SQUIGGLEPIXMAP) {
		const PRectangle rcSquiggle = PixelGridAlign(rc);

		const int width = std::min(4000, static_cast<int>(rcSquiggle.Width()));
		RGBAImage image(width, 3, 1.0, nullptr);
		enum { alphaFull = 0xff, alphaSide = 0x2f, alphaSide2=0x5f };
		for (int x = 0; x < width; x++) {
			if (x%2) {
				// Two halfway columns have a full pixel in middle flanked by light pixels
				image.SetPixel(x, 0, sacDraw.fore, alphaSide);
				image.SetPixel(x, 1, sacDraw.fore, alphaFull);
				image.SetPixel(x, 2, sacDraw.fore, alphaSide);
			} else {
				// Extreme columns have a full pixel at bottom or top and a mid-tone pixel in centre
				image.SetPixel(x, (x % 4) ? 0 : 2, sacDraw.fore, alphaFull);
				image.SetPixel(x, 1, sacDraw.fore, alphaSide2);
			}
		}
		surface->DrawRGBAImage(rcSquiggle, image.GetWidth(), image.GetHeight(), image.Pixels());
	} else if (sacDraw.style == INDIC_SQUIGGLELOW) {
		surface->MoveTo(irc.left, irc.top);
		int x = irc.left + 3;
		int y = 0;
		while (x < rc.right) {
			surface->LineTo(x - 1, irc.top + y);
			y = 1 - y;
			surface->LineTo(x, irc.top + y);
			x += 3;
		}
		surface->LineTo(irc.right, irc.top + y);	// Finish the line
	} else if (sacDraw.style == INDIC_TT) {
		surface->MoveTo(irc.left, ymid);
		int x = irc.left + 5 * dsf();
		const int deltaX = ceil(5.0 * dsf() / 2);
		while (x < rc.right) {
			surface->LineTo(x, ymid);
			surface->MoveTo(x - deltaX, ymid);
			surface->LineTo(x - deltaX, ymid + 2);
			x += dsf();
			surface->MoveTo(x, ymid);
			x += 5 * dsf();
		}
		surface->LineTo(irc.right, ymid);	// Finish the line
		if (x - deltaX <= rc.right) {
			surface->MoveTo(x - deltaX, ymid);
			surface->LineTo(x - deltaX, ymid + 2);
		}
	} else if (sacDraw.style == INDIC_DIAGONAL) {
		int x = irc.left;
		while (x < rc.right) {
			surface->MoveTo(x, irc.top + 2);
			int endX = x+3;
			int endY = irc.top - 1;
			if (endX > rc.right) {
				endY += endX - irc.right;
				endX = irc.right;
			}
			surface->LineTo(endX, endY);
			x += 2 + 2 * dsf();
		}
	} else if (sacDraw.style == INDIC_STRIKE) {
		surface->MoveTo(irc.left, irc.top - (2 + 2 * dsf()));
		surface->LineTo(irc.right, irc.top - (2 + 2 * dsf()));
	} else if ((sacDraw.style == INDIC_HIDDEN) || (sacDraw.style == INDIC_TEXTFORE)) {
		// Draw nothing
	} else if (sacDraw.style == INDIC_BOX) {
		surface->MoveTo(irc.left, ymid + dsf());
		surface->LineTo(irc.right, ymid + dsf());
		const int lineTop = static_cast<int>(rcLine.top) + dsf();
		surface->LineTo(irc.right, lineTop);
		surface->LineTo(irc.left, lineTop);
		surface->LineTo(irc.left, ymid + dsf());
	} else if (sacDraw.style == INDIC_ROUNDBOX ||
		sacDraw.style == INDIC_STRAIGHTBOX ||
		sacDraw.style == INDIC_FULLBOX) {
		PRectangle rcBox = rcLine;
		if (sacDraw.style != INDIC_FULLBOX)
			rcBox.top = rcLine.top + 1;
		rcBox.left = rc.left;
		rcBox.right = rc.right;
		surface->AlphaRectangle(rcBox, (sacDraw.style == INDIC_ROUNDBOX) ? dsf() : 0,
			sacDraw.fore, fillAlpha, sacDraw.fore, outlineAlpha, 0);
	} else if (sacDraw.style == INDIC_GRADIENT ||
		sacDraw.style == INDIC_GRADIENTCENTRE) {
		PRectangle rcBox = rc;
		rcBox.top = rcLine.top + 1;
		rcBox.bottom = rcLine.bottom;
		const Surface::GradientOptions options = Surface::GradientOptions::topToBottom;
		const ColourAlpha start(sacNormal.fore, fillAlpha);
		const ColourAlpha end(sacNormal.fore, 0);
		std::vector<ColourStop> stops;
		switch (sacDraw.style) {
		case INDIC_GRADIENT:
			stops.push_back(ColourStop(0.0, start));
			stops.push_back(ColourStop(1.0, end));
			break;
		case INDIC_GRADIENTCENTRE:
			stops.push_back(ColourStop(0.0, end));
			stops.push_back(ColourStop(0.5, start));
			stops.push_back(ColourStop(1.0, end));
			break;
		}
		surface->GradientRectangle(rcBox, stops, options);
	} else if (sacDraw.style == INDIC_DOTBOX) {
		PRectangle rcBox = PixelGridAlign(rc);
		rcBox.top = rcLine.top + 1;
		rcBox.bottom = rcLine.bottom;
		IntegerRectangle ircBox(rcBox);
		// Cap width at 4000 to avoid large allocations when mistakes made
		const int width = std::min(ircBox.Width(), 4000);
		RGBAImage image(width, ircBox.Height(), 1.0, nullptr);
		// Draw horizontal lines top and bottom
		bool drawPixel = true;
		for (int y = 0; y<ircBox.Height(); y += ircBox.Height() - dsf()) {
			for (int x = 0; x < width; x += dsf()) {
				const int color = drawPixel ? fillAlpha : outlineAlpha;
				for (int j = 0; j < dsf(); j++) {
					for (int k = 0; k < dsf(); k++) {
						image.SetPixel(x + j, y + k, sacDraw.fore, color);
					}
				}
				drawPixel = !drawPixel;
			}
		}
		// Draw vertical lines left and right
		drawPixel = false;
		for (int y = dsf(); y<ircBox.Height(); y+=dsf()) {
			for (int x=0; x<width; x += width - dsf()) {
				const int color = drawPixel ? fillAlpha : outlineAlpha;
				for (int j = 0; j < dsf(); j++) {
					for (int k = 0; k < dsf(); k++) {
						image.SetPixel(x + j, y + k, sacDraw.fore, color);
					}
				}
			}
			drawPixel = !drawPixel;
		}
		surface->DrawRGBAImage(rcBox, image.GetWidth(), image.GetHeight(), image.Pixels());
	} else if (sacDraw.style == INDIC_DASH) {
		int x = irc.left;
		while (x < rc.right) {
			surface->MoveTo(x, ymid);
			surface->LineTo(std::min(x + 2 + 2*dsf(), irc.right), ymid);
			x += 3 + 4 * dsf();
		}
	} else if (sacDraw.style == INDIC_DOTS) {
		int x = irc.left;
		while (x < irc.right) {
			const PRectangle rcDot = PRectangle::FromInts(x, ymid, x + dsf(), ymid + dsf());
			surface->FillRectangle(rcDot, sacDraw.fore);
			x += 2 * dsf();
		}
	} else if (sacDraw.style == INDIC_COMPOSITIONTHICK) {
		const PRectangle rcComposition(rc.left+dsf(), rcLine.bottom-2*dsf(), rc.right-dsf(), rcLine.bottom);
		surface->FillRectangle(rcComposition, sacDraw.fore);
	} else if (sacDraw.style == INDIC_COMPOSITIONTHIN) {
		const PRectangle rcComposition(rc.left+dsf(), rcLine.bottom-2*dsf(), rc.right-dsf(), rcLine.bottom-dsf());
		surface->FillRectangle(rcComposition, sacDraw.fore);
	} else if (sacDraw.style == INDIC_POINT || sacDraw.style == INDIC_POINTCHARACTER) {
		if (rcCharacter.Width() >= 0.1) {
			const XYPOSITION pixelHeight = std::floor(rc.Height() - 1.0f) * dsf();	// 1 pixel onto next line if multiphase
			const XYPOSITION x = (sacDraw.style == INDIC_POINT) ? (rcCharacter.left) : ((rcCharacter.right + rcCharacter.left) / 2);
			const XYPOSITION ix = round(x);
			const XYPOSITION iy = std::floor(rc.top + 1.0f*dsf());
			Point pts[] = {
				Point(ix - pixelHeight, iy + pixelHeight),	// Left
				Point(ix + pixelHeight, iy + pixelHeight),	// Right
				Point(ix, iy)								// Top
			};
			surface->Polygon(pts, ELEMENTS(pts), sacDraw.fore, sacDraw.fore);
		}
	} else {	// Either INDIC_PLAIN or unknown
		surface->MoveTo(irc.left, ymid);
		surface->LineTo(irc.right, ymid);
	}
}
```
[/scintilla/src/Indicator.cxx]


/**Scale highlight border and caret size according to DPI #472**

---

**Split views scrolled after closing Customize Schemes #418**

Modify ``Editor::WrapLines(WrapScope ws)``:
move ``subLineTop`` variable declaration after ``AddSample``-call:

[scintilla/include/Platform.h]:

```
				durationWrapOneLine.AddSample(linesBeingWrapped, epWrapping.Duration());

				const Sci::Line subLineTop = topLine - pcs->DisplayFromDoc(lineDocTop);
				goodTopLine = pcs->DisplayFromDoc(lineDocTop) + std::min(
					subLineTop, static_cast<Sci::Line>(pcs->GetHeight(lineDocTop)-1));
```

/**Split views scrolled after closing Customize Schemes #418**

---

**Alt+Home/End to navigate line, not subline #429**

Move ``case SCI_VCHOMERECTEXTEND``-statement (in method ``int Editor::HorizontalMove(unsigned int iMessage)``):

[scintilla/src/Editor.cxx]:

```
		case SCI_VCHOMERECTEXTEND:
		case SCI_HOMERECTEXTEND:
		case SCI_HOMEEXTEND: // only when sel.IsRectangular() && sel.MoveExtends()
			spCaret = SelectionPosition(
				pdoc->LineStart(pdoc->LineFromPosition(spCaret.Position())));
			break;
		case SCI_VCHOMEEXTEND: // only when sel.IsRectangular() && sel.MoveExtends()
			spCaret = SelectionPosition(pdoc->VCHomePosition(spCaret.Position()));
			break;
		case SCI_LINEENDRECTEXTEND:
		case SCI_LINEENDEXTEND:
```
[/scintilla/src/Editor.cxx]

/**Alt+Home/End to navigate line, not subline #429**

---
