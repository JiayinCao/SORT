/*
	FileName:      sharedmemory.h

	Created Time:  2015-9-16

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
				'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
				modify or publish the source code. It's cross platform. You could compile the source code in
				linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_SHAREDMEMORY
#define SORT_SHAREDMEMORY

#include "sort.h"

#define SharedMemory_Read 0x01
#define	SharedMemory_Write 0x02
#define SharedMmeory_All ( SharedMemory_Read | SharedMemory_Write )

struct SharedMemory
{
	char*	bytes;
	int		size;

	SharedMemory()
	{
		bytes = 0;
		size = 0;
	}
};

#if defined(SORT_IN_WINDOWS)
	#include "winsharedmemory.h"
#elif defined(SORT_IN_MAC) || defined(SORT_IN_LINUX)
    #include "mmapsharedmemory.h"
#endif

#endif // SORT_SHAREDMEMORY
