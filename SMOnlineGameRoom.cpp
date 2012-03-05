#include "SMOnlineGameRoom.h"
#include <iostream>
#include <algorithm>
#include "SQLfunctions.h"
#include "SimpleLog.h"
#include "SMOnlineStats.h"
#include "Crash/crashDefines.h"
#include "MercuryINI.h"
#include "SMOnlineServer.h"


SMOnlineGameRoom::SMOnlineGameRoom(const MString &name, const MString &description, bool allowgamerooms)
	: SMOnlineRoom(name, description, allowgamerooms) {
	m_roomType = ROOMGame; //Game Room
	m_chooser = 0;
	m_selectTime = m_statsTime = time(NULL);

	LoadFromIni();
}

SMOnlineGameRoom::~SMOnlineGameRoom() {
}

void SMOnlineGameRoom::LoadFromIni()
{
	m_selectDelay = PREFSMAN->GetValueI("ServerOptions", "SelectDelay", 5, 1);
}

void SMOnlineGameRoom::Update() {
	SMOnlineRoom::Update();
	if (m_state == RS_INGAME)
		if (time(NULL) > m_statsTime)
		{
			SortStats();
			SendStats();
			m_statsTime = time(NULL);
		}
}

bool SMOnlineGameRoom::AddClient(SMOnlineClient *client, const MString& pw, bool ignorePass) {
	if ((GetTotalPlayers() + client->GetNumPlayers()) <= (unsigned int)m_maxPlayers)
	{
		bool success = SMOnlineRoom::AddClient(client, pw, ignorePass);
		if (success)
			QuickSelect(client);
		return success;
	}
	return false;
}

void SMOnlineGameRoom::RemoveClient(const unsigned int index)
{
	SMOnlineRoom::RemoveClient(index);
	if (m_state == RS_INGAME)
	{
		/*	If someone leaves while we are in the game state check for gameovers again.
			This makes sure that we don't continue to wait for a gameover from a client
			that has left us.
		*/
		WriteGameOverStats();
	}
}

void SMOnlineGameRoom::ParseData(PacketFunctions& Packet, int clientnum)
{
	PacketFunctions tmp = Packet;
	int command = Packet.Read1();
	switch (command)
	{
	case NSCGSR:
		// Start Round
		if ( SERVER->IsSMLAN() )
		{
			if ( StartReqCheck() )
				SyncStart();
		}
		else
			StartRound(clientnum);
		break;
	case NSCGON:
		// GameOver 
		m_clients[clientnum]->SubmitGameOverStats(m_statsID);
		WriteGameOverStats();
		break;
	case NSCGSU:
		// StatsUpdate
		break;
	case NSCRSG:
		//select song
		ParseRequestSongStart(Packet, clientnum); //selects song and parses reply
		if (m_state == RS_SECONDSELECT)
			if (SelectDelay()) {
				StartSong();
			} else {
				PacketFunctions reply;
				reply.ClearPacket();
				reply.Write1((uint8_t)NSCCM+NSServerOffset);
				reply.WriteNT( "Wait a few seconds before confirming selection. " );
				SendDataToClient(0, reply);
			}
		break;
	case NSCSMS:
		//enter exit
		ParseEnterExit(Packet, clientnum);
		break;
	default:
		SMOnlineRoom::ParseData(tmp, clientnum); //If we don't parse, use the derived parser.
		break;
	}
}

void SMOnlineGameRoom::StartRound(unsigned int clientnum) {
	PacketFunctions startcode;
	startcode.ClearPacket();
	startcode.Write1(NSCGSR + NSServerOffset);
	SendDataToClient(clientnum, startcode);
	m_clients[clientnum]->ResetStartreq();
}

