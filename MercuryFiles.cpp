#include "MercuryFiles.h"
//#include "MercuryTheme.h"

//For the store compression on zips
//#include <zlib.h>

#include "global.h"

#if !defined(WIN32)
#include <dirent.h>
#endif

#if defined(_EE)
#define FSHEADER "host:"
#else
#define FSHEADER ""
#endif

MercuryFileManager FILEMAN;

const MString PackagePrefix = "Packages/";
const MString MemoryFileName = "memory.zip";

/********************FILE DRIVER DIRECT*******************/
//Core base only.
MercuryFile::MercuryFile()
{
	//No code
}

MercuryFile::~MercuryFile()
{
	//No code
}

bool MercuryFile::Init( const MString &sPath, FilePermission p )
{
	m_sPath = sPath;
	m_p = p;
	return true;
}

bool MercuryFile::ReadLine( MString & data )
{
	data = "";
	char b1[1];
	bool Success;
	while ( Success = ( Read( b1, 1 ) > 0 ) )
	{
		if ( ( b1[0] == '\r' ) || ( b1[0] == '\n' ) )
			break;
		data += b1[0];
	}

	//We're using windows, check to make sure we don't have an extra \n.
	if ( b1[0] == '\r' )
	{
		unsigned long loc = Tell();
		Read( b1, 1 );
		if ( b1[0] != '\n' )
			Seek(loc);
	}

	return Success;
}

MercuryFileObjectDirect::MercuryFileObjectDirect( ):
	MercuryFile( )
{
	m_fF = NULL;
}

MercuryFileObjectDirect::~MercuryFileObjectDirect()
{
	if ( m_fF != NULL )
	{
		fclose ( m_fF );
		m_fF = NULL;
	}
}

void MercuryFileObjectDirect::Close()
{
	if ( m_fF != NULL )
	{
		fclose ( m_fF );
		m_fF = NULL;
	}
	delete this;
}

bool MercuryFileObjectDirect::Init( const MString & fName, FilePermission p )
{
	MercuryFile::Init( fName, p );
	if ( m_fF != NULL )
		SAFE_DELETE( m_fF );

	switch ( p )
	{
	case MFP_READ_ONLY:
		m_fF = fopen( (FSHEADER+fName).c_str(), "rb" );
		break;
	case MFP_WRITE_ONLY:
		m_fF = fopen( (FSHEADER+fName).c_str(), "wb" );
		break;
	case MFP_READ_AND_WRITE:
		m_fF = fopen( (FSHEADER+fName).c_str(), "wb+" );
		break;
	case MFP_APPEND:
		m_fF = fopen( (FSHEADER+fName).c_str(), "a" );
		break;
	default:
		m_fF = NULL;
		break;
	}

	return ( m_fF != NULL );
}

bool MercuryFileObjectDirect::Seek( unsigned long position )
{
	if ( m_fF == NULL )
		return false;
	return ( fseek( m_fF, position, SEEK_SET ) == 0 );
}

unsigned long MercuryFileObjectDirect::Tell()
{
	if ( m_fF == NULL )
		return false;
	return ftell( m_fF );
}

unsigned long MercuryFileObjectDirect::Length()
{
	if ( m_fF == NULL )
	  return false;
	unsigned long prev = ftell( m_fF );
	fseek( m_fF, 0, SEEK_END );
	unsigned long ret = ftell( m_fF );
	fseek( m_fF, prev, SEEK_SET );
	return ret;
}

bool MercuryFileObjectDirect::Write( void * data, unsigned long length )
{
	if ( m_fF == NULL )
		return false;
	return ( fwrite( data, length, 1, m_fF ) > 0 );
}

unsigned long MercuryFileObjectDirect::Read( void * data, unsigned long length )
{
	if ( m_fF == NULL )
		return false;
	return ( fread( data, 1, length, m_fF) );
}

bool MercuryFileObjectDirect::Check()
{
	if ( m_fF == NULL )
		return true;
	return (ferror( m_fF )!=0);
}

bool MercuryFileObjectDirect::Eof()
{
	if ( m_fF == NULL )
		return false;
	return (feof( m_fF )!=0);
}

MercuryFile * MercuryFileDirverDirect::GetFileHandle( const MString & sPath, FilePermission p )
{
	MercuryFile * ret = new MercuryFileObjectDirect;
	if ( ret->Init( sPath, p ) )
		return ret;

	SAFE_DELETE(ret);

	return NULL;
}

