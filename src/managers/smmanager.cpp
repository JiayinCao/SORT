/*
	FileName:      smmanager.cpp

	Created Time:  2015-9-15

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
				'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
				modify or publish the source code. It's cross platform. You could compile the source code in
				linux and windows , g++ or visual studio 2008 is required.
*/

#include "smmanager.h"

// instance the singleton with tex manager
DEFINE_SINGLETON(SMManager);

// temporary solution, will isolate those windows interfaces later
#include <Windows.h>
static float*	result_buffer = 0;
static int		result_offset = 0;
static HANDLE	hMapFile;

SMManager::SMManager()
{

}

SMManager::~SMManager()
{
	// Release all shared memory
	std::map< string, SharedMemory >::iterator it = m_SharedMemory.begin();
	while (it != m_SharedMemory.end())
	{
		ReleaseSharedMemory(it->first);
		++it;
	}
}

// Initialize shared memory
SharedMemory SMManager::CreateSharedMemory(const string& sm_name, int size, unsigned type)
{
	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	// read/write access
		FALSE,					// do not inherit the name
		sm_name.c_str());		// name of mapping object

	if (hMapFile == NULL)
	{
		cout << GetLastError() << endl;
		cout << "Create." << endl;
		return SharedMemory();
	}
	result_buffer = (float*)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_WRITE, // read/write permission
		0,
		0,
		size);

	if (result_buffer == NULL)
	{
		cout << GetLastError() << endl;
		cout << "Map." << endl;
		CloseHandle(hMapFile);
		return SharedMemory();
	}

	SharedMemory sm;
	sm.bytes = (char*)result_buffer;
	sm.size = size;
	m_SharedMemory.insert(make_pair( sm_name, sm ));
	return sm;
}

// Release shared memory
void SMManager::ReleaseSharedMemory(const string& sm_name)
{
	// unmap the view
	UnmapViewOfFile(result_buffer);

	// close file handle
	CloseHandle(hMapFile);

	// erase it from the map
	m_SharedMemory.erase(sm_name);
}

// Release shared memory
SharedMemory SMManager::GetSharedMemory(const string& sm_name)
{
	// Release all shared memory
	std::map< string, SharedMemory >::iterator it = m_SharedMemory.begin();
	while (it != m_SharedMemory.end())
	{
		if (it->first == sm_name)
			return it->second;
		++it;
	}

	return SharedMemory();
}