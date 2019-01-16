#include "stdafx.h"
#include "ProcessElevationUtils.h"
#include "Externals.h"
#include "IPC/FileMapping.h"
#include "VersionHelper.h"

#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))
#define CSTRLEN(s)  (COUNTOF(s)-1)

BOOL bElevationEnabled = FALSE;
DWORD dwIPCID = 0;

#define FILEMAPPING_IPCDATA_NAME L"Global\\filemapping_ipcdata"
#define FILEMAPPING_NAME L"Global\\filemapping"

HANDLE hChildProcess = NULL;
FileMapping fileMappingIPCData;
FileMapping fileMapping;

BOOL n2e_RunChildProcess();

BOOL n2e_IsIPCIDParam(LPCWSTR lpParam)
{
  return (StrCmpNI(lpParam, IPCID_PARAM, CSTRLEN(IPCID_PARAM)) == 0);
}

BOOL n2e_IsChildProcessOK()
{
  return hChildProcess && (WaitForSingleObject(hChildProcess, 0) == WAIT_TIMEOUT);
}

BOOL n2e_IsElevatedModeEnabled()
{
  return bElevationEnabled;
}

BOOL n2e_IsElevatedMode()
{
  return IsWindowsVistaOrGreater() && n2e_IsElevatedModeEnabled();
}

void CloseProcessHandle(HANDLE* pHandle)
{
  if (*pHandle)
  {
    CloseHandle(*pHandle);
    *pHandle = NULL;
  }
}

BOOL n2e_SwitchElevation()
{
  bElevationEnabled = !n2e_IsElevatedModeEnabled();
  return bElevationEnabled
          ? n2e_RunChildProcess()
          : n2e_FinalizeIPC();
}

void lstrcatdword(LPWSTR lpstr, const DWORD value)
{
  wchar_t wchBuffer[12];
  _itow_s(value, wchBuffer, CSTRLEN(wchBuffer), 10);
  lstrcat(lpstr, wchBuffer);
}

LPCWSTR FormatObjectName(LPCWSTR lpPrefix, const DWORD dwPostfix)
{
  static wchar_t wchName[MAX_PATH];
  lstrcpyn(wchName, lpPrefix, CSTRLEN(wchName));
  lstrcatdword(wchName, dwPostfix);
  return wchName;
}

HANDLE n2e_RunElevatedInstance()
{
  int iNumArgs = 0;
  LPWSTR* lpCmdLineArgs = NULL;
  if ((lstrlen(GetCommandLine()) > 0)
      && (lpCmdLineArgs = CommandLineToArgvW(GetCommandLine(), &iNumArgs))
      && (iNumArgs > 0))
  {
    WCHAR wchParams[MAX_PATH] = { 0 };
    lstrcat(wchParams, L"/" IPCID_PARAM);
    lstrcatdword(wchParams, GetCurrentProcessId());

    SHELLEXECUTEINFO sei = { 0 };
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | /*SEE_MASK_NOZONECHECKS*/0x00800000;
    sei.hwnd = GetForegroundWindow();
    sei.lpVerb = L"runas";
    sei.lpFile = lpCmdLineArgs[0];
    sei.lpParameters = wchParams;
    sei.nShow = SW_HIDE;
    if (ShellExecuteEx(&sei) && sei.hProcess)
    {
      LocalFree(lpCmdLineArgs);
      return sei.hProcess;
    }
  }
  if (lpCmdLineArgs)
  {
    LocalFree(lpCmdLineArgs);
  }
  dwLastIOError = GetLastError();
  return NULL;
}

BOOL n2e_InitializeIPC(const BOOL bInitParent, const DWORD dwID)
{
  n2e_FinalizeIPC();
  dwIPCID = dwID;
  FileMapping_Init(&fileMapping,
                   FormatObjectName(FILEMAPPING_NAME, dwIPCID),
                   !bInitParent);
  FileMapping_Init(&fileMappingIPCData,
                   FormatObjectName(FILEMAPPING_IPCDATA_NAME, dwIPCID),
                   !bInitParent);
  return dwIPCID && FileMapping_IsOK(&fileMapping) && FileMapping_IsOK(&fileMappingIPCData);
}

