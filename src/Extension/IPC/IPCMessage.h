#pragma once
#include <wtypes.h>
#include "Pipe.h"

typedef enum
{
  IPCS_REQUEST,
  IPCS_RESPONSE,
  IPCS_ERROR
} EIPCState;

struct TIPCMessage
{
  EIPCState state;
  WCHAR filename[MAX_PATH];
  LONGLONG size;
  BOOL result;
  DWORD error;
};
typedef struct TIPCMessage IPCMessage;

BOOL IPCMessage_Init(IPCMessage* pIPCMessage, LPCWSTR filename, LONGLONG size);
BOOL IPCMessage_IsRequest(IPCMessage* pIPCMessage);
BOOL IPCMessage_IsResponse(IPCMessage* pIPCMessage);
BOOL IPCMessage_IsError(IPCMessage* pIPCMessage);
BOOL IPCMessage_SetState(IPCMessage* pIPCMessage, const EIPCState state);
BOOL IPCMessage_SetRequest(IPCMessage* pIPCMessage);
BOOL IPCMessage_SetResponse(IPCMessage* pIPCMessage);
BOOL IPCMessage_SetError(IPCMessage* pIPCMessage, const DWORD error);
BOOL IPCMessage_Read(IPCMessage* pIPC, Pipe* pPipe);
BOOL IPCMessage_Write(IPCMessage* pIPC, Pipe* pPipe);
