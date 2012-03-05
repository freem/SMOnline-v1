#include <algorithm>
#include "global.h"
#include "SMOnlineStats.h"
#include "SMOnlineSQL.h"
//#include "SMOnlineIni.h"
#include "SQLfunctions.h"
#include "SimpleLog.h"
#include "Crash/crashDefines.h"
#include "MercuryINI.h"

GameStats::GameStats()
{
	m_playtime = time(NULL);
}

SMOnlineStats::SMOnlineStats()
{
	m_gameEdit = NULL;
	m_curID = 0;
}

SMOnlineStats::~SMOnlineStats()
{
	m_updateThread.Halt( true );

	m_gameEdit = NULL;

	//Don't log incomplete games.
	map<unsigned int, GameStats*>::iterator it = m_gamesPending.begin();
	while (it != m_gamesPending.end())
	{
		SAFE_DELETE( it->second );
		++it;
	}
	m_gamesPending.clear();

	//Try to send all the games that are complete.
	SendStats();
	while(!m_gamesSend.empty())
	{
		SAFE_DELETE(m_gamesSend.front());
		m_gamesSend.pop();
	}

	while(!m_gamesResend.empty())
	{
		SAFE_DELETE(m_gamesResend.front());
		m_gamesResend.pop();
	}

	MLockPtr< queue<GameStats*> > gd(m_gamesDone, m_gamesDoneMUTEX);
	while(!gd->empty())
	{
		SAFE_DELETE(gd->front());
		gd->pop();
	}
}

void SMOnlineStats::Connect()
{
	m_database.Connect(PREFSMAN->GetValueS("StatsSQLOptions", "Address", "", true),
						PREFSMAN->GetValueS("StatsSQLOptions", "Database", "", true),
						PREFSMAN->GetValueS("StatsSQLOptions", "User", "", true),
						PREFSMAN->GetValueS("StatsSQLOptions", "Passwd", "", true),
						PREFSMAN->GetValueI("StatsSQLOptions", "Port", 3306, true) );
}

void SMOnlineStats::Update()
{
	if (m_database.isConnected())
	{
		m_database.Ping();
		TransferDoneQueue();
		SendStats();
		ResendToSendQueue();
	}
	else
	{
		m_database.Reconnect();
	}
}

void* AutoUpdate( void * data )
{
	while ( true )
	{
		((SMOnlineStats *)data)->Update();
		usleep( 2000 );
	}
	return 0;
}

void SMOnlineStats::SetupAutoUpdate()
{
	m_updateThread.Create( AutoUpdate, this );
}

void SMOnlineStats::Begin(unsigned int &ID)
{
	m_gameEdit = new GameStats;
	m_gameEdit->ID = ++m_curID;
	ID = m_gameEdit->ID;
	m_gamesPending[ID] = m_gameEdit; //insert
}

bool SMOnlineStats::End()
{
	if (m_gameEdit == NULL)
	{
		LOG->Write("Stats: Ending NULL stats record. Select a record to edit first.");
		return false;
	}

	OrderPlayers(*m_gameEdit); //Sort by score

	map<unsigned int, GameStats*>::iterator i = m_gamesPending.find(m_gameEdit->ID);

	if (i != m_gamesPending.end())
		m_gamesPending.erase(i);

	MLockPtr< queue<GameStats*> >(m_gamesDone, m_gamesDoneMUTEX)->push(m_gameEdit);
	m_gameEdit = NULL;

	return true;
}

bool SMOnlineStats::Edit(unsigned int ID)
{
	m_gameEdit = Locate(ID);

	if (m_gameEdit == NULL)
	{
		LOG->Write("Stats: Editing nonexistant stats record ");
		return false;
	}

	return true;
}

GameStats* SMOnlineStats::Locate(unsigned int ID)
{
	map<unsigned int, GameStats*>::iterator i = m_gamesPending.find(ID);

	if (i != m_gamesPending.end())
		return i->second;

	return NULL;
}

bool SMOnlineStats::AddRecord(const PlayerStats* stats)
{
	if (m_gameEdit == NULL)
	{
		LOG->Write("Stats: Adding record to NULL stats record.");
		return false;
	}

	m_gameEdit->m_pStats.push_back(*stats);

	return true;
}

bool SMOnlineStats::SetSongInfo(const SongInfo &info)
{
	if (m_gameEdit == NULL)
	{
		LOG->Write("Stats: Setting round info on NULL record.");
		return false;
	}

	m_gameEdit->m_song.m_song = info;

	return true;
}

bool SMOnlineStats::SetRoom(const MString& name)
{
	if (m_gameEdit == NULL)
	{
		LOG->Write("Stats: Setting room name on NULL record.");
		return false;
	}

	m_gameEdit->m_roomname = name;

	return true;
}

