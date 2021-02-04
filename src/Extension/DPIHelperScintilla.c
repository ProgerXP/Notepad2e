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
  SendMessage(hwnd, SCI_SETDPI, GetDPIFromMonitor(MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY), hwnd), 0);
}

void n2e_ScintillaDPIUpdate(const HWND hwnd, const WPARAM dpi)
{
  if (!IsWindowsVistaOrGreater())
  {
    return;
  }
  SendMessage(hwnd, SCI_SETDPI, dpi, 0);
}
