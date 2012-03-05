#include "global.h"
#include <vector>
#include "Crash/crashDefines.h"
#include "MercuryINI.h"
#include "MercuryFiles.h"

using namespace std;

/*	IMPORTANT NOTE ON EDITING THIS FILE:
	MercuryLog uses this file, using MercuryLog in here may
	create hazardous operating conditions! */

MercuryINI::MercuryINI( const MString &sFileName, bool bSaveOnExit )
	: m_bSaveOnExit( bSaveOnExit )
{
	Open( sFileName );
}

MercuryINI::~MercuryINI( )
{
	if ( m_bSaveOnExit )
	{
		Save();
	}
}

bool MercuryINI::Open( const MString &sFileName, bool bAddIncludes, bool bClearToStart )
{
	if ( bClearToStart )
		m_mDatas.clear();

	m_sFileName = sFileName;
	char * data;
	long len = DumpFromFile( sFileName, data );
	if ( len == -1)
		return false;
	bool ret = Load( data, len );
	free ( data );

	//Tricky: We have to load all additional files
	//		and watch all the other files in our folder.
	//		Otherwise -- we go into infinite recursion.

	if ( bAddIncludes )
	{
		vector<MString> AllFiles;
		AllFiles.clear();
		EnumerateValues( "AdditionalFiles", AllFiles );
		for ( unsigned i = 0; i < AllFiles.size(); i++ )
		{
			if ( AllFiles.size() == 0 )
				continue;
			if ( m_mLoaded.find( AllFiles[i] ) != m_mLoaded.end() )
				continue;
			m_mLoaded[AllFiles[i]] = true;
			Open( AllFiles[i] );
		}
	}
	return ret;
}

bool MercuryINI::Load( const char * sIniText, long slen )
{
	int loc = 0;
	MString key = "";
	while ( loc < slen )
	{
		long k = BytesNUntil( sIniText, " ]\r\n", loc, slen, 4 );
		loc += k;
		if ( loc >= slen )
			break;
		char tchar = sIniText[loc];
		if ( tchar == '[' )
		{
			loc++;
			k = BytesUntil( sIniText, "]\r\n", loc, slen, 3 );
			key.assign( sIniText + loc, k );
			loc += k;
		} else {
			k = BytesUntil( sIniText,"=\r\n", loc, slen, 3 );
			MString sName, sValue;
			sName.assign( sIniText + loc, k );
			loc += k;
			if ( sIniText[loc] == '=' )
			{
				loc++;
				k = BytesUntil( sIniText, "\r\n", loc, slen, 2 );
				sValue.assign( sIniText + loc, k );
				loc +=k;
			}
			m_mDatas[key][sName] = sValue;
		}
	}
	return true;
}

bool MercuryINI::Save( const MString sFileName )
{
	MString sName = sFileName;
	if ( sName.length() == 0 )
		sName = m_sFileName;
	char * data;
	long len = Dump( data );
	bool ret = DumpToFile( sName, data, len );
	if ( data != NULL )
		free ( data );
	return ret;
}

long MercuryINI::Dump( char * & toDump )
{
	MString data;
	Dump( data );
	long len = data.length();
	toDump = (char *)malloc( len );
	memcpy( toDump, data.c_str(), len );
	return len;
}

void MercuryINI::Dump( MString & data )
{
	data = "";

	//First dump out all non-keyed elements.
	std::map< MString, MString >::iterator Xiter, Xend;
	Xend = m_mDatas[""].end();
	for ( Xiter = m_mDatas[""].begin(); Xiter != Xend; Xiter++ )
	{
		data += Xiter->first + '=' + ConvertToCFormat(Xiter->second) + PROPERRETURN;
	}
	data += PROPERRETURN;


	//Traverse both maps.
	std::map< MString, std::map< MString, MString > >::iterator iter,end;
	end = m_mDatas.end();
	for ( iter = m_mDatas.begin(); iter != end; iter++ )
	{
		MString key = iter->first;
		if ( key == "" )
			continue;
		data += "[" + key + "]" + PROPERRETURN;
		std::map< MString, MString >::iterator iter, end;
		end = m_mDatas[key].end();
		for ( iter = m_mDatas[key].begin(); iter != end; iter++ )
		{
			data += iter->first + '=' + ConvertToCFormat(iter->second) + PROPERRETURN;
		}
		data += PROPERRETURN;
	}
}

void MercuryINI::EnumerateKeys( vector<MString> &keys )
{
	std::map< MString, std::map< MString, MString > >::iterator iter,end;
	end = m_mDatas.end();
	for ( iter = m_mDatas.begin(); iter != end; iter++ )
		keys.push_back( iter->first );
}

