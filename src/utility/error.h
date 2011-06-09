/*
 * filename :	error.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_ERROR
#define	SORT_ERROR

#include <assert.h>
#include <string>
#include "../managers/logmanager.h"

// u could define SORT_DISABLE_ASSERT to disable all of the assert in sort!!!
// #define	SORT_DISABLE_ASSERT

// here we use macro NOT function to show the file name and line number where the error occurs
#ifdef SORT_DISABLE_ASSERT
	#define	SCrash(s)
	#define SAssert(b)
#else
	#define SCrash(s) { char buf[1024]; sprintf( buf , "Error: %s (File:%s  Line:%d)" , s , __FILE__ , __LINE__ ); LOG(buf); abort(); }
	#define	SAssert(b) assert(b)
#endif

#endif
