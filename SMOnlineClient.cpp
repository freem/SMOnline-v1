#include <iostream>
#include <iomanip>
#include "global.h"
#include "SMOnlineClient.h"
#include "JudgeWindow.h"
//#include "SMOnlineIni.h"
#include "SQLfunctions.h"
#include "SMOnlineRoom.h"
#include "SimpleLog.h"
#include "SMOnlineServer.h"
#include "SMOnlineStats.h"
#include "MercuryINI.h"

using namespace std;

static MString m_userIDField;
static MString m_userTable;
static MString m_userNameField;
static MString m_passwdField;
static MString m_userLevelField;
static MString m_banField;
static MString m_banTable;
static int m_adminLevel;

SMOnlineClient::SMOnlineClient() {
	connection.blocking = 0;
	m_inroom = false;
	m_roundinfo = NULL;
	m_hasSong = false;
	m_gameOver = false;
	m_startreq = false;
	m_isAdmin = false;
	m_salt = rand();

	LoadFromIni();
}

void SMOnlineClient::LoadFromIni()
{
	m_userIDField = PREFSMAN->GetValueS("SQLOptions", "UserIDField", "", true);
	m_userTable = PREFSMAN->GetValueS("SQLOptions", "UserTable", "", true);
	m_userNameField = PREFSMAN->GetValueS("SQLOptions", "UsernameField", "", true);
	m_passwdField = PREFSMAN->GetValueS("SQLOptions", "PasswdField", "", true);
	m_userLevelField = PREFSMAN->GetValueS("SQLOptions", "LevelField", "", true);
	m_banField = PREFSMAN->GetValueS("SQLOptions", "BanUserIDField", "", true);
	m_banTable = PREFSMAN->GetValueS("SQLOptions", "BanTable", "", true);
	m_adminLevel = PREFSMAN->GetValueI("SQLOptions", "AdminLevel", 1, true);
}

SMOnlineClient::~SMOnlineClient() {
	RemoveAllPlayers();

	if (m_roundinfo != NULL) {
		SAFE_DELETE(m_roundinfo);
	}
}

int SMOnlineClient::Update(PacketFunctions &Packet) {
	int length = 0;
	if (CheckConnection()) {
		//Make sure no player's ID is 0
//		m_needID = CheckZeroedIDs();
		length = GetData(m_packet);
		if (length >= 0) {
			Packet = m_packet;
			ParseData(m_packet);
		}
	}
	else
		throw ClientError("Disconnected Client");

	return length;
}

int SMOnlineClient::GetData(PacketFunctions& Packet)
{
	int length = 0;
	Packet.ClearPacket();
	length = connection.ReadPack((char*)Packet.Data, NETMAXBUFFERSIZE);
	Packet.Size = length;
	return length;
}

bool SMOnlineClient::CheckConnection() {
	if (connection.IsError())
		return false;

	return true;
}

void SMOnlineClient::SendData(const PacketFunctions& Packet)
{
	if (!connection.IsError())
		connection.SendPack((char*)Packet.Data, Packet.Position);
}

void SMOnlineClient::ParseData(PacketFunctions &Packet) {
	int command = Packet.Read1();
	switch (command)
	{
	case NSCPing:
		PacketFunctions Reply;
		Reply.ClearPacket();
		Reply.Write1( NSServerOffset + NSCPingR );
		SendData(Reply);
		break;
	case NSCHello:
		// Hello
		Hello(Packet);
		break;
	case NSCSU:
		// Style Update
		//Only lan play uses the style packet
		if ( SERVER->IsSMLAN() )
		{
			StyleUpdate(Packet);
			LOG->Write("Style Update");
		}
		break;
	case NSCGSR:
		//Start Game request
		m_hasSong = false;
		m_gameOver = false;
		m_startreq = true;
		ClearPlayersStats();
		ParseFeet(Packet);
		ParseDiff(Packet);
		SetStartPosition(Packet);
		ParseRoundInfo(Packet);
		ParsePlayerOptions(Packet);
		ParseHashes(Packet);
		break;
	case NSCGON:
		// GameOver
		GameOver();
		break;
	case NSCGSU:
		//Stat Update
		StatsUpdate(Packet);
		CheckTimings();
		break;
	case NSCRSG:
		//Song Selection
		ParseSongSelect(Packet);
		break;
	case NSCUPOpts:
		ParsePlayerOptions(Packet);		
		break;
	case NSCSMS:
		ParseScreen(Packet);
		break;
	case NSSMONL:
		ParseSMOnline(Packet);
		break;
	default:
		break;
	}
}

