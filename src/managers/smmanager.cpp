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

SMManager::SMManager()
{

}

SMManager::~SMManager()
{
	// Release all shared memory
	std::map< string, PlatformSharedMemory >::iterator it = m_SharedMemory.begin();
	while (it != m_SharedMemory.end())
	{
		ReleaseSharedMemory(it->first);
		++it;
	}
}

// Initialize shared memory
SharedMemory SMManager::CreateSharedMemory(const string& sm_name, int size, unsigned type)
{
	// platform dependent shared memory
	PlatformSharedMemory sm;

	// create shared memory
	sm.CreateSharedMemory(sm_name, size, type);

	// push it into the map
	m_SharedMemory.insert(make_pair(sm_name, sm));

	// return shared memory result
	return sm.sharedmemory;
}

// Release shared memory
void SMManager::ReleaseSharedMemory(const string& sm_name)
{
	// Release all shared memory
	std::map< string, PlatformSharedMemory >::iterator it = m_SharedMemory.begin();
	while (it != m_SharedMemory.end())
	{
		if (it->first == sm_name)
		{
			it->second.ReleaseSharedMemory();
			break;
		}

		++it;
	}

	// erase it from the map
	m_SharedMemory.erase(sm_name);
}

// Release shared memory
SharedMemory SMManager::GetSharedMemory(const string& sm_name)
{
	// Release all shared memory
	std::map< string, PlatformSharedMemory >::iterator it = m_SharedMemory.begin();
	while (it != m_SharedMemory.end())
	{
		if (it->first == sm_name)
			return it->second.sharedmemory;
		++it;
	}

	return SharedMemory();
}