void SMOnlineGameRoom::ParseRequestSongStart(PacketFunctions& Packet, unsigned int clientnum) {
	int type = Packet.Read1();
	switch (type) {
	case 0:
		break;
	case 1:
		LackMessage(clientnum);
		break;
	case 2:
		if (clientnum == 0) {
			SelectSong(Packet);
		} else {
			ChatAsServer("You are not permitted to select a song.", clientnum);
		}
		break;
	default:
		break;
	}
}
/*
void SMOnlineGameRoom::LackMessage() {
	if (m_songreplies+1 >= m_clients.size()) {
		if (m_lacksong.size() > 0) {
			MString message;
			for (unsigned int x = 0; x < m_lacksong.size(); x++) {
				message += m_lacksong[x];
				if (x < m_lacksong.size()-1)
					message += ", ";
			}
			message += " lacks " + m_roundinfo.GetTitle();
			if (m_roundinfo.GetSubtitle() != "") {
				message += ", " + m_roundinfo.GetSubtitle();
			}
			message += ".";
			ChatAsServer(message);
		}
	}
}
*/
void SMOnlineGameRoom::LackMessage(int cNum) {
	MString message;
	for (unsigned int x = 0; x < m_clients[cNum]->GetNumPlayers(); x++) {
		message += m_clients[cNum]->GetPlayer(x)->GetName();
		if (x < m_clients[cNum]->GetNumPlayers()-1)
			message += "&";
	}
	message += " lacks " + m_roundinfo.GetTitle();
	if (m_roundinfo.GetSubtitle() != "") {
		message += ", " + m_roundinfo.GetSubtitle();
	}
	message += ".";
	ChatAsServer(message);
}

void SMOnlineGameRoom::SelectSong(PacketFunctions& Packet) {
	if (AllPresent()) {
		SongInfo incomingSongInfo;
		incomingSongInfo.SetTitle(Packet.ReadNT());
		incomingSongInfo.SetArtist(Packet.ReadNT());
		incomingSongInfo.SetSubtitle(Packet.ReadNT());

		bool hashesArePresent = ((unsigned int)Packet.Position) + 3 < Packet.Size;
		if(hashesArePresent)
		{
			uint32_t numberOfHashes = Packet.Read4();
			for(uint32_t i = 0; i < numberOfHashes && ((unsigned int)Packet.Position) + 3 < Packet.Size; i++)
			{
				incomingSongInfo.GetHashes().push_back(Packet.Read4());
			}
		}

		if(incomingSongInfo == m_roundinfo)
			SetState(RS_SECONDSELECT);
		else {
			SetState(RS_FIRSTSELECT);

			m_roundinfo.SetTitle(incomingSongInfo.GetTitle());
			m_roundinfo.SetArtist(incomingSongInfo.GetArtist());
			m_roundinfo.SetSubtitle(incomingSongInfo.GetSubtitle());
			m_roundinfo.SetHashes(incomingSongInfo.GetHashes());
		}

		if (m_state == RS_FIRSTSELECT) {
			ClientsSelectSong(m_roundinfo);
			MString message = "Play " + m_roundinfo.GetTitle();
			if (m_roundinfo.GetSubtitle() != "") {
				message += ", " + m_roundinfo.GetSubtitle();
			}
			message += "?";
			ChatAsServer(message);
			m_selectTime = time(NULL);
		}
	} else {
		ChatAsServer("Selection denied. Not all players in room.");
	}
}

void SMOnlineGameRoom::ClientsSelectSong(const RoundInfo& roundinfo) {
	PacketFunctions reply;
	reply.ClearPacket();
	reply.Write1(NSCRSG+NSServerOffset);
	reply.Write1(1); //Scroll to and report back
	reply.WriteNT(roundinfo.GetTitle());
	reply.WriteNT(roundinfo.GetArtist());
	reply.WriteNT(roundinfo.GetSubtitle());

	reply.Write4(roundinfo.GetHashes().size());
	for(vector<uint32_t>::size_type i = 0; i < roundinfo.GetHashes().size(); i++)
	{
		// clients know that a list of 0 hashes means the server didn't get them.
		reply.Write4((roundinfo.GetHashes())[i]);
	}

	SendToAllClients(reply);
}

bool SMOnlineGameRoom::AllPresent() {
	for (unsigned int x = 0; x < m_clients.size(); x++) {
		for (unsigned y = 0; y < m_clients[x]->GetNumPlayers(); y++) {
			if (m_clients[x]->GetPlayer(y)->ScreenPosition() != 2)
				return false;
		}
	}
	return true;
}

