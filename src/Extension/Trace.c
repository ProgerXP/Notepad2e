#include <corecrt_io.h>
#include "Trace.h"
#include "CommonUtils.h"
#include "Scintilla.h"  // required for Helpers.h
#include "Helpers.h"
#include "Utils.h"

#ifdef _DEBUG
FILE *n2e_log = 0;
#endif

VOID n2e_InitializeTrace()
{
#ifdef _DEBUG
  if (IniGetInt(N2E_INI_SECTION, L"DebugLog", 0))
  {
    n2e_log = _fsopen("n2e_log.log", "w", _SH_DENYNO);
  }
#endif
}

VOID n2e_FinalizeTrace()
{
#ifdef _DEBUG
  if (n2e_log)
  {
    fclose(n2e_log);
  }
  n2e_log = 0;
#endif
}

#ifdef _DEBUG
VOID n2e_PrintTraceTime()
{
  if (n2e_log)
  {
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(n2e_log, "[%02d:%02d:%03d] ", st.wMinute, st.wSecond, st.wMilliseconds);
  }
}

VOID n2e_Trace(const char *fmt, ...)
{
  if (n2e_log)
  {
    va_list vl;
    char buff[0xff + 1];
    char* ch = 0;
    n2e_PrintTraceTime();
    va_start(vl, fmt);
    vsprintf_s(buff, 0xff, fmt, vl);
    va_end(vl);
    ch = buff;
    while (*ch)
    {
      if ('\n' == *ch)
      {
        *ch = '¶';
      }
      ++ch;
    }
    fprintf(n2e_log, "%s\n", buff);
    fflush(n2e_log);
    // _commit() is important - see #129 and https://jeffpar.github.io/kbarchive/kb/066/Q66052/
    _commit(_fileno(n2e_log));
  }
}

VOID n2e_WTrace(const char *fmt, LPCWSTR word)
{
  if (n2e_log)
  {
    int size;
    char *temp = 0;
    n2e_PrintTraceTime();
    temp = n2e_Alloc(size = WideCharToMultiByte(CP_UTF8, 0, word, -1, NULL, 0, NULL, NULL));
    WideCharToMultiByte(CP_UTF8, 0, word, -1, temp, size, NULL, NULL);
    fprintf(n2e_log, fmt, temp);
    n2e_Free(temp);
    fprintf(n2e_log, "\n");
    fflush(n2e_log);
    _commit(_fileno(n2e_log));
  }
}

VOID n2e_WTrace2(const char *fmt, LPCWSTR word1, LPCWSTR word2)
{
  if (n2e_log)
  {
    int size;
    char *temp, *temp2;
    n2e_PrintTraceTime();
    temp = n2e_Alloc(size = WideCharToMultiByte(CP_UTF8, 0, word1, -1, NULL, 0, NULL, NULL));
    WideCharToMultiByte(CP_UTF8, 0, word1, -1, temp, size, NULL, NULL);
    temp2 = n2e_Alloc(size = WideCharToMultiByte(CP_UTF8, 0, word2, -1, NULL, 0, NULL, NULL));
    WideCharToMultiByte(CP_UTF8, 0, word2, -1, temp2, size, NULL, NULL);
    fprintf(n2e_log, fmt, temp, temp2);
    n2e_Free(temp);
    n2e_Free(temp2);
    fprintf(n2e_log, "\n");
    fflush(n2e_log);
    _commit(_fileno(n2e_log));
  }
}
#endif
