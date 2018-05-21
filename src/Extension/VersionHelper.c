#include "stdafx.h"

BOOL IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
  OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0,{ 0 }, 0, 0 };
  DWORDLONG        const dwlConditionMask = VerSetConditionMask(
    VerSetConditionMask(
      VerSetConditionMask(
        0, VER_MAJORVERSION, VER_GREATER_EQUAL),
      VER_MINORVERSION, VER_GREATER_EQUAL),
    VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

  osvi.dwMajorVersion = wMajorVersion;
  osvi.dwMinorVersion = wMinorVersion;
  osvi.wServicePackMajor = wServicePackMajor;

  return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

BOOL IsWindowsVistaOrGreater()
{
  static BOOL bInitialized = FALSE;
  static BOOL bWindowsVistaOrGreater = FALSE;
  if (!bInitialized)
  {
    bWindowsVistaOrGreater = IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
    bInitialized = TRUE;
  }
  return bWindowsVistaOrGreater;
}

BOOL IsWindows7OrGreater()
{
  static BOOL bInitialized = FALSE;
  static BOOL bWindows7OrGreater = FALSE;
  if (!bInitialized)
  {
    bWindows7OrGreater = IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 0);
    bInitialized = TRUE;
  }
  return bWindows7OrGreater;
}
