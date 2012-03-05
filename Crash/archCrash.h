//archCrash
//Charles Lohr 2004 Public Domain
//NO WARRANTY OR GAURENTEE OF ANY KIND PROVIDED

#ifndef _ARCH_CRASH_H
#define _ARCH_CRASH_H

#if defined(WIN32)

#ifndef WINDOWS
#define WINDOWS
#endif

#include "Windows/Crash.h"

inline void StartCrashHandler(char **argv, int argc) {
	SetUnhandledExceptionFilter( CrashHandler );
}

#elif defined(__APPLE__)
#include "Darwin/arch_setup.h"
#include "Darwin/Crash.h"

inline void StartCrashHandler(char **argv, int argc) {
	 //SetUnhandledExceptionFilter( CrashHandler );
}

#else

#include "Linux/arch_setup.h"
#include "Linux/CrashHandler.h"
#include "Linux/SignalHandler.h"
#include "Linux/AssertionHandler.h"

//Hide this
extern char ReasonForCrash[10000];
extern int PosInReason;

static bool IsFatalSignal( int signal )
{
	switch( signal )
	{
	case SIGINT:
	case SIGTERM:
	case SIGHUP:
		return false;
	default:
		return true;
	}
}
static void DoCrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc )
{
        /* Don't dump a debug file if the user just hit ^C. */
	if( !IsFatalSignal(signal) )
		return;

	CrashSignalHandler( signal, si, uc );
}

inline void StartCrashHandler( char ** argv, int argc ) {
	/* First, handle non-fatal termination signals. */
//	SignalHandler::OnClose( DoCleanShutdown );
	//+++ Add any code you want to execute on your program breaking here

	CrashHandlerHandleArgs( argc, argv );
	InitializeCrashHandler();
	SignalHandler::OnClose( DoCrashSignalHandler );

	/* Set up EmergencyShutdown, to try to shut down the window if we crash.
	 * This might blow up, so be sure to do it after the crash handler. */
	//SignalHandler::OnClose( EmergencyShutdown );
	set_terminate( UnexpectedExceptionHandler );

}

#endif

#endif

