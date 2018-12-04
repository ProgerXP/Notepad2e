#pragma once
#include <wtypes.h>
#include "Event.h"
#include "Pipe.h"

typedef enum
{
  IPCC_OPEN_FILE_MAPPING,
  IPCC_CLOSE_FILE_MAPPING,
} EIPCCommand;

struct TIPCMessage
{
  EIPCCommand command;
  WCHAR filename[MAX_PATH];
  LONGLONG size;
  BOOL result;
  DWORD error;
};
typedef struct TIPCMessage IPCMessage;

BOOL IPCMessage_Init(IPCMessage* pIPCMessage, LPCWSTR filename, LONGLONG size, const EIPCCommand command);
BOOL IPCMessage_IsOpenFileMappingCommand(IPCMessage* pIPCMessage);
BOOL IPCMessage_IsCloseFileMappingCommand(IPCMessage* pIPCMessage);
BOOL IPCMessage_SetCommand(IPCMessage* pIPCMessage, const EIPCCommand command);
BOOL IPCMessage_SetError(IPCMessage* pIPCMessage, const DWORD error);
BOOL IPCMessage_Read(IPCMessage* pIPC, Pipe* pPipe);
BOOL IPCMessage_Write(IPCMessage* pIPC, Pipe* pPipe, Event *pEvent);
