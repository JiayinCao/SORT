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
