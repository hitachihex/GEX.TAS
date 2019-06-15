#include "stdafx.h"
#include "PlaybackManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <functional>

bool g_bPaused = false;
signed int g_GameSpeed = 33;


typedef unsigned long(__cdecl * oGexCheckForInput)(unsigned long, unsigned long, unsigned long);
oGexCheckForInput originalGexCheckForInput = (oGexCheckForInput)(GEX_CHECKFORINPUT_ADDRESS);

HOOK_TRACE_INFO GEXCheckForInputHookHandle = { NULL };
PlaybackManager * g_pPlaybackManager = nullptr;

void SetInputMask(EInputState s)
{
	*(unsigned long*)(GEX_INPUTMASK_ADDRESS) |= s;
}

PlaybackManager::PlaybackManager(const char * pszFileName)
{
	this->m_nTotalFrameCount = 0;
	this->m_nLastReadSeed = 1337; 
	this->m_Fp = NULL;
	this->m_bPlaybackReady = false;
	this->m_bPlayingBack = false;
	if (pszFileName)
	{
		// _SH_DENYNO for shared access.
		this->m_Fp = _fsopen(pszFileName, "r", _SH_DENYNO);
	}
}

bool PlaybackManager::ReadInputFile()
{
	bool first = true;

	char LineBuffer[2048] = { 0 };
	unsigned int linecount = 0;

	if (this->m_Fp == NULL)
	{
		DebugOutput("Hey bud, your file pointer is null.");
		return false;
	}

	rewind(this->m_Fp);
	this->m_Inputs.clear();

	try
	{

		while (true)
		{

			if (fgets(LineBuffer, 2048, this->m_Fp) == NULL)
				break;

			// Remove the newline
			LineBuffer[strcspn(LineBuffer, "\n")] = 0;

			if (strlen(LineBuffer) == 0)
			{
				// still increase our linecount
				++linecount;
				memset(LineBuffer, 0, sizeof(LineBuffer) / sizeof(LineBuffer[0]));
				continue;
			}

			if (LineBuffer[0] == '#')
			{
				// still increase our linecount
				++linecount;
				memset(LineBuffer, 0, sizeof(LineBuffer) / sizeof(LineBuffer[0]));
				continue;
			}

			std::string stringBuffer(LineBuffer);
			unsigned int indexRunto = stringBuffer.find("Runto");
			unsigned int indexWalkto = stringBuffer.find("Walkto");
			if (indexRunto != std::string::npos)
			{
				this->m_RuntoLineNo = linecount;
				// still increase linecount
				linecount++;
				continue;
			}
			else if (indexWalkto != std::string::npos)
			{
				this->m_WalktoLineNo = linecount;
				// still increase linecount
				linecount++;
				continue;
			}

			try
			{

				InputRecord * p = new InputRecord(std::string(LineBuffer), ++linecount);
				this->m_nTotalFrameCount += p->m_Frames;
				this->m_Inputs.push_back(p);
			}
			catch (std::exception& e)
			{
				DebugOutput("Caught exception: %s", e.what());
			}


			memset(LineBuffer, 0, sizeof(LineBuffer) / sizeof(LineBuffer[0]));

		}

	}
	// shut up c4101
#pragma warning(disable : 4101)
	catch (std::exception& e)
	{

	}

	return true;
}


unsigned long PlaybackManager::GetTotalFrameCount()
{
	return this->m_nTotalFrameCount;
}

