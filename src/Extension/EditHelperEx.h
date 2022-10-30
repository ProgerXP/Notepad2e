#pragma once
#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

  BOOL n2e_ExplorerCxtMenu(LPCWSTR path, const HWND hwndParent);
  int n2e_isValidRegex(LPCSTR str);
  BOOL n2e_CheckStringMatchRegexp(LPCWSTR text, LPCWSTR regexFormat);
  int n2e_GetUTF8CharLength(const unsigned char ch);
  void n2e_ReplaceSubstring(LPSTR buf, LPCSTR from, LPCSTR to);
  void n2e_ReplaceSubstringFormat(LPSTR buf, LPCSTR from, LPCSTR formatTo, const int value);

  typedef struct tagHLSEdata
  {
    long  pos;
    long  len;
    char* original;
  } SE_DATA, *LPSE_DATA;

  int n2e_GetEditSelectionCount();
  void n2e_ClearEditSelections();
  void n2e_AddEditSelection(LPSE_DATA pData);
  LPSE_DATA n2e_GetEditSelection(const int index);

  void n2e_SaveViewState(HWND hwnd);
  void n2e_LoadViewState(HWND hwnd);

  struct TRTFData
  {
    LPSTR lpData;
    LONG nLength;
    LONG nOffset;
  };
  typedef struct TRTFData RTFData;

  LPSTR n2e_LoadRTFResource(const UINT uiResID, int* pLength);
  DWORD CALLBACK n2e_EditStreamCallBack(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
#ifdef __cplusplus
}//end extern "C"
#endif
