#pragma once

#define IPCID_PARAM L"IPCID="

BOOL n2e_IsIPCIDParam(LPCWSTR lpParam);
BOOL n2e_IsElevatedMode();
BOOL n2e_SwitchElevation();

BOOL n2e_InitializeIPC(const DWORD dwID, const BOOL bIsParentMode);
BOOL n2e_FinalizeIPC();

BOOL n2e_ParentProcess_ElevatedFileIO(LPCWSTR lpFilename);
void n2e_ChildProcess_FileIOHandler(const DWORD pidParentProcess);

HANDLE n2e_CreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                      DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
BOOL n2e_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
BOOL n2e_SetEndOfFile(HANDLE hFile);
BOOL n2e_CloseHandle(HANDLE hFile);
int n2e_CalculateFileLength();
