#pragma once

#define IPCID_PARAM L"IPCID="

BOOL n2e_IsIPCIDParam(LPCWSTR lpParam);
BOOL n2e_InitializeIPC(const DWORD idIPC, const BOOL bSlave);
BOOL n2e_FinalizeIPC(const BOOL bFreeAll);

BOOL n2e_IPC_FileIO(LPCWSTR lpFilename, const LONGLONG size);
BOOL n2e_IPC_ClientProc(const DWORD pidServerProcess);

BOOL n2e_IsIPCInitialized();
BOOL n2e_IsElevatedModeEnabled();
BOOL n2e_IsElevatedMode();
BOOL n2e_SwitchElevation(const BOOL bResetIPCID);

HANDLE n2e_CreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                      DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,  HANDLE hTemplateFile);
BOOL n2e_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
BOOL n2e_SetEndOfFile(HANDLE hFile);
BOOL n2e_CloseHandle(HANDLE hFile);
int n2e_GetFileHeaderLength(const int iEncoding);

void n2e_SetUACIcon(HMENU hMenu, UINT nItem);