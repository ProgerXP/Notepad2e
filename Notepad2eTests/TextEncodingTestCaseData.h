#pragma once
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>

std::string UCS2toUTF8(const std::wstring &str)
{
  if (str.empty())
  {
    return "";
  }
  std::vector<char> res;
  const int requiredSize = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
  if (requiredSize == 0)
  {
    return "";
  }
  res.resize(requiredSize);
  WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &res[0], requiredSize, NULL, NULL);
  return &res[0];
}

std::map<int, std::string> mapEncodingNames = {
  { CPI_DEFAULT, "Default encoding" },
  { CPI_UNICODE, "Unicode encoding" },
  { CPI_UTF8, "UTF-8 encoding" }
};

class CTestCaseData
{
private:
  bool isFile;
  std::string strSrc;
  int iEncoding;
  std::string strExpectedRes;
public:
  CTestCaseData(const bool file, const std::string src, const int encoding, const std::string res)
    : isFile(file), strSrc(src), iEncoding(encoding), strExpectedRes(res)
  {
  }
  CTestCaseData(const bool file, const std::wstring src, const int encoding, const std::string res)
    : isFile(file), strSrc(UCS2toUTF8(src)), iEncoding(encoding), strExpectedRes(res)
  {
  }
  static std::string LoadFile(const std::string filename)
  {
    std::ifstream f("..\\Notepad2eTests\\" + filename);
    std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    assert(!str.empty());
    return str;
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
  LPCSTR GetErrorMessageText() const
  {
    static std::string res;
    res = "unknown message";
    auto it = mapEncodingNames.find(iEncoding);
    assert(it != mapEncodingNames.cend());
    if (it != mapEncodingNames.cend())
    {
      res = it->second;
    }
    return res.c_str();
  }
};
