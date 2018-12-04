#include "IPCMessage.h"

BOOL IPCMessage_Init(IPCMessage* pIPCMessage, LPCWSTR filename, LONGLONG size, const EIPCCommand command)
{
  if (!pIPCMessage || !filename)
  {
    return FALSE;
  }
  ZeroMemory(pIPCMessage, sizeof(IPCMessage));
  pIPCMessage->command = command;
  lstrcpyn(pIPCMessage->filename, filename, CSTRLEN(pIPCMessage->filename));
  pIPCMessage->size = size;
  return TRUE;
}

BOOL IPCMessage_IsOpenFileMappingCommand(IPCMessage* pIPCMessage)
{
  if (!pIPCMessage)
  {
    return FALSE;
  }
  return (pIPCMessage->command == IPCC_OPEN_FILE_MAPPING);
}

BOOL IPCMessage_IsCloseFileMappingCommand(IPCMessage* pIPCMessage)
{
  if (!pIPCMessage)
  {
    return FALSE;
  }
  return (pIPCMessage->command == IPCC_CLOSE_FILE_MAPPING);
}

BOOL IPCMessage_SetCommand(IPCMessage* pIPCMessage, const EIPCCommand command)
{
  if (!pIPCMessage)
  {
    return FALSE;
  }
  pIPCMessage->command = command;
  return TRUE;
}

BOOL IPCMessage_SetError(IPCMessage* pIPCMessage, const DWORD error)
{
  pIPCMessage->error = error;
  return TRUE;
}

BOOL IPCMessage_Read(IPCMessage* pIPC, Pipe* pPipe)
{
  return Pipe_Read(pPipe, (LPBYTE)pIPC, sizeof(IPCMessage));
}

BOOL IPCMessage_Write(IPCMessage* pIPC, Pipe* pPipe, Event *pEvent)
{
  return Pipe_Write(pPipe, pEvent, (LPBYTE)pIPC, sizeof(IPCMessage));
}
