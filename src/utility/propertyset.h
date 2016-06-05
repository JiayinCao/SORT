/*
   FileName:      propertyset.h

   Created Time:  2011-08-04 12:43:13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_PROPERTYSET
#define	SORT_PROPERTYSET

// include the header
#include "sort.h"
#include <string>
#include <map>
#include "managers/logmanager.h"

// pre-declera class
class Texture;

// property hendler
template< typename T >
class PropertyHandler
{
// public metdod
public:
	// default constructor
	PropertyHandler( T* t ):m_target(t){};
    // destructor
    virtual ~PropertyHandler(){}
	// set value
	virtual void SetValue( const string& value ) {}
	// set texture
	virtual void SetValue( Texture* value ) {}

	// property name
	string	m_name;
	// the target
	T*		m_target;

// private copy constructor
private:
	explicit PropertyHandler( PropertyHandler<T>& ){}
};

///////////////////////////////////////////////////////////////////////////
//	definition of property set
template< typename T >
class PropertySet
{
// public method
public:
	// default constructor
	PropertySet() {}
	// destructor
	virtual ~PropertySet() { _clearProperty(); }

	// set property
	bool SetProperty( const string& name , const string& value )
	{
		// get the property handler first
		PropertyHandler<T>* ph = _getPropertyHandler( name );

		if( 0 == ph )
		{
			LOG_WARNING<<"There is no such a property named \'"<<name<<"\'."<<ENDL;
			return false;
		}

		// set the value
		ph->SetValue( value );

		return true;
	}
	// set property
	bool SetProperty( const string& name , Texture* value )
	{
		// get the property handler first
		PropertyHandler<T>* ph = _getPropertyHandler( name );

		if( 0 == ph )
		{
			LOG_WARNING<<"There is no such a property named \'"<<name<<"\'."<<ENDL;
			return false;
		}
		
		// set the texture
		ph->SetValue( value );

		return true;
	}
// protected field
protected:
	// the propery set
	map< string , PropertyHandler<T>* > m_propertySet;

	// register property
	void _registerAllProperty(){}

	// clear registered properties
	void _clearProperty()
	{
		typename map< string , PropertyHandler<T>* >::iterator it = m_propertySet.begin();
		while( it != m_propertySet.end() )
		{
			delete it->second;
			it++;
		}
		m_propertySet.clear();
	}

	// set property
	void _registerProperty( const string& name , PropertyHandler<T>* ph )
	{
		ph->m_name = name;
		m_propertySet.insert( make_pair( name , ph ) );
	}

	// get property handler
	PropertyHandler<T>* _getPropertyHandler( const string& name )
	{
		typename map< string , PropertyHandler<T>* >::iterator it = m_propertySet.find( name );
		if( it != m_propertySet.end() )
			return it->second;
		return 0;
	}
};

#define	CAST_TARGET(T)	static_cast<T*>(m_target)

#define PH_CONSTRUCTOR(T0,T1) T0(T1* t):PropertyHandler<T1>::PropertyHandler(t){}

#endif
