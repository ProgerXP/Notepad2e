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

BOOL bElevationEnabled = FALSE;
BOOL bIPCInitialized = FALSE;
BOOL bIsClientMode = FALSE;
DWORD dwIPCID = 0;

Event eventIPCReset;
Event eventIPCResult;
Event eventIPCServerReset;
Event eventIPCClientInitialized;
Event eventIPCClientQuit;
Pipe pipe;
FileMapping fileMapping;
Thread threadIPCServerWorker;

BOOL n2e_IsIPCIDParam(LPCWSTR lpParam)
{
  return (StrCmpNI(lpParam, IPCID_PARAM, CSTRLEN(IPCID_PARAM)) == 0);
}

BOOL n2e_IsElevatedModeEnabled()
{
  return bElevationEnabled;
}

BOOL n2e_IsElevatedMode()
{
  return IsWindowsVistaOrGreater() && n2e_IsElevatedModeEnabled();
}

DWORD GenerateIPCID()
{
  srand(GetTickCount());
  return rand() % 65536;
}

LPCWSTR toString(const DWORD value)
{
  static wchar_t wchBuffer[12];
  _itow_s(value, wchBuffer, COUNTOF(wchBuffer), 10);
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
#define IPC_SERVER_EVENT_NAME  L"Global\\ipc_server_event"
#define IPC_START_EVENT_NAME  L"Global\\ipc_start_event"
#define PIPE_NAME  L"\\\\.\\pipe\\ipc"
#define PIPE_EVENT_NAME L"Global\\pipe_event"
#define FILEMAPPING_NAME L"Global\\filemapping"
#define FILEMAPPING_EVENT_NAME L"Global\\filemapping_event"

BOOL n2e_InitializeIPC(const DWORD idIPC, const BOOL bClientMode)
{
  n2e_FinalizeIPC(FALSE);

  bIsClientMode = bClientMode;
  dwIPCID = idIPC;

  if (!bIsClientMode)
  {
    if (!Event_GetWaitHandle(&eventIPCReset))
    {
      Event_Init(&eventIPCReset, NULL, FALSE);
    }
    if (!Event_GetWaitHandle(&eventIPCResult))
    {
      Event_Init(&eventIPCResult, NULL, FALSE);
    }
    if (!Event_GetWaitHandle(&eventIPCClientQuit))
    {
      Event_Init(&eventIPCClientQuit, NULL, FALSE);
    }
  }
  Event_Init(&eventIPCServerReset, getObjectName(IPC_SERVER_EVENT_NAME, toString(dwIPCID)), bIsClientMode);
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
    && (bIsClientMode || (Event_IsOK(&eventIPCReset) && Event_IsOK(&eventIPCResult)))
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

BOOL n2e_FinalizeIPC(const BOOL bFreeAll)
{
  bIPCInitialized = FALSE;

  if (!bIsClientMode)
  {
    Event_Set(&eventIPCServerReset);
    if (bFreeAll)
    {
      Event_Free(&eventIPCReset);
      Event_Free(&eventIPCResult);
      Event_Free(&eventIPCClientQuit);
    }
  }
  Event_Free(&eventIPCServerReset);
  Event_Free(&eventIPCClientInitialized);
  Pipe_Free(&pipe);
  FileMapping_Free(&fileMapping);
  if (GetCurrentThreadId() != threadIPCServerWorker.id)
  {
    Thread_Free(&threadIPCServerWorker);
  }
  return TRUE;
}

BOOL RunElevatedInstance(HANDLE* phElevatedProcess)
{
  BOOL res = FALSE;
  LPWSTR lpCmdLine = GetCommandLine();

  STARTUPINFO si = { 0 };
  SHELLEXECUTEINFO sei = { 0 };

  if (lstrlen(lpCmdLine) > 0)
  {
    const int iMaxCmdLength = lstrlen(lpCmdLine) + 25;
    LPWSTR lpCmdLineNew = n2e_Alloc(sizeof(WCHAR) * iMaxCmdLength);
    lstrcpyn(lpCmdLineNew, lpCmdLine, iMaxCmdLength);
    lstrcat(lpCmdLineNew, L" /" IPCID_PARAM);
    lstrcat(lpCmdLineNew, toString(GetCurrentProcessId()));
    lstrcat(lpCmdLineNew, L",");
    lstrcat(lpCmdLineNew, toString(dwIPCID));

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&si);

    LPWSTR lpArg1 = n2e_Alloc(sizeof(WCHAR) * iMaxCmdLength);
    LPWSTR lpArg2 = n2e_Alloc(sizeof(WCHAR) * iMaxCmdLength);
    ExtractFirstArgument(lpCmdLineNew, lpArg1, lpArg2);

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

    n2e_Free(lpArg1);
    n2e_Free(lpArg2);
    n2e_Free(lpCmdLineNew);
  }
  if (res)
  {
    if (phElevatedProcess)
    {
      *phElevatedProcess = sei.hProcess;
    }
  }
  return res;
}

void CloseProcessHandle(HANDLE* pHandle)
{
  if (*pHandle)
  {
    CloseHandle(*pHandle);
    *pHandle = NULL;
  }
}

void n2e_InitIPC(const BOOL bResetIPC)
{
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
}

BOOL AddHandle(HANDLE* pHandles, int* pHandleCount, const HANDLE handle)
{
  if (handle)
  {
    pHandles[*pHandleCount] = handle;
    (*pHandleCount)++;
    return TRUE;
  }
  return FALSE;
}

unsigned __stdcall IPCServerWorkerThreadProc(LPVOID param)
{
  Thread* pThread = (Thread*)param;
  if (pThread)
  {
    BOOL res = TRUE;
    BOOL bShowErrorPrompt = TRUE;
    HANDLE hClientProcess = NULL;
    n2e_InitIPC((BOOL)pThread->param);
    Event_Set(&eventIPCReset);
    Event_Reset(&eventIPCClientQuit);
    while (res)
    {
      HANDLE handles[5] = { 0 };
      int iHandleCount = 0;
      AddHandle(handles, &iHandleCount, Event_GetWaitHandle(&eventIPCReset))
        && AddHandle(handles, &iHandleCount, Event_GetWaitHandle(&pThread->event))
        && AddHandle(handles, &iHandleCount, Event_GetWaitHandle(&eventIPCClientInitialized))
        && AddHandle(handles, &iHandleCount, hClientProcess);
      switch (WaitForMultipleObjects(iHandleCount, handles, FALSE, INFINITE))
      {
        case WAIT_OBJECT_0:     // reset IPC
          Event_Reset(&eventIPCClientQuit);
          n2e_InitIPC(TRUE);
          CloseProcessHandle(&hClientProcess);
          if (RunElevatedInstance(&hClientProcess))
          {
            bShowErrorPrompt = FALSE;
          }
          else
          {
            n2e_FinalizeIPC(FALSE);
            Event_Set(&eventIPCResult);
            if (bShowErrorPrompt)
            {
              bElevationEnabled = FALSE;
              res = FALSE;      // first attempt failed, break
              continue;
            }
          }
          break;
        case WAIT_OBJECT_0 + 1: // force thread quit
          res = FALSE;
          break;
        case WAIT_OBJECT_0 + 2: // IPC initialized
          Event_Set(&eventIPCResult);
          break;
        case WAIT_OBJECT_0 + 3: // client process quit
          Event_Set(&eventIPCClientQuit);
          n2e_FinalizeIPC(FALSE);
          CloseProcessHandle(&hClientProcess);
          break;
        case WAIT_TIMEOUT:
        default:
          break;
      }
    }
    CloseProcessHandle(&hClientProcess);
    n2e_FinalizeIPC(FALSE);
    if (bShowErrorPrompt)
    {
      MsgBox(MBWARN, IDS_ERR_ELEVATE);
    }
  }
  return 0;
}

IPCMessage ipcm = { 0 };
extern DWORD dwLastIOError;

BOOL n2e_IPC_ClientProc(const DWORD pidServerProcess)
{
  BOOL res = TRUE;
  HANDLE hServerProc = OpenProcess(SYNCHRONIZE, FALSE, pidServerProcess);
  while (res)
  {
    HANDLE handles[] = { hServerProc, Event_GetWaitHandle(&eventIPCServerReset), Pipe_GetWaitHandle(&pipe) };
    switch (WaitForMultipleObjects(COUNTOF(handles), handles, FALSE, INFINITE))
    {
      case WAIT_OBJECT_0:                   // server process quit
      case WAIT_OBJECT_0 + 1:               // IPC reset 
        res = FALSE;
        break;
      case WAIT_OBJECT_0 + 2:
        if (IPCMessage_Read(&ipcm, &pipe))  // process IPC message
        {
          if (IPCMessage_IsOpenFileMappingCommand(&ipcm))
          {
            FileMapping_ResetError(&fileMapping);
            if ((lstrlen(ipcm.filename) > 0)
                && FileMapping_Open(&fileMapping, ipcm.filename, ipcm.size, FALSE))
            {
              FileMapping_Set(&fileMapping);
            }
          }
          else if (IPCMessage_IsCloseFileMappingCommand(&ipcm))
          {
            FileMapping_ResetError(&fileMapping);
            FileMapping_Close(&fileMapping, ipcm.size);
          }
          IPCMessage_SetError(&ipcm, FileMapping_GetError(&fileMapping));
          IPCMessage_Write(&ipcm, &pipe);
        }
        break;
      default:
        break;
    }
  }
  CloseProcessHandle(&hServerProc);
  return res;
}

BOOL n2e_IPC_ServerProc(LPCWSTR lpFilename, const LONGLONG size)
{
  extern HWND  hwndEdit;
  extern WCHAR szCurFile[MAX_PATH + 40];
  extern int iEncoding;
  extern int iEOLMode;
  BOOL bCancelDataLoss = FALSE;
  BOOL FileIO(BOOL, LPCWSTR, BOOL, int*, int*, BOOL*, BOOL*, BOOL*, BOOL);

  BOOL res = n2e_IsIPCInitialized();
  BOOL bIOResult = FALSE;
  if (!res)
  {
    return res;
  }
  FileMapping_Reset(&fileMapping);
  res = IPCMessage_Init(&ipcm, lpFilename, size, IPCC_OPEN_FILE_MAPPING) && IPCMessage_Write(&ipcm, &pipe);
  int nRemainingIPCMessages = 1;
  while (res && (nRemainingIPCMessages > 0))
  {
    HANDLE handles[] = { Event_GetWaitHandle(&eventIPCClientQuit), Pipe_GetWaitHandle(&pipe) };
    switch (WaitForMultipleObjects(COUNTOF(handles), handles, FALSE, INFINITE))
    {
      case WAIT_OBJECT_0:   // client process quit
        res = FALSE;
        break;
      case WAIT_OBJECT_0 + 1:   // process IPC message
        if (IPCMessage_Read(&ipcm, &pipe))
        {
          --nRemainingIPCMessages;
          if (ipcm.error)
          {
            dwLastIOError = ipcm.error;
            res = FALSE;
          }
          else if (IPCMessage_IsOpenFileMappingCommand(&ipcm))
          {
            // created file mapping
            if (ipcm.size == 0)
            {
              SendMessage(hwndEdit, SCI_SETSAVEPOINT, 0, 0);
              bIOResult = TRUE;
              IPCMessage_SetCommand(&ipcm, IPCC_CLOSE_FILE_MAPPING);
              FileMapping_ResetError(&fileMapping);
              bIOResult = IPCMessage_Write(&ipcm, &pipe) && FileMapping_Close(&fileMapping, -1);
              ++nRemainingIPCMessages;
            }
            else if (FileMapping_Open(&fileMapping, ipcm.filename, ipcm.size, TRUE)
              && FileMapping_MapViewOfFile(&fileMapping))
            {
              bIOResult = FileIO(FALSE, szCurFile, FALSE, &iEncoding, &iEOLMode, NULL, NULL, &bCancelDataLoss, FALSE);

              bIOResult &= FileMapping_UnmapViewOfFile(&fileMapping);
              ipcm.size = fileMapping.iDataSize;
              IPCMessage_SetCommand(&ipcm, IPCC_CLOSE_FILE_MAPPING);
              bIOResult &= IPCMessage_Write(&ipcm, &pipe);
              FileMapping_ResetError(&fileMapping);
              bIOResult &= FileMapping_Close(&fileMapping, fileMapping.iDataSize);
              ++nRemainingIPCMessages;
            }
            else
            {
              dwLastIOError = FileMapping_GetError(&fileMapping);
              res = FALSE;
            }
          }
          else if (IPCMessage_IsCloseFileMappingCommand(&ipcm))
          {
            break;
          }
        }
        break;
      default:
        break;
    }
  }
  return bIOResult;
}

BOOL n2e_IPC_FileIO(LPCWSTR lpFilename, const LONGLONG size)
{
  BOOL res = FALSE;
  if (!n2e_IsElevatedModeEnabled())
  {
    return res;
  }
  res = n2e_IsIPCInitialized();
  if (!res)
  {
    // reset IPC and wait for result
    Event_Reset(&eventIPCResult);
    Event_Set(&eventIPCReset);
    MSG msg = { 0 };
    while ((msg.message != WM_QUIT) && !Event_Wait(&eventIPCResult, 0))
    {
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }
  return n2e_IPC_ServerProc(lpFilename, size);
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

BOOL n2e_IsIPCInitialized()
{
  return bIPCInitialized;
}

BOOL n2e_SwitchElevation(const BOOL bResetIPC)
{
  if (n2e_IsElevatedModeEnabled())
  {
    bElevationEnabled = FALSE;
    n2e_FinalizeIPC(FALSE);
    ReloadMainMenu();
    return TRUE;
  }
  else
  {
    bElevationEnabled = TRUE;
    return Thread_Init(&threadIPCServerWorker, (TThreadProc)IPCServerWorkerThreadProc, (LPVOID)bResetIPC);
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
