#ifndef SMONLINESTATS_H
#define SMONLINESTATS_H

#include <queue>
#include <vector>
#include <map>
#include "SMLanPlayers.h"
#include "RoundInfo.h"
#include "SMOnlineSQL.h"
#include "MercuryThreads.h"

using namespace std;

struct SongCache
{
	unsigned int ID; //Song ID
	SongInfo m_song;
};

class GameStats
{
public:
	GameStats();
	inline time_t PlayTime() { return m_playtime; }
	unsigned long ID;  //Internal stats man ID!
	unsigned int m_roundID;
	SongCache m_song;
	vector<PlayerStats> m_pStats;
	MString m_roomname;
private:
	time_t m_playtime;
};

class SMOnlineStats {
public:
	SMOnlineStats();
	~SMOnlineStats();
	void Connect();
	void Update();
	void SetupAutoUpdate();

	void Begin(unsigned int &ID);
	bool Edit(unsigned int ID);

	/*	The following functions operate on the data pointed to by m_gameEdit.
		The current game record to edit can be modified by the Edit and Begin functions.
	*/
	bool AddRecord(const PlayerStats* stats);
	bool SetSongInfo(const SongInfo &info);
	bool SetRoom(const MString& name);
	bool End(); //Marks a game record as complete and ready to submit to SQL.
private:
	void SendStats();
	bool SubmitNewSong(const SongInfo& song);
	bool SubmitNewRound(GameStats& game);
	bool SubmitPlayerStats(const GameStats& game);
	void SubmitPlayerTotals(const GameStats& game);
	bool IncrementSongPlayCount(const unsigned int ID, const unsigned int count);
	void OrderPlayers(GameStats& game);
	void TransferDoneQueue();
	void ResendToSendQueue();
	bool GetSongID(const SongInfo& song, unsigned int& ID);

	GameStats* Locate(unsigned int ID);
	unsigned long m_curID;
	GameStats* m_gameEdit;
	SMOnlineSQL m_database;
	map<unsigned int, GameStats*> m_gamesPending;

	volatile queue<GameStats*> m_gamesDone;
	queue<GameStats*> m_gamesSend;
	queue<GameStats*> m_gamesResend;

	MercuryMutex	m_gamesDoneMUTEX;
	MercuryThread m_updateThread;
};

void * AutoUpdate( void * data);

extern SMOnlineStats* STATS;

#endif

/*
 * (c) 2004-2005 Joshua Allen
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
