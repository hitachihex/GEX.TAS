#pragma once

#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cctype>

#include "EasyHookUtils.h"

enum EInputState
{
	DEFAULT_NONE = 0x0,    

	LEFT         = 0x10000000,
	RIGHT        = 0x20000000,  
	UP           = 0x40000000,
	DOWN         = 0x80000000,
	TONGUE       = 0x8000000,
	JUMP         = 0x4000000,
	TAIL         = 0x2000000,
	SPRINT       = 0x200000,
	SLOWDOWN     = 0x100000
};

typedef struct t_InputRecord
{
	EInputState m_InputState;
	int m_Frames;
	int m_TotalFrames;
	int m_Done;
	unsigned int m_LineNo;
	unsigned long m_nSeed;
	unsigned long SceneIndex;
	signed short xPos;
	signed short yPos;

	std::string ToStringReduced()
	{
		std::string result;

		if (this->IsEmpty())
			result += ",None";

		if (this->IsLeft())
			result += ",L";
		if (this->IsRight())
			result += ",R";
		if (this->IsUp())
			result += ",U";
		if (this->IsDown())
			result += ",D";
		if (this->IsJump())
			result += ",J";

		// W for 'whip'
		if (this->IsTail())
			result += ",W";
		if (this->IsTongue())
			result += ",T";
		if (this->IsSprint())
			result += ",S";

		return result;
	}

	std::string ToString()
	{
		std::string result;
		
		if (this->IsEmpty())
			result += ",None";

		if (this->IsLeft())
			result += ",Left";
		if (this->IsRight())
			result += ",Right";
		if (this->IsUp())
			result += ",Up";
		if (this->IsDown())
			result += ",Down";
		if (this->IsJump())
			result += ",Jump";
		if (this->IsTail())
			result += ",Tail";
		if (this->IsTongue())
			result += ",Tongue";
		if (this->IsSprint())
			result += ",Sprint";

		return result;
	}

	bool HasFlag(EInputState state, EInputState which)
	{
		return (state&which) == which;
	}

	bool IsEmpty()
	{
		return this->m_InputState == EInputState::DEFAULT_NONE;
	}

	bool IsLeft()
	{
		return this->HasFlag(this->m_InputState, EInputState::LEFT);
	}

	bool IsRight()
	{
		return this->HasFlag(this->m_InputState, EInputState::RIGHT);
	}

	bool IsUp()
	{
		return this->HasFlag(this->m_InputState, EInputState::UP);
	}

	bool IsDown()
	{
		return this->HasFlag(this->m_InputState, EInputState::DOWN);
	}

	bool IsTail()
	{
		return this->HasFlag(this->m_InputState, EInputState::TAIL);
	}

	bool IsTongue()
	{
		return this->HasFlag(this->m_InputState, EInputState::TONGUE);
	}

	bool IsJump()
	{
		return this->HasFlag(this->m_InputState, EInputState::JUMP);
	}

	bool IsSprint()
	{
		return this->HasFlag(this->m_InputState, EInputState::SPRINT);
	}

	bool IsSlow()
	{
		return this->HasFlag(this->m_InputState, EInputState::SLOWDOWN);
	}


	t_InputRecord(unsigned long frames, EInputState state)
	{
		this->m_Frames = frames;
		this->m_InputState = state;
	}

	t_InputRecord(std::string line, unsigned int ln)
	{
		this->SceneIndex = -1;
		this->m_LineNo = ln;
		this->m_Done = 0;
		this->m_nSeed = -1;
		this->xPos = -1;
		this->yPos = -1;

		std::istringstream ss(line);
		std::string token;

		std::vector<std::string> tokens;
		auto delimited = line.find(',');

		while (std::getline(ss, token, ','))
			tokens.push_back(token);

		this->m_Frames = (delimited == std::string::npos) ? std::stoul(line) : std::stoul(tokens[0]);
		this->m_TotalFrames = this->m_Frames;

		unsigned int tempState = EInputState::DEFAULT_NONE;

		if (tokens.size() > 1 && delimited != std::string::npos)
		{
			for (unsigned int i = 1; i < tokens.size(); i++)
			{
				token = tokens[i];

				// we already handled that integers that should be the first element of a token, ignore as erroneous 
				if (isdigit(tokens[i][0]))
					continue;

				auto negativelamb = [](char& ch) { ch = toupper((unsigned char)ch); };
				std::for_each(token.begin(), token.end(), negativelamb);

				auto lhstrim = [](std::string& in)
				{
					//lambception
					auto iter = std::find_if(in.begin(), in.end(), [](char ch) { return !std::isspace((unsigned char)ch); });
					in.erase(in.begin(), iter);
					return in;
				};

				auto rhstrim = [](std::string& in)
				{
					auto iter2 = std::find_if(in.rbegin(), in.rend(), [](char ch) { return !std::isspace((unsigned char)ch); });
					in.erase(iter2.base(), in.end());
					return in;
				};

				// Remove the leading and trailing spaces.
				token = lhstrim(token);
				token = rhstrim(token);
				if (token == "LEFT")
				{
					tempState |= EInputState::LEFT;
					continue;
				}
				else if (token == "RIGHT")
				{
					tempState |= EInputState::RIGHT;
					continue;
				}
				else if (token == "UP")
				{
					tempState |= EInputState::UP;
					continue;
				}
				else if (token == "DOWN")
				{
					tempState |= EInputState::DOWN;
					continue;
				}
				else if (token == "TAIL")
				{
					tempState |= EInputState::TAIL;
					continue;
				}
				else if (token == "JUMP")
				{
					tempState |= EInputState::JUMP;
					continue;
				}
				else if (token == "TONGUE")
				{
					tempState |= EInputState::TONGUE;
					continue;
				}
				else if (token == "SPRINT")
				{
					tempState |= EInputState::SPRINT;
					continue;
				}
				else if (token == "USPRINT")
				{
					tempState |= (EInputState::UP | EInputState::SPRINT);
					continue;
				}
				else if (token == "DSPRINT")
				{
					tempState |= (EInputState::DOWN | EInputState::SPRINT);
					continue;
				}
				else if (token == "LSPRINT")
				{
					tempState |= (EInputState::LEFT | EInputState::SPRINT);
					continue;
				}
				else if (token == "RSPRINT")
				{
					tempState |= (EInputState::RIGHT || EInputState::SPRINT);
					continue;
				}
				else if (token == "SCENE")
				{
					if ((i + 1) < tokens.size())
						this->SceneIndex = std::stoul(tokens[i + 1]);
					else
					{
						DebugOutput("SCENE command missing numeric value");
					}
					continue;
				}
				else if (token == "XPOS")
				{
					if ((i + 1) < tokens.size())
						this->xPos = std::stold(tokens[i + 1]);
					else
					{
						DebugOutput("XPOS command missing numeric value");
					}
					continue;
				}
				else if (token == "YPOS")
				{
					if ((i + 1) < tokens.size())
						this->yPos = std::stold(tokens[i + 1]);
					else
					{
						DebugOutput("YPOS command missing numeric value");
					}
					continue;
				}
			}
		}

		this->m_InputState = (EInputState)(tempState);
	}

	t_InputRecord() {}
} InputRecord;