#pragma once

enum SH_PLACE
{
  SH_INIT,
  SH_UPDATE,
  SH_MODIF
};
typedef	enum
{
  N2E_SE_APPLY = 1 << 0,
  N2E_SE_REJECT = 1 << 1
}	N2E_SELEDIT_STOP_OPT;

void	n2e_SelectionInit();
void	n2e_SelectionRelease();
UINT	n2e_SelectionGetSciEventMask(BOOL range_not);
int		n2e_SelectionGetWraps(int beg, int end);

void	nn2e_SelectionNotificationHandler(int code, struct SCNotification *scn);
void	n2e_SelectionUpdate(UINT place);
void	n2e_SelectionEditStart(const BOOL highlightAll);
BOOL	n2e_SelectionEditStop(UINT mode);
