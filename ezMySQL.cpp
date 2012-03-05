#include "ezMySQL.h"
#include "ezMySQLConstants.h"

ezMySQLPacket::ezMySQLPacket( string Data )
{
	m_Data = Data;
	m_Location = 0;
	m_Size = ReadNum( 3 );
	m_PacketNum = ReadNum( 1 );
	m_Data = Data.substr( 4, Data.length() - 4 );
}

ezMySQLPacket::ezMySQLPacket( char * data, int len )
{
	for ( int i = 0; i < len; i++ )
		m_Data += data[i];
}

void ezMySQLPacket::Clear( )
{
	m_Location = 0;
	m_PacketNum = 0;
	m_Size = -1;
	m_Data = "";
}

void ezMySQLPacket::GetOutData( string & outdata )
{
	outdata += char( m_Data.size() % 256 );
	outdata += char( ( m_Data.size() / 256 ) % 256 );
	outdata += char( ( m_Data.size() / 65536 ) % 256 );
	outdata += char( m_PacketNum );
	outdata += m_Data;
}

int		ezMySQLPacket::GetOutLength( )
{
	return m_Data.length() + 4;
}

void ezMySQLPacket::WriteStr( string InStr, bool AddNull )
{
	for ( unsigned int i = 0; i < InStr.length(); i++ )
		m_Data += InStr.c_str()[i];

	if ( AddNull )
		m_Data += '\0';
}

string ezMySQLPacket::ReadStr( )
{
	string ret;
	for ( ; ( m_Location < long(m_Data.length()) ) && ( m_Data.c_str()[m_Location] != '\0' ); m_Location++ )
		ret += m_Data.c_str()[m_Location];
	m_Location++;
	return ret;
}

//Rightmost (last) digit is most significant
void ezMySQLPacket::WriteNum( int Data, unsigned int Length )
{
	for ( unsigned int i = 0; i < Length ; i ++ )
	{
		m_Data += char (Data % 256);
		Data /= 256;
	}
}

unsigned int ezMySQLPacket::ReadNum( unsigned int Length )
{
	unsigned int ret = 0;
	unsigned int count = 0;
	unsigned int place = 1;
	//Tricky -- this could, and probally should be a while statement.
	//Basically, we have to start pulling off letters/numbers until
	//we've reached the end, or have all we need.
	for ( ; ( m_Location < long(m_Data.length()) ) && ( count < Length ) ; count++ ) 
	{
		//Tricky -- we need to forcefully typecast to an unsigned char
		//so that we don't have strange negetive values.
		ret += (unsigned char) ( m_Data.c_str()[m_Location] )* place;
		place *= 256;
		m_Location++;
	}
	return ret;
}

//This first finds the size of the upcomming tabular chunk
//then pulls that many bytes off and dumps it back out.
bool ezMySQLPacket::GetTabularChunk( string & TabularData )
{
	TabularData = "";
	if ( m_Location >= long(m_Data.length()) )
		return false;

	int Len = FindTabularLength();

	for ( int i = 0; i < Len; i++ )
	{
		TabularData += char(ReadNum(1));
		if ( m_Location >= long(m_Data.length()) )
			return true;
	}
	return true;
}

int	 ezMySQLPacket::FindTabularLength()
{
	int base = ReadNum(1);
	switch ( base )
	{
	case 251:
		return 0;
	case 252:
		return ReadNum(2);
	case 253:
		return ReadNum(3);
	case 254:
		return ReadNum(8);
	}
	return base;
}


bool ezMySQLPacket::LoadByte( char byte )
{
	m_Data += byte;

	//If we do not know the size yet, but we have enough 
	//data to make a packet header, take what data
	//we have to find size and packet num.
	if ( ( m_Size == -1 ) && ( m_Data.length() >= 4 ) )
	{
		const char * data = m_Data.c_str();
		m_Size = (unsigned char) (data[0]);
		m_Size += (unsigned char) (data[1]) * 256;
		m_Size += (unsigned char) (data[2]) * 65536;
		m_PacketNum = (unsigned char) (data[3]);
		m_Location = 0;
		m_Data = m_Data.substr( 4, m_Data.length() - 4 );
	} 

	//If we have all the bytes we need, return true.
	if ( ( long(m_Data.length()) >= m_Size ) && ( m_Size != -1 ) )
		return true;
	else 
		return false;
}

