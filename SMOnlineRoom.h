#ifndef SMONLINEROOM_H
#define SMONLINEROOM_H

#include <queue>

#include <vector>
#include <iostream>
#include "SMOnlineClient.h"
#include "RoundInfo.h"
#include "ConstructCount.h"
//#include "SMOnlineIni.h"
#include "SMLanPlayers.h"

enum RoomType
{
	ROOMChat = 0,
	ROOMGame,		//1
};

enum RoomState
{
	RS_OPEN = 0,
	RS_UNUSED,
	RS_INGAME,
	RS_FIRSTSELECT,
	RS_SECONDSELECT
};

class KickClient
{
public:
	KickClient(const MString& name = "")
		: m_name(name), m_timeStamp(time(NULL))
	{};
	inline void SetName(const MString& name) { m_name = name; }
	inline const MString& GetName() { return m_name; }
	inline time_t GetTimeStamp() { return m_timeStamp; }
private:
	MString m_name;
	time_t m_timeStamp;
};

struct BanClient
{
	BanClient(const MString& name)
		: m_name(name)
	{};
	MString m_name;
};

enum SMOCHATCMD
{
	NONE = 0,
	ANNOUNCE,
	KICK,
	BAN,
	PM,
	DROP,
	FORCESTART
};

struct ChatCommandPack
{
	ChatCommandPack(SMOCHATCMD smocmd = NONE, const MString& d = "")
		:cmd(smocmd)
	{
		if (!d.empty())
			data.push(d);
	};
	SMOCHATCMD cmd;
	queue<MString> data;
};

class SMOnlineRoom {
public:
	SMOnlineRoom(const MString &name, const MString &description, bool allowgamerooms = 1);
	virtual ~SMOnlineRoom();
	virtual bool AddClient(SMOnlineClient *client, const MString& pw, bool ignorePass = false);
	void JoinToRoom(SMOnlineRoom *room);
	void UnjoinToRoom(const MString &roomname);
	virtual void Update();
	bool CanDrop();
	inline const MString& GetTitle() const { return m_title; }
	inline const MString& GetDescription() const { return m_description; }
	void LoadFromIni();
	inline bool CmpPasswd(const MString& pw) const { if (pw.empty()) return false; return m_passwd == pw; }
	inline RoomState GetState() const { return m_state; }

protected:
	PacketFunctions m_reply;
	RoomType m_roomType;
	bool m_allowgamerooms;
	vector<SMOnlineClient*> m_clients;
	vector<SMOnlineRoom*> m_joinrooms;
	ConstructCount m_counter;
	RoomState m_state;
	int m_maxPlayers;
	MString m_passwd;

	virtual void ParseData(PacketFunctions& Packet, int clientnum);
	virtual void GenAdditionalInfo(const MString& roomname, const int clientnum);
	void SendToAllClients(const PacketFunctions& Packet, int VersionRestriction = -1, int VersionDirection = 0 );
	vector<LanPlayer*> m_PlayersList;
	void PopulatePlayersList();
	virtual void RemoveClient(const unsigned int index);
	void ChatToClient(const MString& message, unsigned int clientnum);
	void ChatAsServer(const MString& message, unsigned int clientnum);
	void ChatAsServer(const MString& message);
	void SendDataToClient(unsigned int clientnum, const PacketFunctions& packet);
	void SendPlayersList();
	bool CheckForBan(SMOnlineClient* client);
	bool CheckForKick(SMOnlineClient* client);
	void SetState(RoomState state);
	unsigned int GetTotalPlayers();

	//chat message stuff
	MString GetCommand(MString& data);
	ChatCommandPack ParseCommands(MString& data, unsigned int m_cNum);
	bool Announce(const MString& command, const MString& data, ChatCommandPack& ccp);
	bool Kick(const MString& command, const MString& data, ChatCommandPack& ccp);
	bool Ban(const MString& command, const MString& data, ChatCommandPack& ccp);
	bool Msg(const MString& command, const MString& data, ChatCommandPack& ccp);
	bool Drop(const MString& command, const MString& data, ChatCommandPack& ccp);
	bool GetPlayerName(const MString& data, MString& name);

private:
	MString m_title;
	MString m_description;
	PacketFunctions m_packet;
	map<MString, MString> m_banned;
	map<MString, KickClient> m_kicked;
	MString m_commonColor;
	MString m_baseColor;
	unsigned int m_roomNameLength;
	unsigned int m_roomKickTime;

	void UpdateClients();
	void SendRoomList(int clientnum);
	void SendRoomList();
	void WriteRoomListPacket(PacketFunctions &Packet);
	void AnalizeChat(unsigned int clientNum, PacketFunctions& Packet);
	void RelayChat(unsigned int clientNum, const MString &chat);;
	void SendRoomTitle(int clientnum);
	void SMOnlineParse(PacketFunctions &Packet, int clientnum);
	void ChangeRoom(int command, const MString& title, int clientnum, const MString& pw, bool ignorePass = false);
	void ClearPlayerList();
	void CreateRoom(PacketFunctions& Packet);
	void WriteChatPacket(PacketFunctions& packet, const MString& message);
	void VerifyUser(PacketFunctions& packet, unsigned int cnum);

	unsigned int m_cNum; //used for chat parsing stuff
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
