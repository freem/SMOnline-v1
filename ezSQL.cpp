#include "ezSQL.h"
#include <time.h>

//Various constructors for QString
QString::QString( const string & start ) 
{
	length = start.length();
	data = (char*)malloc(length);
	memcpy( data, start.c_str(), length ); 
}

QString::QString( const QString & start )
{
	length = start.length;
	data = (char*)malloc(length);
	memcpy( data, start.data, length ); 
}

QString QString::operator = ( const QString & start )
{
	length = start.length;
	data = (char*)malloc(length);
	memcpy( data, start.data, length ); 

	return *this;
}

QString::QString() 
{
	length = 0;
	data = NULL;
}

QString::~QString() 
{ 
	//safe delete
	if ( data != NULL ) 
	{
		delete data;
		data = NULL; 
	} 
}

long QString::ToInt()
{
	char * b = (char*)malloc(length+1);
	memcpy( b, data, length );
	b[length] = 0;
	long ret = atol( b ); 
	free ( b );
	return ret;
}

float QString::ToFloat()
{
	char * b = (char*)malloc(length+1);
	memcpy( b, data, length );
	b[length] = 0;
	double ret = atof( b ); 
	free ( b );
	return (float)ret;
}

string QString::ToString()
{
	if ( data == NULL )
		return "";
	string ret;
	ret.append( data, length );
	return ret;
}

//Convert bit-wise a QString to a number
unsigned int GetNum( QString inData )
{
	int LOC = 0;
	unsigned int ret = 0;
	int count = 0;
	int place = 1;
	for ( ; ( LOC < inData.length ) ; count++ ) 
	{
		ret += (unsigned char) (inData.data[LOC]) * place;
		place *= 256;
		LOC++;
	}
	return ret;
}


void ezSQLResult::Reset()
{
	errorNum = 0;
	errorDesc = "";
	isError = false;
	Header.clear();
	FieldContents.clear();
	Type = RES_STANDALONE;
	State = STATE_BEGIN;
}

ezSQLResult::ezSQLResult()
{
	Reset();
}

void ezSQLConnection::Ping()
{
	//Tricky: we don't send it here.  There _needs_ to be a function
	//in the actual SQL connector that overloads this to actually
	//send a Ping.
	ezSQLQuery tempData;
	tempData.m_InitialQuery = "PING";
	tempData.m_ID = 0; 
	tempData.m_dropResponce = true;
	m_lSent.push_back( tempData );
}

bool ezSQLConnection::BlockingQuery( ezSQLQuery & QueryData, int WaitTime, bool SetDropIfFail )
{
	m_lLastBlockingID++;
	if ( m_lLastBlockingID > 1000000000 )
		m_lLastBlockingID = 0;
	unsigned long LookingFor = m_lLastBlockingID;
	QueryData.m_ID = m_lLastBlockingID;
	Query ( QueryData );		//Send the query

	QueryData.m_ResultInfo.Reset();

	//Forcefully wait until we get the packet we are looking for.
	//Don't worry, GetResult calls Update which checks to see if
	//if we have been disconnected from the server.

	long TimeToStop = time( NULL ) + WaitTime;

	bool failed = false;

	//m_Socket.blocking = true;
	while ( !GetResult( QueryData, LookingFor, true ) )
	{
		if ( time( NULL ) > TimeToStop )
		{
			failed = true;
			Ping( );
			break;
		}
#if defined(WIN32)
		Sleep(10);
#else
		usleep(10000);
#endif
	}

	//m_Socket.blocking = false;
	if ( !m_bConnected || failed )
	{
		if ( SetDropIfFail )
			for ( unsigned int i = 0; i < m_lReceived.size(); i++ )
				if ( m_lSent[i].m_ID == long(LookingFor) )
					m_lSent[i].m_dropResponce = true;
		return false;
	}

	return true;
}

bool ezSQLConnection::Query( ezSQLQuery & QueryData )
{
	if ( m_Socket.IsError() )
		return false;
	if ( (!m_bConnected) || (!m_bLoggedIn) )
		return false;

	WriteQuery( QueryData );
	m_lSent.push_back( QueryData );
	return true;
}

bool ezSQLConnection::PopResult( ezSQLQuery & ResultData )
{
	Update();	//Update to move any bufferd data into result list.

	//If no result, send back empty query.
	if ( m_lReceived.empty() )
	{
		ResultData.m_ID = 0;
		ResultData.m_InitialQuery = "";
		ResultData.m_ResultInfo.Reset();
		return false;
	}

	//If so pop off the front and erease it.
	ResultData = m_lReceived.front();
	m_lReceived.erase( m_lReceived.begin() );
	return true;
}

bool ezSQLConnection::PeekResult( ezSQLQuery & ResultData )
{
	Update();
	ResultData = m_lReceived.front();
	return true;
}

bool ezSQLConnection::GetResult( ezSQLQuery & ResultData, int ID, bool removeWhenDone )
{
	Update();
	//search for matching ID
	for ( unsigned int i = 0; i < m_lReceived.size(); i++ )
		if ( m_lReceived[i].m_ID == ID )
		{
			ResultData = m_lReceived[i];
			m_lReceived.erase( m_lReceived.begin() + i );
			return true;
		}
	return false;
}

void ezSQLConnection::Update( )
{
	//Verify that we can do what we want to go for sure!
	if ( m_Socket.IsError() )
	{
		//don't close twice.  If it thinks we're still connected, we should
		//close the socket cleanly.
		if ( m_bConnected || m_bLoggedIn )
			m_Socket.close();

		m_bConnected = false;
		m_bLoggedIn = false;
	}
	
	if ( !m_bConnected )
		return;

	if ( !m_bLoggedIn )
		return;

	//Look at the frontmost sent query
	ezSQLQuery tmp;

	if ( !m_lSent.empty() )
		tmp = m_lSent.front();


	while ( ReadQuery( tmp ) )
	{
		//We got a query, so pop off the first one
		m_lSent.erase ( m_lSent.begin() );

		//...and if we aren't supposed to drop the responce, dump it on received.
		if ( !tmp.m_dropResponce )
		{
			m_lReceived.push_back( tmp );
			if ( m_Socket.blocking )
				break;
		}
	}
}

void ezSQLConnection::ClearSentStack ( )
{
	m_lSent.clear();
}

bool ezSQLConnection::ResendStack ( )
{
	if ( m_Socket.IsError() )
		return false;
	if ( (!m_bConnected) || (!m_bLoggedIn) )
		return false;

	for ( unsigned int i = 0; i < m_lSent.size(); i++ )
		if ( !WriteQuery( m_lSent[i] ) )
			return false;

	return true;
}

ezSQLException::ezSQLException( int inCode, string inDescription )
{
	Code = inCode;
	Description = inDescription;
}

/* 
 * (c) 2005 Charles Lohr
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
