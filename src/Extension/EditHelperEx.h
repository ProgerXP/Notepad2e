#pragma once
#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

  BOOL n2e_ExplorerCxtMenu(LPCWSTR path, const HWND hwndParent);
  int n2e_isValidRegex(LPCSTR str);
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
#ifdef __cplusplus
}//end extern "C"
#endif