void SMOnlineClient::ParseScreen(PacketFunctions& Packet) {
	switch (Packet.Read1()) {
	case 1:
		ChangeScreenPosition(2); //in song selection
		break;
	case 3:
		ChangeScreenPosition(3); //in options
		break;
	case 4:
		ChangeScreenPosition(4); //in evaluation
		break;
	case 7:
		ChangeScreenPosition(1); //in room
		break;
	}
}

void SMOnlineClient::ChangeScreenPosition(int x) {
	map<unsigned int, LanPlayer*>::iterator iter;
	for (iter = m_mPlayers.begin(); iter != m_mPlayers.end(); ++iter)
		(*iter).second->SetScreenPosition(x);
}

void SMOnlineClient::ParseSongSelect(PacketFunctions& Packet) {
	int type = Packet.Read1();
	switch (type) {
	case 0:
		m_hasSong = true;
		break;
	case 1:
		m_hasSong = false;
		break;
	default:
		break;
	}
}

void SMOnlineClient::ParseFeet(PacketFunctions& Packet) {
	int byte = Packet.Read1();

	int p0 = byte/16;
	int p1 = byte%16;

	if (p0 > 0)
		if (m_mPlayers.find(0) != m_mPlayers.end())
			m_mPlayers[0]->SetFeet(p0);

	if (p1 > 0)
		if (m_mPlayers.find(1) != m_mPlayers.end())
			m_mPlayers[1]->SetFeet(p1);

}

void SMOnlineClient::ParseDiff(PacketFunctions& Packet) {
	uint8_t byte = Packet.Read1();

	uint8_t p0 = byte/16;
	uint8_t p1 = byte%16;

	if (p0 > 0)
		if (m_mPlayers.find(0) != m_mPlayers.end())
			m_mPlayers[0]->SetDiff(p0);

	if (p1 > 0)
		if (m_mPlayers.find(1) != m_mPlayers.end())
			m_mPlayers[1]->SetDiff(p1);

}

void SMOnlineClient::ParseSMOnline(PacketFunctions& Packet) {
	int command = Packet.Read1();
	switch (command) {
	case 0:
		//Login
		VerifyUser(Packet);
		break;
	case 1:
		//Change Room
		//When entering room, reset critical game related stuff
		m_hasSong = false;
		m_gameOver = false;
		m_startreq = false;
		break;
	case 2:
		{
			int typeI = Packet.Read1();

			switch ( typeI )
			{
			case 0:
				{
				//Server Info
				PacketFunctions p;
				p.ClearPacket();

				stringstream s;
				s<<"cusers:%d"<< SERVER->GetNumConnected()<<endl;
				p.WriteNT(s.str().c_str());
				for ( int i = 0; i < SERVER->GetNumConnected(); i++ )
				{
					s.str("");
					s<<"user"<<i<<":"<<SERVER->GetPlayerClient(i)->GetPlayerName(0).c_str()<<":"<<SERVER->GetPlayerClient(i)->GetPlayerName(1).c_str()<<endl;
					p.WriteNT( s.str().c_str() );
				}
				this->SendData( p );
				}
			}
		}
	default:
		break;
	}
}

void SMOnlineClient::Hello(PacketFunctions &Packet)
{
	unsigned char server_version;

	if ( SERVER->IsSMLAN() )
		server_version = 1; //smlan
	else
		server_version = 129; //smonline 128->129 better song identification using chart hashes

	m_version = Packet.Read1();
	m_build = Packet.ReadNT();
	m_reply.ClearPacket();
	m_reply.Write1( NSCHello + NSServerOffset );

	m_reply.Write1(server_version);
	m_reply.WriteNT(SERVER->GetName());	
	m_reply.Write4( m_salt );

	SendData(m_reply);
}

const MString SMOnlineClient::GetPlayerName(unsigned int index) {
	map<unsigned int, LanPlayer*>::iterator iter = m_mPlayers.begin();
	unsigned int i = 0;

	if (m_mPlayers.size() > index) {
		while (i < index) {
			i++;
			iter++;
		}
		return (*iter).second->GetName(); 
	}
	return "";
}

void SMOnlineClient::SetInRoom(bool x) {
	m_inroom = x;
}

bool SMOnlineClient::DropMe() {
	return !m_inroom;
}

