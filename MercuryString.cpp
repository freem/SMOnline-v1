#include "MercuryString.h"
#include <stdlib.h>
#include <string.h>

#define BASE_ALLOC 16

#define NEXT_ALLOC( x, y )  ( x + y + BASE_ALLOC )

#define MANAGE_ALLOC( y ) \
	if ( m_iLen + y + 1 > m_iAlloc ) \
	{ \
		int iOldMalloc = m_iAlloc; \
		m_iAlloc = NEXT_ALLOC( m_iLen, y ); \
		char * newStr = (char*)malloc( m_iAlloc ); \
		memcpy( newStr, m_sCur, m_iLen + 1 ); \
		free( m_sCur ); \
		m_sCur = newStr; \
	}

MString::MString()
{
	m_sCur = (char*)malloc( BASE_ALLOC );
	m_sCur[0] = '\0';
	m_iLen = 0;
	m_iAlloc = BASE_ALLOC;
}

MString::MString( int iPreAlloc )
{
	m_sCur = (char*)malloc( iPreAlloc );
	m_sCur[0] = '\0';
	m_iLen = 0;
	m_iAlloc = iPreAlloc;
}

MString::MString( const char sIn )
{
	m_iLen = 1;
	m_iAlloc = 2;
	m_sCur = (char*)malloc( m_iAlloc );
	m_sCur[0] = sIn;
	m_sCur[1] = '\0';
}

MString::MString( const char * sIn )
{
	m_iLen = strlen( sIn );
	m_iAlloc = m_iLen + 1;
	m_sCur = (char*)malloc( m_iAlloc );
	memcpy( m_sCur, sIn, m_iAlloc );
}

MString::MString( const char * sIn, int iSize )
{
	m_iLen = iSize;
	m_iAlloc = m_iLen + 1;
	m_sCur = (char*)malloc( m_iAlloc );
	memcpy( m_sCur, sIn, m_iLen );
	m_sCur[m_iLen] = '\0';
}

MString::MString( const MString & rhs )
{
	m_iLen = rhs.m_iLen;
	m_iAlloc = rhs.m_iAlloc;
	m_sCur = (char*)malloc( m_iAlloc );
	memcpy( m_sCur, rhs.m_sCur, m_iLen );
	m_sCur[m_iLen] = '\0';
}

MString::~MString()
{
	free(m_sCur);
}

const MString & MString::operator = ( const MString & rhs )
{
	free (m_sCur);
	m_iLen = rhs.m_iLen;
	m_iAlloc = rhs.m_iAlloc;
	m_sCur = (char*)malloc( m_iAlloc );
	memcpy( m_sCur, rhs.m_sCur, m_iLen );
	m_sCur[m_iLen] = '\0';
	return (*this);
}

const MString & MString::operator = ( const char * rhs )
{
	free (m_sCur);
	m_iLen = strlen( rhs );
	m_iAlloc = m_iLen + 1;
	m_sCur = (char*)malloc( m_iAlloc );
	memcpy( m_sCur, rhs, m_iAlloc );
	return (*this);
}

const MString MString::operator + ( const MString & rhs ) const
{
	int iNextMalloc = NEXT_ALLOC( m_iLen, rhs.m_iLen );
	MString ret( iNextMalloc );
	ret.m_iLen = m_iLen + rhs.m_iLen;
	memcpy( ret.m_sCur, m_sCur, m_iLen );
	memcpy( ret.m_sCur + m_iLen, rhs.m_sCur, rhs.m_iLen );
	ret.m_sCur[ret.m_iLen] = '\0';
	return ret;
}

const MString MString::operator + ( const char * rhs ) const
{
	int iRhsLen = strlen( rhs );
	int iNextMalloc = NEXT_ALLOC( m_iLen, iRhsLen );
	MString ret( iNextMalloc );
	ret.m_iLen = m_iLen + iRhsLen;
	memcpy( ret.m_sCur, m_sCur, m_iLen );
	memcpy( ret.m_sCur + m_iLen, rhs, iRhsLen );
	ret.m_sCur[ret.m_iLen] = '\0';
	return ret;
}

const MString MString::operator + ( const char rhs ) const
{
	int iNextMalloc = NEXT_ALLOC( m_iLen, 1 );
	MString ret( iNextMalloc );
	ret.m_iLen = m_iLen + 1;
	memcpy( ret.m_sCur, m_sCur, m_iLen );
	ret.m_sCur[ret.m_iLen - 1] = rhs;
	ret.m_sCur[ret.m_iLen] = '\0';
	return ret;
}

const MString & MString::operator += ( const char * rhs )
{
	int iRhsLen = strlen( rhs );
	MANAGE_ALLOC( iRhsLen )
	memcpy( m_sCur + m_iLen, rhs, iRhsLen );
	m_iLen += iRhsLen;
	m_sCur[m_iLen] = '\0';
	return (*this);
}

const MString & MString::operator += ( const MString & rhs )
{
	MANAGE_ALLOC( rhs.m_iLen )
	memcpy( m_sCur + m_iLen, rhs.m_sCur, rhs.m_iLen );
	m_iLen += rhs.m_iLen;
	m_sCur[m_iLen] = '\0';
	return (*this);
}

const MString & MString::operator += ( const char rhs )
{
	MANAGE_ALLOC( 1 )
	m_sCur[m_iLen] = rhs;
	m_iLen++;
	m_sCur[m_iLen] = '\0';
	return (*this);
}

bool MString::operator == ( const MString & rhs )
{
	return strcmp( m_sCur, rhs.m_sCur ) == 0;
}

