#include "stdafx.h"
#include <AclAPI.h>
#include <ShellAPI.h>
#include "ProcessElevationUtils.h"
#include "CommonUtils.h"
#include "VersionHelper.h"
#include "IPC/Event.h"
#include "IPC/FileMapping.h"
#include "IPC/IPCMessage.h"
#include "Externals.h"

#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))
#define CSTRLEN(s)  (COUNTOF(s)-1)

extern WCHAR g_wchWorkingDirectory[MAX_PATH];
BOOL ExtractFirstArgument(LPCWSTR, LPWSTR, LPWSTR);

BOOL bElevatedMode = FALSE;
BOOL bIPCInitialized = FALSE;
BOOL bIsClientMode = FALSE;
DWORD dwIPCID = 0;

Event eventAppStartup;
Pipe pipe;
FileMapping fileMapping;

BOOL n2e_IsIPCIDParam(LPCWSTR lpParam)
{
  return (StrCmpNI(lpParam, IPCID_PARAM, CSTRLEN(IPCID_PARAM)) == 0);
}

BOOL n2e_IsElevatedMode()
{
  if (!IsWindowsVistaOrGreater())
  {
    return FALSE;
  }
  return bElevatedMode;
}

DWORD GenerateIPCID()
{
  srand(GetTickCount());
  return rand() % 65535;
}

LPCWSTR toString(const DWORD value)
{
  static wchar_t wchBuffer[10];
  _itow_s(value, wchBuffer, sizeof(wchBuffer), 10);
  return wchBuffer;
}

LPCWSTR getObjectName(LPCWSTR lpPrefix, LPCWSTR lpPostfix)
{
  static wchar_t wchName[MAX_PATH];
  lstrcpy(wchName, lpPrefix);
  lstrcat(wchName, lpPostfix);
  return wchName;
}

LPCWSTR getObjectName2(LPCWSTR lpPrefix, LPCWSTR lpPostfix)
{
  static wchar_t wchName[MAX_PATH];
  lstrcpy(wchName, lpPrefix);
  lstrcat(wchName, lpPostfix);
  return wchName;
}

#define APP_STARTUP_EVENT_NAME  L"Global\\startup_event"
#define PIPE_NAME  L"\\\\.\\pipe\\ipc"
#define PIPE_EVENT_NAME L"Global\\pipe_event"
#define FILEMAPPING_NAME L"Global\\filemapping"
#define FILEMAPPING_EVENT_NAME L"Global\\filemapping_event"

#define WAIT_DELAY  5000

BOOL n2e_InitializeIPC(const DWORD idIPC, const BOOL bClientMode)
{
  n2e_FinalizeIPC();

  bIsClientMode = bClientMode;
  dwIPCID = idIPC;

  Event_Init(&eventAppStartup, getObjectName(APP_STARTUP_EVENT_NAME, toString(dwIPCID)), bIsClientMode);
  Pipe_Init(&pipe,
            getObjectName(PIPE_NAME, toString(dwIPCID)),
            getObjectName2(PIPE_EVENT_NAME, toString(dwIPCID)),
            bIsClientMode);
  FileMapping_Init(&fileMapping,
                   getObjectName(FILEMAPPING_NAME, toString(dwIPCID)),
                   getObjectName2(FILEMAPPING_EVENT_NAME, toString(dwIPCID)),
                   bIsClientMode);

  const BOOL res = dwIPCID
    && Event_IsOK(&eventAppStartup)
    && Pipe_IsOK(&pipe)
    && FileMapping_IsOK(&fileMapping);

  if (res && bIsClientMode)
  {
    Event_Set(&eventAppStartup);
  }
  bIPCInitialized = res;
  return res;
}

BOOL n2e_FinalizeIPC()
{
  bIPCInitialized = FALSE;

  Event_Free(&eventAppStartup);
  Pipe_Free(&pipe);
  FileMapping_Free(&fileMapping);
  return TRUE;
}

IPCMessage ipcm = { 0 };

extern DWORD dwLastIOError;

