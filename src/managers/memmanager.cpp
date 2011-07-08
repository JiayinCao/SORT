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
	m_memory = 0;
	m_offset = 0;
	m_size = 0;

	// 1mb memory for default
	PreMalloc( 1024 * 1024 );
}

// destructor
MemManager::~MemManager()
{
	DeAlloc();
}

// pre-allocate memory
void MemManager::PreMalloc( unsigned size )
{
	// if there is old memory , delete them first
	DeAlloc();

	// create new memory
	m_memory = new char[size];
	// reset offset
	m_offset = 0;
	// set size
	m_size = size;
}

// clear the allocated memory
void MemManager::ClearMem()
{
	//reset the offset
	m_offset = 0;
}

// de-allocate memory
void MemManager::DeAlloc()
{
	// delete memory
	SAFE_DELETE(m_memory);

	// reset offset and size
	m_offset = 0;
	m_size = 0;
}