// Gex.TAS.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "GEX.TAS.h"
#include "Addresses.h"
#include "PlaybackManager.h"
#include <timeapi.h>


#pragma comment(lib, "winmm.lib")

extern "C" void __declspec(dllexport) __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo);

typedef void(__cdecl * oGexUpdateRenderState)();
oGexUpdateRenderState originalGexUpdateRenderState = (oGexUpdateRenderState)(GEX_UPDATERENDERSTATE_ADDRESS);

typedef unsigned int(__cdecl *oGexSynchStep)();
oGexSynchStep originalGexSynchStep = (oGexSynchStep)(GEX_SYNCHSTEP_ADDRESS);

typedef unsigned int (__cdecl * oGexGameLevelStep)(void*);
oGexGameLevelStep originalGexGameLevelStep = (oGexGameLevelStep)(GEX_GAMELEVEL_STEP_ADDRESS);

oTimeGetTime original_TimeGetTime = (oTimeGetTime)(*(unsigned long*)GEX_TIMEGETTIME_IAT_ADDRESS);
oGetTickCount original_GetTickCount = (oGetTickCount)(*(unsigned long*)GEX_GETTICKCOUNT_IAT_ADDRESS);

unsigned long g_ACLEntries[1] = { 0 };

unsigned long g_dwTickCount;
unsigned long g_dwBaseTime;
unsigned long startTime = 0;

HOOK_TRACE_INFO GexGameLevelStepHookHandle = { NULL };
HOOK_TRACE_INFO GexSynchStepHookHandle = { NULL };
HOOK_TRACE_INFO GexUpdateRenderStateHookHandle = { NULL };


bool g_bPressedFrameStepThisFrame = false;
bool g_bShowOSD = false;
char g_szPlaybackOutputBuffer[1024] = { 0 };
char g_szGOBPlayerInfo[512] = { 0 };

void SetSceneIndex(unsigned long index)
{
	*(unsigned long*)(GEX_SCENEINDEX_ADDRESS) = index;
}

unsigned long GetSceneIndex()
{
	return *(unsigned long*)(GEX_SCENEINDEX_ADDRESS);
}

GEX_GameObject * GetPlayerObject()
{
	return (GEX_GameObject*)(*(unsigned long*)GEX_PC_ADDRESS);
}

void InitFastForward()
{
	g_dwTickCount = GetTickCount();
	g_dwBaseTime = timeGetTime();

	*(unsigned long*)(GEX_TIMEGETTIME_IAT_ADDRESS) = (unsigned long)TimeGetTime_Hook;
	*(unsigned long*)(GEX_GETTICKCOUNT_IAT_ADDRESS) = (unsigned long)GetTickCount_Hook;
}

unsigned long TimeGetTime_Hook()
{
	DoOnceBlock("timeGetTime hook ok");

	auto curGetTime = original_TimeGetTime();
	unsigned long result = 0;

	result = g_dwBaseTime + ((curGetTime - g_dwBaseTime) * g_GameSpeed);

	return result;
}

unsigned long GetTickCount_Hook()
{
	DoOnceBlock("GetTickCount hook, !bOnce");

	auto curTickCount = original_GetTickCount();
	return g_dwTickCount + ((curTickCount - g_dwTickCount) * g_GameSpeed);
}

