#include "MercuryString.h"
#include "SMOnlineRoom.h"
#include "SMOnlineGameRoom.h"
#include "SimpleLog.h"
#include "SQLfunctions.h"
#include "SMOnlineServer.h"
#include "Crash/crashDefines.h"
#include "MercuryINI.h"

SMOnlineRoom::SMOnlineRoom(const MString &name, const MString &description, bool allowgamerooms)
: m_roomType(ROOMChat), m_allowgamerooms(allowgamerooms), m_state(RS_OPEN), m_maxPlayers(16),
m_title(name), m_description(description)
{
	LoadFromIni();
}

SMOnlineRoom::~SMOnlineRoom()
{
	while (m_clients.size() > 0)
		RemoveClient(m_clients.size()-1);

	while (m_joinrooms.size() > 0)
		UnjoinToRoom(m_joinrooms[m_joinrooms.size()-1]->m_title);
}

void SMOnlineRoom::LoadFromIni()
{
	m_commonColor = PREFSMAN->GetValueS("ChatOptions", "CommonColorCode", "|c0", 1);
	m_baseColor = PREFSMAN->GetValueS("ChatOptions", "Color", "FFFFFF", 1);
	m_roomNameLength = PREFSMAN->GetValueI("ServerOptions", "RoomNameLength", 15, 1);
	m_roomKickTime = PREFSMAN->GetValueI("ServerOptions", "RoomKickTime", 60, 1);
}

bool SMOnlineRoom::AddClient(SMOnlineClient *client, const MString& pw, bool ignorePass)
{
	//IN_GAME is the only state that we are not allowed to add clients
	if (m_state == RS_INGAME)
		return false;

	//Check for self
	for (unsigned int i = 0; i < m_clients.size(); ++i)
			if (m_clients[i] == client)
				return false;

	if ( CheckForBan(client) || CheckForKick(client) )
		return false;

	//ignorePass must be false and m_password must not be empty
	if ( !ignorePass && !m_passwd.empty())
		if (pw != m_passwd)
			return false;

	int cnum = m_clients.size();
	m_clients.push_back(client);
	client->SetInRoom(true);
	SendRoomTitle(cnum);
	SendRoomList(cnum);
	PopulatePlayersList();
	SendPlayersList();
	return true;
}

void SMOnlineRoom::Update() {
	UpdateClients();
}

void SMOnlineRoom::UpdateClients() {
	int length = 0;
	for (unsigned int x = 0; x < m_clients.size(); ++x)
	{
		if (!m_clients[x])
		{
			LOG->Write("Error: Trying to update NULL client");
			RemoveClient(x);
			continue;
		}

		try
		{
			do
			{
				//As long as we keep getting data from the socket, keep processing it
				length = m_clients[x]->Update(m_packet);
				if (length > 0)
					ParseData(m_packet, x);
				//Check for NULL incase the client switched rooms
			} while ((length > 0) && (x < m_clients.size()) && (m_clients[x]));
		}
		catch (ClientError&)
		{
			RemoveClient(x);
		}
	}
}

void SMOnlineRoom::RemoveClient(const unsigned int index)
{
	if (index >= m_clients.size())
		return;

	if (m_clients[index])
	{
		m_clients[index]->SetInRoom(false);
		m_clients[index] = NULL;
	}
	m_clients.erase(m_clients.begin()+index);
//	LOG->Write("Removed client from room");

	PopulatePlayersList();
	SendPlayersList();
}

void SMOnlineRoom::JoinToRoom(SMOnlineRoom *room) {
	//join other room to this room.
	m_joinrooms.push_back(room);

	//join this room to the other room
	room->m_joinrooms.push_back(this);

	//Let users in both rooms know of the added rooms
	SendRoomList();
	room->SendRoomList();
}

void SMOnlineRoom::UnjoinToRoom(const MString &roomname) {
	SMOnlineRoom* tmproom;
	for (unsigned int x = 0; x < m_joinrooms.size(); ++x)
		if (m_joinrooms[x]->GetTitle() == roomname)
		{
			tmproom = m_joinrooms[x];
			m_joinrooms[x] = NULL;
			m_joinrooms.erase(m_joinrooms.begin()+x);
			tmproom->UnjoinToRoom(m_title);  //Remove the other room's link to this room
			break; //end the search
		}

	SendRoomList();
}