void MercuryINI::EnumerateValues( const MString &key, vector<MString> &values )
{
	std::map< MString, MString >::iterator iter, end;
	end = m_mDatas[key].end();
	for ( iter = m_mDatas[key].begin(); iter != end; iter++ )
		values.push_back( iter->first );
}

MString MercuryINI::GetValueS( const MString &key, const MString &value, MString sDefaultValue, bool bMakeValIfNoExist )
{
	MString ret;
	if ( GetValue( key, value, ret ) )
		return ret;
	else
	{
		if ( bMakeValIfNoExist )
			SetValue( key, value, sDefaultValue );
		return sDefaultValue;
	}
}

long	MercuryINI::GetValueI( const MString &key, const MString &value, long iDefaultValue, bool bMakeValIfNoExist )
{ 
	MString dat;
	if ( GetValue( key, value, dat ) )
		return atoi(dat.c_str()); 
	else
	{
		if ( bMakeValIfNoExist )
			SetValue( key, value, ssprintf("%i", iDefaultValue ) );
		return iDefaultValue;
	}
}

float	MercuryINI::GetValueF( const MString &key, const MString &value, float fDefaultValue, bool bMakeValIfNoExist )
{ 
	MString dat;
	if ( GetValue( key, value, dat ) )
		return float(atof(dat.c_str())); 
	else
	{
		if ( bMakeValIfNoExist )
			SetValue( key, value, ssprintf("%f", fDefaultValue ) );
		return fDefaultValue;
	}
}

bool	MercuryINI::GetValueB( const MString &key, const MString &value, bool bDefaultValue, bool bMakeValIfNoExist )
{
	MString dat;
	if ( GetValue( key, value, dat ) )
		return atoi(dat.c_str()) != 0; 
	else
	{
		if ( bMakeValIfNoExist )
			SetValue( key, value, ssprintf("%i", bDefaultValue ) );
		return bDefaultValue;
	}
}

bool MercuryINI::GetValue( const MString &key, const MString &value, MString &data )
{
	if ( m_mDatas.find(key) != m_mDatas.end() )
		if ( m_mDatas[key].find( value ) != m_mDatas[key].end() )
		{
			data = m_mDatas[key][value];
			return true;
		}
	data = "";
	return false;
}

void MercuryINI::SetValue( const MString &key, const MString &value, const MString& data )
{
	m_mDatas[key][value] = data;
}

bool MercuryINI::RemoveValue( const MString &key, const MString &value )
{
	if ( m_mDatas[key].find( value ) == m_mDatas[key].end() )
		return false;
	
	m_mDatas[key].erase( m_mDatas[key].find( value ) );
	return true;
}

long	DumpFromFile( const MString & filename, char * & data )
{
	data = NULL;
	MercuryFile * f = FILEMAN.Open( filename );

	if ( f == NULL )
		return -1;

	unsigned long length = f->Length();

	data = (char*)malloc( length );

	if ( f->Read( data, length ) != length )
	{
		free( data );
		data = NULL;
		SAFE_DELETE(f);
		return -1;
	}
	SAFE_DELETE(f);
	return length;
}

long	BytesNUntil( const char* strin, const char * termin, long start, long slen, long termlen )
{
	int i;
	for ( i = start; i < slen; i++ )
	{
		bool found = false;
		for ( int j = 0; j < termlen; j++ )
		{
			if ( termin[j] == strin[i] )
				found = true;
		}
		if ( !found ) 
			return i - start;
	}
	return i - start;
}

long	BytesUntil( const char* strin, const char * termin, long start, long slen, long termlen )
{
	int i;
	for ( i = start; i < slen; i++ )
		for ( int j = 0; j < termlen; j++ )
			if ( termin[j] == strin[i] )
				return i - start;
	return i - start;
}

bool	DumpToFile( const MString & filename, const char * data, long bytes )
{
	MercuryFile * f = FILEMAN.Open( filename, MFP_WRITE_ONLY );
	if ( f == NULL )
		return false;
	
	bool ret = f->Write( (void*) data, bytes );

	SAFE_DELETE(f);

	return ret;
}

MString	ConvertToCFormat( const MString & ncf )
{
	MString ret;
	const char* nccf = ncf.c_str();

	for ( int i = 0; (unsigned)i < ncf.length(); i++ )
	{
		switch ( nccf[i] )
		{
		case '\t':	ret += "\\t";	break;
		case '\n':	ret += "\\n";	break;
		case '\r':	ret += "\\r";	break;
		case '\f':	ret += "\\f";	break;
		case '\b':	ret += "\\b";	break;
		case '\\':	ret += "\\\\";	break;
		case '\'':	ret += "\\\'";	break;
		case '\"':	ret += "\\\"";	break;
		case '\0':	ret += "\\0";	break;
		default:
			ret += nccf[i];
		}
	}
	return ret;
}

/* 
 * Copyright (c) 2005-2006, Charles Lohr
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

