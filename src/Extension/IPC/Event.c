#pragma once
#include "Event.h"

HANDLE n2e_CreateEvent(LPCWSTR lpEventName)
{
  SECURITY_DESCRIPTOR sd;
  InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
  SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

  SECURITY_ATTRIBUTES sa = { 0 };
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = FALSE;
  sa.lpSecurityDescriptor = &sd;

  return CreateEvent(&sa, FALSE, FALSE, lpEventName);
}

BOOL Event_IsOK(const Event *pEvent)
{
  return pEvent && pEvent->handle;
}

HANDLE Event_GetWaitHandle(const Event *pEvent)
{
  return Event_IsOK(pEvent) ? pEvent->handle : NULL;
}

BOOL Event_Init(Event *pEvent, LPCWSTR lpName, const BOOL bOpenExisting)
{
  if (!pEvent)
  {
    return FALSE;
  }
  if (Event_IsOK(pEvent))
  {
    Event_Free(pEvent);
  }
  ZeroMemory(pEvent, sizeof(Event));
  if (lpName)
  {
    lstrcpyn(pEvent->name, lpName, CSTRLEN(pEvent->name));
  }
  pEvent->handle = bOpenExisting
    ? OpenEvent(EVENT_ALL_ACCESS, FALSE, pEvent->name)
    : n2e_CreateEvent(pEvent->name);

  return Event_IsOK(pEvent);
}

BOOL Event_Free(Event *pEvent)
{
  if (!Event_IsOK(pEvent))
  {
    return FALSE;
  }
  if (pEvent->handle)
  {
    CloseHandle(pEvent->handle);
    pEvent->handle = NULL;
  }
  return TRUE;
}

BOOL Event_Pulse(const Event *pEvent)
{
  if (!Event_IsOK(pEvent))
  {
    return FALSE;
  }
  return PulseEvent(pEvent->handle);
}

BOOL Event_Set(const Event *pEvent)
{
  if (!Event_IsOK(pEvent))
  {
    return FALSE;
  }
  return SetEvent(pEvent->handle);
}

BOOL Event_Wait(const Event *pEvent, const UINT nDelay)
{
  if (!Event_IsOK(pEvent))
  {
    return FALSE;
  }
  return (WaitForSingleObject(pEvent->handle, nDelay) == WAIT_OBJECT_0);
}

BOOL Event_Reset(const Event *pEvent)
{
  if (!Event_IsOK(pEvent))
  {
    return FALSE;
  }
  return ResetEvent(pEvent->handle);
}
