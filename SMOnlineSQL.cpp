#include "SMOnlineSQL.h"
//#include "SMOnlineIni.h"
#include "Crash/crashDefines.h"
#include "MercuryINI.h"

SMOnlineSQL::SMOnlineSQL() {
	m_timeout = 5;
	m_compress = 0;
	m_wasConnected = false;
	LoadFromIni();
	m_con = NULL;
}

SMOnlineSQL::~SMOnlineSQL()
{
	Disconnect();
}

void SMOnlineSQL::LoadFromIni()
{
	m_pingTime = PREFSMAN->GetValueI("SQLOptions", "PingTime", 60, 1);
}

bool SMOnlineSQL::Connect() {
	try {

		m_con->Connect();

		if (m_con->m_bLoggedIn) {
			LOG->Write("SQL Login Successful");
			m_wasConnected = true;
		} else {
			m_error = true;
			LOG->Write("SQL Login FAILED");
			return false;
		}
	}
	catch (const ezSQLException &e) {
		m_error = true;
		LOG->Write("SQL Login FAILED");
		LOG->Write(ssprintf("SQL ERROR THROWN! Code: %d Dec:", e.Code) + e.Description.c_str());
		return false;
	}
	return true;
}

bool SMOnlineSQL::Connect(MString addr, MString dbase, MString user, MString passwd, unsigned int port) {
	ezSQLStartupInfo startInfo;

	startInfo.Database = m_dbase = dbase;	//Not implemented
	startInfo.Password = m_passwd = passwd;
	startInfo.Port = m_port = port;
	startInfo.ServerName = m_addr = addr;
	startInfo.UserName = m_user = user;	

	m_con = new ezMySQLConnection(startInfo);

	return Connect();
}

bool SMOnlineSQL::Reconnect()
{
	if (m_wasConnected)
		return Connect();
	else
		return false;
}

void SMOnlineSQL::Disconnect() {
	if (isConnected()) 
		m_con->Disconnect();
}

bool SMOnlineSQL::BQueryDB(MString command) {
	m_error = true; //Use this to prevent strange crashes if we happen to have an error when receiving data (mySQL++ functions access violations)
	ezSQLQuery query;
	query.m_InitialQuery = command;
	if (isConnected()) {
		m_con->BlockingQuery(query);
		m_result = query.m_ResultInfo;
		m_error = m_result.isError;
		if (m_error)
			LOG->Write("SQL block query ERROR");	
	} else {
		LOG->Write("SQL RECONNECTING");
		if (Connect()) {
			BQueryDB(command);
		} else {
			m_error = true;
			LOG->Write("SQL block query ERROR");
		}
	}
	return true;
}

bool SMOnlineSQL::QueryDB(MString command) {
	ezSQLQuery query;
	query.m_InitialQuery = command;
	query.m_dropResponce = true;
	if (isConnected()) {
		m_con->Query(query);	
	} else {
		LOG->Write("SQL RECONNECTING");
		if (Connect())
			QueryDB(command);
	}
	return true;
}

bool SMOnlineSQL::GetRow(unsigned int index, ezSQLRow& row) {
	if (!m_error)
		if (index < NumRows()) {
			row = m_result.FieldContents[index];
			return true;
		}

	return false;
}

unsigned int SMOnlineSQL::NumRows() {
	if (!m_error) {
		return m_result.FieldContents.size();
	} else {
		return 0;
	}
}

void SMOnlineSQL::Ping() {
	if ((unsigned int)(m_PingTime + m_PingDelay) < (unsigned int)time(NULL)) {
		if (isConnected()) {
			m_con->Ping();
			m_PingTime = time(NULL);
		}
	}
}

bool SMOnlineSQL::isConnected() {
  if (m_con != NULL)
    return m_con->m_bConnected;
  return false;
}

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