void SMOnlineRoom::AnalizeChat(unsigned int clientNum, PacketFunctions& Packet)
{
	MString message = Packet.ReadNT();
	ChatCommandPack ccp = ParseCommands(message, clientNum);

	switch(ccp.cmd)
	{
	case NONE:
		RelayChat(clientNum, message);
		return;
	case ANNOUNCE:
		{
			PacketFunctions reply;
			reply.ClearPacket();
			reply.Write1(NSCSU+NSServerOffset);
			reply.WriteNT(ccp.data.front());
			SERVER->SendToAll(reply);
		}
		return;
	case KICK:
		{
			const MString& name = ccp.data.front();
			for (unsigned int x = 0; x < m_clients.size(); ++x)
				for (unsigned int y = 0; y < m_clients[x]->GetNumPlayers(); ++y) 
					if (m_clients[x]->GetPlayer(y)->GetName() == name)
					{
						LOG->Write("Kicking " + name);
						m_kicked[ccp.data.front()] = KickClient(name);
						ChangeRoom(1, m_joinrooms[0]->GetTitle(), x, "", true);
						return;
					}
		}
		break;
	case BAN:
		{
			const MString& name = ccp.data.front();
			for (unsigned int x = 0; x < m_clients.size(); ++x)
				for (unsigned int y = 0; y < m_clients[x]->GetNumPlayers(); ++y) 
					if (m_clients[x]->GetPlayer(y)->GetName() == name)
					{
							LOG->Write("Banning " + name);
							m_banned[name] = name;
							ChangeRoom(1, m_joinrooms[0]->GetTitle(), x, "", true);
							return;
					}
		}
		break;
	case PM:
		{
			MString user = ccp.data.front();
			ccp.data.pop();
			MString message = ccp.data.front();
			MString tmp;
			MString success = "PM Failed!";

			if ((user.length() > 0) && (message.length() > 0))
			{
				tmp = "PM from ";
				unsigned int numPlayers = m_clients[m_cNum]->GetNumPlayers();
				for (unsigned int x = 0; x < numPlayers; ++x)
				{
					tmp += m_clients[m_cNum]->GetPlayerName(x);
					if ((numPlayers > 1) && (x < (numPlayers - 1)))
						tmp += "&";
				}
				tmp += ": " + message;
				if (SERVER->MsgPlayer(user, tmp))
					success = "PM Success!";
			}
			PacketFunctions result;
			result.ClearPacket();
			result.Write1(NSCCM + NSServerOffset);
			result.WriteNT(success);
			m_clients[m_cNum]->SendData(result);
		}
		break;
	case DROP:
		{
			SMOnlineClient* tmp = SERVER->GetPlayerClient(ccp.data.front());
			if (tmp != NULL)
				tmp->connection.close();
		}
		break;
//	case FORCESTART:
//		break;
	default:
		break;
	};
}

MString Detranslit( const MString & inputText )
{
	MString sEndChat;

	const char* chatStr = inputText.c_str();
	for ( unsigned int i = 0; i < inputText.length(); i++ )
	{
		if ( (unsigned char )( chatStr[i] ) >= (unsigned char )( 0xE0 ) ) 
			i+=2;
		else if ( (unsigned char )( chatStr[i] ) >= (unsigned char )( 0xC0 ) )
			i+=1;
		else
			sEndChat += chatStr[i];
	}
	return sEndChat;
}

