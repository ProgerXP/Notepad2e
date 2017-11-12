#pragma once
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>

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

#define CPI_WINDOWS_1251 30   // code page index in NP2ENCODING mEncoding[]-array
#define CPI_WINDOWS_1250 19

#define CP_WINDOWS_1251 1251
#define CP_WINDOWS_1250 1250
#define CP_SHIFT_JIS 932

std::map<int, std::wstring> mapEncodingNames = {
  { CPI_DEFAULT, L"Default encoding" },
  { CPI_UNICODE, L"Unicode encoding" },
  { CPI_UTF8, L"UTF-8 encoding" },
  { CPI_WINDOWS_1251, L"Windows-1251" },
  { CPI_WINDOWS_1250, L"Windows-1250" }
};

#define TEST_DATA_PATH "..\\test\\data\\Extension\\"

class CTestCaseData
{
private:
  bool isFile;
  std::string strSrc;
  int iEncoding;
  std::string strExpectedRes;
  bool isDecodeOnly;              // result sample is impure, data is for decoding test only
                                  // sufficient buffer size for decoder is required
  int iDecodeOnlyMinBufferSize;
public:
  CTestCaseData(const bool file, const std::string src, const int encoding, const std::string res, 
                const bool decodeOnly = false, const int decodeOnlyMinBufferSize = 0)
    : isFile(file), strSrc(src), iEncoding(encoding), strExpectedRes(res), isDecodeOnly(decodeOnly),
    iDecodeOnlyMinBufferSize(decodeOnlyMinBufferSize)
  {
  }
  CTestCaseData(const bool file, const std::wstring src, const int encoding, const std::string res,
                const bool decodeOnly = false, const int decodeOnlyMinBufferSize = 0)
    : isFile(file), strSrc(UCS2toUTF8(src)), iEncoding(encoding), strExpectedRes(res), isDecodeOnly(decodeOnly),
    iDecodeOnlyMinBufferSize(decodeOnlyMinBufferSize)
  {
  }
  static std::string LoadFile(const std::string filename)
  {
    std::vector<char> vectorBuffer;
    auto file = TEST_DATA_PATH + filename;
    if (PathFileExistsA(file.c_str()))
    {
      HANDLE hFile = CreateFileA(file.c_str(), FILE_READ_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
      if (hFile != INVALID_HANDLE_VALUE)
      {
        auto fileSize = GetFileSize(hFile, NULL);
        vectorBuffer.resize(fileSize);
        DWORD dwBytesRead = 0;
        ReadFile(hFile, &vectorBuffer[0], fileSize, &dwBytesRead, NULL);
        vectorBuffer.push_back(0);
        CloseHandle(hFile);
      }
    }
    return &vectorBuffer[0];
  }
  bool IsFile() const
  {
    return isFile;
  }
  bool IsDecodeOnly() const
  {
    return isDecodeOnly;
  }
  int GetDecodeOnlyMinBufferSize() const
  {
    return iDecodeOnlyMinBufferSize;
  }
  LPCSTR GetPlainSource() const
  {
    return strSrc.c_str();
  }
  LPCSTR GetSourceText() const
  {
    if (isFile)
    {
      static std::string fileSourceText;
      fileSourceText = LoadFile(strSrc);
      return fileSourceText.c_str();
    }
    else
    {
      return strSrc.c_str();
    }
  }
  int GetEncoding() const
  {
    return iEncoding;
  }
  LPCSTR GetExpectedResultText() const
  {
    if (isFile)
    {
      static std::string fileExpectedResultText;
      fileExpectedResultText = LoadFile(strExpectedRes);
      return fileExpectedResultText.c_str();
    }
    else
    {
      return strExpectedRes.c_str();
    }
  }
  std::wstring GetErrorMessageText() const
  {
    static std::wstring res;
    res = L"unknown message";
    auto it = mapEncodingNames.find(iEncoding);
    assert(it != mapEncodingNames.cend());
    if (it != mapEncodingNames.cend())
    {
      res = it->second;
    }
    return res;
  }
};
