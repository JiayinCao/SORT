/*
   FileName:      parallel.cpp

   Created Time:  2011-09-02 12:14:02

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "parallel.h"
#include <omp.h>
#if defined(SORT_IN_WINDOWS)
#include <windows.h>
#endif

// whether multi thread is enabled
// by default it's enabled
#if defined(SORT_DEBUG)
static bool g_bMultiThreadEnabled = false;
#else
static bool g_bMultiThreadEnabled = true;
#endif

// enable or disable multi-thread
bool	MultiThreadEnabled()
{
	return g_bMultiThreadEnabled;
}
void	SetMultiThreadEnabled( bool enabled )
{
	g_bMultiThreadEnabled = enabled;
}

// get the number of cpu cores in the system
unsigned NumSystemCores()
{
	if( g_bMultiThreadEnabled )
	{
		#if defined(SORT_IN_WINDOWS)
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			return sysinfo.dwNumberOfProcessors;
		#elif defined(SORT_IN_LINUX)
			return sysconf(_SC_NPROCESSORS_ONLN);
		#endif
	}
	return 1;
}

// set the number of threads
// thread number is the same as the number of cpu cores
unsigned SetThreadNum()
{
	if( g_bMultiThreadEnabled )
	{
		unsigned cores = NumSystemCores();
		omp_set_num_threads( cores );
		return cores;
	}
	return 1;
}

// get the thread id
unsigned ThreadId()
{
	if( g_bMultiThreadEnabled )
		return omp_get_thread_num();
	return 0;
}