void SMOnlineStats::SendStats()
{
	unsigned int ID;
	GameStats* stats;

	while (!m_gamesSend.empty())
	{
		stats = m_gamesSend.front();
		m_gamesSend.pop();

		if (stats->m_pStats.size() > 0)
		{
			if (!GetSongID(stats->m_song.m_song, ID))
				SubmitNewSong(stats->m_song.m_song);
				
			if (!GetSongID(stats->m_song.m_song, ID))
			{
				m_gamesResend.push(stats);
				continue;
			}

			stats->m_song.ID = ID;
			IncrementSongPlayCount(ID, 1);

			if (SubmitNewRound(*stats))
				SubmitPlayerStats(*stats);

			SubmitPlayerTotals(*stats);
		}

		SAFE_DELETE(stats);
	}
}

bool SMOnlineStats::GetSongID(const SongInfo& song, unsigned int& ID)
{
	ostringstream query;
	ezSQLRow row;

	query << "SELECT song_id, played, title, subtitle, artist FROM `song_stats` WHERE "
		<< "title = "<<sqlTextEnc(song.GetTitle().c_str()) << " and subtitle = " <<sqlTextEnc(song.GetSubtitle().c_str())
		<< " and artist = "	<< sqlTextEnc(song.GetArtist().c_str()) << ";";

	m_database.BQueryDB(query.str().c_str());

	if ( m_database.NumRows() == 0 )
	{
		LOG->Write( "ERROR in GetSongID, no rows returned. Orginal Query: " + query.str() );
		return false;
	}

	if ( !m_database.GetRow(0, row) )
	{
		LOG->Write( "Error.  Could not get row for getsongid!\n" );
		return false;
	}

	ID = row[0].ToInt();
	return true;
}

void CheckSQLError( ezSQLQuery &qin )
{
	if ( qin.m_ResultInfo.errorNum != 0 )
	{
		LOG->Write(" STATS SQL ERROR:" + qin.m_ResultInfo.errorDesc);
	}
}

bool SMOnlineStats::SubmitNewSong(const SongInfo& song)
{
	ezSQLRow row;
	ostringstream query;

	query << "insert into song_stats values("
		  << 0							<< ","
		  << 0								<< ","	//Play count
		  << sqlTextEnc(song.GetTitle().c_str())	<< ","	//Title
		  << sqlTextEnc(song.GetSubtitle().c_str())	<< ","	//Subtitle
		  << sqlTextEnc(song.GetArtist().c_str())	<< ")";	//Artist

	m_database.BQueryDB(query.str().c_str());
	query.str("");

	query << "select `song_ID` from `song_stats` where "
		  << "`title`="		<< sqlTextEnc(song.GetTitle().c_str())		<< " and "			//Title
		  << "`subtitle`="	<< sqlTextEnc(song.GetSubtitle().c_str())	<< " and "			//Subtitle
		  << "`artist`="	<< sqlTextEnc(song.GetArtist().c_str())		<< " limit 0,1";	//Artist

//	LOG->Write(query.str().c_str());
	m_database.BQueryDB(query.str().c_str());

	if (m_database.NumRows() <= 0)
	{
		LOG->Write("SQL error while reading new song submission.");
		return false;
	}

	return true;
}

bool SMOnlineStats::IncrementSongPlayCount(const unsigned int ID, const unsigned int count)
{
	ostringstream query;
	query << "UPDATE IGNORE song_stats SET "
		  << "played=played+" << count
		  << " WHERE song_ID=" << ID;

	m_database.BQueryDB(query.str().c_str());
	return true;
}

bool SMOnlineStats::SubmitNewRound(GameStats& game)
{
	ostringstream query;
	ezSQLRow row;

	query << "insert into rounds values("
		  << 0								<< ","
		  << game.m_song.ID					<< ","
		  << game.PlayTime()				<< ","
		  << sqlTextEnc(game.m_roomname.c_str())	<< ")";

	m_database.BQueryDB(query.str().c_str());

	query.str("");

	query << "select round_ID from rounds where `song_ID`="
		  << game.m_song.ID					<< " and playtime="
		  << game.PlayTime()				<< " and roomname="
		  << sqlTextEnc(game.m_roomname.c_str())	<< " limit 0,1";
	
//	LOG->Write(query.str());
	m_database.BQueryDB(query.str().c_str());

	if (m_database.NumRows() <= 0)
	{
		LOG->Write("SQL error while reading new round submission.");
		return false;
	}

	m_database.GetRow(0, row);
	game.m_roundID = row[0].ToInt();

	return true;
}

