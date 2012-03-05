#include "global.h"
#include "SMOnlineServer.h"
#include "SimpleLog.h"
#include "Crash/crashDefines.h"
#include "MercuryINI.h"

using namespace std;

SMOnlineServer::SMOnlineServer(const int port)
{
	m_port = port;
	m_serversock = NULL;

	LoadFromIni();
	LoadRoom(PREFSMAN->GetValueS("ServerOptions", "LobbyName", "Lobby", 1));

	//Must be constructed after loading INI.
	m_database = new SMOnlineSQL;

	m_PingTime = time(NULL);
	srand(time(NULL));
}

SMOnlineServer::~SMOnlineServer()
{
	Stop();
	SAFE_DELETE(m_database);
}

void SMOnlineServer::LoadFromIni() {
	m_serverName = PREFSMAN->GetValueS("ServerOptions", "Name", "SMOnline Server", true);

	m_judgeWindow = LoadJudge();

	if ( !LoadMotd(PREFSMAN->GetValueS("ServerOptions", "MOTD File", "MOTD.txt", true)) ) 
		LOG->Write("Failed to load MOTD");

	m_useSQL = PREFSMAN->GetValueB("ServerOptions", "EnableSQL", true, true);
	m_pingDelay = PREFSMAN->GetValueI("ServerOptions", "PingTime", 60, true);
	m_isSMLAN = PREFSMAN->GetValueB("ServerOptions", "SMLAN", false, true);

	if ( m_isSMLAN )
		m_useSQL = false;
}

bool SMOnlineServer::Start(ostream &out) {
	if (m_useSQL)
		EstablishSQL();

	return CreateServerSocket();
}

void SMOnlineServer::Stop() {

	//Remove rooms before disconnecting clients or else ther will be a crash
	while(m_rooms.size() > 0) {
		SAFE_DELETE(m_rooms[m_rooms.size()-1]);
		m_rooms.pop_back();
	}

	while (m_clients.size() > 0) {
		Disconnect(m_clients[m_clients.size()-1], m_clients);
	}

	while (m_pendingClients.size() > 0) {
		Disconnect(m_pendingClients[m_pendingClients.size()-1], m_pendingClients);
	}

	if (m_serversock != NULL) {
		SAFE_DELETE(m_serversock);
	}
}

void SMOnlineServer::Update() {
	NewClientCheck();
	PingClients(); //Put this here instead of almost last, trying to avoid crash.
	UpdatePendingClients();

	UpdateRooms();
	CheckConnections();
//	CheckRooms();

	if (m_useSQL)
		m_database->Ping();
}

void SMOnlineServer::CheckConnections() {
	for (unsigned int x = 0; x < m_clients.size(); ++x) {
		if (m_clients[x]->DropMe()) {
			Disconnect(m_clients[x], m_clients);
		}
	}
	for (unsigned int x = 0; x < m_pendingClients.size(); ++x) {
		if (m_pendingClients[x]->IsError()) {
			Disconnect(m_pendingClients[x], m_pendingClients);
		}
	}
}

void SMOnlineServer::Disconnect(SMOnlineClient* client, vector<SMOnlineClient*>& clients) {
	vector<SMOnlineClient*>::iterator iterator = LocateClient(client, clients);
	if (iterator != clients.end()) {
		if (!m_useSQL)
			RemoveClientPlayerIDs((*(*(iterator))));
		for (unsigned int x = 0; x < (*iterator)->GetNumPlayers(); x++) {
			RemovePlayerFromMap((*iterator)->GetPlayer(x)->GetName());
		}
		SAFE_DELETE(*iterator);
		clients.erase(iterator);
//		LOG->Write("Client Disconnected");
	}
}

void SMOnlineServer::NewClientCheck()
{
	//Make a new client and accept a connection to it.
	//If no connection is accepted, delete the client.

	SMOnlineClient *tmpclient = new SMOnlineClient();

	if (m_serversock->accept(tmpclient->connection) == true)
	{
//		if (!IsBanned(tmpclient->connection.address))
		{
			tmpclient->AssignColor();
			m_pendingClients.push_back(tmpclient);
//			LOG->Write("Client connected");
		}
//		else
//			delete tmp;
	}
	else {
		SAFE_DELETE(tmpclient);
	}
}

