/*
 * filename :	smartptr.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_SMARTPTR
#define	SORT_SMARTPTR

// include the header
#include "sort.h"
#include "managers/logmanager.h"

/////////////////////////////////////////////////////////////////////
// definition of smart pointer
template<typename T>
class	SmartPtr
{
// public method
public:
	// default constructor
	SmartPtr():m_ptr(0){}
	// constructor from T*
	SmartPtr( const T* p ):m_ptr(p){}
	// destructor
	virtual ~SmartPtr(){ SAFE_DELETE(m_ptr);}

	// = operator
	T* operator=( T* data )
	{
		m_ptr = data;
		return data;
	}

	// -> operator
	T* operator->()
	{
		if( m_ptr == 0 )
			LOG_ERROR<<"Invalid smart pointer."<<CRASH;
		return m_ptr;
	}
	const T* operator->() const
	{
		if( m_ptr == 0 )
			LOG_ERROR<<"Invalid smart pointer."<<CRASH;
		return m_ptr;
	}

	// * operator
	T& operator*()
	{
		if( m_ptr == 0 )
			LOG_ERROR<<"Invalid smart pointer."<<CRASH;
		return *m_ptr;
	}
	const T& operator*() const
	{
		if( m_ptr == 0 )
			LOG_ERROR<<"Invalid smart pointer."<<CRASH;
		return *m_ptr;
	}

	// whether the smart pointer is valid
	operator bool() const
	{
		return m_ptr != 0 ;
	}

	// delete the smart pointer
	void Delete()
	{
		SAFE_DELETE(m_ptr);
	}

// private field
private:
	// the pointer to the data type T
	T*	m_ptr;
};

#endif
