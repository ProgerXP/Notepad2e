#include "stdafx.h"
#include <AclAPI.h>
#include <ShellAPI.h>
#include "ProcessElevationUtils.h"
#include "CommonUtils.h"
#include "Resource.h"
#include "Dialogs.h"
#include "VersionHelper.h"
#include "IPC/Event.h"
#include "IPC/FileMapping.h"
#include "IPC/IPCMessage.h"
#include "IPC/Thread.h"
#include "Externals.h"
#include "Shell32Helper.h"

#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))
#define CSTRLEN(s)  (COUNTOF(s)-1)

extern WCHAR g_wchWorkingDirectory[MAX_PATH];
extern HWND  hwndMain;
BOOL ExtractFirstArgument(LPCWSTR, LPWSTR, LPWSTR);

BOOL bIPCInitialized = FALSE;
BOOL bIsClientMode = FALSE;
DWORD dwIPCID = 0;

Event eventIPCServerReset;
Event eventIPCClientInitialized;
Pipe pipe;
FileMapping fileMapping;
Thread threadProcessWatcher;

BOOL n2e_IsIPCIDParam(LPCWSTR lpParam)
{
  return (StrCmpNI(lpParam, IPCID_PARAM, CSTRLEN(IPCID_PARAM)) == 0);
}

BOOL n2e_IsElevatedMode()
{
  return IsWindowsVistaOrGreater() && n2e_IsIPCInitialized();
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
  lstrcpyn(wchName, lpPrefix, CSTRLEN(wchName) - lstrlen(lpPostfix));
  lstrcat(wchName, lpPostfix);
  return wchName;
}

LPCWSTR getObjectName2(LPCWSTR lpPrefix, LPCWSTR lpPostfix)
{
  static wchar_t wchName[MAX_PATH];
  lstrcpyn(wchName, lpPrefix, CSTRLEN(wchName) - lstrlen(lpPostfix));
  lstrcat(wchName, lpPostfix);
  return wchName;
}

void ReloadMainMenu()
{
  extern HWND hwndMain;
  PostMessage(hwndMain, WM_INITMENU, (WPARAM)GetMenu(hwndMain), 0);
}

#define IPC_RESET_EVENT_NAME  L"Global\\ipc_reset_event"
#define IPC_START_EVENT_NAME  L"Global\\ipc_start_event"
#define PIPE_NAME  L"\\\\.\\pipe\\ipc"
#define PIPE_EVENT_NAME L"Global\\pipe_event"
#define FILEMAPPING_NAME L"Global\\filemapping"
#define FILEMAPPING_EVENT_NAME L"Global\\filemapping_event"

BOOL n2e_InitializeIPC(const DWORD idIPC, const BOOL bClientMode)
{
  n2e_FinalizeIPC();

  bIsClientMode = bClientMode;
  dwIPCID = idIPC;

  Event_Init(&eventIPCServerReset, getObjectName(IPC_RESET_EVENT_NAME, toString(dwIPCID)), bIsClientMode);
  Event_Init(&eventIPCClientInitialized, getObjectName(IPC_START_EVENT_NAME, toString(dwIPCID)), bIsClientMode);
  Pipe_Init(&pipe,
            getObjectName(PIPE_NAME, toString(dwIPCID)),
            getObjectName2(PIPE_EVENT_NAME, toString(dwIPCID)),
            bIsClientMode);
  FileMapping_Init(&fileMapping,
                   getObjectName(FILEMAPPING_NAME, toString(dwIPCID)),
                   getObjectName2(FILEMAPPING_EVENT_NAME, toString(dwIPCID)),
                   bIsClientMode);
  
  const BOOL res = dwIPCID
    && Event_IsOK(&eventIPCServerReset)
    && Event_IsOK(&eventIPCClientInitialized)
    && Pipe_IsOK(&pipe)
    && FileMapping_IsOK(&fileMapping);

  if (res && bIsClientMode)
  {
    Event_Set(&eventIPCClientInitialized);
  }
  bIPCInitialized = res;
  return res;
}

