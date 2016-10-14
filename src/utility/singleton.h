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

#ifndef	SORT_SINGLETON
#define	SORT_SINGLETON

// include the header
#include "sort.h"

//////////////////////////////////////////////////////////////////////
//	definition of Singleton class
//	actually it's a template, any class wants to be singleton could 
//	inherit from it.
template<typename T>
class	Singleton
{
//public method
public:
	// constructor from a template
	Singleton()
	{
		m_pSingleton = static_cast<T*>(this);
	}
	// destructor
	virtual ~Singleton()
	{
		m_pSingleton = 0;
	}

	// get singleton
	static T& GetSingleton()
	{
		if( m_pSingleton == 0 )
			new T();
		return *m_pSingleton;
	}

	// get singleton pointer
	static T* GetSingletonPtr()
	{
		return m_pSingleton;
	}

	// delete singleton
	static void DeleteSingleton()
	{
		SAFE_DELETE(m_pSingleton);
	}

	// create instance
	static void CreateInstance()
	{
		if( m_pSingleton == 0 )
			new T();
	}

//protected field
protected:
	// the singleton pointer
	static T* m_pSingleton;
};

// a useful macro for instance the templage
#define	DEFINE_SINGLETON(T)	template<> T* Singleton<T>::m_pSingleton = 0

#endif
