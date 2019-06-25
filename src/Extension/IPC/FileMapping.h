#pragma once
#include "Event.h"

#define LODWORD(l)           ((DWORD)(((__int64)(l)) & 0xffffffff))
#define HIDWORD(l)           ((DWORD)((((__int64)(l)) >> 32) & 0xffffffff))

extern DWORD dwFileMapAllocationGranularity;

struct TFileMapping
{
  HANDLE file;
  HANDLE handle;
  WCHAR name[MAX_PATH];
  Event eventTryCreate;
  Event eventCreated;
  Event eventTryClose;
  Event eventClosed;
  __int64 iBufferSize;
  __int64 iFileOffset;
  LPBYTE lpData;
  LPBYTE lpDataCurrent;
  __int64 iDataSize;
  DWORD error;
};
typedef struct TFileMapping FileMapping;

void FileMapping_ResetError(FileMapping *pFileMapping);
DWORD FileMapping_GetError(const FileMapping *pFileMapping);
void FileMapping_SaveError(FileMapping *pFileMapping);
BOOL FileMapping_IsOK(const FileMapping *pFileMapping);
HANDLE FileMapping_GetTryCreateHandle(const FileMapping *pFileMapping);
HANDLE FileMapping_GetCreatedHandle(const FileMapping *pFileMapping);
HANDLE FileMapping_GetTryCloseHandle(const FileMapping *pFileMapping);
HANDLE FileMapping_GetClosedHandle(const FileMapping *pFileMapping);
BOOL FileMapping_Init(FileMapping *pFileMapping, LPCWSTR lpName, const BOOL bOpenExisting);
BOOL FileMapping_Free(FileMapping *pFileMapping);
BOOL FileMapping_TryCreate(FileMapping *pFileMapping);
BOOL FileMapping_TryClose(FileMapping *pFileMapping);
BOOL FileMapping_IsOpened(const FileMapping *pFileMapping);
BOOL FileMapping_Open(FileMapping *pFileMapping, LPCWSTR lpFile, const __int64 size, const BOOL bOpenExisting);
BOOL FileMapping_Close(FileMapping *pFileMapping, const __int64 size);
BOOL FileMapping_MapViewOfFile(FileMapping *pFileMapping);
BOOL FileMapping_UnmapViewOfFile(FileMapping *pFileMapping);
BOOL FileMapping_FlushViewOfFile(FileMapping *pFileMapping);
BOOL FileMapping_Read(FileMapping *pFileMapping, LPBYTE pBuffer, const DWORD count);
BOOL FileMapping_Write(FileMapping *pFileMapping, LPCBYTE pBuffer, const DWORD count);