void __cdecl GexUpdateRenderState_Hook()
{
	DoOnceBlock("GexUpdateRenderState_Hook, !bOnce");
	
	static signed short oldX, oldY;

	GEX_GameObject * gobPlayer = GetPlayerObject();

	if (g_bShowOSD)
	{
		HDC  devCtx = (HDC)(*(HDC*)(GEX_GDI_DEVICECONTEXT));

		HFONT hFont, hOldFont;

		hFont = CreateFontA(16, 7, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, "Arial");

		auto oldCol = SetTextColor(devCtx, RGB(0, 0, 255));	

		if (hOldFont = (HFONT)SelectObject(devCtx, hFont))
		{
			if (gobPlayer != nullptr)
			{
				signed short dx = (gobPlayer->xh - oldX);
				signed short dy = (gobPlayer->yh - oldY);
				sprintf(g_szGOBPlayerInfo, "(x: %d , y: %d) | xv: %d, yv: %d, scene:%d", gobPlayer->xh, gobPlayer->yh, (dx == 0) ? gobPlayer->xVelocityHigh : dx, (dy == 0) ?
					gobPlayer->yvh : dy, GetSceneIndex());
				TextOutA(devCtx, 1, 34, g_szGOBPlayerInfo, strlen(g_szGOBPlayerInfo));
			}

			if (g_pPlaybackManager->IsPlayingBack())
				TextOutA(devCtx, 1, 7, g_szPlaybackOutputBuffer, strlen(g_szPlaybackOutputBuffer));

			DeleteObject(hFont);
			SelectObject(devCtx, hOldFont);
			SetTextColor(devCtx, oldCol);

		}
	}

	if (gobPlayer)
	{
		oldX = gobPlayer->xh;
		oldY = gobPlayer->yh;
	}
	else
	{
		oldX = 0;
		oldY = 0;
	}

	return 	originalGexUpdateRenderState();
}

// Hook this so we can dynamically change the sleep interval
unsigned long __cdecl GexSynchStep_Hook()
{
	DoOnceBlock("GexSynchStep_Hook, !bOnce");

	signed int currentTime; 
	int v2; 
	int deltaTime; 

	currentTime = timeGetTime();

	if (startTime)
	{
		v2 = startTime + g_GameSpeed;

		deltaTime = currentTime - (startTime + g_GameSpeed);

		startTime = v2;
		if (currentTime - v2 < 333)
		{
			startTime = v2;
			// This will be negative once the game loop has taken long enough
			if (deltaTime < 0)
			{
				startTime = v2;
				// Spin until we  are
				if (currentTime < v2)
				{
					while ((signed int)timeGetTime() < startTime)
						Sleep(0);
				}
			} 

		}
		else
		{
			// This case resets start time when we come in\out of loading
			startTime = currentTime;
		}
	}
	else
	{
		startTime = currentTime;
	}

	return 1;
}

unsigned int __cdecl GexGameLevelStep_Hook(void *pArg)
{

	g_bPressedFrameStepThisFrame = false;

	// Not paused, check for pause hotkey.
	if (GetAsyncKeyState(VK_F1) & 1 && !g_bPaused)
	{
		g_bPaused = true;
		DebugOutput("GEX Paused.");
		g_GameSpeed = GEX_DEFAULT_GAMESPEED;
		return 1;
	}

	// Start/Stop playback.
	if (GetAsyncKeyState(VK_F4) & 1)
	{
		if (!g_pPlaybackManager->IsPlayingBack())
		{
			DebugOutput("Starting playback.");
			g_GameSpeed = 0;
		}

		g_pPlaybackManager->InitPlayback(true);
	}

	if (GetAsyncKeyState(VK_F6) & 1)
	{
		// Back to main menu
		SetSceneIndex(eSceneIDS::MAINMENU);
	}

	if (GetAsyncKeyState(VK_F5) & 1)
	{
		g_bShowOSD = !g_bShowOSD;
	}

	// reset to normal game speed
	if (GetAsyncKeyState(VK_DIVIDE) & 1)
	{
		g_GameSpeed = GEX_DEFAULT_GAMESPEED;
	}

	// Decrease game speed
	if (GetAsyncKeyState(VK_ADD) & 1)
	{
		if ((g_GameSpeed + 1) > 50)
			g_GameSpeed = 50;
		else
			g_GameSpeed++;

		DebugOutput("Game speed is now: %d", g_GameSpeed);
	}

	// Increase game speed
	if (GetAsyncKeyState(VK_SUBTRACT) & 1)
	{
		g_GameSpeed--;

		DebugOutput("Game speed is now: %d", g_GameSpeed);
	}


	if (g_bPaused)
	{
		if (GetAsyncKeyState(VK_OEM_6) & 1)
		{
			g_bPressedFrameStepThisFrame = true;
			return originalGexGameLevelStep(pArg);
		}

		// Separate unpause key.
		if (GetAsyncKeyState(0x69) & 1)
		{
			g_bPaused = false;
			g_GameSpeed = GEX_DEFAULT_GAMESPEED;
			DebugOutput("GEX Unpausing.");
			goto stepOutUnpause;
		}

		return 1;
	}

	stepOutUnpause:
	return originalGexGameLevelStep(pArg);
}

