#ifndef PACKETFUNCTIONS_H
#define PACKETFUNCTIONS_H

#include "global.h"

#if defined(__APPLE__)
#include "arch_setup_darwin.h"
#endif

#include "MercuryString.h"
#include "ezsockets.h"

const int NETMAXBUFFERSIZE=1020; //1024 - 4 bytes for EzSockets

class PacketFunctions
{
public:
	unsigned char Data[NETMAXBUFFERSIZE];	//Data
	int Position;				//Other info (Used for following functions)
	uint32_t Size; // HORRIBLE HACK! Only valid for packets read from the network, not packets you're writing.

	//Commands used to operate on NetPackets
	uint8_t Read1();
	uint16_t Read2();
	uint32_t Read4();
	MString ReadNT();

	void Write1(uint8_t Data);
	void Write2(uint16_t Data);
	void Write4(uint32_t Data);
	void WriteNT(const MString& Data);

	void ClearPacket();
};

#endif

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
