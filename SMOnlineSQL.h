#ifndef SMONLINESQL_H
#define SMONLINESQL_H

#include "ezSQL.h"
#include "ezMySQL.h"
#include "MercuryString.h"
#include "SimpleLog.h"

using namespace std;

class SMOnlineSQL {
public:
	SMOnlineSQL();
	~SMOnlineSQL();
	bool Connect(MString addr, MString dbase, MString user, MString passwd, unsigned int port = 3306);
	bool Reconnect();
	void Disconnect();
	inline unsigned int GetPort() { return m_port; }
	inline MString GetAddr() { return m_addr; }
	inline MString GetDbase() { return m_dbase; }
	inline MString GetUser() { return m_user; }
	inline MString GetPasswd() { return m_passwd; }
	inline bool GetCompress() { return m_compress; }
	inline unsigned int GetTimeout() { return m_timeout; }
	bool isConnected();
	void LoadFromIni();
							    
	bool QueryDB(MString command);
	bool BQueryDB(MString command);
	inline ezSQLResult GetResult() { return m_result; }
	bool GetRow(unsigned int index, ezSQLRow& row);
	unsigned int NumRows();
	inline ezSQLConnection* GetConnection() { return m_con; }
	void Ping();

	void SetPort(unsigned int port) { m_port = port; }
	void SetAddr(MString& addr) { m_addr = addr; }
	void SetDbase(MString& dbase) { m_dbase = dbase; }
	void SetUser(MString& user) { m_user = user; }
	void SetPasswd(MString& passwd) { m_passwd = passwd; }
	void SetCompress(bool compress) { m_compress = compress; }
	void SetTimeout(unsigned int timeout) { m_timeout = timeout; }
private:
	time_t m_PingTime;
	bool Connect();
	ezSQLConnection* m_con;
	MString m_addr;
	MString m_dbase;
	MString m_user;
	MString m_passwd;
	unsigned int m_port;
	bool m_compress;
	unsigned int m_timeout;
	ezSQLResult m_result;
	ezSQLRow m_row;
	bool m_error;

	unsigned int m_PingDelay;
	unsigned int m_pingTime;
	
	bool m_wasConnected;
};

#endif

/* 
 * (c) 2005-2006 Joshua Allen
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
