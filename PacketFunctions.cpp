#include "PacketFunctions.h"

uint8_t PacketFunctions::Read1()
{
	if (Position>=NETMAXBUFFERSIZE)
		return 0;
	
	return Data[Position++];
}

uint16_t PacketFunctions::Read2()
{
	if (Position>=NETMAXBUFFERSIZE-1)
		return 0;

	uint16_t Temp;
	memcpy( &Temp, Data + Position,2 );
	Position+=2;		
	return ntohs(Temp);	
}

uint32_t PacketFunctions::Read4()
{
	if (Position>=NETMAXBUFFERSIZE-3)
		return 0;

	uint32_t Temp;
	memcpy( &Temp, Data + Position,4 );
	Position+=4;
	return ntohl(Temp);
}

MString PacketFunctions::ReadNT()
{
	//int Orig=Packet.Position;
	MString TempStr;
	while ((Position<NETMAXBUFFERSIZE)&& (((char*)Data)[Position]!=0))
		TempStr= TempStr + (char)Data[Position++];

	++Position;
	return TempStr;
}


void PacketFunctions::Write1(uint8_t data)
{
	if (Position>=NETMAXBUFFERSIZE)
		return;
	memcpy( &Data[Position], &data, 1 );
	++Position;
}

void PacketFunctions::Write2(uint16_t data)
{
	if (Position>=NETMAXBUFFERSIZE-1)
		return;
	data = htons(data);
	memcpy( &Data[Position], &data, 2 );
	Position+=2;
}

void PacketFunctions::Write4(uint32_t data)
{
	if (Position>=NETMAXBUFFERSIZE-3)
		return ;

	data = htonl(data);
	memcpy( &Data[Position], &data, 4 );
	Position+=4;
}

void PacketFunctions::WriteNT(const MString& data)
{
	unsigned int index=0;
	while ((Position<NETMAXBUFFERSIZE)&&(index<data.length()))
		Data[Position++] = (unsigned char)(data.c_str()[index++]);
	Data[Position++] = 0;
}

void PacketFunctions::ClearPacket()
{
	memset((void*)(&Data),0, NETMAXBUFFERSIZE);
	Position = 0;
	Size = 0;
}

/*
 * (c) 2003-2004 Charles Lohr, Joshua Allen
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