bool SMOnlineServer::DropRoom(unsigned int index) {
	if (index < m_rooms.size()) {
		vector<SMOnlineRoom*>::iterator it = m_rooms.begin();
		it += index;
		if ((*it)->CanDrop()) {
			SAFE_DELETE(*it);
			m_rooms.erase(it);
			return true;
		}
	}
	return false;
}

unsigned int SMOnlineServer::FindUnusedID() {
	bool is_used;
	unsigned int unusedID;
	unsigned int numUsedIDs = m_UsedIDs.size();

	srand(time(NULL));

	do {
		unusedID = rand();
		is_used = false;
		for (unsigned int x = 0; x < numUsedIDs; x++) {
			if (unusedID == m_UsedIDs[x]) {
				is_used = true;
			}
		}
	} while (is_used);

	return unusedID;
}

void SMOnlineServer::RemoveUsedID(unsigned int ID) {
	bool found = false;
	unsigned int vectorSize = m_UsedIDs.size();

	vector<unsigned int>::iterator Iterator;
	Iterator = m_UsedIDs.begin();
	for (unsigned int x = 0; (x < vectorSize) && (!found); ++x)
	{
		if (ID == m_UsedIDs[x])
		{
			m_UsedIDs.erase(Iterator);
			found = true;
		}
		++Iterator;
	}
}

void SMOnlineServer::RemoveClientPlayerIDs(SMOnlineClient &client) {
	unsigned int numPlayers = client.GetNumPlayers();

	for (unsigned int x = 0; x < numPlayers; x++)
		RemoveUsedID(client.GetPlayerID(x));
}

void SMOnlineServer::UpdateRooms() {
	for (unsigned int x = 0; x < m_rooms.size(); ++x)
		if (!DropRoom(x))
			m_rooms[x]->Update();
}

void SMOnlineServer::UpdatePendingClients() {
	PacketFunctions tmp;
	for (unsigned int x = 0; x < m_pendingClients.size(); ++x) {
		tmp.ClearPacket();
		try {
			m_pendingClients[x]->Update(tmp);
			if (m_pendingClients[x]->InRoom()) {
				m_clients.push_back(m_pendingClients[x]);
				m_pendingClients.erase(LocateClient(m_pendingClients[x], m_pendingClients));
			}
		}
		catch (ClientError& er) {
			Disconnect(m_pendingClients[x], m_pendingClients);
			LOG->Write("Pending Client Error:" + er.m_error);
		}
	}
}

vector<SMOnlineClient*>::iterator SMOnlineServer::LocateClient(SMOnlineClient* client, vector<SMOnlineClient*>& clients) {
	vector<SMOnlineClient*>::iterator Iterator;
	for (Iterator = clients.begin(); Iterator != clients.end(); Iterator++)
		if ((*Iterator) == client)
			return Iterator;

	return clients.end();
}

void SMOnlineServer::SendToAll(PacketFunctions& packet) {
	for (unsigned int x = 0; x < m_clients.size(); x++)
		m_clients[x]->SendData(packet);

	for (unsigned int x = 0; x < m_pendingClients.size(); x++)
		m_pendingClients[x]->SendData(packet);
}

bool SMOnlineServer::CreateServerSocket() {
	if (m_serversock == NULL) {
		m_serversock = new EzSockets;

		if (m_serversock == NULL)
			return false;

		m_serversock->blocking = 0; /* Turn off blocking */

		if (m_serversock->create())
			if (m_serversock->bind(m_port))
				if (m_serversock->listen()) {
					LOG->Write("Server Listening");
				}
				else {
					LOG->Write("Error Listening");
					return false;
				}
			else {
					LOG->Write("Could not bind to port");
					return false;
			}
		else {
			LOG->Write("Could not create socket");
			return false;
		}
	}
	return true;
}

bool SMOnlineServer::CycleServerSocket() {
	LOG->Write("Resetting server socket");
	if (m_serversock != NULL) {
		delete m_serversock;
		m_serversock = NULL;
	}

	return CreateServerSocket();
}

void SMOnlineServer::EstablishSQL() {
	if (m_useSQL) {
		//Do not put these variables in LoadFromIni
		m_database->Connect( PREFSMAN->GetValueS("SQLOptions", "Address", "", true),
						PREFSMAN->GetValueS("SQLOptions", "Database", "", true),
						PREFSMAN->GetValueS("SQLOptions", "User", "", true),
						PREFSMAN->GetValueS("SQLOptions", "Passwd", "", true),
						PREFSMAN->GetValueI("SQLOptions", "Port", 3306, true) );
	}
	if (!m_database->isConnected())
		LOG->Write("Failed to connect to database");
}