BOOL n2e_IPCServerProc(LPCWSTR filename, const LONGLONG size)
{
  BOOL res = FALSE;
  FileMapping_Reset(&fileMapping);
  res = IPCMessage_Init(&ipcm, filename, size)
        && IPCMessage_SetRequest(&ipcm)
        && IPCMessage_Write(&ipcm, &pipe);
  if (res)
  {
    res = FALSE;
    HANDLE handles[] = { FileMapping_GetWaitHandle(&fileMapping), Pipe_GetWaitHandle(&pipe) };
    switch (WaitForMultipleObjects(COUNTOF(handles), handles, FALSE, INFINITE))
    {
      case WAIT_OBJECT_0:
        res = TRUE;
        break;
      case WAIT_OBJECT_0 + 1:
        if (IPCMessage_Read(&ipcm, &pipe))
        {
          dwLastIOError = ipcm.error;
        }
        break;
      default:
        break;
    }
  }
  if (res)
  {
    IPCMessage_SetResponse(&ipcm);
    if (ipcm.size == 0)
    {
      res = IPCMessage_Write(&ipcm, &pipe);
      FileMapping_Close(&fileMapping, -1);
    }
    else if (FileMapping_Open(&fileMapping, ipcm.filename, ipcm.size, TRUE)
             && FileMapping_MapViewOfFile(&fileMapping))
    {
      extern WCHAR szCurFile[MAX_PATH + 40];
      extern int iEncoding;
      extern int iEOLMode;
      BOOL bCancelDataLoss = FALSE;
      BOOL FileIO(BOOL, LPCWSTR, BOOL, int*, int*, BOOL*, BOOL*, BOOL*, BOOL);

      res = FileIO(FALSE, szCurFile, FALSE, &iEncoding, &iEOLMode, NULL, NULL, &bCancelDataLoss, FALSE);
      ipcm.size = fileMapping.iDataSize;

      FileMapping_UnmapViewOfFile(&fileMapping);
      IPCMessage_Write(&ipcm, &pipe);
      FileMapping_Close(&fileMapping, fileMapping.iDataSize);
    }
    else
    {
      res = FALSE;
    }
  }
  return res;
}

BOOL n2e_IPCClientProc(const DWORD pidServerProcess)
{
  BOOL res = FALSE;
  HANDLE hServerProc = OpenProcess(SYNCHRONIZE, FALSE, pidServerProcess);
  while (!res)
  {
    HANDLE handles[] = { hServerProc, Pipe_GetWaitHandle(&pipe) };
    switch (WaitForMultipleObjects(COUNTOF(handles), handles, FALSE, INFINITE))
    {
      case WAIT_OBJECT_0:
        CloseHandle(hServerProc);
        hServerProc = NULL;
        res = TRUE;                         // server application quit
        break;
      case WAIT_OBJECT_0 + 1:
        if (IPCMessage_Read(&ipcm, &pipe))  // process IPC message
        {
          if (IPCMessage_IsRequest(&ipcm))
          {
            if (lstrlen(ipcm.filename) > 0)
            {
              if (FileMapping_Open(&fileMapping, ipcm.filename, ipcm.size, FALSE))
              {
                FileMapping_Set(&fileMapping);
              }
              else
              {
                IPCMessage_SetError(&ipcm, FileMapping_GetError(&fileMapping));
                IPCMessage_Write(&ipcm, &pipe);
              }
            }
          }
          else if (IPCMessage_IsResponse(&ipcm))
          {
            if (!FileMapping_Close(&fileMapping, ipcm.size))
            {
              IPCMessage_SetError(&ipcm, FileMapping_GetError(&fileMapping));
              IPCMessage_Write(&ipcm, &pipe);
            }
          }
        }
        break;
      default:
        break;
    }
  }
  return res;
}

