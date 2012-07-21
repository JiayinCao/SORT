/*
   FileName:      referencecount.h

   Created Time:  2011-08-04 12:43:26

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_REFERENCECOUNT
#define	SORT_REFERENCECOUNT

#include "utility/sassert.h"

//////////////////////////////////////////////////////////////////////////
// definition of reference count
// note : there is a BIG difference between this reference count and the
//		  others. When there is no reference point to the current one, 
//		  the memory isn't destroyed, because we use memory managerment to
//		  release the memory.

// any reference count should derive from this class
class ReferenceCount
{
public:
	// default constructor
	ReferenceCount(){ reference = 0; }

	// the count
	unsigned reference;
};

// a reference
template< typename T >
class Reference
{
// public method
public:
	// default constructor
	Reference(){ ptr = 0; }
	// constructor from a pointer T*
	Reference( T* p ){ ptr = p; if(ptr) ptr->reference--; }
	// copy constructor
	Reference( const Reference<T>& r )
	{
		ptr = r.ptr;
		if( ptr ) ptr->reference++;
	}
	// destructor
	virtual ~Reference(){ if( ptr ) ptr->reference--; ptr = 0; }

	// = operator
	Reference& operator=( const Reference<T>& r )
	{
		if( r.ptr )
			r.ptr->reference++;
		if( ptr )
			ptr->reference--;

		ptr = r.ptr;
		return *this;
	}
	Reference& operator=( T* p )
	{
		if( p )
			p->reference++;
		if( ptr )
			ptr->reference--;

		ptr = p;
		return *this;
	}

	// -> operator
	T* operator->()
	{
		Sort_Assert( ptr != 0 );
		return ptr;
	}
	const T* operator->() const
	{
		Sort_Assert( ptr != 0 );
		return ptr;
	}

	// * operator
	T& operator*()
	{
		Sort_Assert( ptr != 0 );
		return *ptr;
	}
	const T& operator*() const
	{
		Sort_Assert( ptr != 0 );
		return *ptr;
	}

	// whether the smart pointer is valid
	operator bool() const
	{
		return ptr != 0 ;
	}

	operator T*() const
	{
		return ptr;
	}

// private field
private:
	// the pointer
	T*	ptr;
};

#endif
