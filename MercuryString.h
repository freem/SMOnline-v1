#ifndef _MERCURY_STRING_H
#define _MERCURY_STRING_H

class MString
{
public:
	MString();
	MString( int iPreAlloc );
	MString( const char sIn );
	MString( const char * sIn );
	MString( const char * sIn, int iSize );
	MString( const MString & rhs );
	~MString();

	const MString & operator = ( const MString & rhs );
	const MString & operator = ( const char * rhs );
	const MString operator + ( const MString & rhs ) const;
	const MString operator + ( const char * rhs ) const;
	const MString operator + ( const char rhs ) const;
	const MString & operator += ( const char * rhs );
	const MString & operator += ( const MString & rhs );
	const MString & operator += ( const char rhs );
	
	bool operator == ( const MString & rhs );
	bool operator == ( const char * rhs );
	bool operator < ( const MString & rhs );
	bool operator > ( const MString & rhs );

	operator const char * () const { return m_sCur; }

	inline const char * c_str() const { return m_sCur; }
	inline unsigned long length() const { return m_iLen; }
	inline unsigned long size() const { return m_iLen; }
	inline bool empty() const { return m_iLen == 0; }

	void append( const MString & app );
	void append( const char app );
	void append( const char * app );
	void append( const char * app, int len );

	///Special: Append app len times
	void append( const char app, int len );

	void assign( const MString & app );
	void assign( const char * app );
	void assign( const char * app, int len );

	int find( const MString & tofind, int start = 0 ) const;
	int rfind( const MString & tofind ) const; 
	inline int find_last_of( const MString & tofind ) const { return rfind( tofind ); }

	int find( const char * tofind,int start = 0 ) const;
	int find( const char tofind, int start = 0 ) const;
	int rfind( const char * tofind ) const;
	int rfind( const char tofind ) const;
	inline int find_last_of( const char * tofind ) const { return rfind( tofind ); }

	const MString substr( int iStart ) const;
	const MString substr( int iStart, int iLength ) const;

	int compare( const MString & cmp ) const;
	int compare( const char * cmp ) const;

	int compare( int start, int len, const MString & cmp ) const;
	int compare( int start, int len, const char * cmp ) const;

	enum 
	{
		npos = -1
	};
private:
	char *	m_sCur;
	unsigned int	m_iLen;
	unsigned int	m_iAlloc;
	friend bool operator < ( const MString & lhs, const MString & rhs );
	friend bool operator > ( const MString & lhs, const MString & rhs );
};

/* //Keep around incase we need to debug a little and use our old string
#include "MercuryString.h"
typedef StdString::CStdString MString;
*/

bool operator < ( const MString & lhs, const MString & rhs );
bool operator > ( const MString & lhs, const MString & rhs );
inline bool operator == ( const MString & lhs, const char * rhs ) { return lhs.compare( rhs ) == 0; }
inline bool operator != ( const MString & lhs, const char * rhs ) { return lhs.compare( rhs ) != 0; }

MString operator + ( const char lhs, const MString & rhs );
MString operator + ( const char * lhs, const MString & rhs );

typedef MString MString;

#endif

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
