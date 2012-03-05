#ifndef SMONLINEGAMEROOM_H
#define SMONLINEGAMEROOM_H

#include <vector>
#include <iostream>
#include "MercuryString.h"
#include "SMOnlineRoom.h"
#include "SMLanPlayers.h"
#include "ConstructCount.h"
//#include "SMOnlineIni.h"

class IndexedPlayer
{
public:
	IndexedPlayer()
		:player(NULL)
	{};
	inline bool operator<(const IndexedPlayer& x) const	{ return *player < *(x.player);	}

	unsigned int index;
	LanPlayer* player;
};

class SMOnlineGameRoom : public SMOnlineRoom {
public:
	SMOnlineGameRoom(const MString &name, const MString &description, bool allowgamerooms = 1);
	virtual ~SMOnlineGameRoom();
	virtual void Update();
	virtual bool AddClient(SMOnlineClient *client, const MString& pw, bool ignorePass = false);
	inline const RoundInfo& LastRound() const { return m_lastRound; }
	void LoadFromIni();
private:
	RoundInfo m_lastRound;
	RoundInfo m_roundinfo;
	time_t m_statsTime;
	time_t m_selectTime;
	vector<int> m_gameoverclientIDs;
	int m_chooser;
	vector<IndexedPlayer> m_OrderedPlayersList;
	ConstructCount m_counter;
	MString m_password;
	unsigned int m_statsID;
	unsigned int m_selectDelay;

	virtual void ParseData(PacketFunctions& Packet, int clientnum);
	void StartRound(unsigned int clientnum);
	void SelectSong(PacketFunctions& Packet);
	void StartSong();
	void ParseRequestSongStart(PacketFunctions& Packet, unsigned int clientnum);
	void ClientsSelectSong(const RoundInfo& roundinfo);
	void SendStats();
	void SortStats();
	void StatsNameColumn(PacketFunctions &data, vector<LanPlayer*>& playersPtr);
	void StatsComboColumn(PacketFunctions& data, vector<LanPlayer*>& playersPtr);
	void StatsProjgradeColumn(PacketFunctions& data, vector<LanPlayer*>& playersPtr);
	void ParseEnterExit(PacketFunctions& data, unsigned int clientnum);
	void WriteGameOverStats();
	void LackMessage();
	void LackMessage(int cNum);
	bool AllPresent();
	bool SelectDelay();
	void QuickSelect(SMOnlineClient *client);
	virtual void RemoveClient(const unsigned int index);
	MString GenerateTimingString(const LanPlayer* Player);
	bool StartReqCheck();
	void SyncStart();
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
