/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2017 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
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
	std::unordered_map< string, PlatformSharedMemory >::iterator it = m_SharedMemory.begin();
	while (it != m_SharedMemory.end())
	{
		ReleaseSharedMemory(it->first);
		
		// restart from the first one
		it = m_SharedMemory.begin();
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
	std::unordered_map< string, PlatformSharedMemory >::iterator it = m_SharedMemory.begin();
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
	std::unordered_map< string, PlatformSharedMemory >::iterator it = m_SharedMemory.begin();
	while (it != m_SharedMemory.end())
	{
		if (it->first == sm_name)
			return it->second.sharedmemory;
		++it;
	}

	return SharedMemory();
}
