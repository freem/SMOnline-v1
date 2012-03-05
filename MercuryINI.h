#ifndef MERCURY_INI_H
#define MERCURY_INI_H

#include <map>
#include <vector>
#include "MercuryString.h"

#if defined(WIN32)
#define PROPERRETURN "\r\n"
#else
#define PROPERRETURN "\n"
#endif

///Framework for INI files
class MercuryINI
{
public:
	MercuryINI( ) { m_bSaveOnExit = false; }
	MercuryINI( const MString &sFileName, bool bSaveOnExit );
	~MercuryINI( );

	///Open a INI from a file; bAddIncludes specifies whether or not to look at [AdditionalFiles], bClearToStart states whether or not all data should be erased when starting
	bool Open( const MString &sFileName, bool bAddIncludes = true, bool bClearToStart = false );
	///Load an INI from a given c-style string
	bool Load( const char * sIniText, long slen );
	///Load an INI from a given string
	bool Load( const MString &sIniText ) { return Load( sIniText.c_str(), sIniText.length() ); }

	///Save the INI file
	bool Save( const MString sFileName = "" );
	///Dump the file into the incoming MString
	void Dump( MString & data );
	///Dump the file into then c-style string, this will also allocate ram.
	long Dump( char * & toDump );

	///Enumerate all keys in this ini file into keys
	void EnumerateKeys( std::vector< MString > &keys );
	///Enumerate all values in the given key to values.
	void EnumerateValues( const MString &key, std::vector< MString > &values );
	///Enumerate all values in the given key to a map.
	void EnumerateValues( const MString &key, std::map< MString, MString > & values ) { values = m_mDatas[key]; }

	///Return string value, blank if non-existant
	MString GetValueS( const MString &key, const MString &value, MString sDefaultValue="", bool bMakeValIfNoExist=false );
	///Return long value, 0 if non-existant
	long	GetValueI( const MString &key, const MString &value, long iDefaultValue=0, bool bMakeValIfNoExist=false );
	///Return float value, 0 if non-existant
	float	GetValueF( const MString &key, const MString &value, float fDefaultValue=0, bool bMakeValIfNoExist=false );
	///Return bool value, 0 if non-existant
	bool	GetValueB( const MString &key, const MString &value, bool bDefaultValue=0, bool bMakeValIfNoExist=false );

	///Return value through data, false if non-existant
	bool GetValue( const MString &key, const MString &value, MString &data );

	///Set a value
	void SetValue( const MString &key, const MString &value, const MString &data );

	///Remove a value
	bool RemoveValue( const MString &key, const MString &value );

	///Get the last error.
	MString GetLastError() { return m_sLastError; }

private:
	///[internal] save all of the values when this ini file is destroyed
	bool	m_bSaveOnExit;
	MString	m_sLastError;
	MString m_sFileName;
	std::map< MString, std::map< MString, MString > > m_mDatas;
	std::map< MString, bool > m_mLoaded;
};

///Instantiation of mercury.ini
extern	MercuryINI * PREFSMAN;

///Check to see if Prefsman was set up.
/** If you are using PREFSMAN in a static time initiation, you should
	execute this function first. */
inline void	CheckPREFSMANSetup() { if ( PREFSMAN == NULL ) PREFSMAN = new MercuryINI( "Mercury.ini", true ); }

///Dump data into the file filename.  bytes specifies the length of data.  Returns true if successful, false else.
bool	DumpToFile( const MString & filename, const char * data, long bytes );

///Dump data from a file into a char * and return the size of data.  If -1 then there was an error opening the file.
long	DumpFromFile( const MString & filename, char * & data );

///Bytes until desired terminal
long	BytesUntil( const char* strin, const char * termin, long start, long slen, long termlen );

///Bytes until something other than a terminal
long	BytesNUntil( const char* strin, const char * termin, long start, long slen, long termlen );

///Convert string containing binary characters to C-style formatted string. 
MString	ConvertToCFormat( const MString & ncf );

#endif

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
