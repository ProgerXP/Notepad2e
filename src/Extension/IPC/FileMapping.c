#include "FileMapping.h"
#include <AclAPI.h>
#include <assert.h>

void FileMapping_ResetError(FileMapping *pFileMapping)
{
  if (FileMapping_IsOK(pFileMapping))
  {
    pFileMapping->error = 0;
  }
}

DWORD FileMapping_GetError(const FileMapping *pFileMapping)
{
  return FileMapping_IsOK(pFileMapping) ? pFileMapping->error : 0;
}

void FileMapping_SaveError(FileMapping *pFileMapping)
{
  if (FileMapping_IsOK(pFileMapping))
  {
    const DWORD dwError = GetLastError();
    if (dwError && !pFileMapping->error)
    {
      pFileMapping->error = dwError;
    }
  }
}

BOOL FileMapping_IsOK(const FileMapping *pFileMapping)
{
  return pFileMapping
    && (lstrlen(pFileMapping->name) > 0)
    && Event_IsOK(&pFileMapping->eventTryCreate)
    && Event_IsOK(&pFileMapping->eventCreated)
    && Event_IsOK(&pFileMapping->eventTryClose)
    && Event_IsOK(&pFileMapping->eventClosed);
}

HANDLE FileMapping_GetTryCreateHandle(const FileMapping *pFileMapping)
{
  return FileMapping_IsOK(pFileMapping) ? Event_GetWaitHandle(&pFileMapping->eventTryCreate) : NULL;
}

HANDLE FileMapping_GetCreatedHandle(const FileMapping *pFileMapping)
{
  return FileMapping_IsOK(pFileMapping) ? Event_GetWaitHandle(&pFileMapping->eventCreated) : NULL;
}

HANDLE FileMapping_GetTryCloseHandle(const FileMapping *pFileMapping)
{
  return FileMapping_IsOK(pFileMapping) ? Event_GetWaitHandle(&pFileMapping->eventTryClose) : NULL;
}

HANDLE FileMapping_GetClosedHandle(const FileMapping *pFileMapping)
{
  return FileMapping_IsOK(pFileMapping) ? Event_GetWaitHandle(&pFileMapping->eventClosed) : NULL;
}

BOOL FileMapping_Init(FileMapping *pFileMapping, LPCWSTR lpName, const BOOL bOpenExisting)
{
  if (!pFileMapping || !lpName)
  {
    return FALSE;
  }
  if (FileMapping_IsOK(pFileMapping))
  {
    FileMapping_Free(pFileMapping);
  }
  ZeroMemory(pFileMapping, sizeof(FileMapping));
  lstrcpyn(pFileMapping->name, lpName, CSTRLEN(pFileMapping->name));
  pFileMapping->file = INVALID_HANDLE_VALUE;

  WCHAR eventName[MAX_PATH] = { 0 };
  lstrcpyn(eventName, lpName, CSTRLEN(eventName));
  lstrcat(eventName, L"-EventTryCreate");
  Event_Init(&pFileMapping->eventTryCreate, &eventName[0], bOpenExisting);
  lstrcpyn(eventName, lpName, CSTRLEN(eventName));
  lstrcat(eventName, L"-EventCreated");
  Event_Init(&pFileMapping->eventCreated, &eventName[0], bOpenExisting);
  lstrcpyn(eventName, lpName, CSTRLEN(eventName));
  lstrcat(eventName, L"-EventTryClose");
  Event_Init(&pFileMapping->eventTryClose, &eventName[0], bOpenExisting);
  lstrcpyn(eventName, lpName, CSTRLEN(eventName));
  lstrcat(eventName, L"-EventClosed");
  Event_Init(&pFileMapping->eventClosed, &eventName[0], bOpenExisting);

  return FileMapping_IsOK(pFileMapping);
}

