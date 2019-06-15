#pragma once

#pragma pack(push, 1)

class CDIO_FileSystem
{
public:
	
	// 0x00 - 0x03
	unsigned char * m_pIDLFile;

	// 0x04 - 0x07
	unsigned int m_nNumOpenFiles;
protected:
private:
};
#pragma pack(pop)