BOOL n2e_FinalizeIPC()
{
  bIPCInitialized = FALSE;

  if (!bIsClientMode)
  {
    Event_Set(&eventIPCServerReset);
  }
  Event_Free(&eventIPCServerReset);
  Event_Free(&eventIPCClientInitialized);
  Pipe_Free(&pipe);
  FileMapping_Free(&fileMapping);
  if (GetCurrentThreadId() != threadProcessWatcher.id)
  {
    Thread_Free(&threadProcessWatcher);
  }
  return TRUE;
}

unsigned __stdcall IPCServerThreadProc(LPVOID param)
{
  Thread* pThread = (Thread*)param;
  LPWSTR lpCmdLine = GetCommandLine();
  if (pThread && (lstrlen(lpCmdLine) > 0))
  {
    const BOOL bResetIPC = (BOOL)pThread->param;
    const int nMaxAttempts = 10;
    int i = 0;
    while (i++ < nMaxAttempts)
    {
      const DWORD ipc = (!bResetIPC && (i == 1)) ? dwIPCID : GenerateIPCID();
      if (n2e_InitializeIPC(ipc, FALSE))
      {
        break;
      }
    }

    BOOL res = FALSE;
    if (n2e_IsIPCInitialized())
    {
      const int iMaxCmdLength = lstrlen(lpCmdLine) + 25;
      LPWSTR lpCmdLineNew = n2e_Alloc(sizeof(WCHAR) * iMaxCmdLength);
      lstrcpyn(lpCmdLineNew, lpCmdLine, iMaxCmdLength);
      lstrcat(lpCmdLineNew, L" /" IPCID_PARAM);
      lstrcat(lpCmdLineNew, toString(GetCurrentProcessId()));
      lstrcat(lpCmdLineNew, L",");
      lstrcat(lpCmdLineNew, toString(dwIPCID));

      STARTUPINFO si = { 0 };
      ZeroMemory(&si, sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);
      GetStartupInfo(&si);

      LPWSTR lpArg1 = n2e_Alloc(sizeof(WCHAR) * iMaxCmdLength);
      LPWSTR lpArg2 = n2e_Alloc(sizeof(WCHAR) * iMaxCmdLength);
      ExtractFirstArgument(lpCmdLineNew, lpArg1, lpArg2);

      SHELLEXECUTEINFO sei = { 0 };
      ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
      sei.cbSize = sizeof(SHELLEXECUTEINFO);
      sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | /*SEE_MASK_NOZONECHECKS*/0x00800000;

      sei.hwnd = GetForegroundWindow();
      sei.lpVerb = L"runas";
      sei.lpFile = lpArg1;
      sei.lpParameters = lpArg2;
      sei.lpDirectory = g_wchWorkingDirectory;
      sei.nShow = SW_HIDE;

      res = ShellExecuteEx(&sei) && sei.hProcess;
      const HANDLE hClientProcess = sei.hProcess;

      if (res)
      {
        HANDLE handles[] = { Event_GetWaitHandle(&eventIPCClientInitialized), Event_GetWaitHandle(&pThread->event), hClientProcess };
        switch (WaitForMultipleObjects(COUNTOF(handles), handles, FALSE, INFINITE))
        {
          case WAIT_OBJECT_0:     // IPC initialized
            break;
          case WAIT_OBJECT_0 + 1: // force thread quit by server
          case WAIT_OBJECT_0 + 2: // client process quit
            res = FALSE;
            break;
          default:
            res = FALSE;
            break;
        }
      }

      n2e_Free(lpArg1);
      n2e_Free(lpArg2);
      n2e_Free(lpCmdLineNew);

      if (res)
      {
        ReloadMainMenu();
        HANDLE handles[] = { Event_GetWaitHandle(&pThread->event), hClientProcess };
        switch (WaitForMultipleObjects(COUNTOF(handles), handles, FALSE, INFINITE))
        {
          case WAIT_OBJECT_0:     // force thread quit by server
            break;
          case WAIT_OBJECT_0 + 1: // client process quit
            PostMessage(hwndMain, WM_IPC_INTERRUPTED, 0, 0);
            break;
          default:
            break;
        }
      }
      if (hClientProcess)
      {
        CloseHandle(hClientProcess);
      }
    }
    if (!res)
    {
      n2e_FinalizeIPC();
      ReloadMainMenu();
      MsgBox(MBWARN, IDS_ERR_ELEVATE);
    }
  }
  return 0;
}

