#include "../crashDefines.h"
#include "GetSysInfo.h"

#include <sys/utsname.h>

void GetKernel( MString &sys, int &vers )
{
	utsname uts;
	uname( &uts );

	sys = uts.sysname;
	vers = 0;

/*	if( sys == "Linux" )
	{
		//+++XXX BROKEN ! BROKEN ! BROKEN !
		static Regex ver( "([0-9]+)\\.([0-9]+)\\.([0-9]+)" );
		vector<MString> matches;
		if( ver.Compare(uts.release, matches) )
		{
			int major = atoi(matches[0]);
			int minor = atoi(matches[1]);
			int revision = atoi(matches[2]);
			vers = (major * 10000) + (minor * 100) + (revision);
		}
	}*/
}

/*
 * (c) 2003-2004 Glenn Maynard
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