void SMOnlineRoom::RelayChat(unsigned int clientNum, const MString &chat) {

	//First, Remove any unicode in the chat message and load it
	//into a temp message.

	MString sEndChat;

	sEndChat = Detranslit( chat );

	unsigned int numPlayers = m_clients[clientNum]->GetNumPlayers();

	m_reply.ClearPacket();

	//For all clients who support colored text:
	MString message = m_commonColor + 
					  m_clients[clientNum]->GetColor();

	for (unsigned int x = 0; x < numPlayers; ++x) {
		message += m_clients[clientNum]->GetPlayerName(x);
		if ((numPlayers > 1) && (x < (numPlayers - 1)))
			message += "&";
	}

	message +=	":" + m_commonColor + m_baseColor + " ";
	message += sEndChat;

	m_reply.ClearPacket();
	m_reply.Write1(NSCCM + NSServerOffset);
	m_reply.WriteNT(message);
	SendToAllClients( m_reply, 2, 2 );


	//For all clients who do not support colored text:
	message = "";

	for (unsigned int x = 0; x < numPlayers; ++x) {
		message += m_clients[clientNum]->GetPlayerName(x);
		if ((numPlayers > 1) && (x < (numPlayers - 1)))
			message += "&";
	}

	message += ": " + sEndChat;// + MString( itoa( m_clients[clientNum]->GetVersion(), "  ",10 ));

	CHATLOG->QuietWrite(message);

	m_reply.ClearPacket();
	m_reply.Write1(NSCCM + NSServerOffset);
	m_reply.WriteNT(message);
	SendToAllClients( m_reply, 1, 1 );
}

void SMOnlineRoom::ParseData(PacketFunctions& Packet, int clientnum)
{
	/* Try not to do any Packet Parsing here here because a lot of parsing
	was done in the Client. Parsing again would be a waste.
	Packet has been unaltered previous to this point.*/
	int command = Packet.Read1();
	switch (command)
	{
	case NSCPing:
		// No Operation
//		SendValue(NSServerOffset + NSCPingR, clientNum);
		break;
	case NSCPingR:
		// No Operation response
		break;
	case NSCHello:
		// Hello
		break;
	case NSCSU:
		// Style Update
		break;
	case NSCCM:
		// Chat message
		AnalizeChat(clientnum, Packet);
		break;
	case NSCSMS:
		//Room Status Change
		SendRoomTitle(clientnum);
		SendRoomList(clientnum);
		break;
	case NSSMONL:
		SMOnlineParse(Packet, clientnum);
		break;
	default:
		break;
	}
}

void SMOnlineRoom::SMOnlineParse(PacketFunctions &Packet, int clientnum) {
	int command = Packet.Read1();
	switch (command) {
	case 0:
		//Login
		break;
	case 1:
		//Change Room
		{
			int command = Packet.Read1();
			MString title = Packet.ReadNT();
			MString pw = Packet.ReadNT();
			ChangeRoom(command, title, clientnum, pw);
		}
		break;
	case 2:
		//Create new room
		if (m_allowgamerooms) {
			CreateRoom(Packet);
			ChangeRoom(1, m_joinrooms[m_joinrooms.size()-1]->GetTitle(), clientnum, "", true);
		}
		break;
	case 3:
		//Additional room info
		GenAdditionalInfo(Packet.ReadNT(), clientnum);
		break;
	default:
		LOG->Write(ssprintf("Invalid SMOnline command:%d", command));
		break;
	}
}

void SMOnlineRoom::WriteRoomListPacket(PacketFunctions &Packet) {
	unsigned int numRooms = m_joinrooms.size();
	Packet.ClearPacket();
	Packet.Write1(NSSMONL + NSServerOffset);
	Packet.Write1(1);  //Room command
	Packet.Write1(1); //Type (List Rooms)
	Packet.Write1(numRooms);
	for (unsigned int x = 0; x < numRooms; ++x) {
		if (m_joinrooms[x] != NULL) {
			Packet.WriteNT(m_joinrooms[x]->GetTitle());
			Packet.WriteNT(m_joinrooms[x]->GetDescription());
		}
	}
	//state
	for (unsigned int x = 0; x < numRooms; ++x) {
		if (m_joinrooms[x] != NULL)
		{
			//TO MAKE CHARLES HAPPY IF PASSWORDED USE 1 (UNSUSED)
			if (!m_joinrooms[x]->m_passwd.empty() && (m_joinrooms[x]->m_state == RS_OPEN))
				Packet.Write1(RS_UNUSED);
			else
				Packet.Write1(m_joinrooms[x]->m_state);
		}
	}
	//Flags
	char flags;
	for (unsigned int x = 0; x < numRooms; ++x) {
		if (m_joinrooms[x] != NULL)
		{
			flags = 0;
			//bit 1 passworded
			if (!m_joinrooms[x]->m_passwd.empty())
				flags += 1;

			Packet.Write1(flags);
		}
	}
}