#if defined(WIN32)
#include <windows.h>
#endif

void MercuryFileDirverDirect::ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly )
{
#if defined(WIN32)
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile( sPath, &fd );
	if( INVALID_HANDLE_VALUE == hFind )		// no files found
		return;
	do
	{
		if( bDirsOnly  &&  !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			continue;	// skip

		if( (!bDirsOnly)  &&  (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			continue;	// skip

		MString sDirName( fd.cFileName );

		if( sDirName == "."  ||  sDirName == ".." )
			continue;

		output.push_back( sDirName );
	} while( ::FindNextFile( hFind, &fd ) );
	::FindClose( hFind );
#elif !defined(_EE)
	DIR * pDe = opendir( "./" + sPath );
	if ( pDe == NULL )
		return;

	dirent * pEnt;

	while ( pEnt = readdir( pDe ) )
	{
		if ( ( pEnt->d_type & DT_DIR ) && !bDirsOnly )
			continue;
		if ( !( pEnt->d_type & DT_DIR ) && bDirsOnly )
			continue;

		if ( strcmp( pEnt->d_name, "." ) == 0 )
			continue;

		if ( strcmp( pEnt->d_name, ".." ) == 0 )
			continue;

		output.push_back( pEnt->d_name );
	}

	closedir( pDe );
#endif
}

/********************FILE DRIVER PACKAGE*******************/

MercuryFileObjectPacked::~MercuryFileObjectPacked()
{
	Close();
}

bool MercuryFileObjectPacked::Init( const MString & fName, 
	FilePermission p, const MString & base, unsigned offset, 
	unsigned size )
{
	if ( p == MFP_APPEND )
		return false;			//Not supported.

	m_sPath = fName;
	m_p = p;
	m_base = FILEMAN.Open( base, p );
	if ( m_base->Check() )
		return false;
	m_offset = offset;
	m_size = size;
	m_location = 0;

	return m_base->Seek( m_offset );
}

bool MercuryFileObjectPacked::Seek( unsigned long position )
{
	if ( position >= m_size )
		return false;
	m_location = position;
	return m_base->Seek( m_location + m_offset );
}

unsigned long MercuryFileObjectPacked::Tell()
{
	return m_location;
}

unsigned long MercuryFileObjectPacked::Length()
{
	return m_size;
}

bool MercuryFileObjectPacked::Write( void * data, unsigned long length )
{
	//First make sure we won't over-write good data.
	if ( length >= ( m_size - m_location ) )
		return false;

	bool ret = m_base->Write( data, length );
	if ( ret )
		m_location += length;
	return ret;
}

unsigned long MercuryFileObjectPacked::Read( void * data, unsigned long length )
{
	int readable = m_size - m_location;
	if ( (unsigned)readable < length )
	{
		int ret = m_base->Read( data, readable );
		m_location += ret;
		return ret;
	}
	else 
	{
		int ret = m_base->Read( data, length );
		m_location += ret;
		return ret;
	}
}

bool MercuryFileObjectPacked::Check()
{
	return ( m_base != 0 );
}

bool MercuryFileObjectPacked::Eof()
{
	return m_location >= m_size;
}

void MercuryFileObjectPacked::Close()
{
	SAFE_DELETE( m_base );
}

inline unsigned long ToLong(char * inx)
{
	unsigned char *in = (unsigned char*)inx;
	return in[0] + long(in[1])*256 + long(in[2])*65536 + long(in[3])*16777216;
}

void MercuryFileDriverPacked::Init()
{
	std::vector< MString > out;
	FILEMAN.ListDirectory( PackagePrefix+"*.*", out, false );
	for ( unsigned i = 0; i < out.size(); i++ )
	{
		MercuryFile * z = FILEMAN.Open( PackagePrefix+out[i] );
		
		unsigned long NumberOfFiles = 0;
		char tmp[256];

		if ( !z->Eof() && !z->Check() )
		{
			z->Read( tmp, 4 );
			if ( strncmp( tmp, "HGPK", 4 ) != 0 )
				continue;
			z->Read( tmp, 128 );
			z->Read( tmp, 4 );
			NumberOfFiles = ToLong( tmp );
		}

		for ( unsigned j = 0; j < NumberOfFiles; j++ )
		{
			MString basename;
			MString path;
			MString name;
			unsigned long filesize;
			unsigned long namelen;
			unsigned long offset;
			if ( z->Eof() || z->Check() )
				break;
			z->Read( tmp, 4 );
			filesize = ToLong( tmp );
			z->Read( tmp, 4 );
			offset = ToLong( tmp );
			z->Read( tmp, 4 );
			namelen = ToLong( tmp );
			z->Read( tmp, namelen );
			tmp[namelen] = '\0';
			name = tmp;
			//First, strip out folders.
			MString LastFolder;
			MString CurrentElement;
			for ( int k = 0; (unsigned)k < namelen; k++ )
			{
				if( tmp[k] == '/' )
				{
					PckFileEntry y;
					y.m_bFolder = true;
					//Other values don't apply.
					m_mFileFolders[LastFolder][CurrentElement] = y;
					LastFolder += CurrentElement + "/";
					CurrentElement = "";
				}
				else
					CurrentElement+=tmp[k];
			}
			PckFileEntry y;
			y.m_bFolder = false;
			y.m_sPackageName = PackagePrefix+out[i];
			y.m_iOffset = offset;
			y.m_iSize = filesize;
			m_mFileFolders[LastFolder][CurrentElement] = y;
		}

		z->Close();
		SAFE_DELETE( z );
	}
}

MercuryFile * MercuryFileDriverPacked::GetFileHandle( const MString & sPath, FilePermission p )
{
	if ( sPath.length() == 0 )
		return NULL;

	//First break up 
	const char * sp = sPath.c_str();
	unsigned int i;
	for ( i = sPath.length()-1; i > 0; i-- )
		if ( sp[i] == '/' )
			break;

	MString ActualName;
	if (i >0)
	    ActualName = sPath.substr(i+1);
	else
	    ActualName = sPath;

	MString Path;
	if (i>0)
	  Path = sPath.substr(0,i+1);
	else
	  Path = "";

	if ( m_mFileFolders.find( Path ) == m_mFileFolders.end() )
		return NULL;

	if ( m_mFileFolders[Path].find( ActualName ) == m_mFileFolders[Path].end() )
		return NULL;

	PckFileEntry z = m_mFileFolders[Path][ActualName];

	if ( z.m_bFolder )
		return NULL;

	MercuryFileObjectPacked * ret = new MercuryFileObjectPacked();

	bool success = ret->Init( ActualName, p, z.m_sPackageName, z.m_iOffset, z.m_iSize );
	if ( !success )
	{
		SAFE_DELETE( ret );
		return NULL;
	}
	return ret;
}

bool FileMatch( const MString & file, const MString & regex )
{
	const char * rx = regex.c_str();
	unsigned j = 0;
	const char * fe = file.c_str();
	unsigned i;
	for ( i = 0; i < regex.length() && j < file.length(); i++ )
	{
		switch ( rx[i] )
		{
		case '*':
			{
				i++;
				if ( i == regex.length() )
					return true;

				while ( ( j < file.length() ) && 
					( fe[j] != rx[i] ) )
					j++;

				j++;
				if ( j == file.length() )
					return false;
			}
			break;
		case '?':
			j++;
			break;
		default:

			if ( fe[j] != rx[i] )
				return false;
			j++;
			break;
		}
	}
	if ( ( j >= file.length() ) && !( i >= regex.length() ) )
		return false;
	return true;
}

void MercuryFileDriverPacked::ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly )
{
	const char * sp = sPath.c_str();
	unsigned i;
	for (i = sPath.length()-1; i > 0; i-- )
		if ( sp[i] == '/' )
			break;

	MString ActualName;
	if (i>0)
	    ActualName = sPath.substr( i+1 );
	else
	    ActualName = sPath;

	MString Path;
	if(i>0)
	    Path = sPath.substr( 0, i+1 );

	if ( m_mFileFolders.find( Path ) == m_mFileFolders.end() )
		return;

	std::map< MString, PckFileEntry > * cDir = &m_mFileFolders[Path];

	std::map< MString, PckFileEntry >::iterator iter, end;
	end = cDir->end();
	for ( iter = cDir->begin(); iter != end; iter++ )
	{
		MString fName = iter->first;
		PckFileEntry z = iter->second;
		if ( bDirsOnly && !z.m_bFolder )
			continue;
		if ( !bDirsOnly && z.m_bFolder )
			continue;

		if ( FileMatch( fName, ActualName ) )
			output.push_back( fName );
	}
}

/********************FILE DRIVER MEMORY********************/

#if defined ( USE_MEMFILE )
extern unsigned char MemFileData[];
#endif

void MercuryFileDriverMem::Init()
{
#if defined ( USE_MEMFILE )
	m_iFileSize = MemFileData[0] + MemFileData[1] * 0x100 + MemFileData[2] * 0x10000 + MemFileData[3] * 0x1000000;
#endif
}

MercuryFile * MercuryFileDriverMem::GetFileHandle( const MString & sPath, enum FilePermission p )
{
#if defined ( USE_MEMFILE )
	if ( sPath == MemoryFileName && p == MFP_READ_ONLY )
	{
		MercuryFileObjectZipped * ret = new MercuryFileObjectZipped;
		ret->MemFileInit( MemoryFileName, (const char *)(&MemFileData[0]) + 4, m_iFileSize );
		return ret;
	}
	else
#endif
		return NULL;
}

void MercuryFileDriverMem::ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly )
{
#if defined ( USE_MEMFILE )
	if ( !bDirsOnly && sPath == "*.*" || sPath == "*.zip" || sPath == MemoryFileName )
		output.push_back( MemoryFileName );
#endif
}

/********************FILE DRIVER ZIPPED*******************/
/*
MercuryFileObjectZipped::~MercuryFileObjectZipped()
{
	Close();
}

bool MercuryFileObjectZipped::Init( const MString & fName, 
	FilePermission p, const MString & base, unsigned offset, 
	unsigned size, unsigned packedsize, unsigned decompID )
{
	if ( p == MFP_APPEND )
		return false;			//Not supported.

	//NOTE: While we don't actually support a file opened for writing,
	//		we shouldn't fail if write permissions are requested.

	m_sPath = fName;
	m_p = p;
	MercuryFile * m_base = FILEMAN.Open( base, p );

	//Seek to the beginning of the actual data
	m_base->Seek( offset );

	char * inbuff = (char*)malloc( packedsize + 258 );
	m_Buffer = (char*)malloc( size );

	m_base->Read( inbuff + 2, packedsize + 2 );
	m_base->Close();

	z_stream_s stream;
	memset( &stream, 0, sizeof(stream) );
	stream.avail_out = size;
	stream.total_out = 0;
	stream.next_out = (unsigned char*)m_Buffer;
	stream.total_in = packedsize+2;
	stream.avail_in = packedsize+2;
	stream.next_in = (unsigned char*)inbuff;

	//Compression type 8 (Always) (These really are magic numbers here)
	inbuff[0] = 0x78;
	inbuff[1] = 0x01;

	int err = inflateInit(&stream);

	if ( err )
	{
		inflateEnd(&stream);
		free( inbuff );
		free( m_Buffer );
		m_Buffer = 0;
		return false;
	}

	err = inflate( &stream, Z_NO_FLUSH );

	if ( err < 0 )
	{
		inflateEnd(&stream);
		free( inbuff );
		free( m_Buffer );
		m_Buffer = 0;
		return false;
	}
	
	inflateEnd(&stream);

	free( inbuff );

	m_size = size;
	m_location = 0;
	m_bIsDummy = false;

	return true;
}

bool MercuryFileObjectZipped::MemFileInit( const MString & sName, const char * pData, unsigned long iSize )
{
	m_bIsDummy = true;
	//Yuck, but we're read only anyways! :)
	m_Buffer = (char*)pData;
	m_size = iSize;
	m_sPath = sName;
	m_location = 0;
	m_p = MFP_READ_ONLY;
	return true;
}

bool MercuryFileObjectZipped::Seek( unsigned long position )
{
	if ( position >= m_size )
		return false;
	m_location = position;
	return true;
}

unsigned long MercuryFileObjectZipped::Tell()
{
	return m_location;
}

unsigned long MercuryFileObjectZipped::Length()
{
	return m_size;
}

bool MercuryFileObjectZipped::Write( void * data, unsigned long length )
{
	//You cannot write to a zipped file in the store form.
	return false;
}

unsigned long MercuryFileObjectZipped::Read( void * data, unsigned long length )
{
	int readable = m_size - m_location;
	int ret;
	if ( (unsigned)readable < length )
		ret = readable;
	else 
		ret = length;
	memcpy( data, m_Buffer + m_location, ret );
	m_location += ret;
	return ret;
}

bool MercuryFileObjectZipped::Check()
{
	if ( m_Buffer )
		return 0;
	else
		return 1;
}

bool MercuryFileObjectZipped::Eof()
{
	return m_location >= m_size;
}

void MercuryFileObjectZipped::Close()
{
	if ( !m_bIsDummy )
		SAFE_DELETE( m_Buffer );
}

inline unsigned long ZIPToLong(char * inx)
{
	unsigned char *in = (unsigned char*)inx;
	return in[0] + long(in[1])*256 + long(in[2])*65536 + long(in[3])*16777216;
}

void MercuryFileDriverZipped::Init()
{
	unsigned i;
	std::vector< MString > out;
	FILEMAN.ListDirectory( PackagePrefix+"*.zip", out, false );
	for ( i = 0; i < out.size(); i++ )
		out[i] = PackagePrefix + out[i];

	FILEMAN.ListDirectory( "*.zip", out, false );
	for ( i = 0; i < out.size(); i++ )
	{
		MercuryFile * z = FILEMAN.Open( out[i] );
		
		unsigned long NumberOfFiles = 0;
		char tmp[512];

		while ( !z->Check() && !z->Eof() )
		{
			MString basename;
			MString path;
			MString name;

			unsigned long filesize;
			unsigned long namelen;
			unsigned long packed;
			unsigned long PK;
			unsigned long PK2;

			z->Read( tmp, 2 );
			if ( tmp[0] != 'P' || tmp[1] != 'K' )
				break;

			z->Read( tmp, 4 );
			PK = ZIPToLong( tmp );

			//If we are not reading a normal entry, we don't know how to continue
			if ( ( PK % 0x10000 ) != 0x0403 )
				break;

			z->Read( tmp, 4 );
			PK2 = ZIPToLong( tmp );

			z->Read( tmp, 4 ); //Time (Throw out)
			z->Read( tmp, 4 ); //CRC32 (Throw out)

			z->Read( tmp, 4 );
			packed = ZIPToLong( tmp );

			z->Read( tmp, 4 );
			filesize = ZIPToLong( tmp );

			z->Read( tmp, 4 );
			namelen = ZIPToLong( tmp );

			z->Read( tmp, namelen );
			tmp[namelen] = '\0';
			name = tmp;

			ZipFileEntry e;

			if ( name.length() > 0 )
				if ( name.c_str()[name.length()-1] == '/' )
					continue;	//Ignore folder entries

			//First, strip out folders.
			MString LastFolder;
			MString CurrentElement;
			for ( int k = 0; (unsigned)k < namelen; k++ )
			{
				if( tmp[k] == '/' )
				{
					ZipFileEntry y;
					y.m_bFolder = true;
					//Other values don't apply.
					m_mFileFolders[LastFolder][CurrentElement] = y;
					LastFolder += CurrentElement + "/";
					CurrentElement = "";
				}
				else
					CurrentElement+=tmp[k];
			}
			ZipFileEntry y;
			y.m_bFolder = false;
			y.m_sPackageName = out[i];
			y.m_iOffset = z->Tell();
			y.m_iSize = filesize;
			y.m_iPackedSize = packed;
			y.m_pkID = PK;
			y.m_pkID2 = PK2;
			m_mFileFolders[LastFolder][CurrentElement] = y;

			z->Seek( z->Tell() + packed );
		}

		SAFE_DELETE( z );
	}
}

MercuryFile * MercuryFileDriverZipped::GetFileHandle( const MString & sPath, FilePermission p )
{
	if ( sPath.length() == 0 )
		return NULL;

	//First break up 
	const char * sp = sPath.c_str();
	unsigned int i;
	for ( i = sPath.length()-1; i > 0; i-- )
		if ( sp[i] == '/' )
			break;

	MString ActualName;
	if (i > 0)
	    ActualName = sPath.substr(i+1);
	else
	    ActualName = sPath;

	MString Path;
	if (i>0)
	  Path = sPath.substr(0,i+1);
	else
	  Path = "";

	if ( m_mFileFolders.find( Path ) == m_mFileFolders.end() )
		return NULL;

	if ( m_mFileFolders[Path].find( ActualName ) == m_mFileFolders[Path].end() )
		return NULL;

	ZipFileEntry z = m_mFileFolders[Path][ActualName];

	if ( z.m_bFolder )
		return NULL;

	bool success;

	MercuryFile * ret = NULL;

	//Zero indicates we're not actually doing compression
	if ( z.m_pkID2 / 65536 == 0 )
	{
		MercuryFileObjectPacked * tmp = new MercuryFileObjectPacked();
		success = tmp->Init( ActualName, p, z.m_sPackageName, z.m_iOffset, z.m_iSize );
		ret = tmp;
	}
	else
	{
		MercuryFileObjectZipped * tmp = new MercuryFileObjectZipped();
		success = tmp->Init( ActualName, p, z.m_sPackageName, z.m_iOffset, z.m_iSize, z.m_iPackedSize, z.m_pkID2 );
		ret = tmp;
	}

	if ( !success )
	{
		SAFE_DELETE( ret );
		return NULL;
	}
	return ret;
}

void MercuryFileDriverZipped::ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly )
{
	const char * sp = sPath.c_str();
	unsigned i;
	for (i = sPath.length()-1; i > 0; i-- )
		if ( sp[i] == '/' )
			break;

	MString ActualName;
	if (i>0)
	    ActualName = sPath.substr( i+1 );
	else
	    ActualName = sPath;

	MString Path;
	if(i>0)
	    Path = sPath.substr( 0, i+1 );

	if ( m_mFileFolders.find( Path ) == m_mFileFolders.end() )
		return;

	std::map< MString, ZipFileEntry > * cDir = &m_mFileFolders[Path];

	std::map< MString, ZipFileEntry >::iterator iter, end;
	end = cDir->end();
	for ( iter = cDir->begin(); iter != end; iter++ )
	{
		MString fName = iter->first;
		ZipFileEntry z = iter->second;
		if ( bDirsOnly && !z.m_bFolder )
			continue;
		if ( !bDirsOnly && z.m_bFolder )
			continue;

		if ( FileMatch( fName, ActualName ) )
			output.push_back( fName );
	}
}
*/
/********************FILE MANAGER**************************/

void MercuryFileManager::Init()
{
	m_bInit = true;

	m_Drivers = new std::vector< MercuryFileDriver * >;

	//Careful!  The order here ACTUALLY MATTERS
	m_Drivers->push_back( new MercuryFileDirverDirect );

	MercuryFileDriver * Mem = new MercuryFileDriverMem;
	Mem->Init();
	m_Drivers->push_back( Mem );
	
	MercuryFileDriver * Packed = new MercuryFileDriverPacked;
	Packed->Init();
	m_Drivers->push_back( Packed );

//	MercuryFileDriver * Zipped = new MercuryFileDriverZipped;
//	Zipped->Init();
//	m_Drivers->push_back( Zipped );
}

MercuryFile * MercuryFileManager::Open( const MString & sPath, FilePermission p )
{
	if( sPath.empty() )
		return NULL;
	//First check to see if we are using an associated FS
	//Currently these are only theme supported.
	/*
	if ( sPath.find( "GRAPHIC:" ) == 0 )
		return Open( THEME.GetPathToGraphic( sPath.substr( 8 ) ) );
	if ( sPath.find( "MODEL:" ) == 0 )
		return Open( THEME.GetPathToModel( sPath.substr( 6 ) ) );
	if ( sPath.find( "FILE:" ) == 0 )
		return Open( THEME.GetPathToFile( sPath.substr( 5 ) ) );
		*/
	CheckInit();

	MercuryFile * ret;
	for ( unsigned int i = 0; i < m_Drivers->size(); i++ )
	{
		ret = m_Drivers->at(i)->GetFileHandle( sPath, p );
		if ( ret != NULL )
			return ret;
	}
	return NULL;
}

void MercuryFileManager::ListDirectory( const MString & sPath, std::vector< MString > & output, bool bDirsOnly )
{
	CheckInit();
	for ( unsigned int i = 0; i < m_Drivers->size(); i++ )
	{
		if ( sPath.substr(0,1).compare("/") )
			m_Drivers->at(i)->ListDirectory( sPath, output, bDirsOnly );
		else
			m_Drivers->at(i)->ListDirectory( sPath.substr(1), output, bDirsOnly );
	}

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

