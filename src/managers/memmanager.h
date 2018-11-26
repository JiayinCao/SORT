/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

// include the header
#include "utility/singleton.h"
#include "utility/multithread/multithread.h"
#include "utility/log.h"
#include "utility/strhelper.h"
#include <unordered_map>

struct Memory
{
	char*		m_memory;
	unsigned	m_offset;
	unsigned	m_size;

	Memory()
	{
		m_memory=0;
		m_offset=0;
		m_size=0;
	}
	~Memory()
	{
		delete[] m_memory;
		m_offset=0;
		m_size=0;
	}
};

//////////////////////////////////////////////////////////////////////////////////
// definition of memory manager
// desc :	Memory manager could allocate small memory space efficiently. Actually
//			it would pre-allocate some memmory space , it will return alloacted space 
//			when asked for memory space , not really allocate it so that to save
//			more time on allocating memory.
class	MemManager : public Singleton<MemManager>
{
public:
	// default constructor
	MemManager();
	// destructor
	~MemManager();

	// pre-allocate memory
	void PreMalloc( unsigned size , unsigned id = 0 );

	// clear the allocated memory
	void ClearMem( unsigned id=0 );

	// de-allocate memory
	void DeAlloc( unsigned id=0 );

	// get pointer and set the offset
	template< typename T >
	T* GetPtr(unsigned count , unsigned id=0)
	{
		// get the memory pointer first
		Memory* mem = _getMemory(id);

        sAssertMsg( nullptr != mem , GENERAL , stringFormat( "No memory with id %d." , id ) );
        sAssertMsg( sizeof( T ) * count + mem->m_offset <= mem->m_size , GENERAL , stringFormat( "There is not enough memory in memory manager.(mem id:%d)" , id ) );
        
		unsigned addr = mem->m_offset;
		mem->m_offset += sizeof(T) * count;
		return (T*)(mem->m_memory + addr);
	}

	// get the offset of the memory
	unsigned GetOffset( unsigned id=0 ) const
	{
		Memory* mem = _getMemory(id);
        sAssertMsg( nullptr != mem , GENERAL , stringFormat( "No memory with id %d. " , id ) );
		return mem->m_offset;
	}

private:
	// the memories
	std::unordered_map<unsigned,Memory*> m_MemPool;

	// get memory
	Memory*	_getMemory( unsigned id ) const
	{
		std::unordered_map<unsigned,Memory*>::const_iterator it = m_MemPool.find(id);
		if( it != m_MemPool.end() )
			return it->second;
		return nullptr;
	}

	// dealloc all memory
	void _deallocAllMemory()
	{
		std::unordered_map<unsigned,Memory*>::const_iterator it = m_MemPool.begin();
		while( it != m_MemPool.end() )
		{
			delete it->second;
			it++;
		}
		m_MemPool.clear();
	}

	friend class Singleton<MemManager>;
};

// allocate memory
#define	SORT_MALLOC(T) SORT_MALLOC_ID(T,ThreadId())
#define SORT_MALLOC_ID(T,id) new (MemManager::GetSingleton().GetPtr<T>(1,id)) T
#define SORT_MALLOC_ARRAY(T,c) SORT_MALLOC_ARRAY_ID( T , c , ThreadId() )
#define	SORT_MALLOC_ARRAY_ID(T,c,id) new (MemManager::GetSingleton().GetPtr<T>(c,id)) T

// get sort memory
#define	SORT_MEMORY_ID(T,id) MemManager::GetSingleton().GetPtr<T>(0,id)

// premalloc memory
inline void SORT_PREMALLOC(unsigned size , unsigned id=0)
{
	MemManager::GetSingleton().PreMalloc(size,id);
}

//clear memory
inline void SORT_CLEARMEM(unsigned id=0)
{
	MemManager::GetSingleton().ClearMem(id);
}

// dealloc
inline void SORT_DEALLOC(unsigned id=0)
{
	MemManager::GetSingleton().DeAlloc(id);
}

// get the offset
inline unsigned SORT_OFFSET(unsigned id=0)
{
	return MemManager::GetSingleton().GetOffset(id);
}