void SMOnlineRoom::SendRoomList(int clientnum) {
	WriteRoomListPacket(m_reply);
	SendDataToClient(clientnum, m_reply);
}

void SMOnlineRoom::SendRoomList() {
	WriteRoomListPacket(m_reply);
	SendToAllClients(m_reply);
}

//Note: VersionRestriction = -1 means send to all clients, regardless.
//		Otherwise, it indicates which version to send it to.
//		VersionDirection of 0 indicates the version must match up perfectly
//		1: Must be less than or eqal to prescribed version
//		2: Must be greater than or equal to prescribed version
void SMOnlineRoom::SendToAllClients(const PacketFunctions& Packet, int VersionRestriction, int VersionDirection )
{
	unsigned int size = m_clients.size();
	for (unsigned int x = 0; x < size; ++x)
	{
		if	( ( VersionRestriction == -1 ) || 
			( ( VersionDirection == 0 ) && ( VersionRestriction == m_clients[x]->GetVersion() ) ) ||
			( ( VersionDirection == 1 ) && ( VersionRestriction >= m_clients[x]->GetVersion() ) ) ||
			( ( VersionDirection == 2 ) && ( VersionRestriction <= m_clients[x]->GetVersion() ) ) )
			SendDataToClient(x, Packet);
	}
}

void SMOnlineRoom::SendRoomTitle(int clientnum) {
	m_reply.ClearPacket();
	m_reply.Write1(NSSMONL + NSServerOffset);
	m_reply.Write1(1); //Room Update command
	m_reply.Write1(0); //Title Update
	m_reply.WriteNT(m_title);
	m_reply.WriteNT(m_description);
	m_reply.Write1(m_roomType);
	SendDataToClient(clientnum, m_reply);
}

void SMOnlineRoom::ChangeRoom(int command, const MString& title, int clientnum, const MString& pw, bool ignorePass) {
	SMOnlineClient *tmpclient;
	if (command == 1) {
		for (unsigned int x = 0;x < m_joinrooms.size(); ++x) {
			if (m_joinrooms[x]->GetTitle() == title)
			{
				//We don't have to handle any state logic here,
				//AddClient handles all of it.
				tmpclient = m_clients[clientnum];
				RemoveClient(clientnum);

				//If we fail add the client back to self
				if (!m_joinrooms[x]->AddClient(tmpclient, pw, ignorePass))
					AddClient(tmpclient, "", true);
			}
		}
	}
}

void SMOnlineRoom::SendPlayersList() {
	unsigned int numplayers = m_PlayersList.size();

	m_reply.ClearPacket();
	m_reply.Write1(NSCUUL + NSServerOffset);
	m_reply.Write1(numplayers);
	m_reply.Write1(numplayers);

	for (unsigned int x = 0; x < numplayers; ++x) {
		m_reply.Write1(m_PlayersList[x]->ScreenPosition());
		m_reply.WriteNT(m_PlayersList[x]->GetName());
	}
	SendToAllClients(m_reply);
}

unsigned int SMOnlineRoom::GetTotalPlayers() {
	unsigned int players = 0;
	for (unsigned int x = 0; x < m_clients.size(); ++x) {
		players += m_clients[x]->GetNumPlayers();
	}
	return players;
}

bool SMOnlineRoom::CanDrop()
{
//don't drop rooms if we are SMLAN, we only have 1 room
	if ( SERVER->IsSMLAN() )
		return false;

	//Only allow removal if we are a game room and have no clients
	if ((m_roomType == ROOMGame) && m_clients.empty())
		return true;

	return false;
}

void SMOnlineRoom::PopulatePlayersList() {
	unsigned int numClients = m_clients.size();
	unsigned int numPlayers;
	ClearPlayerList();

	for (unsigned int x = 0; x < numClients; x++) {
		numPlayers = m_clients[x]->GetNumPlayers();
		for (unsigned int y = 0; y < numPlayers; y++) {
			m_PlayersList.push_back(m_clients[x]->GetPlayer(y));
		}
	}
}

void SMOnlineRoom::ClearPlayerList() {
	unsigned int size = m_PlayersList.size();
	for (unsigned int x = 0; x < size; x++)
		m_PlayersList[x] = NULL;

	m_PlayersList.clear();
}

