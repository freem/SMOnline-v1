#ifndef _EZ_MYSQL_CONSTANTS_H
#define _EZ_MYSQL_CONSTANTS_H

//NOTE: Constants taken from: http://www.redferni.uklinux.net/mysql/MySQL-Protocol.html
//Given website is in public domain and is compatible with MIT license.

#define	SQL_OK		0
#define SQL_ERROR	0xff

enum MySQLCapabilities
{
	LONG_PASSWORD	=	1,		//New more secure passwords
	FOUND_ROWS		=	2,		//Found instead of affected rows
	LONG_FLAG		=	4,		//Get all column flags
	CONNECT_WITH_DB	=	8,		//One can specify db on connect
	NO_SCHEMA		=	16,		//Don't allow database.table.column
	COMPRESS		=	32,		//Can use compression protocol
	ODBC			=	64,		//ODBC client
	LOCAL_FILES		=	128,	//Can use LOAD DATA LOCAL
	IGNORE_SPACE	=	256,	//Ignore spaces before '('
	CHANGE_USER		=	512,	//Support the mysql_change_user()
	INTERACTIVE		=	1024,	//This is an interactive client
	SSL				=	2048,	//Switch to SSL after handshake
	IGNORE_SIGPIPE	=	4096,	//IGNORE sigpipes
	TRANSACTIONS	=	8192	//Client knows about transactions
};

//						(Arguments)
enum MySQLCommand	{
	SLEEP 			=	0,	
	QUIT			=	1,	//""
	INIT_DB			=	2,	//db
	QUERY			=	3,	//query
	FIELD_LIST		=	4,	//table
	CREATE_DB		=	5,	//db
	DROP_DB			=	6,	//db
	REFRESH			=	7,	//option bits
	SHUTDOWN		=	8,	
	STATISTICS		=	9,	
	PROCESS_INFO	=	10,	
	CONNECT			=	11,	
	PROCESS_KILL	=	12,	//4-byte pid
	DEBUG			=	13,
	PING			=	14,
	TIME			=	15,
	DELAYED_INSERT	=	16,
	CMD_CHANGE_USER	=	17,	//user (Special)
	BINLOG_DUMP		=	18,
	TABLE_DUMP		=	19,
	CONNECT_OUT		=	20
};

//Field Flag	Value	Meaning
enum MySQLFieldFlags {
	NOT_NULL		=	1,		//Field can't be NULL
	PRI_KEY			=	2,		//Field is part of a primary key
	UNIQUE_KEY		=	4,		//Field is part of a unique key
	MULTIPLE_KEY	=	8,		//Field is part of a key
	FIELD_BLOB		=	16,		//Field is a blob
	UNSIGNED		=	32,		//Field is unsigned
	ZEROFILL		=	64,		//Field is zerofill
	BINARY			=	128,	//Field is binary
	ENUM			=	256,	//Field is an enum
	AUTO_INCREMENT	=	512,	//Field is an autoincrement field
	TIMESTAMP		=	1024,	//Field is a timestamp
	SET				=	2048,	//Field is a set
	NUM				=	32768	//Field is num (for clients)
};

//Status codes
enum MySQLStatusCodes {
	IN_TRANS		=	1,
	AUTOCOMMIT		=	2
};

enum MySQLCharsets {
	big5		=	1,	
	czech		=	2,	
	dec8		=	3,	
	dos			=	4,	
	german1		=	5,	
	hp8			=	6,	
	koi8_ru		=	7,	
	latin1		=	8,	
	latin2		=	9,	
	swe7		=	10,	
	usa7		=	11,	
	ujis		=	12,	
	sjis		=	13,	
	cp1251		=	14,	
	danish		=	15,	
	hebrew		=	16,
	win1251		=	17,
	tis620		=	18,
	euc_kr		=	19,
	estonia		=	20,
	hungarian	=	21,
	koi8_ukr	=	22,
	win1251ukr	=	23,
	gb2312		=	24,
	greek		=	25,
	win1250		=	26,
	croat		=	27,
	gbk			=	28,
	cp1257		=	29,
	latin5		=	30
};

const char * GetCharsetName( int cNum );

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