bool ezMySQLPacket::PacketDone()
{
	if ( ( long(m_Data.length()) >= m_Size ) && ( m_Size != -1 ) )
		return true;
	else 
		return false;
}

void ezMySQLConnection::Connect( const ezSQLStartupInfo & startInfo, bool ResendPendingQueries )
{
	//Reset all important variables.
	m_bLoggedIn = false;
	m_bConnected = false;
	m_Socket.close();
	m_Socket.create();
	m_Socket.blocking = true;
	LastPacketNumber = -1;
	ReadQueryPacket.Clear();
	tempResult.Reset();

	//If we cannot connect, throw an exception. ( this means big time failure )
	if ( m_Socket.connect( startInfo.ServerName, startInfo.Port ) == false )
		throw ( ezSQLException( -1, "Failed to connect" ) );

	m_bConnected = true;
	char data[1];
	ezMySQLPacket tPacket;

	//Since we're already in blocking mode, we should block until the first
	//packet has been received.
	m_Socket.ReadData( data, 1 );
	while ( tPacket.LoadByte( data[0] ) == false )
		if ( !m_Socket.ReadData( data, 1 ) )
			break;
	
	//Load the first packet into the serverInfo
	m_ServerInfo.m_ProtocolVersion = tPacket.ReadNum(1);
	m_ServerInfo.m_Version = tPacket.ReadStr( );
	m_ServerInfo.m_ThreadID = tPacket.ReadNum(4);
	Salt = tPacket.ReadStr();
	tPacket.ReadNum(2);	//Caps
	m_ServerInfo.m_CharsetName = GetCharsetName ( tPacket.ReadNum(1) );
	tPacket.ReadNum(2);	//Status

	//Clear the packet to begin formulation 
	tPacket.Clear();

	tPacket.m_PacketNum = 1;
	tPacket.WriteNum( LONG_PASSWORD | LONG_FLAG | TRANSACTIONS | INTERACTIVE | CONNECT_WITH_DB, 2 );
	tPacket.WriteNum( 0, 3 );
	tPacket.WriteStr( startInfo.UserName );
	tPacket.WriteStr( EncodeWithSalt( Salt, startInfo.Password ), true );//Send the salt-encoded password
	tPacket.WriteStr( startInfo.Database, false );

	string OutData; 
	tPacket.GetOutData( OutData );
	const int OutLen = tPacket.GetOutLength();

	m_Socket.SendData( OutData.c_str(), OutLen );

	tPacket.Clear();

	m_Socket.ReadData( data, 1 );

	while ( tPacket.LoadByte( data[0] ) == false )
		if ( !m_Socket.ReadData( data, 1 ) )
			break;
	
	int Status = tPacket.ReadNum(1);

	//Check to make sure we logged in.  If not, throw an exception meaning FAILURE
	if ( Status == SQL_OK )
		m_bLoggedIn = true;
	else
		throw ( ezSQLException( tPacket.ReadNum(2), tPacket.ReadStr() + "(On Login)" ) );

	if ( ResendPendingQueries )
		this->ResendStack();
	else
		this->ClearSentStack();

	m_Socket.blocking = false;
}

void ezMySQLConnection::Disconnect( )
{
	m_Socket.close();
	m_bConnected = false;
	m_bLoggedIn = false;
	string OutData;
}

void ezMySQLConnection::Ping( )
{
	ezMySQLPacket PingPacket;
	PingPacket.Clear();
	PingPacket.m_PacketNum = 0;
	PingPacket.WriteNum( PING, 1 );

	string OutData;
	PingPacket.GetOutData( OutData );
	const int OutLen = PingPacket.GetOutLength();

	m_Socket.SendData( OutData.c_str(), OutLen );
	ezSQLConnection::Ping();
}

