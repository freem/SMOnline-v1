#ifndef STATSAPI_H
#define STATSAPI_H

#include <vector>
#include "../StdString.h"
#include "../ezSQL.h"
#include "../ezMySQL.h"

using namespace std;

enum LKTYPE
{
	NONE,
	ID, 
	NAME
};

class statsAPI
{
public:
	statsAPI();
	~statsAPI();
	void ProcessCommandsV(vector<CString>& commands);
private:
	void PrintError(CString& error);
	bool ParseOperation(CString& op);
	bool OpenSQLConnection();
	CString QueryID(const CString& name);
	ezSQLQuery QueryLastRound(const CString& ID);

	void PrintQuery( ezSQLQuery &tQuery, string prefix = "" );

	LKTYPE m_lktype;  //Type of lookup we should do

	//We don't really have to worry about numbers since sql usees them as strings anyway.
	CString m_id;
	CString m_name;

	ezSQLStartupInfo m_startInfo;
	ezSQLConnection* m_database;
	ezSQLQuery m_query;
	ezSQLResult m_result;
	ezSQLRow m_row;
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
