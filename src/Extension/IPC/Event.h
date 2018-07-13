#pragma once
#include <wtypes.h>

struct TEvent
{
  HANDLE handle;
  WCHAR name[MAX_PATH];
};
typedef struct TEvent Event;

BOOL Event_IsOK(const Event *pEvent);
HANDLE Event_GetWaitHandle(const Event *pEvent);
BOOL Event_Init(Event *pEvent, LPCWSTR lpName, const BOOL bOpenExisting);
BOOL Event_Free(Event *pEvent);
BOOL Event_Pulse(const Event *pEvent);
BOOL Event_Set(const Event *pEvent);
BOOL Event_Wait(const Event *pEvent, const UINT nDelay);
BOOL Event_Reset(const Event *pEvent);