bool SMOnlineClient::SetPlayerName(unsigned int index, MString& name) {
	map<unsigned int, LanPlayer*>::iterator iter = m_mPlayers.begin();
	unsigned int i = 0;

	if (m_mPlayers.size() > index) {
		while (i < index) {
			i++;
			iter++;
		}
		(*iter).second->SetName(name);
		return true;
	}
	return false;
}

bool SMOnlineClient::SetPlayerID(unsigned int index, unsigned int PlayerID) {
	map<unsigned int, LanPlayer*>::iterator iter = m_mPlayers.begin();
	unsigned int i = 0;

	if (m_mPlayers.size() > index) {
		while (i < index) {
			i++;
			iter++;
		}
		(*iter).second->SetPlayerID(PlayerID);
		return true;
	}

	return false;
}

unsigned int SMOnlineClient::GetPlayerID(unsigned int index) {
	map<unsigned int, LanPlayer*>::iterator iter = m_mPlayers.begin();
	unsigned int i = 0;

	if (m_mPlayers.size() > index) {
		while (i < index) {
			i++;
			iter++;
		}
		return (*iter).second->GetPlayerID();
	}

	return 0;
}

bool SMOnlineClient::CheckZeroedIDs() {
	map<unsigned int, LanPlayer*>::iterator iter;

	for (iter = m_mPlayers.begin(); iter != m_mPlayers.end(); iter++) {
		if ((*iter).second->GetPlayerID() == 0)
			return true;
	}
	return false;
}

LanPlayer* SMOnlineClient::GetPlayer(unsigned int index) {
	map<unsigned int, LanPlayer*>::iterator iter = m_mPlayers.begin();
	unsigned int i = 0;

	if (m_mPlayers.size() > index) {
		while (i < index) {
			i++;
			iter++;
		}
		return (*iter).second;
	}

	return NULL;
}

void SMOnlineClient::RemoveAllPlayers() {
	map<unsigned int, LanPlayer*>::iterator iter;

	for (iter = m_mPlayers.begin(); iter != m_mPlayers.end(); iter++) {
		SAFE_DELETE((*iter).second);
	}

	m_mPlayers.clear();
}

void SMOnlineClient::RemovePlayer(MString& name) {
	map<unsigned int, LanPlayer*>::iterator iter;
	for (iter = m_mPlayers.begin(); iter != m_mPlayers.end(); iter++)
		if ((*iter).second->GetName() == name) {
			SAFE_DELETE((*iter).second);
			m_mPlayers.erase(iter);
			return;
		}
}

void SMOnlineClient::SendValue(uint8_t value) {
	if (!connection.IsError())
		connection.SendData((char*)&value, sizeof(uint8_t));
}

bool SMOnlineClient::GotStartreq() const {
	return m_startreq;
}

void SMOnlineClient::ResetStartreq() {
	m_startreq = false;
}

void SMOnlineClient::ParseRoundInfo(PacketFunctions& Packet) {
	if (m_roundinfo == NULL)
		m_roundinfo = new RoundInfo;

	m_roundinfo->SetTitle(Packet.ReadNT());
	m_roundinfo->SetSubtitle(Packet.ReadNT());
	m_roundinfo->SetArtist(Packet.ReadNT());
	m_roundinfo->SetCourse(Packet.ReadNT());
	m_roundinfo->SetSongoptions(Packet.ReadNT());
}

void SMOnlineClient::GameOver() {
	if (m_roundinfo != NULL) {
		SAFE_DELETE(m_roundinfo);
	}
	m_gameOver = true;
	m_hasSong = false;
}

void SMOnlineClient::SetStartPosition(PacketFunctions& Packet) {
	int byte = Packet.Read1();
	m_startPos = byte/16;
}

void SMOnlineClient::StatsUpdate(PacketFunctions& Packet) {
	int byte = Packet.Read1();
	unsigned int playerNum = byte/16;
	int stepID = byte%16;
	if (m_mPlayers.find(playerNum) != m_mPlayers.end()) {
		m_mPlayers[playerNum]->SetCurrstep((StepType)stepID);
		m_mPlayers[playerNum]->IncrementSteps();
		m_mPlayers[playerNum]->SetProjgrade(Packet.Read1()/16);
		m_mPlayers[playerNum]->SetScore(Packet.Read4());
		m_mPlayers[playerNum]->SetCombo(Packet.Read2());
		m_mPlayers[playerNum]->SetHealth(Packet.Read2());
		m_mPlayers[playerNum]->SetOffset(abs(long(Packet.Read2())-32767)/(double)2000);
	}
}

