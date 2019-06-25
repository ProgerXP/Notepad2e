#include "Shell32Helper.h"
#include "VersionHelper.h"

#define SHELL32DLL L"shell32.dll"

typedef HRESULT(__stdcall *SHGetStockIconInfo)(enum SHSTOCKICONID siid, UINT uFlags, SHSTOCKICONINFO *psii);

SHGetStockIconInfo pfnSHGetStockIconInfo;

static HMODULE hShell32Module = NULL;

BOOL n2e_Shell32Initialize()
{
  if (IsWindowsVistaOrGreater())
  {
    hShell32Module = GetModuleHandle(SHELL32DLL);
    if (hShell32Module)
    {
      pfnSHGetStockIconInfo = (SHGetStockIconInfo)GetProcAddress(hShell32Module, "SHGetStockIconInfo");
    }
    return TRUE;
  }
  return FALSE;
}

HRESULT n2e_SHGetStockIconInfo(enum SHSTOCKICONID siid, UINT uFlags, SHSTOCKICONINFO *psii)
{
  return pfnSHGetStockIconInfo ? pfnSHGetStockIconInfo(siid, uFlags, psii) : E_FAIL;
}
