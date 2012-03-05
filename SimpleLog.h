#ifndef SIMPLELOG_H
#define SIMPLELOG_H

#include <iostream>
#include <fstream>
#include <time.h>
#include "MercuryString.h"
#include "ConstructCount.h"
#include "MercuryThreads.h"

using namespace std;

class SimpleLog
{
public:
	SimpleLog(const MString& filepath);
	~SimpleLog();
	void OpenFile(const MString& filepath);
	void CloseFile();

	template <class T>
	void Write(const T& data)
	{
		MLockPtr< ofstream > of(m_outfile, m_mutex);
		(*of) << "[" + GetTime() + "] " << data << endl;
		cout << data << endl;
	}

	template <class T>
	void QuietWrite(const T& data)
	{
		MLockPtr< ofstream > of(m_outfile, m_mutex);
		(*of) << "[" + GetTime() + "] " << data << endl;
	}
private:
	MString GetTime();

	volatile ofstream m_outfile;
	ConstructCount m_counter;

	MercuryMutex m_mutex;
};

extern SimpleLog* LOG;
extern SimpleLog* CHATLOG;

#endif

/*
 * (c) 2004-2005 Joshua Allen
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
