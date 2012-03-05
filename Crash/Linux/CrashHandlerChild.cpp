#define __USE_GNU
//#include "global.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>

#include "Backtrace.h"
#include "BacktraceNames.h"

#include "CrashHandler.h"
#include "CrashHandlerInternal.h"
#include "../crashDefines.h"
//#include "ProductInfo.h"

#if defined(DARWIN)
#include "archutils/Darwin/Crash.h"
#endif

#if defined(HAVE_VERSION_INFO)
extern const unsigned version_num;
#endif

//#include <iostream> 
using namespace std; //DEBUG DEBUG
#include <execinfo.h>

CrashData crash;

const char *g_pCrashHandlerArgv0 = NULL;

using namespace std;
static void output_stack_trace( FILE *out, const void **BacktracePointers )
{
	if( BacktracePointers[0] == BACKTRACE_METHOD_NOT_AVAILABLE )
	{
		fprintf( out, "No backtrace method available.\n");
		return;
	}

	if( !BacktracePointers[0] )
	{
		fprintf( out, "Backtrace was empty.\n");
		return;
	}

    for( int i = 0; BacktracePointers[i]; ++i)
    {
        BacktraceNames bn;

	void * bob;
	bob = malloc(128);
	memcpy ( bob, (BacktracePointers[i]), 128 );
	//	char ** j = backtrace_symbols( &bob, 1);
	//cout<<"++"<<endl;
	//cout<<"--"<<j[0]<<"-"<<int(&bob)<<endl;

        bn.FromAddr( BacktracePointers[i] );
        bn.Demangle();

	/* Don't show the main module name. */
        if( bn.File == g_pCrashHandlerArgv0 )
            bn.File = "";

        if( bn.Symbol == "__libc_start_main" )
            break;

        fprintf( out, "%s\n", bn.Format().c_str() );
    }
}

#if !defined(DARWIN)
const char *SignalCodeName( int signo, int code )
{
	switch( code )
	{
	case SI_USER:    return "user signal";
	case SI_KERNEL:  return "kernel signal";
	case SI_QUEUE:   return "sigqueue signal";
	case SI_TIMER:   return "timer expired";
	case SI_MESGQ:   return "mesgq state changed";
	case SI_ASYNCIO: return "async I/O completed";
	case SI_SIGIO:   return "queued SIGIO";
	}

	switch( signo )
	{
	case SIGILL:
		switch( code )
		{
		case ILL_ILLOPC: return "illegal opcode";
		case ILL_ILLOPN: return "illegal operand";
		case ILL_ILLADR: return "illegal addressing mode";
		case ILL_ILLTRP: return "illegal trap";
		case ILL_PRVOPC: return "privileged opcode";
		case ILL_PRVREG: return "privileged register";
		case ILL_COPROC: return "coprocessor error";
		case ILL_BADSTK: return "internal stack error";
		}
		break;

	case SIGFPE:
		switch( code )
		{
		case FPE_INTDIV: return "integer divide by zero";
		case FPE_INTOVF: return "integer overflow";
		case FPE_FLTDIV: return "floating point divide by zero";
		case FPE_FLTOVF: return "floating point overflow";
		case FPE_FLTUND: return "floating point underflow";
		case FPE_FLTRES: return "floating point inexact result";
		case FPE_FLTINV: return "floating point invalid operation";
		case FPE_FLTSUB: return "subscript out of range";
		}
		break;

	case SIGSEGV:
		switch( code )
		{
		case SEGV_MAPERR:    return "address not mapped";
		case SEGV_ACCERR:    return "invalid permissions";
		}
		break;

	case SIGBUS:
		switch( code )
		{
		case BUS_ADRALN: return "invalid address alignment";
		case BUS_ADRERR: return "non‐existent physical address";
		case BUS_OBJERR: return "object specific hardware error";
		}
		break;

	case SIGTRAP:
		switch( code )
		{
		case TRAP_BRKPT: return "process breakpoint";
		case TRAP_TRACE: return "process trace trap";
		}
		break;

	case SIGCHLD:
		switch( code )
		{
		case CLD_EXITED: return "child has exited";
		case CLD_KILLED: return "child was killed";
		case CLD_DUMPED: return "child terminated abnormally";
		case CLD_TRAPPED: return "traced child has trapped";
		case CLD_STOPPED: return "child has stopped";
		case CLD_CONTINUED: return "stopped child has continued";
		}
		break;
		
	case SIGPOLL:
		switch( code )
		{
		case POLL_IN:  return "data input available";
		case POLL_OUT: return "output buffers available";
		case POLL_MSG: return "input message available";
		case POLL_ERR: return "i/o error";
		case POLL_PRI: return "high priority input available";
		case POLL_HUP: return "device disconnected";
		}
		break;
	}

	static char buf[128];
	sprintf( buf, "Unknown code %i", code );
	return buf;
}
#endif

