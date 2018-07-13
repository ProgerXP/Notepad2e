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
  Event event;
  __int64 iBufferSize;
  __int64 iFileOffset;
  LPBYTE lpData;
  LPBYTE lpDataCurrent;
  __int64 iDataSize;
  DWORD error;
};
typedef struct TFileMapping FileMapping;

DWORD FileMapping_GetError(const FileMapping *pFileMapping);
void FileMapping_SaveError(FileMapping *pFileMapping);
BOOL FileMapping_IsOK(const FileMapping *pFileMapping);
HANDLE FileMapping_GetWaitHandle(const FileMapping *pFileMapping);
BOOL FileMapping_Init(FileMapping *pFileMapping, LPCWSTR lpName, LPCWSTR lpEventName, const BOOL bOpenExisting);
BOOL FileMapping_Free(FileMapping *pFileMapping);
BOOL FileMapping_IsOpened(const FileMapping *pFileMapping);
BOOL FileMapping_Open(FileMapping *pFileMapping, LPCWSTR lpFile, const __int64 size, const BOOL bOpenExisting);
BOOL FileMapping_Close(FileMapping *pFileMapping, const __int64 size);
BOOL FileMapping_MapViewOfFile(FileMapping *pFileMapping);
BOOL FileMapping_UnmapViewOfFile(FileMapping *pFileMapping);
BOOL FileMapping_Read(FileMapping *pFileMapping, LPBYTE pBuffer, const DWORD count);
BOOL FileMapping_Write(FileMapping *pFileMapping, LPCBYTE pBuffer, DWORD count);
BOOL FileMapping_Reset(FileMapping *pFileMapping);
BOOL FileMapping_Pulse(FileMapping *pFileMapping);
BOOL FileMapping_Set(FileMapping *pFileMapping);
BOOL FileMapping_Wait(FileMapping *pFileMapping, const UINT nDelay);
