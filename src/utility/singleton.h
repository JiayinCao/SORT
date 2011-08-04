/*
   FileName:      singleton.h

   Created Time:  2011-08-04 12:43:34

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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
