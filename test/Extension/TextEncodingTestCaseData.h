#pragma once
#include <wtypes.h>
#include <string>
#include <vector>

#define CPI_WINDOWS_1251 30   // code page index in NP2ENCODING mEncoding[]-array
#define CPI_WINDOWS_1250 19
#define CPI_WINDOWS_KOI8_R 27

#define CP_WINDOWS_1251 1251
#define CP_WINDOWS_1250 1250
#define CP_SHIFT_JIS 932

std::string UCS2toCP(const std::wstring &str, const UINT codePage);
std::string UCS2toUTF8(const std::wstring &str);
std::wstring CPtoUCS2(const std::string &str, const UINT codePage);

static std::vector<unsigned char> VectorFromString(LPCSTR str, int length = -1)
{
  std::vector<unsigned char> res;
  length = (length <= 0) ? strlen(str) : length;
  res.resize(length);
  std::copy(str, str + length, res.begin());
  return res;
}

static std::string StringFromVector(std::vector<unsigned char> v)
{
  return std::string((LPCSTR)v.data(), v.size());
}

class CTestCaseData
{
private:
  bool isFile;
  std::vector<unsigned char> vectorSource;
  int iEncoding;
  std::vector<unsigned char> vectorExpectedResult;
  bool isDecodeOnly;              // result sample is impure, data is for decoding test only
                                  // sufficient buffer size for decoder is required
  int iDecodeOnlyMinBufferSize;
public:
  CTestCaseData(const bool file, const std::string& src, const int encoding, const std::string& res, 
                const bool decodeOnly = false, const int decodeOnlyMinBufferSize = 0)
    : isFile(file), iEncoding(encoding), isDecodeOnly(decodeOnly),
    iDecodeOnlyMinBufferSize(decodeOnlyMinBufferSize)
  {
    vectorSource = VectorFromString(src.c_str());
    vectorExpectedResult = VectorFromString(res.c_str());
  }
  CTestCaseData(const bool file, const std::vector<unsigned char> src, const int encoding, const std::string res,
                const bool decodeOnly = false, const int decodeOnlyMinBufferSize = 0)
    : isFile(file), iEncoding(encoding), isDecodeOnly(decodeOnly),
    iDecodeOnlyMinBufferSize(decodeOnlyMinBufferSize)
  {
    vectorSource = src;
    vectorExpectedResult = VectorFromString(res.c_str());
  }
  CTestCaseData(const bool file, const std::wstring src, const int encoding, const std::string res,
                const bool decodeOnly = false, const int decodeOnlyMinBufferSize = 0)
    : isFile(file), iEncoding(encoding), isDecodeOnly(decodeOnly),
    iDecodeOnlyMinBufferSize(decodeOnlyMinBufferSize)
  {
    vectorSource = VectorFromString(UCS2toUTF8(src).c_str());
    vectorExpectedResult = VectorFromString(res.c_str());
  }
  static std::vector<unsigned char> LoadFile(const std::string filename);
  bool IsFile() const;
  bool IsDecodeOnly() const;
  int GetDecodeOnlyMinBufferSize() const;
  const std::vector<unsigned char>& GetPlainSource() const;
  const std::vector<unsigned char>& GetSourceText() const;
  int GetEncoding() const;
  const std::vector<unsigned char>& GetPlainResult() const;
  std::vector<unsigned char> GetExpectedResultText() const;
  std::wstring GetEncodingName() const;
};
