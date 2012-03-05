#ifndef _EZMYSQL_H
#define _EZMYSQL_H

#include "ezSQL.h"

//MySQL Packet structure: this is used to store information that is either
//being received, or planned to be sent.  It can be written to byte-at-a-time
//or chunk-at-a-time.  You can read or write numbers to it.
//
//When Sending a packet to a remote SQL server, you should make one of 
//these, then clear it with Clear( ) and begin assembling it by using
//the WriteStr( ) function for strings and WriteNum( ) function for
//numbers.  You then simply have to run the GetDataOut( ) function,
//and send the data it provides to you.
//
//When Receiving a packet, you should make one of these, then clear it
//using Clear(), then proceed to use LoadByte( ) to load in the first
//few bytes, until size != -1.  Once this occours, you can load in however
//many bytes you wish at a time as long as the m_Data buffer does not 
//get more bytes than m_Size.  Once PacketDone( ) returns true, you may
//begin to read the data out using ReadNum( ), ReadStr( ) and
//GetTabularChunk().
struct ezMySQLPacket
{
	ezMySQLPacket( ) { Clear (); }

	//Create a packet based off of a string as if the string was the packet.
	ezMySQLPacket( string m_Data );
	ezMySQLPacket( char * data, int len );

	//Reset the packet, does what a constructor should do.
	void Clear ();

	//Used when you want to send a packet.
	void	GetOutData( string & outData );
	int		GetOutLength( );

	string	m_Data;
	int		m_Location;
	int		m_PacketNum;
	int		m_Size;

	//Write and read a potentially null terminating string
	//If a string is the last element, it does not need to be null terminating.
	void WriteStr( string InStr, bool AddNull = true );
	string ReadStr( );

	//Use Length number of bytes out of
	//Rightmost (last) digit is most significant
	void WriteNum( int Data, unsigned int Length );
	unsigned int ReadNum( unsigned int Length );

	//Utils for reading one tabular element
	bool GetTabularChunk( string & TabularData );
	int	 FindTabularLength();

	//For socket use ( returns true if it now has a full packet )
	bool LoadByte( char byte );
	bool PacketDone();
};

//Actual ezSQLConnection class.  This can be created.  You should never
//access any of it's members directly.  Everything you need to know about
//how to use it is in ezSQL.
class ezMySQLConnection : public ezSQLConnection
{
public:
	ezMySQLConnection( const ezSQLStartupInfo & startInfo ) : ezSQLConnection(startInfo) { };

	virtual ~ezMySQLConnection() { }

	//these functions _must_ exist here.
	virtual void Connect( const ezSQLStartupInfo & startInfobool, bool ResendPendingQueries = false );
	virtual void Disconnect( );
	virtual void Ping( );

protected:
	//Send just the query blindly.
	virtual bool WriteQuery( const ezSQLQuery & QueryData );

	//Blindly read the upcomming query into ResponceData.
	virtual bool ReadQuery( ezSQLQuery & ResponceData );

	//Temporary packet that data is loaded into when reading
	ezMySQLPacket ReadQueryPacket;
	
	//Temporary result that data is loaded into when reading
	ezSQLResult tempResult;

	//Last packet number (used to keep packets in order)
	int LastPacketNumber;
private:
	string Salt;

	//Hash algorythms (as described in the MySQL details)
	void HashAlg( string Password, unsigned long & n1, unsigned long & n2 );
	double RandNum( unsigned long &seed1, unsigned long &seed2 );
	string EncodeWithSalt( string Salt, string Password );
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