bool SMOnlineStats::SubmitPlayerStats(const GameStats& game)
{
	unsigned int length = game.m_pStats.size();
	ostringstream query;
	const PlayerStats* Player = NULL;

	for (unsigned int i = 0; i < length; i++)
	{
		Player = &(game.m_pStats[i]);

		query << "insert into player_stats values("
			  << 0									<< ","  //Round key (Auto-Increments, Ignore Value)
			  << game.m_roundID						<< ","	//Round ID
			  << Player->GetPlayerID()				<< ","	//Player ID
			  << Player->GetScore()					<< ","	//Score
			  << Player->GetProjgrade()				<< ","	//Grade
			  << Player->GetDiff()					<< ","	//Difficulty
			  << Player->GetFeet()					<< ","	//Feet
			  << Player->GetToasty()				<< ","	//Toasty
			  << Player->GetSteps(W1)				<< ","	//Marv
			  << Player->GetSteps(W2)				<< ","	//Perfect
			  << Player->GetSteps(W3)				<< ","	//Great
			  << Player->GetSteps(W4)				<< ","	//Good
			  << Player->GetSteps(W5)				<< ","	//Boo
			  << Player->GetSteps(MISS)				<< ","	//Miss
			  << Player->GetSteps(HELD)				<< ","	//OK
			  << Player->GetSteps(LETGO)			<< ","	//NG
  			  << Player->GetSteps(HITMINE)			<< ","	//Hit Mine
  			  << Player->GetSteps(AVOIDMINE)		<< ","	//Avoid Mine
			  << Player->GetMaxCombo()				<< ","	//Max Combo
			  << sqlTextEnc(Player->GetOptions().c_str())	<< ","  //Options
			  << Player->GetTiming()				<< ")";	//Timing

//		LOG->Write(query.str().c_str());
		if (!m_database.BQueryDB(query.str().c_str()))
			return false;

		query.str("");
	}

	Player = NULL;
	return true;
}


void SMOnlineStats::SubmitPlayerTotals(const GameStats& game)
{
	ostringstream query;
	unsigned int length = game.m_pStats.size();

	for (unsigned int i = 0; i < length; i++)
	{

		query.str("");

		if (!(game.m_pStats[i].GetTiming()))
		{

			//Make a new record, fail if it exists. Then update the record.
			query << "INSERT IGNORE INTO player_totals (pid) values("
				  << game.m_pStats[i].GetPlayerID()	<< ")";

//			LOG->Write(query.str());

			m_database.BQueryDB(query.str().c_str());

			query.str("");

			query << "UPDATE player_totals SET "
				  //Average in score
				  << "score=(score*(rounds/(rounds+1)))+("<< game.m_pStats[i].GetScore()<< "*(1/(rounds+1))), "
				  << "rounds=rounds+1"													<< ", "
				  << "grade=grade+"				<< game.m_pStats[i].GetProjgrade()		<< ", "
				  << "difficulty=difficulty+"	<< game.m_pStats[i].GetDiff()			<< ", "
				  << "feet=feet+"				<< game.m_pStats[i].GetFeet()			<< ", "
				  << "toasty=toasty+"			<< game.m_pStats[i].GetToasty()			<< ", "
				  << "marvelous=marvelous+"		<< game.m_pStats[i].GetSteps(W1)		<< ", "
				  << "perfect=perfect+"			<< game.m_pStats[i].GetSteps(W2)		<< ", "
				  << "great=great+"				<< game.m_pStats[i].GetSteps(W3)		<< ", "
				  << "good=good+"				<< game.m_pStats[i].GetSteps(W4)		<< ", "
				  << "boo=boo+"					<< game.m_pStats[i].GetSteps(W5)		<< ", "
				  << "miss=miss+"				<< game.m_pStats[i].GetSteps(MISS)		<< ", "
				  << "ok=ok+"					<< game.m_pStats[i].GetSteps(HELD)		<< ", "
				  << "ng=ng+"					<< game.m_pStats[i].GetSteps(LETGO)		<< ", "
				  << "hmine=hmine+"				<< game.m_pStats[i].GetSteps(HITMINE)	<< ", "
				  << "amine=amine+"				<< game.m_pStats[i].GetSteps(AVOIDMINE)	<< ", "
				  << "maxcombo=maxcombo+"		<< game.m_pStats[i].GetMaxCombo()		<< ", "
				  //Nasty seed equation
				  << "seed=((LOG(rounds)+(((marvelous*16)+(perfect*12)+(great*6)+(ok*16))/(marvelous+perfect+great+good+boo+miss+ok+ng)))*1000)"
				  << "WHERE pid=" << game.m_pStats[i].GetPlayerID();

//			LOG->Write(query.str());
			m_database.BQueryDB(query.str().c_str());
			query.str("");
		}
	}
}

void SMOnlineStats::OrderPlayers(GameStats& game)
{
	//Quick sort and reverse order from highest to lowest
	sort(game.m_pStats.begin(), game.m_pStats.end());
	reverse(game.m_pStats.begin(), game.m_pStats.end());
}

void SMOnlineStats::TransferDoneQueue()
{
	MLockPtr< queue<GameStats*> > gd(m_gamesDone, m_gamesDoneMUTEX);

	while (!gd->empty())
	{
		m_gamesSend.push(gd->front());
		gd->pop();
	}
}

void SMOnlineStats::ResendToSendQueue()
{
	while (!m_gamesResend.empty())
	{
		m_gamesSend.push(m_gamesResend.front());
		m_gamesResend.pop();
	}
}

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