void mpatch(unsigned long addr, const char *bytes)
{
	unsigned int len = sizeof(bytes) / sizeof(bytes[0]);

	unsigned long dwOldProt = 0;
	VirtualProtect((LPVOID)addr, len, PAGE_EXECUTE_READWRITE, &dwOldProt);

	for (unsigned int i = 0; i < len; i++)
		*(unsigned char*)(addr + i) = bytes[i];
}

void QOL()
{
	// for hotkeys
	mpatch(0x403CC9, "\xEB\x25");
	mpatch(0x403D94, "\xE9\xA4\x00\x00\x00\x90");
	mpatch(0x403E40, "\xEB\x1D");
	mpatch(0x403E62, "\xEB\x1D");

	// for tabbing\tally screen
	mpatch(0x40214F, "\xE9\x9E\x02\x00\x00\x90");

	// for tally screen fix
	mpatch(0x40D374, "\x90\x90\x90\x90\x90\x90\x90\x90\x90");

	// for tabbing
	mpatch(0x4051D0, "\xC3\x90\x90\x90\x90");
}

void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo)
{

	NTSTATUS result = AddHook((void*)GEX_GAMELEVEL_STEP_ADDRESS, GexGameLevelStep_Hook, NULL, &GexGameLevelStepHookHandle);


	// in memory patch
	QOL();

	if (FAILED(result))
	{
		std::wstring err(RtlGetLastErrorString());
		DebugOutputW(err.c_str());
	}
	else
	{
		DebugOutput("GexGameLevelStepHook installed.");
		ExclusiveHook(&GexGameLevelStepHookHandle);
	}

	result = AddHook((void*)GEX_SYNCHSTEP_ADDRESS, GexSynchStep_Hook, NULL, &GexSynchStepHookHandle);
	
	if (FAILED(result))
	{
		std::wstring err(RtlGetLastErrorString());
		DebugOutputW(err.c_str());
	}
	else
	{
		DebugOutput("GexSynchStepHook installed.");

		ExclusiveHook(&GexSynchStepHookHandle);
	}


	result = AddHook((void*)GEX_UPDATERENDERSTATE_ADDRESS, GexUpdateRenderState_Hook, NULL, &GexUpdateRenderStateHookHandle);

	if (FAILED(result))
	{
		std::wstring err(RtlGetLastErrorString());
		DebugOutputW(err.c_str());
	}
	else
	{
		DebugOutput("GexUpdateRenderStateHook installed.");

		ExclusiveHook(&GexUpdateRenderStateHookHandle);
	}

	result = AddHook((void*)GEX_CHECKFORINPUT_ADDRESS, GEXCheckForInput_Hook, NULL, &GEXCheckForInputHookHandle);

	if (FAILED(result))
	{
		std::wstring err(RtlGetLastErrorString());
		DebugOutputW(err.c_str());
	}
	else
	{
		DebugOutput("GexCheckForInputHook installed.");

		ExclusiveHook(&GEXCheckForInputHookHandle);
	}
	
	g_pPlaybackManager = new PlaybackManager("Gex.rec");
	//InitFastForward();
}

