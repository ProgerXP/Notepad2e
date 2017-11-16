#pragma once
#include "base64.h"

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus

LPCSTR EncodeStringToBase64(LPCSTR text, const int encoding, const int bufferSize);
LPCSTR DecodeBase64ToString(LPCSTR text, const int encoding, const int bufferSize);
void EncodeStrToBase64(const HWND hwnd);
void DecodeBase64ToStr(const HWND hwnd);

#ifdef __cplusplus
} // C-Declarations
#endif //__cplusplus
