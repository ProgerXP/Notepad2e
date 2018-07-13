#pragma once
#include "Event.h"

struct TPipe
{
  HANDLE handle;
  WCHAR name[MAX_PATH];
  BOOL created;
  BOOL connected;
  Event event;
};
typedef struct TPipe Pipe;

BOOL Pipe_IsOK(const Pipe *pPipe);
HANDLE Pipe_GetWaitHandle(const Pipe *pPipe);
BOOL Pipe_Init(Pipe *pPipe, LPCWSTR lpName, LPCWSTR lpEventName, const BOOL bOpenExisting);
BOOL Pipe_Free(Pipe *pPipe);
BOOL Pipe_Read(Pipe *pPipe, LPBYTE pBuffer, const UINT count);
BOOL Pipe_Write(Pipe *pPipe, LPCBYTE pBuffer, const UINT count);
