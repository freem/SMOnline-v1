#ifndef SMONLINESERVER_H
#define SMONLINESERVER_H

#include <vector>
#include <iostream>
#include <map>
#include "MercuryString.h"
#include "ezsockets.h"
#include "SMOnlineClient.h"
#include "SMOnlineRoom.h"
#include "SMOnlineGameRoom.h"
#include "JudgeWindow.h"
#include "SMOnlineSQL.h"
#include "ConstructCount.h"

using namespace std;

class SMOnlineServer {
	public:
		SMOnlineServer(const int port = 8765);
		~SMOnlineServer();
		bool Start(ostream &out);
		void Stop();
		void Update();
		inline const MString& GetName() const { return m_serverName; }
		inline void SetName(const MString& name) { m_serverName = name; }
		void LoadFromIni();
		void SendToAll(PacketFunctions& packet);
		bool CycleServerSocket();
		void AddPlayerToMap(MString name, SMOnlineClient* client);
		bool MsgPlayer(MString& name, MString& message);
		SMOnlineClient* GetPlayerClient(MString name);
		SMOnlineClient* GetPlayerClient( unsigned int ID ) { return (ID<m_clients.size())?m_clients[ID]:NULL; };
		int GetNumConnected() { return m_clients.size(); }
		inline bool UseSQL() const { return m_useSQL; }
		inline bool IsSMLAN() const { return m_isSMLAN; }
		inline SMOnlineSQL* GetDatabase() { return m_database; }
		const JudgeWindow& GetJudgeWindows() const { return m_judgeWindow; }
		inline SMOnlineRoom* GetRoom(unsigned int i) { return m_rooms[i]; }
		inline vector<SMOnlineRoom*>& GetRooms() { return m_rooms; }
		inline const MString& GetMOTD() const { return m_MOTD; }

	private:
		vector<PacketFunctions> m_PacketQueue;
		int m_port;
		EzSockets* m_serversock;
		vector<SMOnlineClient*> m_clients;
		vector<SMOnlineClient*> m_pendingClients;
		map<MString, SMOnlineClient*> m_playerclientmap;
		vector<unsigned int> m_UsedIDs;
		PacketFunctions Packet;
		time_t m_PingTime;
		unsigned int m_pingDelay;
		vector<SMOnlineRoom*> m_rooms;
		bool m_useSQL;
		bool m_isSMLAN;
		SMOnlineSQL* m_database;
		JudgeWindow m_judgeWindow;
		MString m_serverName;
		MString m_MOTD;

		void CheckConnections();
		void NewClientCheck();
		void Disconnect(SMOnlineClient* client, vector<SMOnlineClient*>& clients);
//		void CheckRooms(SMOnlineRoom* room);
		bool DropRoom(unsigned int index);
//		void AssignPlayerIDs();
		unsigned int FindUnusedID();
		void RemoveUsedID(unsigned int ID);
		void RemoveClientPlayerIDs(SMOnlineClient &client);
		void UpdateRooms();
		void UpdatePendingClients();
		vector<SMOnlineClient*>::iterator LocateClient(SMOnlineClient* client, vector<SMOnlineClient*>& clients);
		ConstructCount m_counter;
		bool CreateServerSocket();
		void EstablishSQL();
		void PingClients();
		void RemovePlayerFromMap(MString name);
		bool LoadMotd(MString path);
		JudgeWindow LoadJudge();
		void LoadRoom(const MString& RoomName, SMOnlineRoom* parentRoom = NULL);
};

extern SMOnlineServer* SERVER;

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
