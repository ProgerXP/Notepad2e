#pragma once

/******************************************************************************
*
*
* Notepad2
*
* SciCall.h
*   Inline wrappers for Scintilla API calls, arranged in the order and grouping
*   in which they appear in the Scintilla documentation.
*
* The use of these inline wrapper functions with declared types will ensure
* that we get the benefit of the compiler's type checking.
*
*
******************************************************************************/

#include "Scintilla.h"
#include "Extension/ViewHelper.h"
//=============================================================================
//
//  g_hScintilla
//
//

__forceinline void InitScintillaHandle(HWND hwnd)
{
  SetProp(hwnd, L"SCINTILLA_DIRECTPOINTER", (HANDLE)SendMessage(hwnd, SCI_GETDIRECTPOINTER, 0, 0));
}

__forceinline HANDLE GetScintillaHandle(HWND hwnd)
{
  return (HANDLE)GetProp(hwnd, L"SCINTILLA_DIRECTPOINTER");
}

//=============================================================================
//
//  SciCall()
//
//
LRESULT WINAPI Scintilla_DirectFunction(HANDLE, UINT, WPARAM, LPARAM);
#define SciCall(m, w, l) Scintilla_DirectFunction(GetScintillaHandle(hwndEdit), m, w, l)

//=============================================================================
//
//  DeclareSciCall[RV][0-2] Macros
//
//  R: With an explicit return type
//  V: No return type defined ("void"); defaults to SendMessage's LRESULT
//  0-2: Number of parameters to define
//
//
#define DeclareSciCallR0(fn, msg, ret)                             \
__forceinline ret SciCall_##fn() {                                 \
  return((ret)SciCall(SCI_##msg, 0, 0));                           \
}
#define DeclareSciCallR1(fn, msg, ret, type1, var1)                \
__forceinline ret SciCall_##fn(type1 var1) {                       \
  return((ret)SciCall(SCI_##msg, (WPARAM)(var1), 0));              \
}
#define DeclareSciCallR2(fn, msg, ret, type1, var1, type2, var2)   \
__forceinline ret SciCall_##fn(type1 var1, type2 var2) {           \
  return((ret)SciCall(SCI_##msg, (WPARAM)(var1), (LPARAM)(var2))); \
}
#define DeclareSciCallV0(fn, msg)                                  \
__forceinline LRESULT SciCall_##fn() {                             \
  return(SciCall(SCI_##msg, 0, 0));                                \
}
#define DeclareSciCallV1(fn, msg, type1, var1)                     \
__forceinline LRESULT SciCall_##fn(type1 var1) {                   \
  return(SciCall(SCI_##msg, (WPARAM)(var1), 0));                   \
}
#define DeclareSciCallV2(fn, msg, type1, var1, type2, var2)        \
__forceinline LRESULT SciCall_##fn(type1 var1, type2 var2) {       \
  return(SciCall(SCI_##msg, (WPARAM)(var1), (LPARAM)(var2)));      \
}

//=============================================================================
//
//  Selection and information
//
//
DeclareSciCallR0(GetCodePage, GETCODEPAGE, int);
DeclareSciCallR0(GetLineCount, GETLINECOUNT, int);
DeclareSciCallR0(GetLinesOnScreen, LINESONSCREEN, int);
DeclareSciCallR0(GetLength, GETLENGTH, int);
DeclareSciCallR0(GetSelectionMode, GETSELECTIONMODE, int);
DeclareSciCallR0(GetSelStart, GETSELECTIONSTART, int);
DeclareSciCallR0(GetSelEnd, GETSELECTIONEND, int);
DeclareSciCallV2(SetSel, SETSEL, int, anchorPos, int, currentPos);
DeclareSciCallV2(ReplaceSel, REPLACESEL, int, unused, const char*, text);
DeclareSciCallR1(GetCharAt, GETCHARAT, char, int, position);
DeclareSciCallR0(CharLeftExtEnd, CHARLEFTEXTEND, int);
DeclareSciCallR0(CharRightExtEnd, CHARRIGHTEXTEND, int);
DeclareSciCallR0(GetSelections, GETSELECTIONS, int);
DeclareSciCallR1(GetSelectionNStart, GETSELECTIONNSTART, Sci_Position, int, position);
DeclareSciCallR1(GetSelectionNEnd, GETSELECTIONNEND, Sci_Position, int, position);
DeclareSciCallV1(GotoPos, GOTOPOS, int, position);
DeclareSciCallV1(GotoLine, GOTOLINE, int, line);
DeclareSciCallR0(GetCurrentPos, GETCURRENTPOS, int);
DeclareSciCallV1(SetCurrentPos, SETCURRENTPOS, int, position);
DeclareSciCallR0(GetAnchor, GETANCHOR, int);
DeclareSciCallV1(SetAnchor, SETANCHOR, int, position);
DeclareSciCallR1(LineFromPosition, LINEFROMPOSITION, int, int, position);
DeclareSciCallR1(LineEndPosition, GETLINEENDPOSITION, int, int, line);
DeclareSciCallR1(PositionFromLine, POSITIONFROMLINE, int, int, line);
DeclareSciCallV1(PositionAfter, POSITIONAFTER, int, position);
DeclareSciCallV1(PositionBefore, POSITIONBEFORE, int, position);
DeclareSciCallV2(DeleteRange, DELETERANGE, int, start, int, lengthDelete);
DeclareSciCallV2(FindText, FINDTEXT, int, searchFlags, struct Sci_TextToFind *, ft);
DeclareSciCallV2(GetStyledText, GETSTYLEDTEXT, int, unused, struct Sci_TextToFind *, ft);
DeclareSciCallV2(AddText, ADDTEXT, int, length, const char*, text);
DeclareSciCallR2(GetText, GETTEXT, int, int, length, char*, text);
DeclareSciCallR2(GetTargetText, GETTARGETTEXT, int, int, unused, char*, text);
DeclareSciCallR2(GetTextRange, GETTEXTRANGE, int, int, unused, struct Sci_TextRange*, tr);
DeclareSciCallR2(GetWordStartPos, WORDSTARTPOSITION, int, int, pos, BOOL, onlyWordCharacters);
DeclareSciCallR2(GetWordEndPos, WORDENDPOSITION, int, int, pos, BOOL, onlyWordCharacters);
DeclareSciCallR2(PointXFromPosition, POINTXFROMPOSITION, int, int, unused, int, position);
DeclareSciCallR2(PointYFromPosition, POINTYFROMPOSITION, int, int, unused, int, position);
DeclareSciCallR0(GetFirstVisibleLine, GETFIRSTVISIBLELINE, int);
DeclareSciCallR1(DocLineFromVisible, DOCLINEFROMVISIBLE, int, int, displayLine);
DeclareSciCallV2(BraceMatch, BRACEMATCH, int, pos, unsigned int, bracesMatchMode);

DeclareSciCallV0(ChooseCaretX, CHOOSECARETX);

DeclareSciCallV0(SetSavePoint, SETSAVEPOINT);
DeclareSciCallV0(BeginUndoAction, BEGINUNDOACTION);
DeclareSciCallV0(EndUndoAction, ENDUNDOACTION);
DeclareSciCallV1(SetSkipUIUpdate, SETSKIPUIUPDATE, int, skipUIUpdate);

//=============================================================================
//
//  Scrolling and automatic scrolling
//
//
DeclareSciCallV2(LineScroll, LINESCROLL, int, columns, int, lines);
DeclareSciCallV0(ScrollCaret, SCROLLCARET);
DeclareSciCallV2(SetXCaretPolicy, SETXCARETPOLICY, int, caretPolicy, int, caretSlop);
DeclareSciCallV2(SetYCaretPolicy, SETYCARETPOLICY, int, caretPolicy, int, caretSlop);

//=============================================================================
//
//  Style definition
//
//
DeclareSciCallR1(StyleGetFore, STYLEGETFORE, COLORREF, int, styleNumber);
DeclareSciCallR1(StyleGetBack, STYLEGETBACK, COLORREF, int, styleNumber);

//=============================================================================
//
//  Margins
//
//
DeclareSciCallV2(SetMarginType, SETMARGINTYPEN, int, margin, int, type);
DeclareSciCallV2(SetMarginWidth, SETMARGINWIDTHN, int, margin, int, pixelWidth);
DeclareSciCallV2(SetMarginMask, SETMARGINMASKN, int, margin, int, mask);
DeclareSciCallV2(SetMarginSensitive, SETMARGINSENSITIVEN, int, margin, BOOL, sensitive);
DeclareSciCallV2(SetFoldMarginColour, SETFOLDMARGINCOLOUR, BOOL, useSetting, COLORREF, colour);
DeclareSciCallV2(SetFoldMarginHiColour, SETFOLDMARGINHICOLOUR, BOOL, useSetting, COLORREF, colour);

//=============================================================================
//
//  Markers
//
//
DeclareSciCallV2(MarkerDefine, MARKERDEFINE, int, markerNumber, int, markerSymbols);
DeclareSciCallV2(MarkerSetFore, MARKERSETFORE, int, markerNumber, COLORREF, colour);
DeclareSciCallV2(MarkerSetBack, MARKERSETBACK, int, markerNumber, COLORREF, colour);

//=============================================================================
//
//  Folding
//
//
DeclareSciCallR1(GetLineVisible, GETLINEVISIBLE, BOOL, int, line);
DeclareSciCallR1(GetFoldLevel, GETFOLDLEVEL, int, int, line);
DeclareSciCallV1(SetFoldFlags, SETFOLDFLAGS, int, flags);
DeclareSciCallR1(GetFoldParent, GETFOLDPARENT, int, int, line);
DeclareSciCallR1(GetFoldExpanded, GETFOLDEXPANDED, int, int, line);
DeclareSciCallV1(ToggleFold, TOGGLEFOLD, int, line);
DeclareSciCallV1(EnsureVisible, ENSUREVISIBLE, int, line);
DeclareSciCallV1(SetAutomaticFold, SETAUTOMATICFOLD, int, automaticFold);
DeclareSciCallV2(FoldLine, FOLDLINE, int, line, int, action);
DeclareSciCallV1(FoldAll, FOLDALL, int, action);

//=============================================================================
//
//  Lexer
//
//
DeclareSciCallV2(SetProperty, SETPROPERTY, const char *, key, const char *, value);
DeclareSciCallV2(SetLexerLanguage, SETLEXERLANGUAGE, int, unused, const char *, language);
DeclareSciCallV2(PrivateLexerCall, PRIVATELEXERCALL, int, operation, void *, pointer);
DeclareSciCallR0(GetDirectFunction, GETDIRECTFUNCTION, void*);
DeclareSciCallR0(GetDirectPointer, GETDIRECTPOINTER, void*);
