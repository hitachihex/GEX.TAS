#pragma once

#include <string>
#include "PlaybackManager.h"
#include "GameObject.h"
#include "EasyHookUtils.h"
#pragma warning(disable : 4996)



#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING

#define ExclusiveHookWithCount(a,b) MakeHookExclusive(g_ACLEntries, a, b)
#define ExclusiveHook(b) MakeHookExclusive(g_ACLEntries, 1, b)

#define DoOnceBlock(s) static bool bOnce=false; if(!bOnce) {  bOnce=true; DebugOutput(s); }

#define GEX_DEFAULT_GAMESPEED 33

extern unsigned long startTime;

typedef unsigned long(__cdecl * oTimeGetTime)();
extern oTimeGetTime original_TimeGetTime;

typedef unsigned long(__cdecl * oGetTickCount)();
extern oGetTickCount original_GetTickCount;

extern unsigned long g_ACLEntries[1];


extern unsigned long g_dwTickCount;
extern unsigned long g_dwBaseTime;

extern void InitFastForward();
extern unsigned long TimeGetTime_Hook();
extern unsigned long GetTickCount_Hook();

extern unsigned long __cdecl GexSynchStep_Hook();
extern void __cdecl GexUpdateRenderState_Hook();
//extern void __cdecl SynchStep_Stub();

extern void SetSceneIndex(unsigned long);

extern GEX_GameObject * GetPlayerObject();
extern void mpatch(unsigned long addr, const char *bytes);
extern void QOL();
extern HOOK_TRACE_INFO GexGameLevelStepHookHandle;
extern HOOK_TRACE_INFO GexSynchStepHookHandle;
extern HOOK_TRACE_INFO GexUpdateRenderStateHookHandle;

extern char g_szPlaybackOutputBuffer[1024];
extern char g_szGOBPlayerInfo[512];
extern bool g_bPressedFrameStepThisFrame;
extern bool g_bShowOSD;