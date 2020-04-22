#include "stdafx.h"
#include "TextEncodingTestCaseData.h"
#include <assert.h>
#include <map>
#include <vector>
#include <fstream>
#include <streambuf>
#include <ShlWapi.h>
#include "../src/Extension/Externals.h"

std::string UCS2toCP(const std::wstring &str, const UINT codePage)
{
  if (str.empty())
  {
    return "";
  }
  std::vector<char> res;
  const int requiredSize = WideCharToMultiByte(codePage, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
  if (requiredSize == 0)
  {
    return "";
  }
  res.resize(requiredSize);
  WideCharToMultiByte(codePage, 0, str.c_str(), -1, &res[0], requiredSize, NULL, NULL);
  return &res[0];
}

std::string UCS2toUTF8(const std::wstring &str)
{
  return UCS2toCP(str, CP_UTF8);
}

std::wstring CPtoUCS2(const std::string &str, const UINT codePage)
{
  if (str.empty())
  {
    return L"";
  }
  std::vector<wchar_t> res;
  const int requiredSize = MultiByteToWideChar(codePage, 0, str.c_str(), -1, NULL, 0);
  if (requiredSize == 0)
  {
    return L"";
  }
  res.resize(requiredSize);
  MultiByteToWideChar(codePage, 0, str.c_str(), -1, &res[0], requiredSize);
  return &res[0];
}

std::map<int, std::wstring> mapEncodingNames = {
  { CPI_DEFAULT, L"Default" },
  { CPI_UNICODE, L"Unicode" },
  { CPI_UTF8, L"UTF-8" },
  { CPI_WINDOWS_1251, L"Windows-1251" },
  { CPI_WINDOWS_1250, L"Windows-1250" },
  { CPI_WINDOWS_KOI8_R, L"Windows-KOI8-R" }
};

#define TEST_DATA_PATH "..\\..\\..\\test\\data\\Extension"

std::vector<unsigned char> CTestCaseData::LoadFile(const std::string filename)
{
  std::vector<unsigned char> vectorBuffer;
  char* pFileSamplesPath;
  size_t len = 0;
  if (_dupenv_s(&pFileSamplesPath, &len, "FileSamplesPath") || !pFileSamplesPath)
  {
    pFileSamplesPath = TEST_DATA_PATH;
  }
  std::string fileSamplesPath(pFileSamplesPath);
  if (fileSamplesPath.size() && (*fileSamplesPath.rbegin() != '\\'))
  {
    fileSamplesPath += L'\\';
  }
  auto file = fileSamplesPath + filename;
  if (PathFileExistsA(file.c_str()))
  {
    HANDLE hFile = CreateFileA(file.c_str(), FILE_READ_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
      auto fileSize = GetFileSize(hFile, NULL);
      vectorBuffer.resize(fileSize);
      DWORD dwBytesRead = 0;
      ReadFile(hFile, &vectorBuffer[0], fileSize, &dwBytesRead, NULL);
      CloseHandle(hFile);
    }
  }
  else
  {
    vectorBuffer.resize(MAX_PATH);
    GetCurrentDirectoryA(vectorBuffer.size() - 1, (LPSTR)&vectorBuffer[0]);
    std::string filePath((LPSTR)&vectorBuffer[0]);
    filePath += "\\" + file;
    const std::wstring errorMessage(L"File not found: " + CPtoUCS2(filePath, CP_ACP));
    Microsoft::VisualStudio::CppUnitTestFramework::Assert::Fail(errorMessage.c_str(), LINE_INFO());
  }
  return vectorBuffer;
}

bool CTestCaseData::IsFile() const
{
  return isFile;
}

bool CTestCaseData::IsDecodeOnly() const
{
  return isDecodeOnly;
}

int CTestCaseData::GetDecodeOnlyMinBufferSize() const
{
  return iDecodeOnlyMinBufferSize;
}

const std::vector<unsigned char>& CTestCaseData::GetPlainSource() const
{
  return vectorSource;
}

const std::vector<unsigned char>& CTestCaseData::GetSourceText() const
{
  if (isFile)
  {
    static std::vector<unsigned char> fileSourceText;
    fileSourceText = LoadFile(StringFromVector(vectorSource));
    return fileSourceText;
  }
  else
  {
    return vectorSource;
  }
}

int CTestCaseData::GetEncoding() const
{
  return iEncoding;
}

const std::vector<unsigned char>& CTestCaseData::GetPlainResult() const
{
  return vectorExpectedResult;
}

std::vector<unsigned char> CTestCaseData::GetExpectedResultText() const
{
  if (isFile)
  {
    static std::vector<unsigned char> fileExpectedResultText;
    fileExpectedResultText = LoadFile(StringFromVector(vectorExpectedResult));
    return fileExpectedResultText;
  }
  else
  {
    return vectorExpectedResult;
  }
}

std::wstring CTestCaseData::GetEncodingName() const
{
  static std::wstring res;
  res = L"unknown";
  auto it = mapEncodingNames.find(iEncoding);
  assert(it != mapEncodingNames.cend());
  if (it != mapEncodingNames.cend())
  {
    res = it->second;
  }
  return res;
}
