/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2017 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef	SORT_CREATOR
#define	SORT_CREATOR

#include "singleton.h"
#include <unordered_map>
#include <algorithm>
#include "utility/strhelper.h"

// item creator
class ItemCreator
{
public:
	virtual void* CreateInstance() = 0;
};

typedef unordered_map<string,ItemCreator*> CREATOR_CONTAINER;

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
		string _str = str;
		::transform(_str.begin(),_str.end(),_str.begin(),ToLower());
		CREATOR_CONTAINER::const_iterator it = m_container.find( _str );
		if( it == m_container.end() )
			return nullptr;
		return (T*)(it->second->CreateInstance());
	}
	
	// get container
	CREATOR_CONTAINER& GetContainer() {
        return m_container;
    }

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
		std::string _str( N );\
		::transform(_str.begin(),_str.end(),_str.begin(),ToLower());\
		CREATOR_CONTAINER& container = Creator::GetSingleton().GetContainer();\
		if( container.count( _str ) )\
		{\
			LOG_WARNING<<"The creator type with specific name of "<<N<<" already existed."<<ENDL;\
			return;\
		}\
		container.insert( make_pair( _str , this ) );\
	}\
	void* CreateInstance() { return new T(); }\
};

#define IMPLEMENT_CREATOR( T ) static T::T##Creator g_creator##T;

#endif
