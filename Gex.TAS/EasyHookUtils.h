#pragma once

#include <easyhook.h>
//towari natte
extern NTSTATUS AddHook(void*, void*, void*, TRACED_HOOK_HANDLE);
extern NTSTATUS MakeHookExclusive(unsigned long*, unsigned long, TRACED_HOOK_HANDLE);

extern void DebugOutput(const char *, ...);

extern void DebugOutputW(const wchar_t*, ...);