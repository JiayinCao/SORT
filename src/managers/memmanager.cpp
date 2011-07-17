/*
 * filename :	memmanager.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the headers
#include "memmanager.h"
#include "utility/define.h"

// instance the singleton with tex manager
DEFINE_SINGLETON(MemManager);

// default constructor
MemManager::MemManager()
{
	m_uDefault = 0;

	// 4mb memory for default
	PreMalloc( 1024 * 1024 * 4 );
}

// destructor
MemManager::~MemManager()
{
	_deallocAllMemory();
}

// pre-allocate memory
void MemManager::PreMalloc( unsigned size , unsigned id )
{
	Memory* mem = _getMemory( id );

	if( mem != 0 )
		LOG_ERROR<<"Memory with id "<<id<<" already exist."<<CRASH;

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
		LOG_ERROR<<"Can't delete memory, because there is no memory with id "<<id<<"."<<CRASH;

	// reset offset and size
	delete mem;

	m_MemPool.erase( id );
}
