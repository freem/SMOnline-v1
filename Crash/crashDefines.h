//File global to a lot of the crash stuff
#include <vector>

#define PRODUCT_NAME_VER	"CVS Build"
#define PRODUCT_NAME		"SMOnline"

#define VERSION_NUM			1000

#define BACKTRACE_LOOKUP_METHOD_DLADDR 
#define HAVE_CXA_DEMANGLE

#ifndef WIN32 
#if __WORDSIZE == 64
#define CPU_X86_64
#endif
#endif

#ifndef CPU_X86_64
#define CPU_X86
#endif
//#define CPU_X86_64

#define CRASH_HANDLER



#if !defined (WIN32)
#define LINUX
#endif

#ifndef ALREADY_DEFINED_TYPES
#define ALREADY_DEFINED_TYPES

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
/*
#if ( !defined (__int64) && defined(LINUX) )
#define __int64 long long
#endif


typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
*/

#if defined( WIN32 ) && !defined(__GNUC__)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#ifndef CPU_X86_64
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif
#endif




#if defined(_MSC_VER)
#define NORETURN __declspec(noreturn)
#elif defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5 ))
#define NORETURN __attribute__ ((__noreturn__))
#else
#define NORETURN
#endif


#if defined(__GNUC__)
#define PRINTF(a,b) __attribute__((format(__printf__,a,b)))
#else
#define PRINTF(a,b)
#endif

#if defined (WIN32)
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#endif

#include "../MercuryString.h"

MString vssprintf( const char *fmt, va_list argList);

using namespace std;
// Splits a MString into an MStringArray according the Delimitor.
void split( const MString &Source, const MString &Delimitor, vector< MString > & AddIt, const bool bIgnoreEmpty = true );
//void split( const wstring &Source, const wstring &Delimitor, vector<wstring> &AddIt, const bool bIgnoreEmpty = true );

/* In-place split. */
void split( const MString &Source, const MString &Delimitor, int &begin, int &size, const bool bIgnoreEmpty = true );
//void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, const bool bIgnoreEmpty = true );

/* In-place split of partial string. */
void split( const MString &Source, const MString &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty ); /* no default to avoid ambiguity */
//void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty );


MString ssprintf( const char *fmt, ...) PRINTF(1,2);


#endif