void SMOnlineRoom::CreateRoom(PacketFunctions& Packet) {
	bool origional_name = true;
	SMOnlineRoom* tmp = NULL;
	int type = Packet.Read1();
	MString title = Packet.ReadNT();
	MString sub = Packet.ReadNT();
	MString passwd = Packet.ReadNT();

	if (!title.empty())
	{
		if (title.size() > m_roomNameLength)
			title = title.substr(0, m_roomNameLength);

		for (unsigned int x = 0; (x < m_joinrooms.size()) && origional_name; ++x)
			if (title == m_joinrooms[x]->m_title)
				origional_name = false;

		if (origional_name) {
			if (type) {
				tmp = new SMOnlineGameRoom(title, sub);
			} else {
				tmp = new SMOnlineRoom(title, sub);
			}

			if (tmp) {
				LOG->Write("Created room");
				if (!passwd.empty())
					tmp->m_passwd = passwd;
				SERVER->GetRooms().push_back(tmp);
				JoinToRoom(tmp);
				SendRoomList();
			}
		}
	}
}

void SMOnlineRoom::ChatToClient(const MString& message, unsigned int clientnum) {
	m_reply.ClearPacket();
	m_reply.Write1(NSCCM + NSServerOffset);
	m_reply.WriteNT(Detranslit(message));
	SendDataToClient(clientnum, m_reply);
}

void SMOnlineRoom::ChatAsServer(const MString& message, unsigned int clientnum) {
	MString tmp = SERVER->GetName();
	tmp += (": " + Detranslit(message));
	if (clientnum < m_clients.size()) {
		ChatToClient(tmp, clientnum);
		LOG->Write(tmp);
	}
}

void SMOnlineRoom::ChatAsServer(const MString& message) {
	WriteChatPacket(m_reply, SERVER->GetName() + ": " + Detranslit(message));
	SendToAllClients(m_reply);
}

void SMOnlineRoom::WriteChatPacket(PacketFunctions& packet, const MString& message) {
	packet.ClearPacket();
	packet.Write1(NSCCM + NSServerOffset);
	packet.WriteNT(message);
}

void SMOnlineRoom::SendDataToClient(unsigned int clientnum, const PacketFunctions& packet) {
	if (clientnum < m_clients.size())
		m_clients[clientnum]->SendData(packet);
}

void SMOnlineRoom::SetState(RoomState state) {
	if (state == m_state)
		return;

	m_state = state;

	//Since we changed state, alert rooms that know about us.
	for (unsigned int x = 0; x < m_joinrooms.size(); x++) {
		if (m_joinrooms[x] != NULL)
			m_joinrooms[x]->SendRoomList();
		else
			LOG->Write("ERROR: Send room list of NULL room");
	}
}

bool SMOnlineRoom::CheckForBan(SMOnlineClient* client) {
	MString name;
	unsigned int bannedsize = m_banned.size();
	if (bannedsize > 0) {
		for (unsigned int x = 0;x < client->GetNumPlayers(); x++) {
			if (m_banned.find(client->GetPlayer(x)->GetName()) != m_banned.end())
				return true;
		}
	}
	return false;
}

bool SMOnlineRoom::CheckForKick(SMOnlineClient* client) {
	MString name;
	map<MString, KickClient>::iterator iterator;
	for (unsigned int x = 0;x < client->GetNumPlayers(); x++) {
		name = client->GetPlayer(x)->GetName();
		iterator = m_kicked.find(name);
		if (iterator != m_kicked.end()) {
			if ((unsigned int)(m_kicked[name].GetTimeStamp() + m_roomKickTime) < (unsigned int)time(NULL)) {
				m_kicked.erase(iterator);
				return false;
			} else {
				return true;
			}
		}
	}
	return false;
}

