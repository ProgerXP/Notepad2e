#pragma once
#include "Event.h"

typedef unsigned(__stdcall* TThreadProc)(void*);

struct TThread
{
  DWORD id;
  HANDLE handle;
  TThreadProc proc;
  LPVOID param;
  Event event;
};
typedef struct TThread Thread;

BOOL Thread_IsOK(const Thread *pThread);
HANDLE Thread_GetWaitHandle(const Thread *pThread);
BOOL Thread_Init(Thread *pThread, TThreadProc proc, LPVOID param);
BOOL Thread_Free(Thread *pThread);
BOOL Thread_Wait(const Thread *pThread, const UINT nDelay);
