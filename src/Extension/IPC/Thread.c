#pragma once
#include "Thread.h"
#include <process.h>

BOOL Thread_IsOK(const Thread *pThread)
{
  return pThread && pThread->handle && Event_IsOK(&pThread->event);
}

HANDLE Thread_GetWaitHandle(const Thread *pThread)
{
  return Thread_IsOK(pThread) ? pThread->handle : NULL;
}

BOOL Thread_Init(Thread *pThread, TThreadProc proc, LPVOID param)
{
  if (!pThread)
  {
    return FALSE;
  }
  if (Thread_IsOK(pThread))
  {
    Thread_Free(pThread);
  }
  Event_Init(&pThread->event, NULL, FALSE);
  pThread->proc = proc;
  pThread->param = param;
  pThread->handle = (HANDLE)_beginthreadex(NULL, 0, pThread->proc, (void*)pThread, 0, &pThread->id);

  return Thread_IsOK(pThread);
}

BOOL Thread_Free(Thread *pThread)
{
  if (!Thread_IsOK(pThread))
  {
    return FALSE;
  }
  if (pThread->handle)
  {
    if (WaitForSingleObject(pThread->handle, 0) == WAIT_TIMEOUT)
    {
      Event_Set(&pThread->event);
      Thread_Wait(pThread, INFINITE);
    }
    CloseHandle(pThread->handle);
    pThread->handle = NULL;
  }
  Event_Free(&pThread->event);
  return TRUE;
}

BOOL Thread_Wait(const Thread *pThread, const UINT nDelay)
{
  if (!Thread_IsOK(pThread))
  {
    return FALSE;
  }
  return (WaitForSingleObject(pThread->handle, nDelay) == WAIT_OBJECT_0);
}
