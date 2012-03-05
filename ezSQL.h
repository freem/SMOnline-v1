#ifndef _EZSQL_H
#define _EZSQL_H

#include "ezSQLConstants.h"
#include "ezsockets.h"
#include <vector>

//We use our own mini-string class otherwise we get warnings
//everywhere if we try using strings in STLs.  It is also geared
//for SQL use with it's ToInt and ToFloat functions.
struct QString
{
public:
	QString();
	QString( const string & start );
	QString( const QString & start );
	~QString();
	QString operator = ( const QString & start );

	//Conversions
	string			ToString();
	long			ToInt();
	float			ToFloat();

	//Special byte-wise conversion
	unsigned int	ToMySQLNumber();

	int length;
	char * data;
};

//Convert a string/QString into a number as if it were in binary conditions
//with rightmost digit most significant.
unsigned int GetNum( QString inData );
inline unsigned int GetNum( string inData ) { return GetNum(QString(inData)); } //Convert a string byte-wise into a number

//Hold information regarding the header of a table it contains all 
//necessiary information about the elements of the table
struct ezSQLFieldDescriptor
{
	string Table;			//Table title
	string Field;			//Field title
	int Length;				//Maximum length of field in bytes
	ezSQLFieldTypes Type;	//Type of data held in field
	int Flags;				//translates to SQL-specific flags
	int Decimals;			//translates to SQL-specific decmil
};

//Hold one row of SQL Data
typedef vector < QString > ezSQLRow;

//Structure for an entire tabular result from a query
struct ezSQLResult
{
	ezSQLResult();

	bool isError;		//An error occoured
	int errorNum;		//Error code for error
	string errorDesc;	//Description of error

	//Contains all information regarding the header of the result
	vector < ezSQLFieldDescriptor > Header;

	//Contains all rows of data in the result
	vector < ezSQLRow > FieldContents;

	//Return the number of rows contained in this result
	inline unsigned int size() { return FieldContents.size(); }

	//Result type (describes the data in this result)
	enum ResultType { RES_STANDALONE = 0, RES_TABULAR, RES_ERROR } Type;

	//Internal use only
	enum ResultState{ STATE_BEGIN = 0, STATE_HEADER, STATE_FIELDS, STATE_DONE } State;

	//Internal use only
	void Reset();
};

//Structure for a complete query (both query, and result)
struct ezSQLQuery
{
	ezSQLQuery( ):
		m_ID ( 0 ), m_dropResponce( false )
	{ } //No codde

	ezSQLResult	m_ResultInfo;	//Full result from server
	int			m_ID;			//User ID for query EzSQL Ignores this, the user may use it however they wish
	string		m_InitialQuery;	//Initial query (in plaintext) for the user to generate.
	bool		m_dropResponce;	//Ignore responce from server.
};

//Startup structure for user to pass the connection all necessiary information
struct ezSQLStartupInfo
{
	string	ServerName;	//Address of server
	int		Port;		//Port on server to connect to
	string	UserName;	//Username for use on server 
	string	Password;	//Plain text password of user
	string	Database;	//Database to connect to on start (not always supported)
};

//Structure containing all information about the server that was connected to
struct ezSQLServerInfo
{
	int		m_ProtocolVersion;
	string	m_Version;
	int		m_ThreadID;
	string	m_CharsetName;
};

//In the event of a problem when connecting to an SQL server this will be 
//thrown.  Currently, only a Connect() will ever throw anything.
class ezSQLException
{
public:
	ezSQLException( int inCode, string inDescription );
	int Code;				//SQL-specific error code
	string Description;		//SQL-specific description
};

//A complete SQL connection.  Please note: This class is not
//created as is.  It's pure virtual.  You have to create a specific
//sql class, like ezMySQLConnection
class ezSQLConnection
{
public:

	//When creating an ezSQLConnection, you must pass in the startup info.
	ezSQLConnection( const ezSQLStartupInfo & startInfo ) : 
		m_bLoggedIn( false ),
		m_bConnected( false ),
		m_lLastBlockingID( 0 )
	{ 
		m_Info = startInfo;
		m_Socket.create(); 
	}

	//stubbed virtual destructor to shut GCC up.
	virtual ~ezSQLConnection() { }

	//Ping server to request that it does not disconnect you.
	virtual void Ping( );

	//Connect to the remote SQL Server.
	//In the event ResendPendingQueries is true, the Connect command will resend
	//every query that it did not receive a responce to.
	void Connect( bool ResendPendingQueries = false ) { Connect( m_Info, ResendPendingQueries ); }
	virtual void Connect( const ezSQLStartupInfo & startInfo, bool ResendPendingQueries = false ) = 0;

	//Disconnect from the remote SQL Server
	virtual void Disconnect( ) = 0;

	//Create a blocking query.  This will halt program execution until
	//the server responds with a responce. (Up until the given wait time)
	//If it fails based on wait time, it will ping the server in an effort
	//to get the TCP stack to disconnect in the event that the connection
	//with the server failed in an unclean fashion.
	//Returns TRUE if the QueryData contains the result, false if it failed.
	//
	//If it fails, it will still leave a pending query in the stack.  
	//Upon reconnect, it will resend this packet. 
	//
	//The problem is, if it failed in timeout, it's impossible for the client
	//to know if the query was actually received or not.  It could easily
	//cause the client to become out-of-sync with the server.
	bool BlockingQuery( ezSQLQuery & QuereyData, int WaitTime = 5, bool SetDropIfFail = true );

	//Add a query to the stack.
	bool Query( ezSQLQuery & QueryData );

	//Pop the topmost result
	bool PopResult( ezSQLQuery & ResultData );

	//Peek at the topmost result
	bool PeekResult( ezSQLQuery & ResultData );

	//Does not currently work.
	bool GetResult( ezSQLQuery & ResultData, int ID, bool removeWhenDone = true );

	//Update the receive stack, this will load any query responces
	//that are in the input buffer into the results buffer
	void Update( );

	//If true, then login was successful.
	bool				m_bLoggedIn;

	//If true, then we are connected to the server.
	bool				m_bConnected;

	//Information regarding the server (usable once connected)
	ezSQLServerInfo		m_ServerInfo;	
protected:	
	//For actual SQL interface class.  Send a query and Return true 
	//if it worked.
	virtual bool WriteQuery( const ezSQLQuery & QueryData ) = 0;
	virtual bool ReadQuery( ezSQLQuery & ResponceData ) = 0;

	EzSockets			m_Socket;
	ezSQLStartupInfo	m_Info;

	//List of sent queries that have not gotten a result
	vector < ezSQLQuery > m_lSent;	

	//List of queries that have matching results
	vector < ezSQLQuery > m_lReceived;	

	unsigned long m_lLastBlockingID;

	void ClearSentStack ( );
	bool ResendStack ( );
};


#endif

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
