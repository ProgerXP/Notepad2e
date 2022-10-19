#include "User32Helper.h"
#include "VersionHelper.h"

#define USER32DLL L"user32.dll"

typedef BOOL(__stdcall *TShutdownBlockReasonCreate)(HWND hWnd, LPCWSTR pwszReason);

BOOL n2e_ShutdownBlockReasonCreate(HWND hWnd, LPCWSTR pwszReason);

TShutdownBlockReasonCreate pfnShutdownBlockReasonCreate;

static HMODULE hUser32Module = NULL;

BOOL n2e_User32Initialize()
{
  if (IsWindowsVistaOrGreater())
  {
    hUser32Module = GetModuleHandle(USER32DLL);
    if (hUser32Module)
    {
      pfnShutdownBlockReasonCreate = (TShutdownBlockReasonCreate)GetProcAddress(hUser32Module, "ShutdownBlockReasonCreate");
    }
    return TRUE;
  }
  return FALSE;
}

BOOL n2e_ShutdownBlockReasonCreate(HWND hWnd, LPCWSTR pwszReason)
{
  return pfnShutdownBlockReasonCreate ? pfnShutdownBlockReasonCreate(hWnd, pwszReason) : FALSE;
}
