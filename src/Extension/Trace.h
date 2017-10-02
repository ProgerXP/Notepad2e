#pragma once
#include "stdafx.h"

VOID n2e_InitializeTrace();
VOID n2e_FinalizeTrace();

#ifdef _DEBUG
VOID n2e_Trace(const char *fmt, ...);
VOID n2e_WTrace(const char *fmt, LPCWSTR word);
VOID n2e_WTrace2(const char *fmt, LPCWSTR word1, LPCWSTR word2);

#define __FILE_LOC (1+strrchr(__FILE__,'\\')/*?strrchr(__FILE__,'\\')+1:__FILE__*/)
#define N2E_TRACE_PLAIN(FMT,...)	n2e_Trace ( #FMT , __VA_ARGS__ );
#define N2E_TRACE(FMT,...)	n2e_Trace ( "[%s: %d] - "#FMT , __FILE_LOC , __LINE__ , __VA_ARGS__ );
#define N2E_TRACE_S(OBJ)		n2e_Trace ( "[%s: %d] [%s]=%s " , __FILE_LOC , __LINE__ , #OBJ , OBJ );
#define N2E_TRACE_I(OBJ)		n2e_Trace ( "[%s: %d] [%s]=%d (0x%04xd) " , __FILE_LOC , __LINE__ , #OBJ , OBJ , OBJ );
#define N2E_TRACE_TR(OBJ)		n2e_Trace ( "[%s: %d] [%s]= TEXTRANGE %d:%d(%s) " , __FILE_LOC , __LINE__ , #OBJ , OBJ.chrg.cpMin ,OBJ.chrg.cpMax ,OBJ.lpstrText );
#define N2E_WTRACE_PLAIN(FMT,...)	n2e_WTrace ( #FMT , __VA_ARGS__ );
#else
#define N2E_TRACE_PLAIN(FMT,...)	(void)(FMT);
#define N2E_TRACE(FMT,...)	 (void)(FMT);
#define N2E_TRACE_S(OBJ)		(void)(OBJ);
#define N2E_TRACE_I(OBJ)		(void)(OBJ);
#define N2E_TRACE_TR(OBJ)		(void)(OBJ);
#define N2E_WTRACE_PLAIN(FMT,...)	(void)(FMT);
#endif
