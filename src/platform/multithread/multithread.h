/*
	FileName:      multithread.h

	Created Time:  2015-9-17

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
				'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
				modify or publish the source code. It's cross platform. You could compile the source code in
				linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_MULTI_THREAD
#define SORT_MULTI_THREAD

#include "sort.h"

#if defined(SORT_IN_WINDOWS)
	#include "winmultithread.h"
#elif defined(SORT_IN_MAC)
	#include "ptmultithread.h"
#elif defined(SORT_IN_LINUX)
// to be implemented
#endif

#endif // SORT_SHAREDMEMORY