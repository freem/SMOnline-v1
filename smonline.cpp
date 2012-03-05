#include <iostream>
#include <time.h>
#include "global.h"
#include "concursor.h"
#include "SMOnlineServer.h"
#include "MercuryString.h"
#include "SimpleLog.h"
#include "ConstructCount.h"
#include "Crash/archCrash.h"
#include "SMOnlineStats.h"
#include "MercuryINI.h"

using namespace std;

SimpleLog* LOG = NULL;
SimpleLog* CHATLOG = NULL;
SMOnlineStats* STATS = NULL;
MercuryINI* PREFSMAN = NULL;
SMOnlineServer* SERVER = NULL;

unsigned int CPS = 0;

void LoadIni();
void KeyParse(int key);

int main(int argv, char* argc[]) {

	cout << "This is the version built on or after 09/02/2006" <<endl;
	bool useConsole = true;
	ConstructCount counter;

	StartCrashHandler(argc, argv);

#if defined ( WIN32 )
	g_CallBack = 1;	//Force safe crash handler
#endif

	int key;
	bool stop = false;

	{
		time_t st = time(NULL);
		LOG = new SimpleLog(ssprintf("Log(%d).txt", st));
		CHATLOG = new SimpleLog(ssprintf("ChatLog(%d).txt", st));
	}

	LoadIni();
	SERVER = new SMOnlineServer( PREFSMAN->GetValueI("ServerOptions", "Port", 8765, true) );
	LOG->Write("Stepmania Online Server");

	if (SERVER->UseSQL())
	{
		STATS = new SMOnlineStats;
		STATS->Connect();
		STATS->SetupAutoUpdate();
		LOG->Write("Stats thread started");
	}

	SERVER->Start(cout);
	useConsole = PREFSMAN->GetValueB("ServerOptions", "UseConsole", true, true); 
	if (useConsole)
	{
		cout << "Press ESC to stop" << endl;
		cout << "r to reload the INI" << endl;
		cout << "s to reset the server socket" << endl;
		SetKBBlocking(0);
	} else
		LOG->Write("Running with out UseConsole, ignore input from keyboard");

	while (!stop)
	{
		usleep(1000000/CPS);
		SERVER->Update();

		if (useConsole) {
			key = GetCH();
			if (key == 27)
				stop = true;
			else
				KeyParse(key);
		}
	}

	SERVER->Stop();
	SAFE_DELETE(SERVER);
	SAFE_DELETE(STATS);
	SAFE_DELETE(PREFSMAN);

	SAFE_DELETE(CHATLOG);
	SAFE_DELETE(LOG);

	cout << "Final con:" << counter.GetCon() << endl;
	cout << "Final dec:" << counter.GetDec() << endl;
	cout << "Final Diff:" << counter.GetDiff() << endl;

	if (counter.GetDiff() > 0)
		cout << "Leaked somewhere!" << endl;

	SetKBBlocking(false);

	return 0;
}

void LoadIni() {
	SAFE_DELETE(PREFSMAN);
	LOG->Write("Loading INI");
	PREFSMAN = new MercuryINI("SMOnline.ini", 1);
	CPS = PREFSMAN->GetValueI("ServerOptions", "CPS", 20, true);
}

void KeyParse(int key) {
	switch (key) {
	case int('r'):
		LoadIni();
		SERVER->LoadFromIni();
		break;
	case int('s'):
		SERVER->CycleServerSocket();
		break;
	default:
		break;
	}
}

/*
 * (c) 2003-2004 Joshua Allen
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