void SMOnlineServer::PingClients() {
	if ((unsigned int)(m_PingTime + m_pingDelay) < (unsigned int)time(NULL)) {
		PacketFunctions ping;
		ping.ClearPacket();
		ping.Write1(128);
		for (unsigned int x = 0; x < m_clients.size(); x++) {
			m_clients[x]->SendData(ping);
		}
		for (unsigned int x = 0; x < m_pendingClients.size(); x++) {
			m_pendingClients[x]->SendData(ping);
		}
		m_PingTime = time(NULL);
	}
}

void SMOnlineServer::AddPlayerToMap(MString name, SMOnlineClient* client) {
	if (m_playerclientmap.find(name) == m_playerclientmap.end()) {
		m_playerclientmap[name] = client; //This will create a new player name in the map.
	}
}

void SMOnlineServer::RemovePlayerFromMap(MString name) {
	m_playerclientmap.erase(name);
}

bool SMOnlineServer::MsgPlayer(MString& name, MString& message) {
	PacketFunctions pMsg;
	pMsg.ClearPacket();
	if (m_playerclientmap.find(name) != m_playerclientmap.end()) {
		pMsg.Write1(NSCCM + NSServerOffset);
		pMsg.WriteNT(message);
		CHATLOG->QuietWrite(message);
		m_playerclientmap[name]->SendData(pMsg);
		return true;
	}
	return false;
}

bool SMOnlineServer::LoadMotd(MString path) {
	string tmp;
	ifstream file;
	file.open(path.c_str());
	if (file.is_open() == false)
		return false;

	m_MOTD = "";

	while (!file.eof()) {
		getline(file, tmp);
		m_MOTD += MString(tmp.c_str()) + "\n";
	}
	file.close();
	return true;
}

SMOnlineClient* SMOnlineServer::GetPlayerClient(MString name) {
	if (m_playerclientmap.find(name) != m_playerclientmap.end())
		return m_playerclientmap[name];

	return NULL;
}

JudgeWindow SMOnlineServer::LoadJudge()
{
	JudgeWindow jw;

	jw.SetMarv(PREFSMAN->GetValueF("JudgeWindows", "Marvelous", 0.022500f, 1));
	jw.SetPerfect(PREFSMAN->GetValueF("JudgeWindows", "Perfect", 0.045000f, 1));
	jw.SetGreat(PREFSMAN->GetValueF("JudgeWindows", "Great", 0.090000f, 1));
	jw.SetGood(PREFSMAN->GetValueF("JudgeWindows", "Good", 0.135000f, 1));
	jw.SetBoo(PREFSMAN->GetValueF("JudgeWindows", "Boo", 0.180000f, 1));

	return jw;
}

void SMOnlineServer::LoadRoom(const MString& RoomName, SMOnlineRoom* parentRoom)
{
	SMOnlineRoom* tmproom;
	MString key = RoomName + "Room";
	MString RoomDesc = PREFSMAN->GetValueS(key, "Description", "", true);
	MString RoomType = PREFSMAN->GetValueS(key, "Type", "chat", true);
	bool AllowGameRooms = PREFSMAN->GetValueB(key, "AllowGameRooms", false, true);
	unsigned int numSubRooms = PREFSMAN->GetValueI(key, "NumberSubRooms", 0, true);

	PREFSMAN->GetValueS(key, "Name", RoomName, true);

	if ( IsSMLAN() )
	{
		//SMLAN only needs one game room
		tmproom = new SMOnlineGameRoom(RoomName, RoomDesc, false);
		m_rooms.push_back(tmproom);
		return;
	}

	if (RoomType == "chat")
		tmproom = new SMOnlineRoom(RoomName, RoomDesc, AllowGameRooms);
	else if (RoomType == "game")
		tmproom = new SMOnlineGameRoom(RoomName, RoomDesc, AllowGameRooms);
	else
	{
		LOG->Write("Invalid room type:" + RoomType);
		return;
	}

	if (parentRoom)
		parentRoom->JoinToRoom(tmproom);

	m_rooms.push_back(tmproom);

	for (unsigned int i = 0; i < numSubRooms; ++i)
		LoadRoom( PREFSMAN->GetValueS(key, ssprintf("SubRoom%d", i), "", true ), tmproom );
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
