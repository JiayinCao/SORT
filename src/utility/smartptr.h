/*
   FileName:      smartptr.h

   Created Time:  2011-08-04 12:43:37

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SMARTPTR
#define	SORT_SMARTPTR

// include the header
#include "utility/define.h"
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
