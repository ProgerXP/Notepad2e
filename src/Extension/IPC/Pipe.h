#pragma once
#include "Event.h"

struct TPipe
{
  HANDLE handle;
  WCHAR name[MAX_PATH];
  BOOL created;
  BOOL connected;
};
typedef struct TPipe Pipe;

BOOL Pipe_IsOK(const Pipe *pPipe);
BOOL Pipe_Init(Pipe *pPipe, LPCWSTR lpName, const BOOL bOpenExisting);
BOOL Pipe_Free(Pipe *pPipe);
BOOL Pipe_Read(Pipe *pPipe, LPBYTE pBuffer, const UINT count);
BOOL Pipe_Write(Pipe *pPipe, Event *pEvent, LPCBYTE pBuffer, const UINT count);