void SMOnlineGameRoom::StartSong() {
	PacketFunctions reply;
	bool allHaveSong = true;
	unsigned int size = m_clients.size();
	if (m_state == RS_SECONDSELECT) {
		for (unsigned int x = 0; x < size; x++) {
			if (!m_clients[x]->HasSong())
				allHaveSong = false;
		}

		if (AllPresent()) {
			if (allHaveSong) {
				reply.ClearPacket();
				reply.Write1((uint8_t)NSCRSG+NSServerOffset);
				reply.Write1((uint8_t)2);  //Scoll and start
				reply.WriteNT(m_roundinfo.GetTitle());
				reply.WriteNT(m_roundinfo.GetArtist());
				reply.WriteNT(m_roundinfo.GetSubtitle());

				if ( STATS )
				{
					STATS->Begin(m_statsID);
					STATS->SetSongInfo(m_roundinfo);
					STATS->SetRoom(GetTitle());
				}

				m_lastRound = m_roundinfo;
				SendToAllClients(reply);
				SetState(RS_INGAME);
			}
		}
	}
}

void  SMOnlineGameRoom::SortStats()
{
//	unsigned int tmp;
//	bool isChanged;
	unsigned int size = m_PlayersList.size();

	m_OrderedPlayersList.empty(); //Make sure this calls the destructor
	m_OrderedPlayersList.resize(size); //Make sure this allocates new space.
	for (unsigned int i = 0; i < size; i++)
	{
		m_OrderedPlayersList[i].player = m_PlayersList[i];
		m_OrderedPlayersList[i].index = i;
	}

	//Quick sort and reverse order from highest to lowest
	sort(m_OrderedPlayersList.begin(), m_OrderedPlayersList.end());
	reverse(m_OrderedPlayersList.begin(), m_OrderedPlayersList.end());
}

void SMOnlineGameRoom::SendStats()
{
	unsigned int size = m_OrderedPlayersList.size();
	PacketFunctions reply;

	reply.ClearPacket();
	reply.Write1((uint8_t)NSCGSU + NSServerOffset);
	reply.Write1((uint8_t)0);
	reply.Write1((uint8_t)m_OrderedPlayersList.size());
	for (unsigned int x = 0; x < size; x++) {
		reply.Write1((uint8_t)m_OrderedPlayersList[x].index);
	}
	SendToAllClients(reply);

	reply.ClearPacket();
	reply.Write1((uint8_t)NSCGSU + NSServerOffset);
	reply.Write1((uint8_t)1);
	reply.Write1((uint8_t)size);
	for(unsigned int x = 0; x < size; ++x ) {
		reply.Write2( (uint16_t) m_OrderedPlayersList[x].player->GetCombo());
	}
	SendToAllClients(reply);

	reply.ClearPacket();
	reply.Write1((uint8_t)NSCGSU + NSServerOffset);
	reply.Write1((uint8_t)2);
	reply.Write1((uint8_t)size);
	for(unsigned int x = 0; x < size; ++x ) {
		reply.Write1( (uint8_t) m_OrderedPlayersList[x].player->GetProjgrade() );
	}
	SendToAllClients(reply);
}

void SMOnlineGameRoom::ParseEnterExit(PacketFunctions& data, unsigned int clientnum) {
	int subcommand = data.Read1();
	switch (subcommand) {
	case 1:
		SendPlayersList();
		break;
	case 3:
		SendPlayersList();
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 0:
		//this parhaps should do the same as below
		break;
	case 7:
		SMOnlineClient* tmp = m_clients[clientnum];
		RemoveClient(clientnum);
		if (m_joinrooms.size() > 0) {
			m_joinrooms[0]->AddClient(tmp, "", false);
		}
		break;
	}
}

