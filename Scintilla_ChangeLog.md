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
typedef void ( * wheel_action ) ( int );  
typedef int ( * key_action ) ( int , int );  
extern wheel_action hl_wheel_action ;  
extern key_action hl_proc_action;  
  
[Scintilla/win32/ScintillaWin.cxx]  
wheel_action hl_wheel_action = 0;  
key_action hl_proc_action = 0;  
  
...  
  
          if (wParam & MK_CONTROL)  
          {  
            // Zoom! We play with the font sizes in the styles.  
            // Number of steps/line is ignored, we just care if sizing up or down  
            if ( hl_wheel_action ) {  
                hl_wheel_action ( linesToScroll );  
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
	  
		int ret = hl_proc_action(wParam, WM_CHAR);  
          
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
  
            int ret = hl_proc_action(wParam, WM_KEYDOWN);  
  
            if (ret >= 0)  
  
            {  
  
              return ret;  
  
            }  
          
          }  
  
  
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
[/**Enable additional Lexers**]  
  
  
[**13. "No line selection on active selection"-feature**]:  
Remove triple-click handler in Editor::ButtonDown():  
[scintilla/src/Editor.cxx]  
                if ( selectionType == selChar ) {  
                    selectionType = selWord;  
                    doubleClick = true;  
                } else if ( selectionType == selWord ) {  
                    // do nothing on *triple* click  
                } else {  
                    selectionType = selChar;  
                    originalAnchorPos = sel.MainCaret();  
                }  
[/**13.**]  
  
  
[**6. "Scroll margin"-feature**]:  
New notification code added:  
[scintilla/include/Scintilla.h]  
\#define SCN_CARETMOVED 2031  
  
New notification proc added:  
[scintilla/src/Editor.h]  
void NotifyCaretMoved();  
  
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
[/**6.**]  
  
  
[**"Update gutter width"-feature**]:  
New notification code added:  
[scintilla/include/Scintilla.h]  
\#define SCN_LINECOUNTCHANGED 2032  
  
New notification proc added:  
[scintilla/src/Editor.h]  
void NotifyLineCountChanged();  
  
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
    DropAt(movePos, &data[0], data.size() - 1, \*pdwEffect == DROPEFFECT_MOVE, hrRectangular == S_OK);  
    if (bAddNewLine)  
    {  
      KeyCommand(SCI_CHARRIGHT);  
    }  
[/**Drag & drop improvement #63**]  
  
[**Implement Notepad's right click behavior #54**]  
Add new message SCI_MOVECARETONRCLICK:  
  
[scintilla/include/Scintilla.h]  
\#define SCI_MOVECARETONRCLICK 2369  
[/scintilla/include/Scintilla.h]  
  
[scintilla/src/ScintillaBase.h]  
 	enum { maxLenInputIME = 200 };  
  
    *bool moveCaretOnRClick;*  
    bool displayPopupMenu;  
[/scintilla/src/ScintillaBase.h]  
  
[scintilla/src/ScintillaBase.cxx]  
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
[/scintilla/src/ScintillaBase.cxx]  
  
[scintilla/win32/ScintillaWin.cxx]  
      case WM_RBUTTONDOWN:  
        ::SetFocus(MainHWND());  
        if ( *moveCaretOnRClick &&* !PointInSelection(Point::FromLong(static_cast<long>(lParam))))  
        {  
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
    }    
    
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
    }  
  
  
[scintilla/src/Editor.cxx]  
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
			if (pos > 0)  
				pos--;  
			while (pos > 0)  
			{  
				CharClassify::cc ccPrev = WordCharClass(cb.CharAt(pos - 1));  
				if ((ccPrev == CharClassify::ccNewLine) || (ccPrev == CharClassify::ccSpace))  
					break;  
				pos--;  
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
			if (pos < Length())  
				++pos;  
			while (pos < Length())  
			{  
				CharClassify::cc ccCurrent = WordCharClass(cb.CharAt(pos));  
				CharClassify::cc ccPrev = WordCharClass(cb.CharAt(pos - 1));  
				if ((ccCurrent == CharClassify::ccNewLine) || (ccPrev == CharClassify::ccSpace))  
					break;  
				pos++;  
			}  
			break;  
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
  
[/scintilla/src/Document.cxx]  
[/**ctrl+arrow behavior toggle #89**]  
  
[**Regexp: confine to single line #90**]  
Move class RESearchRange declaration/implementation from Document.cxx to Document.h  
  
[/**Regexp: confine to single line #90**]  