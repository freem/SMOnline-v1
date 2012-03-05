#include <stdlib.h>

#include "global.h"
#include "MercuryThreads.h"

#if defined( WIN32 )
#include <windows.h>
#endif

MercuryThread::MercuryThread()
{
	m_thread = NULL;
	m_pName = "(null)";
}

MercuryThread::MercuryThread( const MString &name )
{
	m_thread = NULL;
	m_pName = name;
}

MercuryThread::~MercuryThread( )
{
	Close();
}

#if defined( WIN32 )
struct StartThreadData
{
	void * (*m_pFunc)( void *pData );
	void *m_pData;
};

static DWORD WINAPI StartThread( LPVOID pData )
{
	StartThreadData * in = (StartThreadData *)pData;
	DWORD ret = (DWORD)in->m_pFunc( in->m_pData );
	delete in;
	return ret;
}
#endif

int MercuryThread::Create( void * (*fn)(void *), void *data )
{
#if defined( WIN32 )
	StartThreadData *k = new StartThreadData;
	k->m_pData = data;
	k->m_pFunc = fn;
	m_thread = CreateThread( NULL, 0, &StartThread, k, 0, NULL );
	return m_thread != NULL;
#else
	pthread_attr_t pthread_custom_attr;
	pthread_attr_init(&pthread_custom_attr);
	if ( pthread_create( &m_thread, &pthread_custom_attr, fn, data ) )
		return false;
	else
		return true;
#endif
}

int MercuryThread::Halt( bool kill )
{
#if defined( WIN32 )
	if ( kill )
		return TerminateThread( m_thread, 0 );
	else
		return SuspendThread( m_thread );
#else
	pthread_cancel( m_thread );
	return 0;
#endif
}

int MercuryThread::Resume( )
{
#if defined( WIN32 )
	return ResumeThread( m_thread );
#else
	return 0;
#endif
}

int MercuryThread::Wait( long lMilliseconds )
{
#if defined( WIN32 )
	return WaitForSingleObject( m_thread, lMilliseconds ) == 0;
#else
	pthread_join( m_thread, NULL );
	return 0;
#endif
}

void MercuryThread::Close( )
{
	if ( m_thread )
	{
		Halt( true );

#if defined( WIN32 )
		CloseHandle( m_thread );
		m_thread = NULL;
#else
#endif
	}
}

//Mutex functions
MercuryMutex::MercuryMutex( )
{
	m_pName = "(null)";
	Open( );
	UnLock();
}

MercuryMutex::MercuryMutex( const MString &name )
{
	m_pName = name;
	Open( );
	UnLock();
}

MercuryMutex::~MercuryMutex( )
{
	Close( );
}

int MercuryMutex::Wait( long lMilliseconds )
{
#if defined( WIN32 )
	return WaitForSingleObject( m_mutex, lMilliseconds );
#else
/*	timespec abstime;
	abstime.tv_sec = 0;
	abstime.tv_nsec = lMilliseconds;
	
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	pthread_cond_timedwait( &cond, &m_mutex, &abstime ); */
	pthread_mutex_lock( &m_mutex );
	return 0;
#endif
}

int MercuryMutex::UnLock( )
{
#if defined( WIN32 )
	return ReleaseMutex( m_mutex );
#else
	pthread_mutex_unlock( &m_mutex );
	return 0;
#endif
}

int MercuryMutex::Open( )
{
#if defined( WIN32 )
	SECURITY_ATTRIBUTES *p = ( SECURITY_ATTRIBUTES* ) malloc( sizeof( SECURITY_ATTRIBUTES ) );
	p->nLength = sizeof( SECURITY_ATTRIBUTES );
	p->bInheritHandle = true;
	p->lpSecurityDescriptor = NULL;
	m_mutex = CreateMutex( p, true, m_pName.c_str() );
	return (int)m_mutex;
#else
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &m_mutex, &attr );
	return 0;
#endif
}

int MercuryMutex::Close( )
{
#if defined( WIN32 )
	return CloseHandle( m_mutex );
#else
	pthread_mutex_destroy( &m_mutex );
	return 0;
#endif
}
