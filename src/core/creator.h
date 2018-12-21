/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#include <unordered_map>
#include "core/singleton.h"
#include "core/log.h"

//! @brief		This class serves as 'namespace' for each specific type so that
//!				different class type names won't pollute each others pool.
template<class T>
class ItemCreator{
public:
	virtual std::shared_ptr<T> CreateInstance() const = 0;
};

//! @brief		Creator class is responsible for creating instances based on names.
template<class T>
class Creator : public Singleton<Creator<T>>{
    typedef std::unordered_map<std::string,ItemCreator<T>*> CREATOR_CONTAINER;

public:
	//! @brief	Create an instance of a specific type based on class name.
	//!
	//! @return		Return a reference of a newly created instance.
	std::shared_ptr<T> CreateType( std::string str ) const{
		std::transform(str.begin(),str.end(),str.begin(),[](char c){ return tolower(c); });
		auto it = m_container.find( str );
		return it == m_container.end() ? nullptr : it->second->CreateInstance();
	}

	//! @brief	Get the container that could be further modified.
	//!
	//! It is not perfect to return a reference of the class member. Since this class is not directly
	//! exposed to upper level code, no code should directly call this interface.
	//!
	//! @return		Return the reference of the container.
	CREATOR_CONTAINER& GetContainer(){
		return m_container;
	}

private:
	/**< Container for the class creators. */
	CREATOR_CONTAINER	m_container;

	//! @brief	Make sure constructor is private.
	Creator(){}
	//! @brief	Make sure copy constructor is private.
    Creator(const Creator<T>&){}

	friend class Singleton<Creator>;
};

#define IMPLEMENT_CREATOR( T )      static T::T##Creator g_creator##T;
#define	DEFINE_CREATOR( T , B , N ) class T##Creator : public ItemCreator<B> \
{public: \
	T##Creator(){\
		std::string _str( N );\
		std::transform(_str.begin(),_str.end(),_str.begin(),[](char c){return tolower(c);});\
		auto& container = Creator<B>::GetSingleton().GetContainer();\
		if( container.count( _str ) ){\
            slog( WARNING , GENERAL , "The creator type with specific name of %s already exxisted." , N );\
			return;\
		}\
        container.insert( std::make_pair(_str , this) );\
	}\
	std::shared_ptr<B> CreateInstance() const { return std::make_shared<T>(); }\
};

//! @brief	Instance a class type based on name.
//!
//! @param	name		Name of the class. This has to match what is defined in python plugin.
template<class T>
std::shared_ptr<T> MakeInstance( const std::string& name ){
	return Creator<T>::GetSingleton().CreateType( name );
}