#pragma once


enum eSceneIDS
{
	DISCOINFERNO=0x02,
	MAINMENU=0x3F,
	TALLYSCREEN=0x44,
	CONGOCHAOS=0x10
};

#pragma pack(push, 1)

class GEX_GameObject
{
public:

	// 0x00 - 0x03
	unsigned long m_dwUnknown;

	// 0x04 - 0x4F
	unsigned char m_ucUnk01[0x50 - 0x04];

	// 0x50 - 0x53
	unsigned long m_dwSpriteIndex;

	// 0x54 - 0x57
	unsigned long m_dwSpriteAnimationFrame;

	// 0x58 - 0x5B
	unsigned long m_dwUnknown01;

	// 0x5C - 0x5F
	// ptr to UpdatePlayer function
	void(*updatePlayer)(GEX_GameObject*);

	// 0x60 - 0x6F
	unsigned char m_ucUnk02[0x70 - 0x60];

	// 0x70 - 0x73
	unsigned long m_dwUnknown02;

	// 0x74 - 0x77
	unsigned long m_dwUnknown03;

	// unions of pos & scroll
	// 0x78 - 0x79
	unsigned short xl;

	// 0x7A - 0x7B
	unsigned short xh;

	// 0x7C - 0x7D
	unsigned short yl;

	// 0x7E - 0x7F
	unsigned short yh;

	// 0x80 - 0x81
	signed short xVelocityLow;

	// 0x82 - 0x83
	signed short xVelocityHigh;

	// 0x84 - 0x87
	unsigned long m_dwUnknown05;

	// 0x88 - 0x89
	unsigned short xvl;

	// 0x8A - 0x8B
	signed short xvh;

	// 0x8C - 0x8D
	unsigned short yvl;

	// 0x8E - 0x8F
	signed short yvh;

	// 0x90 - 0xD3
	unsigned char m_ucUnk03[0xD4 - 0x90];

	// 0xD4 - 0xD7
	signed int m_PosX;

	// 0xD8 - 0xDB
	signed int m_PosY;

	// 0xDC - 0xF3
	unsigned char m_ucUnk04[0xF4 - 0xDC];

	// 0xF4 - 0xF7
	unsigned long m_dwMirroredSpriteIndex;

	// 0xF8 - 0xFB
	unsigned long m_dwMirroredSpriteAnimationFrame;

protected:
private:

};

#pragma pack(pop)