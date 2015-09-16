/*
	FileName:      winsharedmemory.cpp

	Created Time:  2015-9-16

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
			'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
			modify or publish the source code. It's cross platform. You could compile the source code in
			linux and windows , g++ or visual studio 2008 is required.
*/

#include "sort.h"

#if defined(SORT_IN_MAC)

#include "sharedmemory.h"
#include "managers/logmanager.h"
#include <sys/mman.h>
#include <sys/types.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// default constructor
MmapSharedMemory::MmapSharedMemory()
{
}

void MmapSharedMemory::CreateSharedMemory( const string& name , int size , unsigned type )
{
    fd = open( "blender_intermediate/sharedmem.bin" , O_RDWR , 0 );
    if( fd == -1 )
    {
        cout<< "Failed to load shared memory file"<<endl;
        return;
    }
    
    // map a new file
    sharedmemory.bytes = (char*)mmap(0, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
    if( sharedmemory.bytes == MAP_FAILED)
        sharedmemory.bytes = 0;
    sharedmemory.size = size;
}

// Release share memory resource
void MmapSharedMemory::ReleaseSharedMemory()
{
    if( sharedmemory.bytes != 0 )
        munmap(sharedmemory.bytes, sharedmemory.size);
    if( fd != -1 )
        close(fd);
}

#endif