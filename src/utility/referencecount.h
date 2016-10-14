/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
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