bool ezMySQLConnection::WriteQuery( const ezSQLQuery & QueryData )
{
	ezMySQLPacket QueryPacket;

	QueryPacket.Clear();			//Let's be safe

	//Create the query packet.
	QueryPacket.m_PacketNum = 0;
	QueryPacket.WriteNum( QUERY, 1 );
	QueryPacket.WriteStr( QueryData.m_InitialQuery, false );
	
	string OutData;
	QueryPacket.GetOutData( OutData );
	const int OutLen = QueryPacket.GetOutLength();

	m_Socket.SendData( OutData.c_str(), OutLen );
	return true;
}

bool ezMySQLConnection::ReadQuery( ezSQLQuery & ResponceData )
{
//	ezSQLResult tempResult;
//
//	HUH? Why is this here, this is a major question, so I am actually
//	writing in-program comment instead of commit comment.  IT is 
//	fundamentally WRONG.  I want to understand this, if anyone figures
//	out why this is the case, tell me so.
//	static ezSQLResult tempResult;

	char data[1024];

	while ( ( m_Socket.DataAvailable() || m_Socket.blocking ) && 
		!ReadQueryPacket.PacketDone() && !m_Socket.IsError() )
	{
		if ( ReadQueryPacket.m_Size > 0 )
		{
			//Chunk in as much as we can
			int dataToMove;
			if ( 1024 > ReadQueryPacket.m_Size - ReadQueryPacket.m_Data.length() )
				dataToMove = ReadQueryPacket.m_Size - ReadQueryPacket.m_Data.length();
			else
				dataToMove = 1024;
			int moved = m_Socket.ReadData( data, dataToMove );
			ReadQueryPacket.m_Data.append( data, moved );
		}
		else
		{
			//One-byte-at-a-time
			if ( !m_Socket.ReadData( data, 1 ) )
				break;
			ReadQueryPacket.LoadByte( data[0] );
		}
	}

	//If we were kicked out beacuse of no data available don't go on!
	if ( !ReadQueryPacket.PacketDone() )
		return false;

	//Oops we're out of order!  Hurry up! Get rid of the current packet!
	if ( ( LastPacketNumber >= ReadQueryPacket.m_PacketNum ) && 
		! ( ( LastPacketNumber == 255 ) && ( ReadQueryPacket.m_PacketNum == 0 ) ) )
	{
		ResponceData.m_ResultInfo = tempResult;
		tempResult.Reset();
		ReadQueryPacket.Clear();
		LastPacketNumber = -1;
		return true;
	}
	//Update the packet numbers for ordering purposes
	LastPacketNumber = ReadQueryPacket.m_PacketNum;
	
	switch ( tempResult.State )
	{
	case ezSQLResult::STATE_BEGIN:
		{
			unsigned int Code = ReadQueryPacket.ReadNum(1);
			switch ( Code )
			{
			case 0:
				//The following is provisioned but not implemented:
				ReadQueryPacket.ReadNum(1);	//Effected Rows
				ReadQueryPacket.ReadNum(1);	//Insert ID
				ReadQueryPacket.ReadNum(2); //Autocommit or In_trans

				tempResult.isError = false;
				ResponceData.m_ResultInfo = tempResult;
				tempResult.Reset();
				ReadQueryPacket.Clear();
				LastPacketNumber = -1;
				return true;
			case 255:
				tempResult.errorNum = ReadQueryPacket.ReadNum(2);
				tempResult.errorDesc = ReadQueryPacket.ReadStr();
				tempResult.isError = true;
				ResponceData.m_ResultInfo = tempResult;
				tempResult.Reset();
				ReadQueryPacket.Clear();
				LastPacketNumber = -1;
				return true;
			default:
				//Tabular format. (Simply go away)
				//The following is provisioned but not impelemented
				ReadQueryPacket.ReadNum(1);//Num of table stuffs
				ReadQueryPacket.Clear();

				tempResult.State = ezSQLResult::STATE_HEADER;
				return false;
			}
		}
		break;
	case ezSQLResult::STATE_HEADER:
		{
			unsigned int Code = ReadQueryPacket.ReadNum(1);
			if ( Code == 254 )
			{
				tempResult.State = tempResult.ezSQLResult::STATE_FIELDS;
				ReadQueryPacket.Clear();
				return false;
			}
			
			//XXX: We need to fool the packet into putting a byte back
			//Maybe I should write "PeekNum()"?
			ReadQueryPacket.m_Location--;

			ezSQLFieldDescriptor tmp;
			string TempStr;
			
			ReadQueryPacket.GetTabularChunk( tmp.Table );
			ReadQueryPacket.GetTabularChunk( tmp.Field );
			ReadQueryPacket.GetTabularChunk( TempStr );
			tmp.Length = GetNum( TempStr );
			ReadQueryPacket.GetTabularChunk( TempStr );
			tmp.Type = ezSQLFieldTypes(GetNum( TempStr ));
			ReadQueryPacket.GetTabularChunk( TempStr );
			tmp.Flags = GetNum( TempStr );
			ReadQueryPacket.GetTabularChunk( TempStr );
			tmp.Decimals = GetNum( TempStr );
			
			tempResult.Header.push_back( tmp );
		}
		break;
	case ezSQLResult::STATE_FIELDS:
		{
			unsigned int Code = ReadQueryPacket.ReadNum(1);
			if ( Code == 254 )
			{
				ResponceData.m_ResultInfo = tempResult;
				tempResult.Reset();
				ReadQueryPacket.Clear();
				LastPacketNumber = -1;
				return true;
			}
			
			//XXX: We need to fool the packet into putting a byte back
			//Maybe I should write "PeekNum()"?
			ReadQueryPacket.m_Location--;

			vector < QString > Line;

			string TempStr;
			
			while ( ReadQueryPacket.GetTabularChunk( TempStr ) )
				Line.push_back( QString(TempStr) );
			
			tempResult.FieldContents.push_back( Line );
		}
		break;
	default:	//g++ needs this to be quiet. (We have a "Done" state)
		break;
	}

	ReadQueryPacket.Clear();

	return false;
}

