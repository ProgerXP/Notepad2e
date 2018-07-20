#include "IPCMessage.h"
#include "Pipe.h"

BOOL IPCMessage_Init(IPCMessage* pIPCMessage, LPCWSTR filename, LONGLONG size)
{
  if (!pIPCMessage || !filename)
  {
    return FALSE;
  }
  ZeroMemory(pIPCMessage, sizeof(IPCMessage));
  lstrcpyn(pIPCMessage->filename, filename, CSTRLEN(pIPCMessage->filename));
  pIPCMessage->size = size;
  return TRUE;
}

BOOL IPCMessage_IsRequest(IPCMessage* pIPCMessage)
{
  if (!pIPCMessage)
  {
    return FALSE;
  }
  return (pIPCMessage->state == IPCS_REQUEST);
}

BOOL IPCMessage_IsResponse(IPCMessage* pIPCMessage)
{
  if (!pIPCMessage)
  {
    return FALSE;
  }
  return (pIPCMessage->state == IPCS_RESPONSE);
}

BOOL IPCMessage_SetState(IPCMessage* pIPCMessage, const EIPCState state)
{
  if (!pIPCMessage)
  {
    return FALSE;
  }
  pIPCMessage->state = state;
  return TRUE;
}

BOOL IPCMessage_SetRequest(IPCMessage* pIPCMessage)
{
  return IPCMessage_SetState(pIPCMessage, IPCS_REQUEST);
}

BOOL IPCMessage_SetResponse(IPCMessage* pIPCMessage)
{
  return IPCMessage_SetState(pIPCMessage, IPCS_RESPONSE);
}

BOOL IPCMessage_SetError(IPCMessage* pIPCMessage, const DWORD error)
{
  pIPCMessage->error = error;
  return IPCMessage_SetState(pIPCMessage, IPCS_ERROR);
}

BOOL IPCMessage_Read(IPCMessage* pIPC, Pipe* pPipe)
{
  return Pipe_Read(pPipe, (LPBYTE)pIPC, sizeof(IPCMessage));
}

BOOL IPCMessage_Write(IPCMessage* pIPC, Pipe* pPipe)
{
  return Pipe_Write(pPipe, (LPBYTE)pIPC, sizeof(IPCMessage));
}
