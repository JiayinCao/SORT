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
#include "platform/multithread/multithread.h"

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

// spawn new thread
PlatformThreadUnit*	SpawnNewRenderThread(unsigned int tid)
{
	return new PlatformThreadUnit(tid);
}

// push render task
void PushRenderTask( const RenderTask& renderTask )
{
	RenderTaskQueue::GetSingleton().PushTask( renderTask );
}