void SMOnlineGameRoom::WriteGameOverStats() {
	MString options;
	unsigned int x = 0;
	bool allowSend = true;
	unsigned int size = m_clients.size();
	for (x = 0; x < size; x++) {
		if (!m_clients[x]->GetGameOver())
			allowSend = false;
	}

	if (allowSend) {
//		LOG->Write("========Sending EOG Stats==========");
//		LOG->Write(m_roundinfo.GetTitle());
//		LOG->Write(m_roundinfo.GetSubtitle());
//		LOG->Write(m_roundinfo.GetArtist());
//		LOG->Write(time(NULL));
		SortStats();
		
		unsigned int numPlayers = m_OrderedPlayersList.size();
	
		m_reply.ClearPacket();
		m_reply.Write1((uint8_t)(NSCGON + NSServerOffset));
		m_reply.Write1((uint8_t)numPlayers);
//		LOG->Write("Num Players:");
//		LOG->Write(numPlayers);
//		LOG->Write("Player Index:");
		for (x = 0; x < numPlayers; ++x) {
			m_reply.Write1((uint8_t)m_OrderedPlayersList[x].index);
//			LOG->Write(m_OrderedPlayersList[x].index);
		}

//		LOG->Write("Scores:");
		for (x = 0; x < numPlayers; ++x) {
			m_reply.Write4(m_OrderedPlayersList[x].player->GetScore());
//			LOG->Write(m_OrderedPlayersList[x].player->GetScore());
		}

//		LOG->Write("Grade");
		for (x = 0; x < numPlayers; ++x) {
			m_reply.Write1((uint8_t)m_OrderedPlayersList[x].player->GetProjgrade());
//			LOG->Write(m_OrderedPlayersList[x].player->GetProjgrade());
		}
	
//		LOG->Write("Diff");
		for (x = 0; x < numPlayers; ++x) {
			m_reply.Write1((uint8_t)m_OrderedPlayersList[x].player->GetDiff());
//			LOG->Write(m_OrderedPlayersList[x].player->GetDiff());
		}

//		LOG->Write("Steps");
		for (int y = W1; y >= MISS; --y)
			for (x = 0; x < numPlayers; ++x) {
				m_reply.Write2((uint16_t)m_OrderedPlayersList[x].player->GetSteps(y));
//				LOG->Write((uint16_t)m_OrderedPlayersList[x].player->GetSteps(y));
			}

//		LOG->Write("OKs");
		for (x = 0; x < numPlayers; ++x) {
			m_reply.Write2((uint16_t)m_OrderedPlayersList[x].player->GetSteps(HELD));  //Tack on OK
//			LOG->Write((uint16_t)m_OrderedPlayersList[x].player->GetSteps(HELD));
		}

//		LOG->Write("MaxCombos");
		for (x = 0; x < numPlayers; ++x) {
			m_reply.Write2((uint16_t)m_OrderedPlayersList[x].player->GetMaxCombo());
//			LOG->Write((uint16_t)m_OrderedPlayersList[x].player->GetMaxCombo());
		}

//		LOG->Write("Options");
		for (x = 0; x < numPlayers; ++x) {
			options = GenerateTimingString(m_OrderedPlayersList[x].player) + " ";
			options += m_OrderedPlayersList[x].player->GetOptions();
			m_reply.WriteNT( options );
//			LOG->Write(options);
		}

//		LOG->Write("Final Packet");
//		LOG->Write(m_reply.Data);
		SendToAllClients(m_reply);
//		LOG->Write("=======END EOG STATS SEND===========");

		if ( STATS )
		{
			STATS->Edit(m_statsID);
			STATS->End();
		}

		m_roundinfo.Reset();  //Reset this so playing the same song again acts propery

		SetState(RS_OPEN);
	}
}

bool SMOnlineGameRoom::SelectDelay() {
	if ((unsigned int)(time(NULL) - m_selectTime) >= m_selectDelay)
		return true;
	return false;
}

void SMOnlineGameRoom::QuickSelect(SMOnlineClient *client) {
	PacketFunctions reply;
	reply.ClearPacket();

	//If the room is in the process of selecting a song, quickly tell the client to select the song
	if ((m_state == RS_FIRSTSELECT) || (m_state == RS_SECONDSELECT)) {
		reply.Write1(NSCRSG+NSServerOffset);
		reply.Write1(1); //Scroll to and report back
		reply.WriteNT(m_roundinfo.GetTitle());
		reply.WriteNT(m_roundinfo.GetArtist());
		reply.WriteNT(m_roundinfo.GetSubtitle());
		client->SendData(reply);
	}
}

MString SMOnlineGameRoom::GenerateTimingString(const LanPlayer* Player) {
	if (Player->GetTiming())
		return MString("TIMING");
	
	return "";
}

bool SMOnlineGameRoom::StartReqCheck()
{
	for (unsigned int i = 0; i < m_clients.size(); ++i)
		if ( !m_clients[i]->GotStartreq() )
			return false;

	return true;
}

void SMOnlineGameRoom::SyncStart()
{
	for (unsigned int i = 0; i < m_clients.size(); ++i)
		StartRound(i);
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