BOOL FileMapping_Free(FileMapping *pFileMapping)
{
  FileMapping_Close(pFileMapping, -1);
  Event_Free(&pFileMapping->eventTryCreate);
  Event_Free(&pFileMapping->eventCreated);
  Event_Free(&pFileMapping->eventTryClose);
  Event_Free(&pFileMapping->eventClosed);
  return TRUE;
}

BOOL FileMapping_TryCreate(FileMapping *pFileMapping)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  Event_Set(&pFileMapping->eventTryCreate);
  return TRUE;
}

BOOL FileMapping_TryClose(FileMapping *pFileMapping)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  FileMapping_Close(pFileMapping, -1);      // release handles
  Event_Set(&pFileMapping->eventTryClose);
  return TRUE;
}

BOOL FileMapping_IsOpened(const FileMapping *pFileMapping)
{
  return FileMapping_IsOK(pFileMapping) && ((pFileMapping->file != INVALID_HANDLE_VALUE) || pFileMapping->handle);
}

BOOL FileMapping_Open(FileMapping *pFileMapping, LPCWSTR lpFile, const __int64 size, const BOOL bOpenExisting)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  FileMapping_Close(pFileMapping, -1);
  if (!bOpenExisting)
  {
    if (lstrlen(lpFile))
    {
      pFileMapping->file = CreateFile(lpFile,
                                      GENERIC_READ | GENERIC_WRITE,
                                      0,
                                      NULL,
                                      OPEN_ALWAYS,
                                      0,
                                      NULL);
      if (pFileMapping->file == INVALID_HANDLE_VALUE)
      {
        FileMapping_SaveError(pFileMapping);
        FileMapping_Close(pFileMapping, -1);
        return FALSE;
      }
    }
    if (size != 0)
    {
      pFileMapping->handle = CreateFileMapping(pFileMapping->file, NULL, PAGE_READWRITE,
                                               HIDWORD(size), LODWORD(size), pFileMapping->name);
      if (pFileMapping->handle)
      {
        SetSecurityInfo(pFileMapping->handle, SE_KERNEL_OBJECT,
                        DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                        NULL, NULL, NULL, NULL);
      }
      else
      {
        FileMapping_SaveError(pFileMapping);
      }
    }
  }
  else
  {
    pFileMapping->handle = OpenFileMapping(FILE_MAP_WRITE, FALSE, pFileMapping->name);
    if (!pFileMapping->handle)
    {
      FileMapping_SaveError(pFileMapping);
    }
  }
  const BOOL bResult = (!bOpenExisting && (size == 0)) || (pFileMapping->handle != NULL);
  if (bResult && !bOpenExisting)
  {
    Event_Set(&pFileMapping->eventCreated);
  }
  return bResult;
}

BOOL FileMapping_Close(FileMapping *pFileMapping, const __int64 size)
{
  BOOL res = TRUE;
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  if (pFileMapping->lpData && !FileMapping_FlushViewOfFile(pFileMapping))
  {
    FileMapping_SaveError(pFileMapping);
    res = FALSE;
  }
  if (pFileMapping->lpData && !FileMapping_UnmapViewOfFile(pFileMapping))
  {
    FileMapping_SaveError(pFileMapping);
    res = FALSE;
  }
  if (pFileMapping->handle)
  {
    CloseHandle(pFileMapping->handle);
    pFileMapping->handle = NULL;
  }
  if (pFileMapping->file != INVALID_HANDLE_VALUE)
  {
    if (size >= 0)
    {
      LONG hiDWORD = HIDWORD(size);
      if (INVALID_SET_FILE_POINTER == SetFilePointer(pFileMapping->file, LODWORD(size), &hiDWORD, FILE_BEGIN))
      {
        FileMapping_SaveError(pFileMapping);
        res = FALSE;
      }
      if (!SetEndOfFile(pFileMapping->file))
      {
        FileMapping_SaveError(pFileMapping);
        res = FALSE;
      }
    }
    CloseHandle(pFileMapping->file);
    pFileMapping->file = INVALID_HANDLE_VALUE;
  }
  pFileMapping->iFileOffset = 0;
  pFileMapping->iDataSize = 0;
  if (res && (size >= 0))
  {
    Event_Set(&pFileMapping->eventClosed);
  }
  return res;
}

