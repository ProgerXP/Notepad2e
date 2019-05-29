#include "DPIHelperScintilla.h"
#include "DPIHelper.h"
#include "Scintilla.h"
#include "SciCall.h"
#include "VersionHelper.h"

void n2e_ScintillaDPIInit(const HWND hwnd)
{
  if (!IsWindowsVistaOrGreater())
  {
    return;
  }
  RECT rc = { 0 };
  GetWindowRect(hwnd, &rc);
  const POINT pt = { rc.left, rc.top };
  SciCall_SetDPI(GetDPIFromMonitor(MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY), hwnd));
}

void n2e_ScintillaDPIUpdate(const HWND hwnd, const WPARAM dpi)
{
  if (!IsWindowsVistaOrGreater())
  {
    return;
  }
  SciCall_SetDPI(dpi);
}
