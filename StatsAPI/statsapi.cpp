#include <iostream>
#include <vector>
#include "statsapi.h"

using namespace std;

void main(int argc, char* argv[])
{
	statsAPI* api = NULL;
	vector<CString> args;

	if ((argc < 2) || (argc > 21))
		return;
	
	args.resize(argc - 1);

	for(unsigned int i = 1; i < argc; i++)
		args[i - 1] = argv[i];

	api = new statsAPI;

	api->ProcessCommandsV(args);

	if (api)
	{
		delete api;
		api = NULL;
	}
	
}

statsAPI::statsAPI()
{
	m_lktype = NONE;
	m_database = NULL;

	m_startInfo.Database = "stepmani_newstats";	//Not implemented
	m_startInfo.Password = "1000Hilltop";
	m_startInfo.Port = 3306;
	m_startInfo.ServerName = "stepmaniaonline.com";
	m_startInfo.UserName = "stepmani_stats";
}

statsAPI::~statsAPI()
{
	//Make the API signal the end of the data when destructing.
	cout << "END:END";
	if (m_database)
		delete m_database;
}

void statsAPI::ProcessCommandsV(vector<CString>& commands)
{
	CString flag, value;
	unsigned int pos = 0;

	for (unsigned int i = 0; i < commands.size(); i++)
	{
		pos = commands[i].Find("=");
		if (pos == CString::npos)
		{
			PrintError("Command \"" +  commands[i] + "\" invalid. Ignoring.");
			return;
		}
		flag = commands[i].substr(0,pos);
		value = commands[i].substr(pos+1);

		if (flag == "id")
		{
			m_id = value;
			m_lktype = ID;
		}
		else if (flag == "name")
		{
			m_name = value;
			m_lktype = NAME;
		}
		else if(flag == "op")
		{
			if (OpenSQLConnection())
				ParseOperation(value);
			else
				PrintError(CString("SQL connection failed"));
		}
		else
		{
			PrintError("Unknown command \"" + commands[i] +"\".");
		}
	}
}

void statsAPI::PrintError(CString& error)
{
	cout << "ERROR:" << error << endl;
}

bool statsAPI::ParseOperation(CString& op)
{
	if (op == "lr")
	{
		switch (m_lktype)
		{
		case NONE:
			break;
		case NAME:
			m_id = QueryID(m_name);
			if (m_id != "Unknown" )
				m_lktype = ID;
			else
			{
				PrintError("Unknown name " + m_name);
				return false;
			}
		case ID:
			PrintQuery(QueryLastRound(m_id), "lr");
			break;
		}
		return true;
	}
	else if (op == "total")
	{
		return true;
	}
	else
		PrintError("Unknown operation \"" + op + "\". Ignoring.");
	return false;
}


bool statsAPI::OpenSQLConnection()
{
	try {
		m_database = new ezMySQLConnection(m_startInfo);
		m_database->Connect();
	}
		catch (const ezSQLException &e) {
		PrintError(CString("SQL ERROR THROWN! Dec:" + e.Description));
		return false;
	}
	return m_database->m_bLoggedIn;
}

CString statsAPI::QueryID(const CString& name)
{
	m_query.m_InitialQuery = "select user_id from `stepmani_stats`.`phpbb_users` where username=\"" + name + "\" limit 0,1";
	m_database->BlockingQuery( m_query );

	if (m_result.isError)
		throw;

	m_result = m_query.m_ResultInfo;
	if (m_result.FieldContents.size() <= 0)
		return "Unknown";

	return m_result.FieldContents[0][0].ToString();
}

void statsAPI::PrintQuery( ezSQLQuery &tQuery, string prefix )
{
	//In the event we have an error, show it
	if ( tQuery.m_ResultInfo.errorNum != 0 )
	{
		PrintError(CString(tQuery.m_ResultInfo.errorDesc));
		exit ( 0 );
	}
	if ( tQuery.m_ResultInfo.FieldContents.size() == 0 )
	{
		PrintError(CString("No data for this user."));
		exit (0);
	}
	//Record seed for finding rank.
	//show table header
	unsigned i = 0;
	for ( i = 0; i < tQuery.m_ResultInfo.Header.size(); i++ )
	{
		cout << prefix<<tQuery.m_ResultInfo.Header[i].Field<<":";
		if ( tQuery.m_ResultInfo.FieldContents.size() > 0 )
			if ( tQuery.m_ResultInfo.FieldContents[0].size() > i )
			{
				cout<< tQuery.m_ResultInfo.FieldContents[0][i].ToString()<<endl;
//				if ( tQuery.m_ResultInfo.Header[i].Field == "seed" )
//					seed = tQuery.m_ResultInfo.FieldContents[0][i].ToString();
			}
	}
}

ezSQLQuery statsAPI::QueryLastRound(const CString& ID)
{
	m_query.m_InitialQuery = "select * from player_stats where pid=" + ID + " order by round_ID desc limit 0,1";
	m_database->BlockingQuery( m_query );
	return m_query;
}

/*
 * (c) 2003-2004 Joshua Allen, Charles Lohr
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
