/*
   FileName:      creator.h

   Created Time:  2011-08-04 12:42:21

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_CREATOR
#define	SORT_CREATOR

#include "singleton.h"
#include "logmanager.h"
#include <map>

// item creator
class ItemCreator
{
public:
	virtual void* CreateInstance() = 0;
};

typedef map<string,ItemCreator*> CREATOR_CONTAINER;

/////////////////////////////////////////////////////////////////////////////////////////
//	creator is responsible for creating different kinds of object
class Creator : public Singleton<Creator>
{
// public method
public:
	// destructor
	~Creator(){}

	// Create instance
	template< typename T >
	T* CreateType( const string& str ) const
	{
		CREATOR_CONTAINER::const_iterator it = m_container.find( str );
		if( it == m_container.end() )
			return 0;
		return (T*)(it->second->CreateInstance());
	}
	
	// get container
	CREATOR_CONTAINER& GetContainer() { return m_container; }

// private field
private:
	// the container
	CREATOR_CONTAINER	m_container;

	// default constructor
	Creator(){}

	friend class Singleton<Creator>;
};

#define CREATE_TYPE(str,T) Creator::GetSingleton().CreateType<T>( str )

#define	DEFINE_CREATOR( T , N ) class T##Creator : public ItemCreator \
{ public: \
	T##Creator()\
	{\
		CREATOR_CONTAINER& container = Creator::GetSingleton().GetContainer();\
		CREATOR_CONTAINER::const_iterator it = container.find( N );\
		if( it != container.end() )\
		{\
			LOG_WARNING<<"The creator type with specific name of "<<N<<" already existed."<<ENDL;\
			return;\
		}\
		container.insert( make_pair( N , this ) );\
	}\
	void* CreateInstance() { return new T(); }\
};

#define IMPLEMENT_CREATOR( T ) static T::T##Creator g_creator;

#endif