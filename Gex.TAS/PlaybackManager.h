#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "GEX.TAS.h"
#include "Addresses.h"
#include "InputRecord.h"
#include "EasyHookUtils.h"


extern bool g_bPaused;
extern signed int g_GameSpeed;
extern void SetInputMask(EInputState);
extern HOOK_TRACE_INFO	GEXCheckForInputHookHandle;


extern unsigned long __cdecl GEXCheckForInput_Hook(unsigned long, unsigned long, unsigned long);

class PlaybackManager
{
public:

	PlaybackManager(const char*);

	InputRecord * GetCurrentInput();

	InputRecord * GetCurrentInputIndexBased();

	unsigned long GetCurrentInputIndex();

	void DoPlayback(bool);

	unsigned long ReloadPlayback();

	bool ReadInputFile();

	void InitPlayback(bool);

	bool IsPlayingBack();

	unsigned long GetLastReadSeed();

	unsigned long GetTotalFrameCount();

private:
	InputRecord * m_pCurrentInput;

	std::vector<InputRecord*> m_Inputs;

	unsigned long m_InputIndex;

	FILE * m_Fp;

	unsigned long m_nTotalFrameCount;

	unsigned long m_nLastReadSeed;

	unsigned long m_RuntoLineNo;

	unsigned long m_WalktoLineNo;

	unsigned long m_TotalFrameCountOfInputFile;

	unsigned long m_CurrentFrame;

	unsigned long m_FrameToNext;

	bool m_bPlaybackReady;

	bool m_bPlayingBack;

protected:

};

extern PlaybackManager *g_pPlaybackManager;