BOOL FileMapping_MapViewOfFile(FileMapping *pFileMapping)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  pFileMapping->lpData = MapViewOfFile(pFileMapping->handle, FILE_MAP_WRITE | FILE_MAP_READ,
                                       HIDWORD(pFileMapping->iFileOffset), 
                                       LODWORD(pFileMapping->iFileOffset), 0);
  pFileMapping->lpDataCurrent = pFileMapping->lpData;
  if (pFileMapping->lpData != NULL)
  {
    MEMORY_BASIC_INFORMATION mbi = { 0 };
    if (VirtualQuery(pFileMapping->lpData, &mbi, sizeof(mbi)))
    {
      pFileMapping->iBufferSize = mbi.RegionSize;
      pFileMapping->iFileOffset += pFileMapping->iBufferSize;
      return TRUE;
    }
  }
  FileMapping_SaveError(pFileMapping);
  return FALSE;
}

BOOL FileMapping_UnmapViewOfFile(FileMapping *pFileMapping)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  if (pFileMapping->lpData
      && UnmapViewOfFile(pFileMapping->lpData))
  {
    pFileMapping->lpData = NULL;
    pFileMapping->lpDataCurrent = NULL;
    return TRUE;
  }
  FileMapping_SaveError(pFileMapping);
  return FALSE;
}

BOOL FileMapping_FlushViewOfFile(FileMapping *pFileMapping)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  if (pFileMapping->lpData)
  {
    if (FlushViewOfFile(pFileMapping->lpData, pFileMapping->iBufferSize))
    {
      return TRUE;
    }
    FileMapping_SaveError(pFileMapping);
  }
  return FALSE;
}

DWORD FileMapping_BufferAvailable(FileMapping *pFileMapping)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return 0;
  }
  return (pFileMapping->lpData && pFileMapping->lpDataCurrent)
    ? pFileMapping->lpData + pFileMapping->iBufferSize - pFileMapping->lpDataCurrent
    : 0;
}

BOOL FileMapping_TransferData(FileMapping *pFileMapping, LPBYTE pBuffer, DWORD count, const BOOL bFromFileMapping)
{
  if (!FileMapping_IsOK(pFileMapping)
      || !pFileMapping->lpData
      || !pFileMapping->lpDataCurrent)
  {
    return FALSE;
  }

  while (count > 0)
  {
    DWORD sizeDest = FileMapping_BufferAvailable(pFileMapping);
    if (sizeDest == 0)
    {
      if (!FileMapping_FlushViewOfFile(pFileMapping)
          || !FileMapping_UnmapViewOfFile(pFileMapping)
          || !FileMapping_MapViewOfFile(pFileMapping))
      {
        return FALSE;
      }
      sizeDest = FileMapping_BufferAvailable(pFileMapping);
      assert(sizeDest > 0);
    }
    const size_t sizeCopy = min(sizeDest, count);
    if (bFromFileMapping)
    {
      memcpy_s(pBuffer, sizeCopy, pFileMapping->lpDataCurrent, sizeCopy);
    }
    else
    {
      memcpy_s(pFileMapping->lpDataCurrent, sizeCopy, pBuffer, sizeCopy);
    }
    pFileMapping->iDataSize += sizeCopy;
    pFileMapping->lpDataCurrent += sizeCopy;

    count -= sizeCopy;
    pBuffer += sizeCopy;
  }
  return TRUE;
}

BOOL FileMapping_Read(FileMapping *pFileMapping, LPBYTE pBuffer, const DWORD count)
{
  return FileMapping_TransferData(pFileMapping, pBuffer, count, TRUE);
}

BOOL FileMapping_Write(FileMapping *pFileMapping, LPCBYTE pBuffer, const DWORD count)
{
  return FileMapping_TransferData(pFileMapping, (LPBYTE)pBuffer, count, FALSE);
}
