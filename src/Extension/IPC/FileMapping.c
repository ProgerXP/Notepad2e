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
    && Event_IsOK(&pFileMapping->event);
}

HANDLE FileMapping_GetWaitHandle(const FileMapping *pFileMapping)
{
  return FileMapping_IsOK(pFileMapping) ? Event_GetWaitHandle(&pFileMapping->event) : NULL;
}

BOOL FileMapping_Init(FileMapping *pFileMapping, LPCWSTR lpName, LPCWSTR lpEventName, const BOOL bOpenExisting)
{
  if (!pFileMapping || !lpName || !lpEventName)
  {
    return FALSE;
  }
  if (FileMapping_IsOK(pFileMapping))
  {
    FileMapping_Free(pFileMapping);
  }
  ZeroMemory(pFileMapping, sizeof(FileMapping));
  lstrcpyn(pFileMapping->name, lpName, CSTRLEN(pFileMapping->name));
  Event_Init(&pFileMapping->event, lpEventName, bOpenExisting);

  return FileMapping_IsOK(pFileMapping);
}

BOOL FileMapping_Free(FileMapping *pFileMapping)
{
  FileMapping_Close(pFileMapping, -1);
  Event_Free(&pFileMapping->event);
  return TRUE;
}

BOOL FileMapping_IsOpened(const FileMapping *pFileMapping)
{
  return FileMapping_IsOK(pFileMapping) && (pFileMapping->file || pFileMapping->handle);
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
  return (!bOpenExisting && (size == 0)) || (pFileMapping->handle != NULL);
}

BOOL FileMapping_Close(FileMapping *pFileMapping, const __int64 size)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  if (pFileMapping->lpData && !FileMapping_FlushViewOfFile(pFileMapping))
  {
    FileMapping_SaveError(pFileMapping);
  }
  if (pFileMapping->lpData && !FileMapping_UnmapViewOfFile(pFileMapping))
  {
    FileMapping_SaveError(pFileMapping);
  }
  if (pFileMapping->handle)
  {
    CloseHandle(pFileMapping->handle);
    pFileMapping->handle = NULL;
  }
  if (pFileMapping->file)
  {
    if (size >= 0)
    {
      LONG hiDWORD = HIDWORD(size);
      if (INVALID_SET_FILE_POINTER == SetFilePointer(pFileMapping->file, LODWORD(size), &hiDWORD, FILE_BEGIN))
      {
        FileMapping_SaveError(pFileMapping);
      }
      if (!SetEndOfFile(pFileMapping->file))
      {
        FileMapping_SaveError(pFileMapping);
      }
    }
    CloseHandle(pFileMapping->file);
    pFileMapping->file = NULL;
  }
  pFileMapping->iFileOffset = 0;
  pFileMapping->iDataSize = 0;
  return TRUE;
}

BOOL FileMapping_MapViewOfFile(FileMapping *pFileMapping)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  pFileMapping->lpData = MapViewOfFile(pFileMapping->handle, FILE_MAP_WRITE,
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
  return pFileMapping->lpData && FlushViewOfFile(pFileMapping->lpData, pFileMapping->iBufferSize);
}

BOOL FileMapping_Read(FileMapping *pFileMapping, LPBYTE pBuffer, const DWORD count)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  DWORD dwRead = 0;
  return ReadFile(pFileMapping->handle, pBuffer, count, &dwRead, NULL) && (dwRead == count);
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

BOOL FileMapping_Write(FileMapping *pFileMapping, LPCBYTE pBuffer, DWORD count)
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
      FileMapping_FlushViewOfFile(pFileMapping);
      FileMapping_UnmapViewOfFile(pFileMapping);
      if (!FileMapping_MapViewOfFile(pFileMapping))
      {
        return FALSE;
      }
      sizeDest = FileMapping_BufferAvailable(pFileMapping);
      assert(sizeDest > 0);
    }
    const size_t sizeCopy = min(sizeDest, count);
    memcpy_s(pFileMapping->lpDataCurrent, sizeCopy, pBuffer, sizeCopy);
    pFileMapping->iDataSize += sizeCopy;
    pFileMapping->lpDataCurrent += sizeCopy;

    count -= sizeCopy;
    pBuffer += sizeCopy;
  }
  return TRUE;
}

BOOL FileMapping_Reset(FileMapping *pFileMapping)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  return Event_Reset(&pFileMapping->event);
}

BOOL FileMapping_Set(FileMapping *pFileMapping)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  return Event_Set(&pFileMapping->event);
}

BOOL FileMapping_Wait(FileMapping *pFileMapping, const UINT nDelay)
{
  if (!FileMapping_IsOK(pFileMapping))
  {
    return FALSE;
  }
  return Event_Wait(&pFileMapping->event, nDelay);
}
