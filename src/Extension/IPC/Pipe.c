#include "Pipe.h"

#define PIPE_BUFSIZE 4096

BOOL Pipe_IsOK(const Pipe *pPipe)
{
  return pPipe && pPipe->handle
    && (pPipe->handle != INVALID_HANDLE_VALUE)
    && Event_IsOK(&pPipe->event);
}

HANDLE Pipe_GetWaitHandle(const Pipe *pPipe)
{
  return Pipe_IsOK(pPipe) ? Event_GetWaitHandle(&pPipe->event) : NULL;
}

BOOL Pipe_Init(Pipe *pPipe, LPCWSTR lpName, LPCWSTR lpEventName, const BOOL bOpenExisting)
{
  if (!pPipe)
  {
    return FALSE;
  }
  ZeroMemory(pPipe, sizeof(Pipe));
  lstrcpy(pPipe->name, lpName);
  Event_Init(&pPipe->event, lpEventName, bOpenExisting);
  pPipe->handle = !bOpenExisting
    ? CreateNamedPipe(lpName,
                      PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
                      PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                      1,
                      PIPE_BUFSIZE * sizeof(WCHAR),
                      PIPE_BUFSIZE * sizeof(WCHAR),
                      0, NULL)
    : CreateFile(lpName,
                 GENERIC_READ | GENERIC_WRITE,
                 0,
                 NULL,
                 OPEN_EXISTING,
                 0,
                 NULL);

  if (Pipe_IsOK(pPipe))
  {
    if (bOpenExisting)
    {
      pPipe->connected = TRUE;
    }
    else
    {
      pPipe->created = TRUE;
    }
  }

  return Pipe_IsOK(pPipe);
}

BOOL Pipe_Free(Pipe *pPipe)
{
  if (!Pipe_IsOK(pPipe))
  {
    return FALSE;
  }
  if (pPipe->connected)
  {
    if (pPipe->created)
    {
      DisconnectNamedPipe(pPipe->handle);
      pPipe->created = FALSE;
    }
    pPipe->connected = FALSE;
  }
  Event_Free(&pPipe->event);
  if (pPipe->handle)
  {
    CloseHandle(pPipe->handle);
    pPipe->handle = NULL;
  }
  return TRUE;
}

BOOL Pipe_Read(Pipe *pPipe, LPBYTE pBuffer, const UINT count)
{
  if (!Pipe_IsOK(pPipe))
  {
    return FALSE;
  }
  DWORD dwRead = 0;
  return ReadFile(pPipe->handle, pBuffer, count, &dwRead, NULL) && (dwRead == count);
}

BOOL Pipe_Write(Pipe *pPipe, LPCBYTE pBuffer, const UINT count)
{
  if (!Pipe_IsOK(pPipe))
  {
    return FALSE;
  }
  if (!pPipe->connected)
  {
    pPipe->connected = (ConnectNamedPipe(pPipe->handle, NULL) || (GetLastError() == ERROR_PIPE_CONNECTED));
  }
  if (!pPipe->connected)
  {
    return FALSE;
  }

  DWORD dwWritten = 0;
  if (WriteFile(pPipe->handle, pBuffer, count, &dwWritten, NULL) && (dwWritten == count))
  {
    Event_Set(&pPipe->event);
    return TRUE;
  }

  return FALSE;
}