bool child_read( int fd, void *p, int size )
{
	char *buf = (char *) p;
	int got = 0;
	while( got < size )
	{
		int ret = read( fd, buf+got, size-got );
		if( ret == -1 )
		{
			if( errno == EINTR )
				continue;
			fprintf( stderr, "Crash handler: error communicating with parent: %s\n", strerror(errno) );
			return false;
		}

		if( ret == 0 )
		{
			fprintf( stderr, "Crash handler: EOF communicating with parent\n", strerror(errno) );
			return false;
		}

		got += ret;
	}

	return true;
}

/* Once we get here, we should be * safe to do whatever we want;
* heavyweights like malloc and MString are OK. (Don't crash!) */
static void child_process()
{
	//cout<<"in process"<<endl;
    /* 1. Read the CrashData. */
//    if( !child_read(3, &crash, sizeof(CrashData)) )
	///WOOT FOR HACKS

	//return;

	const char *home = getenv( "HOME" );
	MString sCrashInfoPath = "/tmp";
	if( home )
		sCrashInfoPath = home;
	sCrashInfoPath += "/crashinfo.txt";

	FILE *CrashDump = fopen( sCrashInfoPath, "w+" );
	if(CrashDump == NULL)
	{
		fprintf( stderr, "Couldn't open " + sCrashInfoPath + ": %s\n", strerror(errno) );
		exit(1);
	}

    fprintf(CrashDump, "%s crash report", PRODUCT_NAME_VER );
#if defined(HAVE_VERSION_INFO)
    fprintf(CrashDump, " (build %u)", version_num);
#endif
    fprintf(CrashDump, "\n");
    fprintf(CrashDump, "--------------------------------------\n");
    fprintf(CrashDump, "\n");

    MString reason;
    switch( crash.type )
    {
    case CrashData::SIGNAL:
    {
	MString Signal = SignalName( crash.signal );

#if !defined(DARWIN)
	reason = ssprintf( "%s - %s", Signal.c_str(), SignalCodeName(crash.signal, crash.si.si_code) );
#else
	reason = Signal;
#endif
	switch( crash.signal )
	{
	case SIGILL:
	case SIGFPE:
	case SIGSEGV:
	case SIGBUS:
	    if( crash.si.si_code == SI_USER )
		    reason += ssprintf( " from pid %li", (long) crash.si.si_addr );
	    else
		    reason += ssprintf( " at 0x%0*lx", int(sizeof(void*)*2), (unsigned long) crash.si.si_addr );
	}
	break;
    }
    case CrashData::FORCE_CRASH:
	crash.reason[ sizeof(crash.reason)-1] = 0;
	reason = crash.reason;
	break;
    }

    fprintf( CrashDump, "Crash reason: %s\n", reason.c_str() );
    //fprintf( CrashDump, "Crashed thread: %s\n\n", CrashedThread.c_str() );

//    fprintf(CrashDump, "Checkpoints:\n");
//    for (unsigned i=0; i<Checkpoints.size(); ++i)
//        fprintf(CrashDump, Checkpoints[i]);
//    fprintf(CrashDump, "\n");

    for( int i = 0; i < CrashData::MAX_BACKTRACE_THREADS; ++i )
    {
	    if( !crash.BacktracePointers[i][0] )
		    break;
	    fprintf( CrashDump, "Thread: %s\n", crash.m_ThreadName[i] );
	    output_stack_trace( CrashDump, crash.BacktracePointers[i] );
	    fprintf(CrashDump, "\n");
    }

    fprintf(CrashDump, "Static log:\n");
	//+++ add static log here
    fprintf(CrashDump, "-- End of report\n");
    fclose(CrashDump);

#if defined(DARWIN)
    InformUserOfCrash( sCrashInfoPath );
    
    /* Forcibly kill our parent. */
    kill( getppid(), SIGKILL );
#else
    /* stdout may have been inadvertently closed by the crash in the parent;
     * write to /dev/tty instead. */
    FILE *tty = fopen( "/dev/tty", "w" );
    if( tty == NULL )
        tty = stderr;

    fprintf(tty,
            "\n"
            PRODUCT_NAME " has crashed.  Debug information has been output to\n"
            "\n"
            "    " + sCrashInfoPath + "\n"
            "\n"
            "Please report a bug at:\n"
            "\n"
            "    http://sourceforge.net/tracker/?func=add&group_id=121611&atid=690872"
            "\n"
            );
#endif
}


void CrashHandlerHandleArgs( int argc, char* argv[] )
{
	g_pCrashHandlerArgv0 = argv[0];
	if( argc == 2 && !strcmp(argv[1], CHILD_MAGIC_PARAMETER) )
	{
		child_process();
		exit(0);
	}
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
