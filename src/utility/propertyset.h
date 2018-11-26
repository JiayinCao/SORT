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

// include the header
#include "sort.h"
#include <string>
#include <unordered_map>
#include "utility/log.h"
#include "utility/strhelper.h"

// pre-declera class
class Texture;

// property hendler
template< typename T >
class PropertyHandler
{
public:
	// default constructor
	PropertyHandler( T* t ):m_target(t){};
    // destructor
    virtual ~PropertyHandler(){}
	// set value
	virtual void SetValue( const std::string& value ) {}
	// set texture
	virtual void SetValue( Texture* value ) {}

	// property name
	std::string	m_name;
	// the target
	T*		m_target;

private:
	explicit PropertyHandler( PropertyHandler<T>& ){}
};

///////////////////////////////////////////////////////////////////////////
//	definition of property set
template< typename T >
class PropertySet
{

public:
	// default constructor
	PropertySet() {}
	// destructor
	virtual ~PropertySet() { _clearProperty(); }

	// set property
	bool SetProperty( const std::string& name , const std::string& value )
	{
		// get the property handler first
		PropertyHandler<T>* ph = _getPropertyHandler( name );

		if( 0 == ph )
		{
            slog( WARNING , GENERAL , stringFormat( "There is no such a property named %s." , name.c_str() ) );
			return false;
		}

		// set the value
		ph->SetValue( value );

		return true;
	}
	// set property
	bool SetProperty( const std::string& name , Texture* value )
	{
		// get the property handler first
		PropertyHandler<T>* ph = _getPropertyHandler( name );

		if( 0 == ph )
		{
            slog( WARNING , GENERAL , stringFormat( "There is no such a property named %s." , name.c_str() ) );
			return false;
		}
		
		// set the texture
		ph->SetValue( value );

		return true;
	}
// protected field
protected:
	// the propery set
	std::unordered_map< std::string , PropertyHandler<T>* > m_propertySet;

	// register property
	void _registerAllProperty(){}

	// clear registered properties
	void _clearProperty()
	{
		typename std::unordered_map< std::string , PropertyHandler<T>* >::iterator it = m_propertySet.begin();
		while( it != m_propertySet.end() )
		{
			delete it->second;
			it++;
		}
		m_propertySet.clear();
	}

	// set property
	void _registerProperty( const std::string& name , PropertyHandler<T>* ph )
	{
		ph->m_name = name;
		m_propertySet.insert( make_pair( name , ph ) );
	}

	// get property handler
	PropertyHandler<T>* _getPropertyHandler( const std::string& name ){
        return m_propertySet.count( name ) ? m_propertySet[name] : nullptr;
	}
};

#define	CAST_TARGET(T)	static_cast<T*>(m_target)

#define PH_CONSTRUCTOR(T0,T1) T0(T1* t):PropertyHandler<T1>::PropertyHandler(t){}
