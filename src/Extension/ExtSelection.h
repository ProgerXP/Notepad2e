#pragma once

typedef enum
{
  SUM_INIT,
  SUM_UPDATE,
  SUM_MODIF
} ESelectionUpdateMode;

typedef enum
{
  SES_APPLY = 1 << 0,
  SES_REJECT = 1 << 1
} ESelectionEditStopMode;

void n2e_EditInit();
void n2e_SelectionRelease();
UINT n2e_SelectionGetSciEventMask(const BOOL range_not);
int n2e_SelectionGetWraps(const int beg, const int end);
void n2e_SelectionNotificationHandler(const int code, const struct SCNotification *scn);
void n2e_SelectionUpdate(const ESelectionUpdateMode place);
BOOL n2e_IsSelectionEditModeOn();
void n2e_SelectionEditStart(const BOOL highlightAll);
BOOL n2e_SelectionEditStop(const ESelectionEditStopMode mode);
void n2e_OnMouseVanishEvent(const BOOL showCursor);
