/*
 * filename :	singleton.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SOTR_SINGLETON
#define	SORT_SINGLETON
#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////
//	defination of Singleton class
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
	~Singleton()
	{
		m_pSingleton = 0;
	}

	// get singleton
	static T& GetSingleton()
	{
		return *m_pSingleton;
	}

	// get singleton pointer
	static T* GetSingletonPtr()
	{
		return m_pSingleton;
	}

//protected field
protected:
	// the singleton pointer
	static T* m_pSingleton;
};

#endif
