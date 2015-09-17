/*
   FileName:      parallel.h

   Created Time:  2011-09-02 12:14:03

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_PARALLEL
#define	SORT_PARALLEL

#include "sort.h"
#include "taskqueue.h"
#include "parallel.h"
#include "platform/multithread/multithread.h"

// get the number of cpu cores in the system
unsigned NumSystemCores();

// spawn new thread
PlatformThreadUnit*	SpawnNewRenderThread(unsigned tid);

// push render task
void PushRenderTask( const RenderTask& renderTask );

#endif