BOOL n2e_FinalizeIPC()
{
  if (n2e_IsChildProcessOK())
  {
    TerminateProcess(hChildProcess, 0);
  }
  if (hChildProcess)
  {
    CloseProcessHandle(&hChildProcess);
  }
  FileMapping_Free(&fileMappingIPCData);
  FileMapping_Free(&fileMapping);
  return TRUE;
}

BOOL n2e_RunChildProcess()
{
  return n2e_InitializeIPC(TRUE, GetCurrentProcessId()) && (hChildProcess = n2e_RunElevatedInstance());
}

struct TIPCData
{
  WCHAR wchFileName[MAX_PATH];
  LONGLONG llFileSize;
};
typedef struct TIPCData IPCData;

void n2e_ChildProcess_Exit(const DWORD dwExitCode, HANDLE hParentProcess)
{
  CloseProcessHandle(&hParentProcess);
  n2e_FinalizeIPC();
  ExitProcess(dwExitCode);
}

void n2e_ChildProcess_FileIOHandler(const DWORD pidParentProcess)
{
  IPCData ipcData = { 0 };
  HANDLE hParentProcess = OpenProcess(SYNCHRONIZE, FALSE, pidParentProcess);
  while (1)
  {
    HANDLE handles[] = {
                         hParentProcess,
                         FileMapping_GetTryCreateHandle(&fileMappingIPCData),
                         FileMapping_GetTryCloseHandle(&fileMappingIPCData),
                         FileMapping_GetTryCreateHandle(&fileMapping),
                         FileMapping_GetTryCloseHandle(&fileMapping)
                       };
    switch (WaitForMultipleObjects(COUNTOF(handles), handles, FALSE, INFINITE))
    {
      case WAIT_OBJECT_0:         // parent process quit
        n2e_ChildProcess_Exit(0, hParentProcess);
        return;
      case WAIT_OBJECT_0 + 1:     // create IPC data filemapping
        if (!FileMapping_Open(&fileMappingIPCData, NULL, sizeof(ipcData), FALSE))
        {
          n2e_ChildProcess_Exit(FileMapping_GetError(&fileMappingIPCData), hParentProcess);
        }
        break;
      case WAIT_OBJECT_0 + 2:     // close IPC data filemapping
        if (!FileMapping_MapViewOfFile(&fileMappingIPCData)
            || !FileMapping_Read(&fileMappingIPCData, (LPBYTE)&ipcData, sizeof(ipcData))
            || !FileMapping_UnmapViewOfFile(&fileMappingIPCData))
        {
          n2e_ChildProcess_Exit(FileMapping_GetError(&fileMappingIPCData), hParentProcess);
        }
        FileMapping_Close(&fileMappingIPCData, sizeof(ipcData));
        break;
      case WAIT_OBJECT_0 + 3:     // create filemapping
        if (!FileMapping_Open(&fileMapping, &ipcData.wchFileName[0], ipcData.llFileSize, FALSE))
        {
          n2e_ChildProcess_Exit(FileMapping_GetError(&fileMapping), hParentProcess);
        }
        break;
      case WAIT_OBJECT_0 + 4:     // close filemapping
        if (!FileMapping_Close(&fileMapping, ipcData.llFileSize))
        {
          n2e_ChildProcess_Exit(FileMapping_GetError(&fileMapping), hParentProcess);
        }
        break;
      default:
        break;
    }
  }
}

DWORD n2e_GetChildProcessQuitCode()
{
  DWORD dwCode = 0;
  if (hChildProcess && GetExitCodeProcess(hChildProcess, &dwCode))
  {
    return dwCode;
  }
  return 0;
}