void SMOnlineRoom::GenAdditionalInfo(const MString& roomname, const int clientnum)
{
	for (unsigned int x = 0; x < m_joinrooms.size(); x++) {
		if (m_joinrooms[x]->GetTitle() == roomname) {
			MString title, subtitle, artist;
			m_reply.ClearPacket();
			m_reply.Write1(NSSMONL + NSServerOffset);
			m_reply.Write1(3);

			if (m_joinrooms[x]->m_roomType == ROOMGame)
			{
			    RoundInfo last = ((SMOnlineGameRoom*)m_joinrooms[x])->LastRound();
			    title = last.GetTitle();
			    subtitle = last.GetSubtitle();
			    artist = last.GetArtist();
			}

			m_reply.WriteNT(title); //Title
			m_reply.WriteNT(subtitle); //Subtitle
			m_reply.WriteNT(artist); //Artist

			m_reply.Write1((unsigned char)(m_joinrooms[x]->GetTotalPlayers()));
			m_reply.Write1((unsigned char)m_maxPlayers);
			
			//Player names
			for (unsigned int i = 0; i < m_joinrooms[x]->m_clients.size(); i++)
				for (unsigned int j = 0; j < m_joinrooms[x]->m_clients[i]->GetNumPlayers(); j++)
					m_reply.WriteNT(m_joinrooms[x]->m_clients[i]->GetPlayer(j)->GetName());

			SendDataToClient(clientnum, m_reply);
			return;
		}
	}
}

ChatCommandPack SMOnlineRoom::ParseCommands(MString& data, unsigned int clientNum) {
	MString command;
	m_cNum = clientNum;

	if (data.find("/") != 0) {
		return ChatCommandPack(NONE);
	}
	command = GetCommand(data);

	ChatCommandPack ccp;

	if (m_clients[clientNum]->IsAdmin())
	{
		if (Announce(command, data, ccp))
			return ccp;
		if (Drop(command, data, ccp))
			return ccp;
	}

	if (Kick(command, data, ccp))
		return ccp;
	if (Ban(command, data, ccp))
		return ccp;
	if (Msg(command, data, ccp))
		return ccp;

	return ccp;
}

MString SMOnlineRoom::GetCommand(MString& data) {
	MString command;
	int pos = data.find(" ");
	if (pos > 0) {
		command = data.substr(1, pos);
	}
	return command;
}

bool SMOnlineRoom::Announce(const MString& command, const MString& data, ChatCommandPack& ccp)
{
	MString message;

	if (command.find("announce") != 0)
		return false;

	message = data.substr(command.length()+1);
	ccp.cmd = ANNOUNCE;
	ccp.data.push(data.substr(command.length()+1));
	return true;
}

bool SMOnlineRoom::Ban(const MString& command, const MString& data, ChatCommandPack& ccp)
{
	MString name;
	if (m_roomType == ROOMGame)
		if (command.find("ban") == 0) {
			if (m_cNum == 0) {
				GetPlayerName(data, name);
				ccp.cmd = BAN;
				ccp.data.push(name);
				return true;
			}
		}

	return false;
}

bool SMOnlineRoom::Kick(const MString& command, const MString& data, ChatCommandPack& ccp)
{
	MString name;
	if (m_roomType == ROOMGame)
		if (command.find("kick") == 0) {
			if (m_cNum == 0) {
				GetPlayerName(data, name);
				ccp.cmd = KICK;
				ccp.data.push(name);
				return true;
			}
		}

	return false;
}

bool SMOnlineRoom::Msg(const MString& command, const MString& data, ChatCommandPack& ccp)
{
	MString tmp;
	MString user;
	MString message;
	if (command.find("msg") == 0)
	{
		tmp = data.substr(command.length()+1);
		if (tmp.length() > 0)
		{
			if (GetPlayerName(data, user))
				if (tmp.length() >= (user.length()+3))
				{
					ccp.cmd = PM;
					ccp.data.push(user);
					ccp.data.push(tmp.substr(user.length()+3));
					return true;
				}
		}
	}
	return false;
}

bool SMOnlineRoom::Drop(const MString& command, const MString& data, ChatCommandPack& ccp)
{
	MString name;
	if (command.find("drop") == 0)
	{
		GetPlayerName(data, name);
		ccp.cmd = DROP;
		ccp.data.push(name);
		return true;
	}
	return false;
}

bool SMOnlineRoom::GetPlayerName(const MString& data, MString& name)
{
	unsigned int pos1 = data.find('"');
	unsigned int pos2 = data.find('"', pos1+1);
	if (pos1 != pos2)
		if (pos1 != string::npos)
			if (pos2 != string::npos) {
				pos1++;
				name = data.substr(pos1, pos2 - pos1);
				return true;
			}
	return false;
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
