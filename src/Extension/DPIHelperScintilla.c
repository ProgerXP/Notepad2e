#include "DPIHelperScintilla.h"
#include "DPIHelper.h"
#include "Scintilla.h"
#include "SciCall.h"
#include "VersionHelper.h"

void n2e_ScintillaDPIInit(const HWND hwndScintilla)
{
  if (!IsWindowsVistaOrGreater())
  {
    return;
  }
  const HWND hwnd = GetAncestor(hwndScintilla, GA_ROOTOWNER);
  RECT rc = { 0 };
  GetWindowRect(hwnd, &rc);
  const POINT pt = { rc.left, rc.top };
  const HWND hwndPrev = SetFocus(hwndScintilla);
  SciCall_SetDPI(GetDPIFromMonitor(MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY), hwnd));
  SetFocus(hwndPrev);
}

void n2e_ScintillaDPIUpdate(const HWND hwnd, const WPARAM dpi)
{
  if (!IsWindowsVistaOrGreater())
  {
    return;
  }
  const HWND hwndPrev = SetFocus(hwnd);
  SciCall_SetDPI(dpi);
  SetFocus(hwndPrev);
}