void PlaybackManager::InitPlayback(bool bReload = true)
{
	this->m_RuntoLineNo = -1;
	this->m_WalktoLineNo = -1;
	this->m_nTotalFrameCount = 0;

	if (this->m_bPlayingBack && bReload)
	{
		this->m_bPlayingBack = false;
		this->m_bPlaybackReady = false;
		return;
	}


	bool result = this->ReadInputFile();

	if (!result)
	{
		DebugOutput("failed to read input file");
		return;
	}


	this->m_bPlayingBack = true;

	this->m_CurrentFrame = 0;
	this->m_InputIndex = 0;

	if (this->m_Inputs.size() > 0)
	{
		this->m_pCurrentInput = this->m_Inputs[0];
		this->m_FrameToNext = m_pCurrentInput->m_Frames;
	}
	else
	{
		this->m_FrameToNext = 1;
		// Disable playback
		this->m_bPlaybackReady = false;
		this->m_bPlayingBack = false;
		return;
	}

	this->m_bPlaybackReady = true;

}

unsigned long PlaybackManager::ReloadPlayback()
{
	// Save it
	unsigned long dwPlayedBackFrames = this->m_CurrentFrame;
	this->InitPlayback(false);

	// Restore it
	this->m_CurrentFrame = dwPlayedBackFrames;

	// Step on the index until we get  back to where we were.
	while (this->m_CurrentFrame > this->m_FrameToNext)
	{
		if (this->m_InputIndex + 1 >= this->m_Inputs.size())
		{
			this->m_InputIndex++;
			return this->m_Inputs.size();
		}

		this->m_pCurrentInput = this->m_Inputs[++this->m_InputIndex];
		this->m_FrameToNext += this->m_pCurrentInput->m_Frames;
	}

	return this->m_Inputs.size();
}

bool PlaybackManager::IsPlayingBack()
{
	return this->m_bPlayingBack;
}

unsigned long PlaybackManager::GetCurrentInputIndex()
{
	return this->m_InputIndex;
}