HANDLE n2e_CreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                      DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
  if (FileMapping_IsOpened(&fileMapping))
  {
    return (HANDLE)1; // fake file handler
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

BOOL n2e_IsIPCInitialized()
{
  return bIPCInitialized;
}

BOOL n2e_RunElevatedInstance()
{
  LPWSTR lpCmdLine = GetCommandLine();
  if (lstrlen(lpCmdLine) == 0)
  {
    return FALSE;
  }

  const int nMaxAttempts = 10;
  int i = 0;
  while (i++ < nMaxAttempts)
  {
    if (n2e_InitializeIPC(GenerateIPCID(), FALSE))
    {
      break;
    }
  }

  if (!n2e_IsIPCInitialized())
  {
    return FALSE;
  }

  LPWSTR lpCmdLineNew = n2e_Alloc(sizeof(WCHAR) * (lstrlen(lpCmdLine) + 50));
  lstrcpy(lpCmdLineNew, lpCmdLine);
  lstrcat(lpCmdLineNew, L" /" IPCID_PARAM);
  lstrcat(lpCmdLineNew, toString(GetCurrentProcessId()));
  lstrcat(lpCmdLineNew, L",");
  lstrcat(lpCmdLineNew, toString(dwIPCID));

  STARTUPINFO si = { 0 };
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  GetStartupInfo(&si);

  LPWSTR lpArg1 = n2e_Alloc(sizeof(WCHAR) * (lstrlen(lpCmdLineNew) + 1));
  LPWSTR lpArg2 = n2e_Alloc(sizeof(WCHAR) * (lstrlen(lpCmdLineNew) + 1));
  ExtractFirstArgument(lpCmdLineNew, lpArg1, lpArg2);

  SHELLEXECUTEINFO sei = { 0 };
  ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
  sei.cbSize = sizeof(SHELLEXECUTEINFO);
  sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | /*SEE_MASK_NOZONECHECKS*/0x00800000;

  extern HWND  hwndMain;

  sei.hwnd = hwndMain;
  sei.lpVerb = L"runas";
  sei.lpFile = lpArg1;
  sei.lpParameters = lpArg2;
  sei.lpDirectory = g_wchWorkingDirectory;
  sei.nShow = SW_HIDE;

  bIPCInitialized = FALSE;
  
  const BOOL res = ShellExecuteEx(&sei) && Event_Wait(&eventAppStartup, WAIT_DELAY);

  n2e_Free(lpArg1);
  n2e_Free(lpArg2);
  n2e_Free(lpCmdLineNew);

  bElevatedMode = res;

  if (res)
  {
    bIPCInitialized = TRUE;
  }
  else
  {
    n2e_FinalizeIPC();
  }

  return res;
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

HBITMAP ConvertIconToBitmap(const HICON hIcon, const int cx, const int cy)
{
  HDC hScreenDC = GetDC(NULL);
  HBITMAP hbmpTmp = CreateCompatibleBitmap(hScreenDC, cx, cy);
  HDC hMemDC = CreateCompatibleDC(hScreenDC);
  HBITMAP hOldBmp = SelectObject(hMemDC, hbmpTmp);
  DrawIconEx(hMemDC, 0, 0, hIcon, cx, cy, 0, NULL, DI_NORMAL);
  SelectObject(hMemDC, hOldBmp);

  HBITMAP hDibBmp = (HBITMAP)CopyImage((HANDLE)hbmpTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);

  DeleteObject(hbmpTmp);
  DeleteDC(hMemDC);
  ReleaseDC(NULL, hScreenDC);

  return hDibBmp;
}

void n2e_SetUACIcon(HMENU hMenu, UINT nItem)
{
  static BOOL bInitialized = FALSE;
  if (bInitialized)
  {
    return;
  }

#define IDI_SHIELD          32518

  if (IsWindowsVistaOrGreater())
  {
    const int cx = GetSystemMetrics(SM_CYMENU);
    const int cy = cx;
    HICON hIconShield = LoadImage(NULL, (LPCWSTR)IDI_SHIELD, IMAGE_ICON, cx, cy, LR_SHARED);
    if (hIconShield)
    {
      MENUITEMINFO mii = { 0 };
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_BITMAP;
      mii.hbmpItem = ConvertIconToBitmap(hIconShield, cx, cy);
      SetMenuItemInfo(hMenu, nItem, FALSE, &mii);
    }
  }
  bInitialized = TRUE;
}