IPCMessage ipcm = { 0 };

extern DWORD dwLastIOError;

BOOL n2e_IPC_ServerProc(LPCWSTR filename, const LONGLONG size)
{
  BOOL res = FALSE;
  if (!n2e_IsIPCInitialized())
  {
    return res;
  }
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
          if (ipcm.size == 0)
          {
            res = TRUE;   // expected error for empty file mapping
          }
          else
          {
            dwLastIOError = ipcm.error;
          }
        }
        break;
      default:
        break;
    }
  }
  if (res)
  {
    res = FALSE;
    IPCMessage_SetResponse(&ipcm);
    if (ipcm.size == 0)
    {
      IPCMessage_Write(&ipcm, &pipe);
      res = FileMapping_Close(&fileMapping, -1);
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
      res = TRUE;
    }
  }
  return res;
}

BOOL n2e_IPC_ClientProc(const DWORD pidServerProcess)
{
  BOOL res = FALSE;
  HANDLE hServerProc = OpenProcess(SYNCHRONIZE, FALSE, pidServerProcess);
  while (!res)
  {
    HANDLE handles[] = { hServerProc, Event_GetWaitHandle(&eventIPCServerReset), Pipe_GetWaitHandle(&pipe) };
    switch (WaitForMultipleObjects(COUNTOF(handles), handles, FALSE, INFINITE))
    {
      case WAIT_OBJECT_0:                   // server process quit
      case WAIT_OBJECT_0 + 1:               // IPC reset 
        CloseHandle(hServerProc);
        hServerProc = NULL;
        res = TRUE;
        break;
      case WAIT_OBJECT_0 + 2:
        if (IPCMessage_Read(&ipcm, &pipe))  // process IPC message
        {
          BOOL bSuccess = FALSE;
          if (IPCMessage_IsRequest(&ipcm))
          {
            if ((lstrlen(ipcm.filename) > 0)
                 && FileMapping_Open(&fileMapping, ipcm.filename, ipcm.size, FALSE))
            {
              bSuccess = FileMapping_Set(&fileMapping);
            }
            if (!bSuccess)
            {
              IPCMessage_SetError(&ipcm, FileMapping_GetError(&fileMapping));
            }
          }
          else if (IPCMessage_IsResponse(&ipcm))
          {
            if (FileMapping_Close(&fileMapping, ipcm.size))
            {
              bSuccess = TRUE;
            }
            else
            {
              IPCMessage_SetError(&ipcm, FileMapping_GetError(&fileMapping));
            }
          }
          if (!bSuccess)
          {
            IPCMessage_Write(&ipcm, &pipe);
          }
        }
        break;
      default:
        break;
    }
  }
  return res;
}

BOOL n2e_IPC_FileIO(LPCWSTR lpFilename, const LONGLONG size)
{
  return n2e_IPC_ServerProc(lpFilename, size);
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

BOOL n2e_SwitchElevation(const BOOL bResetIPC)
{
  if (bResetIPC && n2e_IsIPCInitialized() && Thread_IsOK(&threadProcessWatcher))
  {
    n2e_FinalizeIPC();
    ReloadMainMenu();
    return TRUE;
  }
  else
  {
    return Thread_Init(&threadProcessWatcher, (TThreadProc)IPCServerThreadProc, (LPVOID)bResetIPC);
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
    const int cx = GetSystemMetrics(SM_CYMENU) - 4;
    const int cy = cx;

    HICON hIconShield = NULL;
    SHSTOCKICONINFO sii = { 0 };
    sii.cbSize = sizeof(sii);
    if (SUCCEEDED(n2e_SHGetStockIconInfo(SIID_SHIELD, SHGFI_ICON | SHGFI_SMALLICON, &sii)))
    {
      hIconShield = sii.hIcon;
    }
    if (!hIconShield)
    {
      hIconShield = LoadImage(NULL, (LPCWSTR)IDI_SHIELD, IMAGE_ICON, cx, cy, LR_SHARED);
    }
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
