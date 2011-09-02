/*
   FileName:      memmanager.cpp

   Created Time:  2011-08-04 12:47:28

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the headers
#include "memmanager.h"
#include "utility/define.h"

// instance the singleton with tex manager
DEFINE_SINGLETON(MemManager);

// default constructor
MemManager::MemManager()
{
	// 16mb memory for default
	PreMalloc( 1024 * 1024 * 16 );
}

// destructor
MemManager::~MemManager()
{
	_deallocAllMemory();
}

// pre-allocate memory
void MemManager::PreMalloc( unsigned size , unsigned id )
{
	// if size is equal to zero , just return
	if( size == 0 )
		return;

	Memory* mem = _getMemory( id );

	if( mem != 0 )
	{
		if( size != mem->m_size )
			DeAlloc( id );
		else
		{
			ClearMem( id );
			return;
		}
	}

	// create new memory
	mem = new Memory();
	mem->m_memory = new char[size];
	// reset offset
	mem->m_offset = 0;
	// set size
	mem->m_size = size;

	// push it into the map
	m_MemPool.insert( make_pair( id , mem ) );
}

// clear the allocated memory
void MemManager::ClearMem( unsigned id )
{
	Memory* mem = _getMemory( id );
	if( mem == 0 )
		LOG_ERROR<<"Can't clear memory, because there is no memory with id "<<id<<"."<<CRASH;

	//reset the offset
	mem->m_offset = 0;
}

// de-allocate memory
void MemManager::DeAlloc( unsigned id )
{
	Memory* mem = _getMemory( id );
	if( mem == 0 )
		LOG_WARNING<<"Can't delete memory, because there is no memory with id "<<id<<"."<<ENDL;

	// reset offset and size
	delete mem;

	m_MemPool.erase( id );
}
