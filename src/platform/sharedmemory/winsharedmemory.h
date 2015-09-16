/*
	FileName:      winsharedmemory.h

	Created Time:  2015-9-16

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
				'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
				modify or publish the source code. It's cross platform. You could compile the source code in
				linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_WINSHAREDMEMORY
#define SORT_WINSHAREDMEMORY

#include "sort.h"

#if defined(SORT_IN_WINDOWS)

#include <Windows.h>

class WinSharedMemory
{
// public method
public:
	// default constructor
	WinSharedMemory();

	// Initialize shared memory
	void CreateSharedMemory(const string& sm_name, int size, unsigned type);

	// Release share memory resource
	void ReleaseSharedMemory();

	// shared memory data
	SharedMemory	sharedmemory;

// platform dependent fields, still invisible from others
private:
	HANDLE	hMapFile;
};

#endif

#define PlatformSharedMemory	WinSharedMemory

#endif