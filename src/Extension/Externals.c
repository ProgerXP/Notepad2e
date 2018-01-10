#include "Externals.h"

#ifdef N2E_TESTING

int iEncoding = CPI_DEFAULT;

// Code duplication for mEncoding:
// https://github.com/ProgerXP/Notepad2e/issues/118#issuecomment-353869333
NP2ENCODING mEncoding[] = {
  { NCP_DEFAULT | NCP_RECODE, 0, "ansi,ansi,ascii,", 61000, L"" },
  { NCP_8BIT | NCP_RECODE, 0, "oem,oem,", 61001, L"" },
  { NCP_UNICODE | NCP_UNICODE_BOM, 0, "", 61002, L"" },
  { NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_UNICODE_BOM, 0, "", 61003, L"" },
  { NCP_UNICODE | NCP_RECODE, 0, "utf-16,utf16,unicode,", 61004, L"" },
  { NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_RECODE, 0, "utf-16be,utf16be,unicodebe,", 61005, L"" },
  { NCP_UTF8 | NCP_RECODE, 0, "utf-8,utf8,", 61006, L"" },
  { NCP_UTF8 | NCP_UTF8_SIGN, 0, "utf-8,utf8,", 61007, L"" },
  { NCP_8BIT | NCP_RECODE, CP_UTF7, "utf-7,utf7,", 61008, L"" },
  { NCP_8BIT | NCP_RECODE, 720, "DOS-720,dos720,", 61009, L"" },
  { NCP_8BIT | NCP_RECODE, 28596, "iso-8859-6,iso88596,arabic,csisolatinarabic,ecma114,isoir127,", 61010, L"" },
  { NCP_8BIT | NCP_RECODE, 10004, "x-mac-arabic,xmacarabic,", 61011, L"" },
  { NCP_8BIT | NCP_RECODE, 1256, "windows-1256,windows1256,cp1256", 61012, L"" },
  { NCP_8BIT | NCP_RECODE, 775, "ibm775,ibm775,cp500,", 61013, L"" },
  { NCP_8BIT | NCP_RECODE, 28594, "iso-8859-4,iso88594,csisolatin4,isoir110,l4,latin4,", 61014, L"" },
  { NCP_8BIT | NCP_RECODE, 1257, "windows-1257,windows1257,", 61015, L"" },
  { NCP_8BIT | NCP_RECODE, 852, "ibm852,ibm852,cp852,", 61016, L"" },
  { NCP_8BIT | NCP_RECODE, 28592, "iso-8859-2,iso88592,csisolatin2,isoir101,latin2,l2,", 61017, L"" },
  { NCP_8BIT | NCP_RECODE, 10029, "x-mac-ce,xmacce,", 61018, L"" },
  { NCP_8BIT | NCP_RECODE, 1250, "windows-1250,windows1250,xcp1250,", 61019, L"" },
  { NCP_8BIT | NCP_RECODE, 936, "gb2312,gb2312,chinese,cngb,csgb2312,csgb231280,gb231280,gbk,", 61020, L"" },
  { NCP_8BIT | NCP_RECODE, 10008, "x-mac-chinesesimp,xmacchinesesimp,", 61021, L"" },
  { NCP_8BIT | NCP_RECODE, 950, "big5,big5,cnbig5,csbig5,xxbig5,", 61022, L"" },
  { NCP_8BIT | NCP_RECODE, 10002, "x-mac-chinesetrad,xmacchinesetrad,", 61023, L"" },
  { NCP_8BIT | NCP_RECODE, 10082, "x-mac-croatian,xmaccroatian,", 61024, L"" },
  { NCP_8BIT | NCP_RECODE, 866, "cp866,cp866,ibm866,", 61025, L"" },
  { NCP_8BIT | NCP_RECODE, 28595, "iso-8859-5,iso88595,csisolatin5,csisolatincyrillic,cyrillic,isoir144,", 61026, L"" },
  { NCP_8BIT | NCP_RECODE, 20866, "koi8-r,koi8r,cskoi8r,koi,koi8,", 61027, L"" },
  { NCP_8BIT | NCP_RECODE, 21866, "koi8-u,koi8u,koi8ru,", 61028, L"" },
  { NCP_8BIT | NCP_RECODE, 10007, "x-mac-cyrillic,xmaccyrillic,", 61029, L"" },
  { NCP_8BIT | NCP_RECODE, 1251, "windows-1251,windows1251,xcp1251,", 61030, L"" },
  { NCP_8BIT | NCP_RECODE, 28603, "iso-8859-13,iso885913,", 61031, L"" },
  { NCP_8BIT | NCP_RECODE, 863, "ibm863,ibm863,", 61032, L"" },
  { NCP_8BIT | NCP_RECODE, 737, "ibm737,ibm737,", 61033, L"" },
  { NCP_8BIT | NCP_RECODE, 28597, "iso-8859-7,iso88597,csisolatingreek,ecma118,elot928,greek,greek8,isoir126,", 61034, L"" },
  { NCP_8BIT | NCP_RECODE, 10006, "x-mac-greek,xmacgreek,", 61035, L"" },
  { NCP_8BIT | NCP_RECODE, 1253, "windows-1253,windows1253,", 61036, L"" },
  { NCP_8BIT | NCP_RECODE, 869, "ibm869,ibm869,", 61037, L"" },
  { NCP_8BIT | NCP_RECODE, 862, "DOS-862,dos862,", 61038, L"" },
  { NCP_8BIT | NCP_RECODE, 38598, "iso-8859-8-i,iso88598i,logical,", 61039, L"" },
  { NCP_8BIT | NCP_RECODE, 28598, "iso-8859-8,iso88598,csisolatinhebrew,hebrew,isoir138,visual,", 61040, L"" },
  { NCP_8BIT | NCP_RECODE, 10005, "x-mac-hebrew,xmachebrew,", 61041, L"" },
  { NCP_8BIT | NCP_RECODE, 1255, "windows-1255,windows1255,", 61042, L"" },
  { NCP_8BIT | NCP_RECODE, 861, "ibm861,ibm861,", 61043, L"" },
  { NCP_8BIT | NCP_RECODE, 10079, "x-mac-icelandic,xmacicelandic,", 61044, L"" },
  { NCP_8BIT | NCP_RECODE, 10001, "x-mac-japanese,xmacjapanese,", 61045, L"" },
  { NCP_8BIT | NCP_RECODE, 932, "shift_jis,shiftjis,shiftjs,csshiftjis,cswindows31j,mskanji,xmscp932,xsjis,", 61046, L"" },
  { NCP_8BIT | NCP_RECODE, 10003, "x-mac-korean,xmackorean,", 61047, L"" },
  { NCP_8BIT | NCP_RECODE, 949, "windows-949,windows949,ksc56011987,csksc5601,euckr,isoir149,korean,ksc56011989", 61048, L"" },
  { NCP_8BIT | NCP_RECODE, 28593, "iso-8859-3,iso88593,latin3,isoir109,l3,", 61049, L"" },
  { NCP_8BIT | NCP_RECODE, 28605, "iso-8859-15,iso885915,latin9,l9,", 61050, L"" },
  { NCP_8BIT | NCP_RECODE, 865, "ibm865,ibm865,", 61051, L"" },
  { NCP_8BIT | NCP_RECODE, 437, "ibm437,ibm437,437,cp437,cspc8,codepage437,", 61052, L"" },
  { NCP_8BIT | NCP_RECODE, 858, "ibm858,ibm858,ibm00858,", 61053, L"" },
  { NCP_8BIT | NCP_RECODE, 860, "ibm860,ibm860,", 61054, L"" },
  { NCP_8BIT | NCP_RECODE, 10010, "x-mac-romanian,xmacromanian,", 61055, L"" },
  { NCP_8BIT | NCP_RECODE, 10021, "x-mac-thai,xmacthai,", 61056, L"" },
  { NCP_8BIT | NCP_RECODE, 874, "windows-874,windows874,dos874,iso885911,tis620,", 61057, L"" },
  { NCP_8BIT | NCP_RECODE, 857, "ibm857,ibm857,", 61058, L"" },
  { NCP_8BIT | NCP_RECODE, 28599, "iso-8859-9,iso88599,latin5,isoir148,l5,", 61059, L"" },
  { NCP_8BIT | NCP_RECODE, 10081, "x-mac-turkish,xmacturkish,", 61060, L"" },
  { NCP_8BIT | NCP_RECODE, 1254, "windows-1254,windows1254,", 61061, L"" },
  { NCP_8BIT | NCP_RECODE, 10017, "x-mac-ukrainian,xmacukrainian,", 61062, L"" },
  { NCP_8BIT | NCP_RECODE, 1258, "windows-1258,windows-258,", 61063, L"" },
  { NCP_8BIT | NCP_RECODE, 850, "ibm850,ibm850,", 61064, L"" },
  { NCP_8BIT | NCP_RECODE, 28591, "iso-8859-1,iso88591,cp819,latin1,ibm819,isoir100,latin1,l1,", 61065, L"" },
  { NCP_8BIT | NCP_RECODE, 10000, "macintosh,macintosh,", 61066, L"" },
  { NCP_8BIT | NCP_RECODE, 1252, "windows-1252,windows1252,cp367,cp819,ibm367,us,xansi,", 61067, L"" },
  { NCP_8BIT | NCP_RECODE, 37, "ebcdic-cp-us,ebcdiccpus,ebcdiccpca,ebcdiccpwt,ebcdiccpnl,ibm037,cp037,", 61068, L"" },
  { NCP_8BIT | NCP_RECODE, 500, "x-ebcdic-international,xebcdicinternational,", 61069, L"" },
  { NCP_8BIT | NCP_RECODE, 875, "x-EBCDIC-GreekModern,xebcdicgreekmodern,", 61070, L"" },
  { NCP_8BIT | NCP_RECODE, 1026, "CP1026,cp1026,csibm1026,ibm1026,", 61071, L"" },
  { NCP_8BIT | NCP_RECODE, 54936, "gb18030,gb18030,", 61072, L"" },
};

void n2e_ShowProgressBarInStatusBar(LPCWSTR pProgressText, const long nCurPos, const long nMaxPos)
{
}

void n2e_HideProgressBarInStatusBar()
{
}

void n2e_UpdateProgressBarInStatusBar(const long nCurPos) 
{
}

WCHAR szIniFile[MAX_PATH];

#endif // #ifdef N2E_TESTING
