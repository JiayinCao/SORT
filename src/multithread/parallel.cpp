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
#if defined(SORT_IN_WINDOWS)
#include "renderthread_win.h"
#endif

// critical section
#if defined(SORT_IN_WINDOWS)
CRITICAL_SECTION g_CS_for_counters;
extern CRITICAL_SECTION gCS;
#endif

// get the number of cpu cores in the system
unsigned NumSystemCores()
{
	#if defined(SORT_IN_WINDOWS)
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		return sysinfo.dwNumberOfProcessors;
	#elif defined(SORT_IN_LINUX) || defined(SORT_IN_MAC)
		return sysconf(_SC_NPROCESSORS_ONLN);
	#endif
}

// get the thread id
unsigned ThreadId()
{
#if defined(SORT_IN_WINDOWS)
#include "renderthread_win.h"
	return RenderThreadWin::m_WinThreadId;
#else
	return 0;
#endif
}

// spawn new thread
ThreadUnit*	SpawnNewRenderThread( unsigned int tid )
{
#if defined(SORT_IN_WINDOWS)
	return new RenderThreadWin(tid);
#else
	return 0;	// to be implemented
#endif
}

// push render task
void PushRenderTask( const RenderTask& renderTask )
{
	RenderTaskQueue::GetSingleton().PushTask( renderTask );
}

// Init Critical Sections
void InitCriticalSections()
{
	InitializeCriticalSection(&gCS);
}
// Destroy Critical Sections
void DestroyCriticalSections()
{
	DeleteCriticalSection(&gCS);
}