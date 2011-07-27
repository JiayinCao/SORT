/*
 * filename :	creator.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_CREATOR
#define	SORT_CREATOR

#include "singleton.h"
#include <map>

// item creator
class ItemCreator
{
public:
	virtual void* CreateInstance() = 0;
};

#define	DEFINE_CREATOR( T ) class T##Creator : public ItemCreator \
{ public: \
	void* CreateInstance() { return new T(); } \
};

/////////////////////////////////////////////////////////////////////////////////////////
//	creator is responsible for creating different kinds of object
class Creator : public Singleton<Creator>
{
// public method
public:
	// destructor
	~Creator(){_release();}

	// Create instance
	template< typename T >
	T* CreateType( const string& str ) const
	{
		map<string,ItemCreator*>::const_iterator it = m_container.find( str );
		if( it == m_container.end() )
			return 0;
		return (T*)(it->second->CreateInstance());
	}

// private field
private:
	// the container
	map<string,ItemCreator*>	m_container;

	// default constructor
	Creator(){_init();}
	// initialize the system
	void	_init();
	// release the system
	void	_release();

	friend class Singleton<Creator>;
};

#define CREATE_TYPE(str,T) Creator::GetSingleton().CreateType<T>( str )
#endif