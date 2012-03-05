#ifndef _MERCURY_FILES_H
#define _MERCURY_FILES_H

//#include "global.h"
#include <vector>
#include <map>
#include "MercuryString.h"

/****************************************************************/
//File Objects:

///File permissions (self-explainatory)
enum FilePermission
{
	MFP_READ_ONLY,
	MFP_WRITE_ONLY,
	MFP_READ_AND_WRITE,
	MFP_APPEND,
};

///Base file object for accessing files
class MercuryFile
{
public:
	MercuryFile();
	virtual ~MercuryFile();
	///[internal] Initialize the file
	virtual bool Init( const MString & sPath, FilePermission p );
	///Close the file
	virtual void Close() = 0;
	///Set the current seek position
	virtual bool Seek( unsigned long position ) = 0;
	///Return the current seek position
	virtual unsigned long Tell() = 0;
	///Return the length of the file (in bytes)
	virtual unsigned long Length() = 0;
	///Writes length bytes, returns true if it wrote successfully
	virtual bool Write( void * data, unsigned long length ) = 0;
	///Returns the number of bytes read
	virtual unsigned long Read( void * data, unsigned long length ) = 0;
	///Reads one line from the file.
	virtual bool ReadLine( MString & data );
	///Returns non-zero value if there's an error
	virtual bool Check() = 0;
	///Return true if end of file
	virtual bool Eof() = 0;

	MString GetName() { return m_sPath; }
protected:
	MString m_sPath;
	FilePermission m_p;
};

///[internal] File object that access the normal file system
class MercuryFileObjectDirect: public MercuryFile
{
public:
	MercuryFileObjectDirect();
	virtual ~MercuryFileObjectDirect();

	virtual bool Init( const MString & fName, FilePermission p );
	virtual bool Seek( unsigned long position );
	virtual void Close();
	virtual unsigned long Tell();
	virtual unsigned long Length();
	virtual bool Write( void * data, unsigned long length );
	virtual unsigned long Read( void * data, unsigned long length );
	virtual bool Check();
	virtual bool Eof();
private:
	FILE * m_fF;
	unsigned long length;
};

///[internal] File object that accesses packed files in the Packages/ folder
class MercuryFileObjectPacked: public MercuryFile
{
public:
	~MercuryFileObjectPacked();
	virtual bool Init( const MString & fName, FilePermission p, const MString & base, unsigned offset, unsigned size );
	virtual bool Seek( unsigned long position );
	virtual void Close();
	virtual unsigned long Tell();
	virtual unsigned long Length();
	virtual bool Write( void * data, unsigned long length );
	virtual unsigned long Read( void * data, unsigned long length );
	virtual bool Check();
	virtual bool Eof();
private:
	MercuryFile * m_base;
	unsigned long m_offset;
	unsigned long m_size;
	unsigned long m_location;
};
/*
///[internal] File object that accesses zipped files in the root or Packed/ folder
class MercuryFileObjectZipped: public MercuryFile
{
public:
	~MercuryFileObjectZipped();
	virtual bool Init( const MString & fName, FilePermission p, const MString & base, unsigned offset, unsigned size, unsigned packedsize, unsigned decompID );
	virtual bool MemFileInit( const MString & sName, const char * pData, unsigned long iSize );
	virtual bool Seek( unsigned long position );
	virtual void Close();
	virtual unsigned long Tell();
	virtual unsigned long Length();
	virtual bool Write( void * data, unsigned long length );
	virtual unsigned long Read( void * data, unsigned long length );
	virtual bool Check();
	virtual bool Eof();
private:
	unsigned long m_size;
	unsigned long m_location;

	bool m_bIsDummy;

	//Used for the file dump buffer as well as the file itself when done
	char * m_Buffer;
};
*/
/****************************************************************/
//File Drivers:

///[internal] Base file driver
class MercuryFileDriver
{
public:
	virtual void Init() { }
	virtual MercuryFile * GetFileHandle( const MString & sPath, FilePermission p  ) = 0;
	virtual void ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly ) = 0;
};

///[internal] File driver that accesses files on the hard drive
class MercuryFileDirverDirect : public MercuryFileDriver
{
public:
	virtual MercuryFile * GetFileHandle( const MString & sPath, FilePermission p  );
	virtual void ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly );
};

///[internal] File driver that accesses files in packed files
class MercuryFileDriverPacked : public MercuryFileDriver
{
public:	
	virtual void Init();
	virtual MercuryFile * GetFileHandle( const MString & sPath, FilePermission p  );
	virtual void ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly );
private:
	///An entry in a packed file
	struct PckFileEntry
	{
		bool m_bFolder;
		MString m_sPackageName;
		unsigned long m_iOffset;
		unsigned long m_iSize;
	};
	std::map< MString, std::map< MString, PckFileEntry > > m_mFileFolders;
};
/*
///[internal] File driver that accesses files in a zip file
class MercuryFileDriverZipped : public MercuryFileDriver
{
public:	
	virtual void Init();
	virtual MercuryFile * GetFileHandle( const MString & sPath, FilePermission p  );
	virtual void ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly );
private:
	///An entry in a packed file
	struct ZipFileEntry
	{
		bool m_bFolder;
		MString m_sPackageName;
		unsigned long m_iOffset;
		unsigned long m_iSize;

		unsigned long m_iPackedSize;
		unsigned long m_pkID;
		unsigned long m_pkID2;
	};
	std::map< MString, std::map< MString, ZipFileEntry > > m_mFileFolders;
};
*/
class MercuryFileDriverMem : public MercuryFileDriver
{
public:
	virtual void Init();
	virtual MercuryFile * GetFileHandle( const MString & sPath, enum FilePermission p );
	virtual void ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly );
	int m_iFileSize;
};

/****************************************************************/
//File Manager:

///The Mercury File Manager -- accessable via FILEMAN.
class MercuryFileManager
{
public:
	///[internal] Initialize the file manager
	void Init();
	///[internal] Check to see if our static-time driver has been initalized
	inline void CheckInit() { if ( m_bInit ) return; Init(); }

	///Open a file by name and permission type.  This returns a MercuryFile.
	MercuryFile * Open( const MString & sPath, FilePermission p = MFP_READ_ONLY );
	///List the contents of a directory, use DOS-style wildcards here.
	void ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly=false );
private:
	///[internal] Has the DB been initialized
	bool m_bInit;
	std::vector< MercuryFileDriver * > * m_Drivers;
};

extern MercuryFileManager FILEMAN;

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