//#pragma optimize("", off)
void PlaybackManager::DoPlayback(bool wasFramestepped)
{
	if (!this->m_bPlayingBack)
	{
		DebugOutput("Not playing back, but DoPlayback was called?");
		return;
	}


	if (this->m_InputIndex < this->m_Inputs.size())
	{
		if (wasFramestepped)
		{
			unsigned long OldInputDoneCount = m_pCurrentInput->m_Done;
			unsigned long ReloadedCount = this->ReloadPlayback();
			m_pCurrentInput->m_Done += OldInputDoneCount;
		}

		if (this->m_CurrentFrame >= this->m_FrameToNext)
		{
			if (this->m_InputIndex + 1 >= this->m_Inputs.size())
			{
				if (wasFramestepped)
				{
					unsigned long ReloadedCountScope2 = this->ReloadPlayback();
					if (this->m_InputIndex + 1 >= ReloadedCountScope2)
					{
						this->m_InputIndex++;

						// disable playback
						this->m_bPlaybackReady = false;
						this->m_bPlayingBack = false;

						return;
					}
				}
				else
				{
					if (this->m_InputIndex + 1 >= this->m_Inputs.size())
					{
						this->m_InputIndex++;

						// Disable playback
						this->m_bPlaybackReady = false;
						this->m_bPlayingBack = false;

						return;
					}
				}

			} // index + 1 > = inputs.size() scope end

			this->m_pCurrentInput = this->m_Inputs[++this->m_InputIndex];

			// Seed set
			if (m_pCurrentInput->m_nSeed != -1)
			{
				/*
				g_dwConstantSeed = m_pCurrentInput->m_nSeed;
				g_mt1997.seed(m_pCurrentInput->m_nSeed);
				g_pUnifDist->reset();

				this->m_nLastReadSeed = m_pCurrentInput->m_nSeed;*/
			}

			if (this->m_RuntoLineNo != -1)
			{
				if (m_pCurrentInput->m_LineNo < this->m_RuntoLineNo)
				{

					if (this->m_pCurrentInput->IsSlow())
					{
						g_GameSpeed = GEX_DEFAULT_GAMESPEED;
					}
					else
					{
						g_GameSpeed = 0;
					}
				}
				else
				{
					this->m_RuntoLineNo = -1;
					g_bPaused = true;
					g_GameSpeed = GEX_DEFAULT_GAMESPEED;
				}
			}
			else if (this->m_WalktoLineNo != -1)
			{
				if (m_pCurrentInput->m_LineNo < this->m_WalktoLineNo)
					g_GameSpeed = GEX_DEFAULT_GAMESPEED;
				else
				{
					this->m_WalktoLineNo = -1;
					g_bPaused = true;
					g_GameSpeed = GEX_DEFAULT_GAMESPEED;
				}
			}

			this->m_FrameToNext += this->m_pCurrentInput->m_Frames;
		} // frame to next scope end
		else
		{
			this->m_pCurrentInput->m_Done++;
		}

		//  next frame
		this->m_CurrentFrame++;

		sprintf(g_szPlaybackOutputBuffer, "Ln: %u (%u / %u) - [%s]\n(C:%u / T:%u)", this->m_pCurrentInput->m_LineNo, this->m_pCurrentInput->m_Done, this->m_pCurrentInput->m_Frames,
			this->m_pCurrentInput->ToString().c_str(), this->m_CurrentFrame, this->m_nTotalFrameCount);

		// check our current frame first, is it the last frame?
		if (this->m_CurrentFrame == this->m_FrameToNext)
		{

		}

	}

	if (this->m_pCurrentInput->IsJump())
		SetInputMask(EInputState::JUMP);

	if (this->m_pCurrentInput->IsTail())
		SetInputMask(EInputState::TAIL);

	if (this->m_pCurrentInput->IsTongue())
		SetInputMask(EInputState::TONGUE);

	if (this->m_pCurrentInput->IsSprint())
		SetInputMask(EInputState::SPRINT);

	if (this->m_pCurrentInput->IsLeft())
		SetInputMask(EInputState::LEFT);

	if (this->m_pCurrentInput->IsRight())
		SetInputMask(EInputState::RIGHT);

	if (this->m_pCurrentInput->IsUp())
		SetInputMask(EInputState::UP);

	if (this->m_pCurrentInput->IsDown())
		SetInputMask(EInputState::DOWN);

	if (this->m_pCurrentInput->SceneIndex != -1)
	{
		DebugOutput("Changing scene to %u", this->m_pCurrentInput->SceneIndex);
		SetSceneIndex(this->m_pCurrentInput->SceneIndex);
	}

	if (this->m_pCurrentInput->xPos != -1)
	{
		DebugOutput("XPOS to %d", this->m_pCurrentInput->xPos);
		auto plr = GetPlayerObject();
		plr->xh = this->m_pCurrentInput->xPos;
	}
	
	if (this->m_pCurrentInput->yPos != -1)
	{
		DebugOutput("YPOS to %d", this->m_pCurrentInput->yPos);
		auto plr = GetPlayerObject();
		plr->yh = this->m_pCurrentInput->yPos;
	}
	return;
}
//#pragma optimize("", on)

unsigned long __cdecl GEXCheckForInput_Hook(unsigned long a, unsigned long b, unsigned long c)
{
	DoOnceBlock("GEXCheckForInput_Hook, !bOnce");

	// okay, null the mask out
	*(unsigned long*)(GEX_INPUTMASK_ADDRESS) = 0x0;

	if (g_pPlaybackManager)
	{
		if (g_pPlaybackManager->IsPlayingBack())
		{
			g_pPlaybackManager->DoPlayback(g_bPressedFrameStepThisFrame);
			g_bPressedFrameStepThisFrame = false;

			unsigned long * pInputMask = (unsigned long*)(GEX_INPUTMASK_ADDRESS);
			// lol
			unsigned long eax = *pInputMask;
			unsigned long ecx = *(unsigned long*)(0x4A27D8);
			ecx |= eax;

			*(unsigned long*)(0x4A27D8) = ecx;

			ecx = *pInputMask;
			*(unsigned long*)(c) = ecx;

			return 1;
		}
	}

	return originalGexCheckForInput(a, b, c);
}