BOOL n2e_ParentProcess_ElevatedFileIO(LPCWSTR lpFilename, const LONGLONG size)
{
  if (!n2e_IsElevatedModeEnabled())
  {
    return FALSE;
  }
  if (!hChildProcess || !n2e_IsChildProcessOK())
  {
    if (!n2e_RunChildProcess() || !n2e_IsChildProcessOK())
    {
      if (hChildProcess)
      {
        dwLastIOError = n2e_GetChildProcessQuitCode(hChildProcess);
        CloseProcessHandle(&hChildProcess);
      }
      return FALSE;
    }
  }

  IPCData ipcData = { 0 };
  wcsncpy_s(&ipcData.wchFileName[0], CSTRLEN(ipcData.wchFileName), lpFilename, wcslen(lpFilename));
  ipcData.llFileSize = size;

  FileMapping_TryCreate(&fileMappingIPCData);
  while (1)
  {
    HANDLE handles[] = {
                          hChildProcess,
                          FileMapping_GetCreatedHandle(&fileMappingIPCData),
                          FileMapping_GetClosedHandle(&fileMappingIPCData),
                          FileMapping_GetCreatedHandle(&fileMapping),
                          FileMapping_GetClosedHandle(&fileMapping)
                       };
    switch (WaitForMultipleObjects(COUNTOF(handles), handles, FALSE, INFINITE))
    {
      case WAIT_OBJECT_0:       // client process quit
        dwLastIOError = n2e_GetChildProcessQuitCode(hChildProcess);
        return FALSE;
      case WAIT_OBJECT_0 + 1:   // IPC data filemapping created
        if (!FileMapping_Open(&fileMappingIPCData, NULL, sizeof(ipcData), TRUE)
            || !FileMapping_MapViewOfFile(&fileMappingIPCData)
            || !FileMapping_Write(&fileMappingIPCData, (LPBYTE)&ipcData, sizeof(ipcData))
            || !FileMapping_UnmapViewOfFile(&fileMappingIPCData)
            || !FileMapping_TryClose(&fileMappingIPCData))
        {
          TerminateProcess(hChildProcess, FileMapping_GetError(&fileMappingIPCData));
        }
        break;
      case WAIT_OBJECT_0 + 2:   // IPC data filemapping closed
        FileMapping_TryCreate(&fileMapping);
        break;
      case WAIT_OBJECT_0 + 3:   // filemapping created
        if (size == 0)
        {
          SendMessage(hwndEdit, SCI_SETSAVEPOINT, 0, 0);
          FileMapping_TryClose(&fileMapping);
        }
        else if (FileMapping_Open(&fileMapping, lpFilename, size, TRUE)
                 && FileMapping_MapViewOfFile(&fileMapping))
        {
          BOOL bCancelDataLoss = FALSE;
          if (!FileIO(FALSE, lpFilename, FALSE, &iEncoding, &iEOLMode, NULL, NULL, &bCancelDataLoss, FALSE))
          {
            TerminateProcess(hChildProcess, GetLastError());
          }
          if (FileMapping_UnmapViewOfFile(&fileMapping))
          {
            FileMapping_TryClose(&fileMapping);
          }
        }
        else
        {
          TerminateProcess(hChildProcess, FileMapping_GetError(&fileMapping));
        }
        break;
      case WAIT_OBJECT_0 + 4:   // filemapping closed
        return TRUE;
      default:
        break;
    }
  }
  return FALSE;
}

HANDLE n2e_CreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                      DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
  if (FileMapping_IsOpened(&fileMapping))
  {
    return (HANDLE)TRUE; // fake file handler
  }
  else
  {
    return CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
  }
}

BOOL n2e_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
  if (FileMapping_IsOpened(&fileMapping))
  {
    return FileMapping_Write(&fileMapping, lpBuffer, nNumberOfBytesToWrite);
  }
  else
  {
    return WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
  }
}

BOOL n2e_SetEndOfFile(HANDLE hFile)
{
  if (FileMapping_IsOpened(&fileMapping))
  {
    return TRUE;
  }
  else
  {
    return SetEndOfFile(hFile);
  }
}

BOOL n2e_CloseHandle(HANDLE hFile)
{
  if (FileMapping_IsOpened(&fileMapping))
  {
    return TRUE;
  }
  else
  {
    return CloseHandle(hFile);
  }
}

int n2e_GetFileHeaderLength(const int iEncoding)
{
  if (mEncoding[iEncoding].uFlags & NCP_UNICODE)
  {
    if (mEncoding[iEncoding].uFlags & NCP_UNICODE_BOM)
    {
      return 2;
    }
  }
  else if (mEncoding[iEncoding].uFlags & NCP_UTF8)
  {
    if (mEncoding[iEncoding].uFlags & NCP_UTF8_SIGN)
    {
      return 3;
    }
  }
  return 0;
}