//functions based off of:
//http://www.redferni.uklinux.net/mysql/MySQL-Protocol.html

void ezMySQLConnection::HashAlg( string Password, unsigned long & n1, unsigned long & n2 )
{
	n1 = 1345345333;
	long add = 7;
	n2 = 305419889;
	for (int i = 0; i < long(Password.length()); ++i) {
		char c = Password.c_str()[i];
		if ( ( c == '\t' ) || ( c == ' ' ) )
			continue;
		n1 ^= (((n1 & 63) + add) * c) + (n1 * 256);
		n2 += (n2 * 256) ^ n1;
		add += c;
	}
	n1 = n1 & 2147483647;
	n2 = n2 & 2147483647;
}

double ezMySQLConnection::RandNum( unsigned long &seed1, unsigned long &seed2 )
{
	seed1 = (seed1 * 3 + seed2) % 1073741823;
	seed2 = (seed1 + seed2 + 33) % 1073741823;
	return double(seed1) / double(1073741823);
}

string ezMySQLConnection::EncodeWithSalt( string Salt, string Password )
{
	string ret;

	unsigned long p1, p2, s1, s2;

	HashAlg( Password, p1, p2 );
	HashAlg( Salt, s1, s2 );

	unsigned long seed1 = ( p1 ^ s1 ) % 1073741823;
	unsigned long seed2 = ( p2 ^ s2 ) % 1073741823;

	int i;
	
	//Salt cannot be 100 bytes long!
	if ( Salt.length() >= 100 )
		return "";
	unsigned char msg[100];	

	for ( i = 0; i < long(Salt.length()); i++ )
	{
		msg[i] = char( 64 + (unsigned int)(RandNum( seed1, seed2 ) * 31) );
	}

	unsigned char b = (unsigned int)(RandNum( seed1, seed2 ) * 31);

	for ( i = 0; i < long(Salt.length()); i++ )
	{
		ret += msg[i] ^ b;
	}

	return ret;
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
