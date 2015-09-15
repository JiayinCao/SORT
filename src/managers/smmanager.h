/*
	FileName:      smmanager.h

	Created Time:  2015-9-15

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
				'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
				modify or publish the source code. It's cross platform. You could compile the source code in
				linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_SMMANAGER
#define SORT_SMMANAGER

#include "utility/singleton.h"
#include <map>

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

/////////////////////////////////////////////////////////////
// definition of shared memory manager
class	SMManager : public Singleton<SMManager>
{
public:
	// destructor
	~SMManager();

	// Initialize shared memory
	SharedMemory CreateSharedMemory(const string& sm_name, int length, unsigned type);

	// Release share memory resource
	void ReleaseSharedMemory(const string& sm_name);

	// Get Shared Memory
	SharedMemory GetSharedMemory(const string& sm_name);

private:
	// the map for shared memory
	std::map< string, SharedMemory > m_SharedMemory;

	// private constructor
	SMManager();

	friend class Singleton<SMManager>;
};

#endif