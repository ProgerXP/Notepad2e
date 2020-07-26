#include "stdafx.h"
#include "../src/Extension/tinyexpr/tinyexpr.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

class CEvalTestData
{
public:
  std::string source;
  double result;

  CEvalTestData(const std::string& src, const double res) : source(src), result(res)
  {
  }
};

namespace Notepad2eTests
{
  TEST_CLASS(CEval)
  {
  public:
    TEST_METHOD(EvalTests)
    {
      const CEvalTestData data[] = {
        // original tests from TinyExpr
        CEvalTestData("1", 1),
        CEvalTestData("(1)", 1),

        CEvalTestData("pi", 3.14159265358979323846),
        CEvalTestData("atan(1)*4 - pi", 0),
        CEvalTestData("e", 2.71828182845904523536),

        CEvalTestData("2+1", 2 + 1),
        CEvalTestData("(((2+(1))))", 2 + 1),
        CEvalTestData("3+2", 3 + 2),

        CEvalTestData("3+2+4", 3 + 2 + 4),
        CEvalTestData("(3+2)+4", 3 + 2 + 4),
        CEvalTestData("3+(2+4)", 3 + 2 + 4),
        CEvalTestData("(3+2+4)", 3 + 2 + 4),

        CEvalTestData("3*2*4", 3 * 2 * 4),
        CEvalTestData("(3*2)*4", 3 * 2 * 4),
        CEvalTestData("3*(2*4)", 3 * 2 * 4),
        CEvalTestData("(3*2*4)", 3 * 2 * 4),

        CEvalTestData("3-2-4", 3 - 2 - 4),
        CEvalTestData("(3-2)-4", (3 - 2) - 4),
        CEvalTestData("3-(2-4)", 3 - (2 - 4)),
        CEvalTestData("(3-2-4)", 3 - 2 - 4),

        CEvalTestData("3/2/4", 3.0 / 2.0 / 4.0),
        CEvalTestData("(3/2)/4", (3.0 / 2.0) / 4.0),
        CEvalTestData("3/(2/4)", 3.0 / (2.0 / 4.0)),
        CEvalTestData("(3/2/4)", 3.0 / 2.0 / 4.0),

        CEvalTestData("(3*2/4)", 3.0*2.0 / 4.0),
        CEvalTestData("(3/2*4)", 3.0 / 2.0*4.0),
        CEvalTestData("3*(2/4)", 3.0*(2.0 / 4.0)),

        CEvalTestData("asin sin .5", 0.5),
        CEvalTestData("sin asin .5", 0.5),
        CEvalTestData("ln exp .5", 0.5),
        CEvalTestData("exp ln .5", 0.5),

        CEvalTestData("asin sin-.5", -0.5),
        CEvalTestData("asin sin-0.5", -0.5),
        CEvalTestData("asin sin -0.5", -0.5),
        CEvalTestData("asin (sin -0.5)", -0.5),
        CEvalTestData("asin (sin (-0.5))", -0.5),
        CEvalTestData("asin sin (-0.5)", -0.5),
        CEvalTestData("(asin sin (-0.5))", -0.5),

        CEvalTestData("log 1000", 3),
        CEvalTestData("log 1e3", 3),
        CEvalTestData("log 1000", 3),
        CEvalTestData("log 1e3", 3),
        CEvalTestData("log(1000)", 3),
        CEvalTestData("log(1e3)", 3),
        CEvalTestData("log 1.0e3", 3),
        CEvalTestData("10^5*5e-5", 5),

        CEvalTestData("100^.5+1", 11),
        CEvalTestData("100 ^.5+1", 11),
        CEvalTestData("100^+.5+1", 11),
        CEvalTestData("100^--.5+1", 11),
        CEvalTestData("100^---+-++---++-+-+-.5+1", 11),

        CEvalTestData("100^-.5+1", 1.1),
        CEvalTestData("100^---.5+1", 1.1),
        CEvalTestData("100^+---.5+1", 1.1),
        CEvalTestData("1e2^+---.5e0+1e0", 1.1),
        CEvalTestData("--(1e2^(+(-(-(-.5e0))))+1e0)", 1.1),

        CEvalTestData("sqrt 100 + 7", 17),
        CEvalTestData("sqrt 100 * 7", 70),
        CEvalTestData("sqrt (100 * 100)", 100),

        CEvalTestData("1,2", 1.2),
        CEvalTestData("1,2+1", 2.2),
        CEvalTestData("1+1,2+2,2+1", 5.4),
        CEvalTestData("1,2,3", 123),

// Next tests are no longer passed (expressions are not interpretable) due to added enhancements:
//         CEvalTestData("(1,2),3", 3),
//         CEvalTestData("1,(2,3)", 3),
//         CEvalTestData("-(1,(2,3))", -3),

        CEvalTestData("2^3", 8),

        // custom tests
        CEvalTestData("1+2+3", 6),
        CEvalTestData("1 2 3", 6),
        CEvalTestData("1\r\n2\r\n3", 6),
        CEvalTestData("\r\n1\r\n\r\n2\r\n3", 6),
        CEvalTestData("1+2=4", 3),
        CEvalTestData("12.34+.1", 12.44),
        CEvalTestData("12,34+,1", 12.44),
        CEvalTestData("1,2,3+1", 124),
        CEvalTestData("1,2,3 + 1,2 + ,3", 138),
        CEvalTestData("12,3 + 1,2 + ,3", 13.80),
        CEvalTestData("12,3 45.6 $78 10h 0xe", 276.6),
        CEvalTestData("1,    2    \t\t3", 6),
        CEvalTestData("(1 + 2) * (4 - 1) / (4 ^ 2)", 0.5625),
        CEvalTestData("0xF % 2", 1),
        CEvalTestData("6.7 div 2 mod 2 shl 3 shr 2", 2),
        CEvalTestData("not (15 and 3 or 7 xor 1)", -7),
        CEvalTestData("not 1", -2),
        CEvalTestData("not -2", 1),
        CEvalTestData("not-1000", 999),
        CEvalTestData("11b + 11o + 11d + 11h", 40),
        CEvalTestData("sin 0 + cos 0", 1),
        CEvalTestData("1e3 1", 1001),
        CEvalTestData("floor (pi + e)", 5),
        CEvalTestData("1e+1 + 1e-1", 10.1),

        // Eval bugs #130: e-specific tests for shortcut operator:
        CEvalTestData("1e1 1e 1", 20),                      // converted/evaluated to: 1e1+1e+1 => 1*10^1+1*10^1
        CEvalTestData("1 e 1", 2 + 2.71828182845904523536), // converted/evaluated to: 1+e+1 => 2+exp

        CEvalTestData("12345$ / 1,01", 12222.77227722772),
      };

      for (auto i = 0; i < _countof(data); i++)
      {
        const auto info = data[i];
        double exprValue = 0.0;
        if (is_valid_expression((unsigned char*)info.source.c_str(), 1, &exprValue))
        {
          Assert::AreEqual(info.result, exprValue, 0.000000001);
        }
        else
        {
          Assert::Fail(L"Invalid expression");
        }
      }
    }
  };

}