void SMOnlineClient::ClearPlayersStats() {
	map<unsigned int, LanPlayer*>::iterator iter;

	for (iter = m_mPlayers.begin(); iter != m_mPlayers.end(); iter++)
		(*iter).second->ClearStats();
}

void SMOnlineClient::ParsePlayerOptions(PacketFunctions& Packet) {
	map<unsigned int, LanPlayer*>::iterator iter;

	MString options[2];
	options[0] = Packet.ReadNT();
	options[1] = Packet.ReadNT();

	for (unsigned int i = 0; i < 2; i++) {
		iter = m_mPlayers.find(i);
		if (iter != m_mPlayers.end())
			(*iter).second->SetOptions(options[i]);
	}
	
}

void SMOnlineClient::ParseHashes(PacketFunctions& Packet) {
	m_roundinfo->GetHashes().clear();
	uint32_t numberOfHashes;
	if(((unsigned int)Packet.Position) + 3 < Packet.Size) // Can't use client protocol version, hashes don't get sent if leader is using an old client
	{
		numberOfHashes = Packet.Read4();
	}
	for(uint32_t i = 0; i < numberOfHashes && ((unsigned int)Packet.Position + 3) < Packet.Size; i++)
	{
		m_roundinfo->GetHashes().push_back(Packet.Read4());
	}
}

void SMOnlineClient::CheckTimings() {
	map<unsigned int, LanPlayer*>::iterator iter;
	const JudgeWindow& jw = SERVER->GetJudgeWindows();

	for (iter = m_mPlayers.begin(); iter != m_mPlayers.end(); iter++) {
		if (!(*iter).second->GetTiming()) {
			double offset = (*iter).second->GetOffset();
			switch ((*iter).second->GetCurrstep()) {
			case W5:
				if (jw.GetBoo() < offset)
					(*iter).second->SetTiming(true);
				break;
			case W4:
				if (jw.GetGood() < offset)
					(*iter).second->SetTiming(true);
				break;
			case W3:
				if (jw.GetGreat() < offset)
					(*iter).second->SetTiming(true);
				break;
			case W2:
				if (jw.GetPerfect() < offset)
					(*iter).second->SetTiming(true);
				break;
			case W1:
				if (jw.GetMarv() < offset)
					(*iter).second->SetTiming(true);
				break;
			default:
				break;
			};
		}
	}
}

ClientError::ClientError(const MString& error) {
	m_error = error;
}

bool SMOnlineClient::IsError() {
	return connection.IsError();
}

void SMOnlineClient::VerifyUser(PacketFunctions& Packet) {
	LanPlayer *tmpPlayer;
	int PID = -1;
	int pnum = Packet.Read1();
	int enctype = Packet.Read1();
	MString user = Packet.ReadNT();
	MString passwd = Packet.ReadNT();
	LOG->Write("Log user in: " + user);
	m_reply.ClearPacket();
	m_reply.Write1(NSSMONL + NSServerOffset);
	m_reply.Write1(0); //login reply

	if (SERVER->UseSQL())
	{
		try
		{
			PID = getPlayerID(user, passwd, enctype, m_salt);
			if (IsBanned(PID)) {
				LOG->Write("Banned user logging in");
				m_reply.Write1(1); //Failed
				m_reply.WriteNT("User Banned.");
				SendData(m_reply);
				return;
			}
		}
		catch (const MString& e) {
			//Bad username or passwd, tell client
			m_reply.Write1(1); //Failed
			m_reply.WriteNT(e);
			SendData(m_reply);
			LOG->Write(e);
			return;
		}
	}
	RemovePlayer(user);	//Should remove duplicate players

	if (m_mPlayers.find(pnum) == m_mPlayers.end()) {
		SMOnlineClient* tmpClient = SERVER->GetPlayerClient(user);
		if ((tmpClient != NULL) && (tmpClient != this)) {
			/* Hack to disconnect multiple same players.
			Fix this when making a manager.
			I would like the other client to be disconnected but I can't
			do this right now because of how it all works. */
			connection.close();
			tmpClient = NULL;
		} else {

			tmpPlayer = new LanPlayer(user, PID);
			m_mPlayers[pnum] = tmpPlayer;
			tmpPlayer->SetVersion(m_version);
		}
	}
	if (SERVER->UseSQL())
		m_isAdmin = adminCheck(PID);
//	LOG->Write("Admin:");
//	LOG->Write(m_isAdmin);
	SERVER->AddPlayerToMap(user, this);

	m_reply.Write1(0); //success
	m_reply.WriteNT("Login Success");
	SendData(m_reply);

	LoginSuccess();
}

