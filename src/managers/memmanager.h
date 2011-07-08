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
	void PreMalloc( unsigned size );

	// clear the allocated memory
	void ClearMem();

	// de-allocate memory
	void DeAlloc();

	// allocate a new memory
	template< typename T >
	T* Allocate()
	{
		// check if the memory is enough
		if( sizeof( T ) + m_offset > m_size )
			LOG_ERROR<<"There is not enough memory in memory manager."<<ENDL;

		T* r = new ((T*)(m_memory + m_offset)) T() ;
		m_offset += sizeof(T);
		m_offset = ( m_offset + 15 ) & (~15);
		return r;
	}

// private field
private:
	// the managed memory
	char*	m_memory;

	// current used offset
	unsigned m_offset;

	// size of the memory
	unsigned m_size;

	friend class Singleton<MemManager>;
};

// allocate memory
#define	SORT_MALLOC(T) MemManager::GetSingleton().Allocate<T>()

#endif