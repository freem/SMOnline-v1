#ifndef SMONLINECLIENT_H
#define SMONLINECLIENT_H

#include <vector>
#include <time.h>
#include <map>
#include "ezsockets.h"
#include "SMLanPlayers.h"
#include "PacketFunctions.h"
#include "RoundInfo.h"
#include "ConstructCount.h"

enum NSCommand
{
	NSCPing = 0,
	NSCPingR,		//1
	NSCHello,		//2
	NSCGSR,			//3
	NSCGON,			//4
	NSCGSU,			//5
	NSCSU,			//6
	NSCCM,			//7 Chat
	NSCRSG,			//8
	NSCUUL,			//9
	NSCSMS,			//10
	NSCUPOpts,		//11 Plyer options
	NSSMONL,		//12
	NUM_NS_COMMANDS
};

const NSCommand NSServerOffset = (NSCommand)128;

class SMOnlineClient {
public:
	EzSockets connection;
	SMOnlineClient();
	~SMOnlineClient();
	int Update(PacketFunctions& Packet);
	int GetData(PacketFunctions& Packet);
	void SendData(const PacketFunctions& Packet);
	bool CheckConnection();
	const MString GetPlayerName(unsigned int playernum);
	void SetInRoom(bool x);
	bool InRoom() const { return m_inroom; }
	bool DropMe();
	bool SetPlayerName(unsigned int PlayerNumber, MString& name);
	bool SetPlayerID(unsigned int PlayerNumber, unsigned int PlayerID);
	unsigned int GetPlayerID(unsigned int PlayerNumber);
//	bool NeedID();
	LanPlayer* GetPlayer(unsigned int PlayerNumber);
	void SendValue(uint8_t value);
	bool GotStartreq() const;
	void ResetStartreq();
	inline bool HasSong() const { return m_hasSong; }
	inline bool GetGameOver() const { return m_gameOver; }
	inline int	GetVersion() const { return m_version; }
	bool IsError();
	inline unsigned int GetNumPlayers() { return m_mPlayers.size(); }
	inline bool IsAdmin() { return m_isAdmin; }
	inline MString GetColor() { return m_color; }
	void AssignColor();
	void SubmitGameOverStats(unsigned int ID);
	void LoadFromIni();

private:
	PacketFunctions m_packet;
	PacketFunctions m_reply;
	int m_version;
	MString m_build;
	map<unsigned int, LanPlayer*> m_mPlayers; //why did I use a map, a client probably won't have more than 2 players
//	vector<LanPlayer*> m_players;
//	map<unsigned int, unsigned int> m_map;
	typedef map<unsigned int, unsigned int>::value_type add;
	bool m_inroom;
//	bool m_needID;
	bool m_startreq;
	int m_startPos;
	RoundInfo* m_roundinfo;
	bool m_hasSong;
	bool m_gameOver;
	bool m_isAdmin;
	ConstructCount m_counter;
	MString	m_color;
	int m_salt;

	void ParseData(PacketFunctions& Packet);
	void Hello(PacketFunctions& Packet);
	void StyleUpdate(PacketFunctions& Packet);
	bool CheckZeroedIDs();
	void RemoveAllPlayers();
	void ParseFeet(PacketFunctions& Packet);
	void ParseDiff(PacketFunctions& Packet);
	void ParseRoundInfo(PacketFunctions& Packet);
	void SetStartPosition(PacketFunctions& Packet);
	void StatsUpdate(PacketFunctions& Packet);
	void ClearPlayersStats();
	void ParsePlayerOptions(PacketFunctions& Packet);
	void ParseHashes(PacketFunctions& Packet);
	void CheckTimings();
	inline void ResetHasSong() { m_hasSong = 0; }
	void ParseSongSelect(PacketFunctions& Packet);
	void GameOver();
	void ParseScreen(PacketFunctions& Packet);
	void ParseSMOnline(PacketFunctions& Packet);
	void VerifyUser(PacketFunctions& Packet);
	void ChangeScreenPosition(int x);
	void RemovePlayer(MString& name);
	void LoginSuccess();

	//SQL query functions
	unsigned int getPlayerID(MString user, MString password, int logintype, int m_salt);
	bool adminCheck(unsigned int uID);
	bool IsBanned(const int uID);

};

class ClientError {
public:
	ClientError(const MString& error);
	MString m_error;
};

#endif

/*
 * (c) 2003-2004 Joshua Allen
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
