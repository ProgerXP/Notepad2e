#pragma once
#include "stdafx.h"
#include "Scintilla.h"
#include "StringRecoding-fwd.h"
#include "Edit.h"
#include "Utils.h"

#define ICON_FIND_OK 9
#define ICON_FIND_FAILED 26
#define FIND_INFO_INDEX 15

extern BOOL bUsePrefixInOpenDialog;
extern EHighlightCurrentSelectionMode iHighlightSelection;
extern BOOL bEditSelectionScope;
extern ESaveOnLoseFocus iSaveOnLoseFocus;
extern BOOL bCtrlWheelScroll;
extern BOOL bMoveCaretOnRightClick;
extern EExpressionEvaluationMode iEvaluateMathExpression;
extern EWordNavigationMode iWordNavigationMode;
extern EUrlEncodeMode iUrlEncodeMode;
extern BOOL bExtendedSplitLines;
extern WCHAR wchLastHTMLTag[TEXT_BUFFER_LENGTH];
extern WCHAR wchLastHTMLEndTag[TEXT_BUFFER_LENGTH];

void n2e_SplitLines(const HWND hwnd, const int iLineSizeLimit, const BOOL bColumnWrap);
BOOL n2e_JoinLines_InitSelection(const BOOL noSpaceDelimiter);
void n2e_StripHTMLTags(const HWND hwnd);
void n2e_JumpToOffset(const HWND hwnd, const int iNewPos);
void n2e_EditInsertNewLine(const HWND hwnd, const BOOL insertAbove);
BOOL n2e_ShowPromptIfSelectionModeIsRectangle(const HWND hwnd);
void n2e_FindNextWord(const HWND hwnd, LPCEDITFINDREPLACE lpefr, const BOOL next);
BOOL n2e_IsCommentStyleById(const int iStyle);
int n2e_GetSingleLineCommentPrefixLength(const int iLexer);
BOOL n2e_IsSingleLineCommentStyleAtPos(const HWND hwnd, const int iLexer, const int iPos, EncodingData* pED);
int n2e_FindTextImpl(const HWND hwnd, LPCEDITFINDREPLACE lpefr, struct TextToFind* pttf);
BOOL n2e_CheckTextExists(const HWND hwnd, LPCEDITFINDREPLACE lpefr, const struct TextToFind* pttf, const int iPos);
BOOL n2e_CommentStyleIsDefined(const HWND hwnd);
BOOL n2e_OpenNextFile(const HWND hwnd, LPCWSTR file, const BOOL next);
void n2e_UnwrapSelection(const HWND hwnd, const int unwrapMode);
void n2e_EscapeHTML(const HWND hwnd);
void n2e_UpdateFindIcon(const BOOL findOK);
void n2e_UpdateFindIconAndFlashWindow(const BOOL findOK);
void n2e_ResetFindIcon();
void n2e_EditString2Hex(const HWND hwnd);
void n2e_EditHex2String(const HWND hwnd);
void n2e_EditString2Base64(const HWND hwnd);
void n2e_EditBase642String(const HWND hwnd);
void n2e_EditString2QP(const HWND hwnd);
void n2e_EditQP2String(const HWND hwnd);
void n2e_EditString2URL(const HWND hwnd);
void n2e_EditURL2String(const HWND hwnd);

void n2e_SaveCheckboxes(const HWND hwnd);
BOOL n2e_IsCheckboxChecked(const HWND hwnd, const UINT nCtrlID, const BOOL bCheckRestoredState);
void n2e_EditFindReplaceUpdateCheckboxes(const HWND hwnd, const UINT nCtrlID);
void n2e_EditFindReplaceInitialUpdateCheckboxes(const HWND hwnd);

int n2e_MultiByteToWideChar(LPCSTR lpMultiByteStr, const int cbMultiByte, LPWSTR lpWideCharStr, const int cchWideChar);
LPWSTR n2e_MultiByteToWideString(LPCSTR text);

BOOL n2e_CheckWindowClassName(const HWND hwnd, LPCWSTR lpwstrClassname);
BOOL n2e_EnableClipboardFiltering(const HWND hwnd, const UINT idEdit);
BOOL n2e_SubclassFindEditInCombo(const HWND hwnd, const UINT idCombo);
BOOL n2e_SubclassOpenDialog(const HWND hwnd);

BOOL n2e_IsValidClosingTagA(LPCSTR pTag);
BOOL n2e_IsValidClosingTagW(LPCWSTR pwchTag);

void n2e_Init_EditInsertTagDlg(const HWND hwnd);
WCHAR* n2e_GetClosingTagText_EditInsertTagDlg(WCHAR* buf);
void n2e_SaveTagsData_EditInsertTagDlg(PTAGSDATA pdata);

void n2e_int2bin(unsigned int val, LPWSTR binString);
BOOL n2e_IsExpressionEvaluationEnabled();
int n2e_GetExpressionTextRange(int* piStart, int* piEnd);
void n2e_CopyEvaluatedExpressionToClipboard();

BOOL n2e_IsFindReplaceAvailable(LPCEDITFINDREPLACE lpefr);

LPCSTR n2e_FormatLineText(LPSTR buf, const int iLineStart, const int iLineIndex, const int iDocumentLineIndex,
  LPCSTR lpPrefixAbsFormat, LPCSTR lpPrefixAbsZeroFormat,
  LPCSTR lpPrefixRelFormat, LPCSTR lpPrefixRelZeroFormat,
  LPCSTR lpPrefixRel0Format, LPCSTR lpPrefixRel0ZeroFormat);

LPCSTR n2e_GetBracesList();
void n2e_UpdateAlwaysOnTopButton();
BOOL n2e_InitTextFromSelection(HWND hwnd, const UINT uiControlID, HWND _hwndEdit, const BOOL bAllowEmptyString);
BOOL n2e_IsEscapedChar(const int iPos);
int n2e_GetFoldLevel(const int iLine);
BOOL n2e_CheckFoldLevel(const int iLine);
int n2e_GetNextFoldLine(const BOOL lookForward, int iLineFrom);
int n2e_GetPreviousFoldLevels(const HWND hwndListView, int iLineFrom);
void n2e_SelectListViewItem(const HWND hwndListView, const int iSelItem);

void n2e_FindMatchingBraceProc(int* piBrace1, int* piBrace2);