void SMOnlineClient::AssignColor()
{
	stringstream s;
	s << ios::fixed << setw(2) << hex;
	s.str("");
	s << rand()%255 << rand()%255 << rand()%255;
	m_color = s.str().c_str();
	while ( m_color.length() < 6 )
		m_color += '0';
}

void SMOnlineClient::SubmitGameOverStats(unsigned int ID)
{
	if (!STATS)
		return;

	STATS->Edit(ID);
	for (map<unsigned int, LanPlayer*>::iterator i = m_mPlayers.begin(); i != m_mPlayers.end(); i++)
	{
		if ((*i).second->GetPlayerID() > 0)
			STATS->AddRecord((*i).second);
		else
			LOG->Write("Invalid Player ID. Player probably did not log in correctly. Ignoring player stats.");
	}
}

unsigned int SMOnlineClient::getPlayerID(MString user, MString password, int logintype, int salt )
{ //Returns PlayerID or throw exception on fail
	ezSQLRow row;

	MString query;

	switch ( logintype )
	{
	case 0:
		query = "select " + m_userIDField + ", user_active from " + m_userTable
			  + " where " + m_userNameField + "='" + user
			  + "' and "  + m_passwdField + "='" + password + "' limit 0,1";
		break;
	case 1:
	{
		stringstream c;
		c.str("");
		c << salt;
		query = "select " + m_userIDField + ", user_active from " + m_userTable
			  + " where " + m_userNameField + "='" + user
		  + "' and cast( MD5( concat( upper(" + m_passwdField + "), '" + MString(c.str().c_str()) + "' )) as char)='" + password + "' limit 0,1";

//		cout<<"QUERY:"<<query<<endl;
		break;
	}
	default:
		return ( 0 );
	}

	SERVER->GetDatabase()->BQueryDB(query);

	if (SERVER->GetDatabase()->NumRows() < 1)
		throw MString("Invalid login.");

	SERVER->GetDatabase()->GetRow(0, row);

	if (row[1].ToInt() == 0)
		throw MString("Inactive account.");

	return row[0].ToInt();
}

bool SMOnlineClient::adminCheck(unsigned int uID) {
	ostringstream query;
	query << "select " << m_userIDField << " from " << m_userTable
			<< " where " << m_userIDField << "='" << uID
			<< "' and " << m_userLevelField << "='" << m_adminLevel << "' limit 0,1";

	SERVER->GetDatabase()->BQueryDB(query.str().c_str());

	if(SERVER->GetDatabase()->NumRows()<1)
		return false;

	return true;
}

bool SMOnlineClient::IsBanned(const int uID) {
	ostringstream query;
	query << "select " << m_banField << " from " << m_banTable
			<< " where " << m_banField << "=" << uID << " limit 0,1";

	SERVER->GetDatabase()->BQueryDB(query.str().c_str());

	if(SERVER->GetDatabase()->NumRows()<1)
		return false;

	return true;
}

void SMOnlineClient::StyleUpdate(PacketFunctions& packet)
{
	LanPlayer *tmpPlayer = NULL;
	PacketFunctions p = packet;
	unsigned int numPlayers = p.Read1();

	for (unsigned int i = 0; i < numPlayers; ++i)
	{
		int playerNum = p.Read1();
		MString user = p.ReadNT();
		RemovePlayer(user);	//Should remove duplicate players

		if (m_mPlayers.find(playerNum) == m_mPlayers.end())
		{
			SMOnlineClient* tmpClient = SERVER->GetPlayerClient(user);
			if ((tmpClient != NULL) && (tmpClient != this))
			{
				/* Hack to disconnect multiple same players.
				Fix this when making a manager.
				I would like the other client to be disconnected but I can't
				do this right now because of how it all works. */
				connection.close();
				tmpClient = NULL;
			}
			else
			{
				//Player ID does not matter for LAN
				tmpPlayer = new LanPlayer(user, 0);
				m_mPlayers[playerNum] = tmpPlayer;
				tmpPlayer->SetVersion(m_version);
			}
		}
		SERVER->AddPlayerToMap(user, this);
	}

	LoginSuccess();
}


void SMOnlineClient::LoginSuccess()
{
	m_reply.ClearPacket();
	m_reply.Write1(NSCCM + NSServerOffset); //Chat message
	m_reply.WriteNT(SERVER->GetName() + ": " + SERVER->GetMOTD());
	SendData(m_reply);

	if ( !InRoom() )
		SERVER->GetRoom(0)->AddClient(this, "", false); //Add myself to the lobby
}

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



