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

#if defined(SORT_IN_WINDOWS)

#include "sharedmemory.h"
#include "managers/logmanager.h"

// default constructor
WinSharedMemory::WinSharedMemory()
{
	hMapFile = 0;
}

void WinSharedMemory::CreateSharedMemory( const string& name , int size , unsigned type )
{
	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	// read/write access
		FALSE,					// do not inherit the name
		name.c_str());		// name of mapping object

	if (hMapFile == NULL)
	{
		LOG_WARNING<<"Creating Shared Memory Failed "<<name<<ENDL;
		return;
	}
	sharedmemory.size = size;
	sharedmemory.bytes = (char*)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_WRITE, // read/write permission
		0,
		0,
		size);

	if (sharedmemory.bytes == NULL)
	{
		sharedmemory.size = 0;
		CloseHandle(hMapFile);
		LOG_WARNING<<"Mapping Shared Memory Failed "<<name<<ENDL;
	}
}

// Release share memory resource
void WinSharedMemory::ReleaseSharedMemory()
{
	// unmap the view
	if (sharedmemory.bytes)
		UnmapViewOfFile(sharedmemory.bytes);

	// close file handle
	if (hMapFile)
	{
		CloseHandle(hMapFile);
		hMapFile = 0;
	}
}

#endif