#pragma once
#include <wtypes.h>
#include <string>

#define CPI_WINDOWS_1251 30   // code page index in NP2ENCODING mEncoding[]-array
#define CPI_WINDOWS_1250 19
#define CPI_WINDOWS_KOI8_R 27

#define CP_WINDOWS_1251 1251
#define CP_WINDOWS_1250 1250
#define CP_SHIFT_JIS 932

std::string UCS2toCP(const std::wstring &str, const UINT codePage);
std::string UCS2toUTF8(const std::wstring &str);
std::wstring CPtoUCS2(const std::string &str, const UINT codePage);

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
  static std::string LoadFile(const std::string filename);
  bool IsFile() const;
  bool IsDecodeOnly() const;
  int GetDecodeOnlyMinBufferSize() const;
  LPCSTR GetPlainSource() const;
  LPCSTR GetSourceText() const;
  int GetEncoding() const;
  LPCSTR GetExpectedResultText() const;
  std::wstring GetErrorMessageText() const;
};