bool MString::operator == ( const char * rhs )
{
	return strcmp( m_sCur, rhs ) == 0;
}

bool MString::operator < ( const MString & rhs )
{
	return strcmp( m_sCur, rhs.m_sCur ) < 0;
}

bool MString::operator > ( const MString & rhs )
{
	return strcmp( m_sCur, rhs.m_sCur ) > 0;
}

void MString::append( const MString & app )
{
	MANAGE_ALLOC( app.m_iLen )
	memcpy( m_sCur + m_iLen, app.m_sCur, app.m_iLen );
	m_iLen += app.m_iLen;
	m_sCur[m_iLen] = '\0';
}

void MString::append( const char app )
{
	MANAGE_ALLOC( 1 )
	m_sCur[m_iLen] = app;
	m_iLen++;
	m_sCur[m_iLen] = '\0';
}

void MString::append( const char * app )
{
	int iRhsLen = strlen( app );
	MANAGE_ALLOC( iRhsLen )
	memcpy( m_sCur + m_iLen, app, iRhsLen );
	m_iLen += iRhsLen;
	m_sCur[m_iLen] = '\0';
}

void MString::append( const char * app, int len )
{
	MANAGE_ALLOC( len )
	memcpy( m_sCur + m_iLen, app, len );
	m_iLen += len;
	m_sCur[m_iLen] = '\0';
}

void MString::append( const char app, int len )
{
	MANAGE_ALLOC( len )
	memset( m_sCur + m_iLen, app, len );
	m_iLen += len;
	m_sCur[m_iLen] = '\0';
}

void MString::assign( const MString & app )
{
	free( m_sCur );
	m_iLen = app.m_iLen;
	m_iAlloc = app.m_iAlloc;
	m_sCur = (char*)malloc( m_iAlloc );
	memcpy( m_sCur, app.m_sCur, m_iLen );
	m_sCur[m_iLen] = '\0';
}

void MString::assign( const char * app )
{
	free( m_sCur );
	m_iLen = strlen( app );
	m_iAlloc = m_iLen + 1;
	m_sCur = (char*)malloc( m_iAlloc );
	memcpy( m_sCur, app, m_iAlloc );
}

void MString::assign( const char * app, int len )
{
	free( m_sCur );
	m_iLen = len;
	m_iAlloc = m_iLen + 1;
	m_sCur = (char*)malloc( m_iAlloc );
	memcpy( m_sCur, app, m_iLen );
	m_sCur[m_iLen] = '\0';
}

int MString::find( const MString & tofind, int start ) const
{
	const char * ret = strstr( m_sCur + start, tofind.m_sCur );
	return (ret)?(ret-m_sCur):npos;
}

int MString::rfind( const MString & tofind ) const
{
	int iLen = tofind.length();
	int iTarg = m_iLen - iLen;

	while ( iTarg >= 0 )
	{
		if ( strncmp( tofind.m_sCur+iTarg, tofind, iLen ) == 0 )
			return iTarg;
		iTarg--;
	}
	return npos;
}

int MString::find( const char * tofind, int start ) const
{
	const char * ret = strstr( m_sCur + start, tofind );
	return (ret)?(ret-m_sCur):npos;
}

int MString::rfind( const char * tofind ) const
{
	int iLen = strlen( tofind );
	int iTarg = m_iLen - iLen;

	while ( iTarg >= 0 )
	{
		if ( strncmp( m_sCur+iTarg, tofind, iLen ) == 0 )
			return iTarg;
		iTarg--;
	}
	return npos;
}

int MString::find( const char tofind, int start ) const
{
	const char * ret = strchr( m_sCur + start, tofind );
	return (ret)?(ret-m_sCur):npos;
}

int MString::rfind( const char tofind ) const
{
	const char * ret = strrchr( m_sCur, tofind );
	return (ret)?(ret-m_sCur):npos;
}

const MString MString::substr( int iStart ) const
{
	return MString( m_sCur + iStart, m_iLen - iStart );
}

const MString MString::substr( int iStart, int iLength ) const
{
	return MString( m_sCur + iStart, iLength );
}

int MString::compare( const MString & cmp ) const
{
	return strcmp( m_sCur, cmp.m_sCur );
}

int MString::compare( const char * cmp ) const
{
	return strcmp( m_sCur, cmp );
}

int MString::compare( int start, int len, const MString & cmp ) const
{
	return strncmp( m_sCur + start, cmp.m_sCur, len );
}

int MString::compare( int start, int len, const char * cmp ) const
{
	return strncmp( m_sCur + start, cmp, len );
}

bool operator < ( const MString & lhs, const MString & rhs )
{
	return strcmp( lhs.m_sCur, rhs.m_sCur ) < 0;
}

bool operator > ( const MString & lhs, const MString & rhs )
{
	return strcmp( lhs.m_sCur, rhs.m_sCur ) > 0;
}

//YUCK! This cannot be a friend function becuse of VC6 internal compiler error!
MString operator + ( const char * lhs, const MString & rhs )
{
	return MString( lhs ) + rhs;
}

MString operator + ( const char lhs, const MString & rhs ) 
{
	return MString( lhs ) + rhs;
}

/* 
 * Copyright (c) 2006, Charles Lohr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *	-	Redistributions of source code must retain the above
 *		copyright notice, this list of conditions and the following disclaimer.
 *	-	Redistributions in binary form must reproduce the above copyright
 *		notice, this list of conditions and the following disclaimer in
 *		the documentation and/or other materials provided with the distribution.
 *	-	Neither the name of the Mercury Engine nor the names of its
 *		contributors may be used to endorse or promote products derived from
 *		this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
