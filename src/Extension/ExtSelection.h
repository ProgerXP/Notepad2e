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

void	HLS_init();
void	HLS_release();
UINT	HLS_Sci_event_mask(BOOL range_not);
int		HLS_get_wraps(int beg, int end);

void	HLS_on_notification(int code, struct SCNotification *scn);
void	HLS_Update_selection(UINT place);
void	HLS_Edit_selection_start(const BOOL highlightAll);
BOOL	HLS_Edit_selection_stop(UINT mode);
