#if defined(WIN32)
#pragma warning(disable : 4018)	//Vector problem
#endif
#include "crashDefines.h"

#include "StdString.h"

#include <vector>
using namespace std;
typedef vector<MString> MStringArray;

MString ssprintf( const char *fmt, ...)
{
	if ( strlen(fmt)==0 )
		return MString("");

	va_list     va;
	va_start(va, fmt);
	int CurMal = FMT_BLOCK_SIZE;
	MString ret;

	while ( true )
	{
		char * base = (char*)malloc( CurMal );
		#if defined(WIN32)
		  int len = _vsnprintf( base, CurMal, fmt, va );
		#else
		  int len = vsnprintf( base, CurMal, fmt, va );
		#endif

		if (len > CurMal)
			while (CurMal < len)
				CurMal*=2;
		else
			if ( len > 0 )
			{
				ret.assign( base, len );
				free( base );
				break;
			} else
				CurMal*=2;

		free(base);
	}
	va_end( va );
	return ret;
}
   


template <class S>
static int DelimitorLength( const S &Delimitor )
{
	return Delimitor.size();
}

static int DelimitorLength( char Delimitor )
{
	return 1;
}

template <class S, class C>
void do_split( const S &Source, const C Delimitor, vector<S> &AddIt, const bool bIgnoreEmpty )
{
	size_t startpos = 0;

	do {
		size_t pos;
		pos = Source.find( Delimitor, startpos );
		if( pos == Source.npos )
			pos = Source.size();

		if( pos-startpos > 0 || !bIgnoreEmpty )
		{
			/* Optimization: if we're copying the whole string, avoid substr; this
			 * allows this copy to be refcounted, which is much faster. */
			if( startpos == 0 && pos-startpos == Source.size() )
				AddIt.push_back(Source);
			else
			{
				const S AddMString = Source.substr(startpos, pos-startpos);
				AddIt.push_back(AddMString);
			}
		}

		startpos = pos+DelimitorLength(Delimitor);
	} while ( startpos <= Source.size() );
}


void split( const MString &Source, const MString &Delimitor, MStringArray &AddIt, const bool bIgnoreEmpty )
{
	if( Delimitor.size() == 1 )
		do_split( Source, Delimitor[0], AddIt, bIgnoreEmpty );
	else
		do_split( Source, Delimitor, AddIt, bIgnoreEmpty );
}

/*void split( const wstring &Source, const wstring &Delimitor, vector<wstring> &AddIt, const bool bIgnoreEmpty )
{
	if( Delimitor.size() == 1 )
		do_split( Source, Delimitor[0], AddIt, bIgnoreEmpty );
	else
		do_split( Source, Delimitor, AddIt, bIgnoreEmpty );
}*/

/* Use:

MString str="a,b,c";
int start = 0, size = -1;
while( 1 )
{
	do_split( str, ",", begin, size );
	if( begin == str.end() )
		break;
	str[begin] = 'Q';
}

 */

template <class S>
void do_split( const S &Source, const S &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	if( size != -1 )
	{
		/* Start points to the beginning of the last delimiter.  Move it up. */
		begin += size+Delimitor.size();
		begin = min( begin, len );
	}

	size = 0;

	if( bIgnoreEmpty )
	{
		/* Skip delims. */
		while( begin + Delimitor.size() < Source.size() &&
			!Source.compare( begin, Delimitor.size(), Delimitor ) )
			++begin;
	}

	/* Where's the string function to find within a substring?  C++ strings apparently
	 * are missing that ... */
	size_t pos;
	if( Delimitor.size() == 1 )
		pos = Source.find( Delimitor[0], begin );
	else
		pos = Source.find( Delimitor, begin );
	if( pos == Source.npos || (int) pos > len )
		pos = len;
	size = pos - begin;
}

void split( const MString &Source, const MString &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, len, bIgnoreEmpty );

}

/*void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, len, bIgnoreEmpty );
}*/

void split( const MString &Source, const MString &Delimitor, int &begin, int &size, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, Source.size(), bIgnoreEmpty );
}

/*void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, Source.size(), bIgnoreEmpty );
}*/


