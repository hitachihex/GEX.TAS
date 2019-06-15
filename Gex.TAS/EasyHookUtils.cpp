#include "stdafx.h"
#include "EasyHookUtils.h"
#include <stdio.h>


NTSTATUS AddHook(void* pInEntryPoint, void * pHookProc, void * pCallback, TRACED_HOOK_HANDLE traceRecord)
{
	return LhInstallHook(pInEntryPoint, pHookProc, pCallback, traceRecord);
}


NTSTATUS MakeHookExclusive(unsigned long * pThreadListArray, unsigned long dwThreadCount, TRACED_HOOK_HANDLE traceRecord)
{
	return LhSetExclusiveACL(pThreadListArray, dwThreadCount, traceRecord);
}


void DebugOutput(const char * fmt, ...)
{
	char outputBuffer[2048] = { 0 };
	va_list vaList;

	va_start(vaList, fmt);
	vsnprintf(outputBuffer, 2048, fmt, vaList);
	va_end(vaList);

	OutputDebugStringA(outputBuffer);
}

void DebugOutputW(const wchar_t* fmt, ...)
{
	wchar_t outputBuffer[2048] = { 0 };

	va_list args;
	va_start(args, fmt);
	vswprintf_s(outputBuffer, 2048, fmt, args);
	va_end(args);

	OutputDebugStringW(outputBuffer);
}