/*
 * filename :	memmanager.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MEMMANAGER
#define	SORT_MEMMANAGER

// include the header
#include "utility/singleton.h"
#include "logmanager.h"
#include <map>

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
// public method
public:
	// default constructor
	MemManager();
	// destructor
	~MemManager();

	// pre-allocate memory
	void PreMalloc( unsigned size ) { PreMalloc( size , m_uDefault); }
	void PreMalloc( unsigned size , unsigned id );

	// clear the allocated memory
	void ClearMem( unsigned id );
	void ClearMem() { ClearMem(m_uDefault); }

	// de-allocate memory
	void DeAlloc( unsigned id );
	void DeAlloc() { ClearMem(m_uDefault); }

	// get pointer and set the offset
	template< typename T >
	T* GetPtr(unsigned id)
	{
		// get the memory pointer first
		Memory* mem = _getMemory(id);

		// check if the memory is enough
		if( sizeof( T ) + mem->m_offset > mem->m_size )
			LOG_ERROR<<"There is not enough memory in memory manager.(mem id:"<<id<<")"<<CRASH;

		unsigned addr = mem->m_offset;
		mem->m_offset += sizeof(T);
		mem->m_offset = ( mem->m_offset + 15 ) & (~15);
		return (T*)(mem->m_memory + addr);
	}
	template< typename T >
	T* GetPtr()
	{return GetPtr<T>(m_uDefault);}

	// set default memory id
	void SetDefaultId( unsigned id )
	{m_uDefault = id;}
	unsigned GetDefaultId() const
	{return m_uDefault;}

// private field
private:
	// the memories
	map<unsigned,Memory*> m_MemPool;

	// default memory id
	unsigned	m_uDefault;

	// get memory
	Memory*	_getMemory( unsigned id ) const
	{
		map<unsigned,Memory*>::const_iterator it = m_MemPool.find(id);
		if( it != m_MemPool.end() )
			return it->second;
		return 0;
	}

	// dealloc all memory
	void _deallocAllMemory()
	{
		map<unsigned,Memory*>::const_iterator it = m_MemPool.begin();
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
#define	SORT_MALLOC(T) new (MemManager::GetSingleton().GetPtr<T>()) T

// set default memory id
inline void SORT_MEMID(unsigned id)
{
	MemManager::GetSingleton().SetDefaultId(id);
}
inline unsigned SORT_MEMID()
{
	MemManager::GetSingleton().GetDefaultId();
}

// premalloc memory
inline void SORT_PREMALLOC(unsigned size)
{
	MemManager::GetSingleton().PreMalloc(size);
}
inline void SORT_PREMALLOC(unsigned size , unsigned id )
{
	MemManager::GetSingleton().PreMalloc(size,id);
}

//clear memory
inline void SORT_CLEARMEM()
{
	MemManager::GetSingleton().ClearMem();
}
inline void SORT_CLEARMEM(unsigned id)
{
	MemManager::GetSingleton().ClearMem(id);
}

// dealloc
inline void SORT_DEALLOC()
{
	MemManager::GetSingleton().DeAlloc();
}
inline void SORT_DEALLOC(unsigned id)
{
	MemManager::GetSingleton().DeAlloc(id);
}

#endif
