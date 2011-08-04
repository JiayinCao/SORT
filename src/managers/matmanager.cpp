/*
   FileName:      matmanager.cpp

   Created Time:  2011-08-04 12:47:21

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "sort.h"
#include "matmanager.h"
#include "material/material.h"
#include "utility/path.h"
#include "thirdparty/tinyxml/tinyxml.h"
#include "logmanager.h"
#include "material/matte.h"
#include "texmanager.h"
#include "texture/constanttexture.h"
#include "utility/creator.h"

// instance the singleton with tex manager
DEFINE_SINGLETON(MatManager);

// default constructor
MatManager::MatManager()
{
	// initialize default material
	ConstantTexture* ct = new ConstantTexture( 0.1f , 0.1f , 0.1f );
	m_Default = new Matte();
	m_Default->SetProperty( "color" , ct );
}

// destructor
MatManager::~MatManager()
{
	_clearMatPool();
}

// clear the material pool
void MatManager::_clearMatPool()
{
	SAFE_DELETE(m_Default);

	map< string , Material* >::iterator it = m_matPool.begin();
	while( it != m_matPool.end() )
	{
		delete it->second;
		it++;
	}
	m_matPool.clear();
}

// find specific material
Material* MatManager::FindMaterial( const string& mat_name ) const
{
	map< string , Material* >::const_iterator it = m_matPool.find( mat_name );
	if( it != m_matPool.end() )
		return it->second;
	return 0;
}

// whether the material exists
Material* MatManager::GetDefaultMat() const
{
	return m_Default;
}

// parse material file and add the materials into the manager
unsigned MatManager::ParseMatFile( const string& str )
{
	// load the xml file
	TiXmlDocument doc( GetFullPath(str).c_str() );
	doc.LoadFile();

	// if there is error , return false
	if( doc.Error() )
	{
		LOG_WARNING<<doc.ErrorDesc()<<ENDL;
		LOG_WARNING<<"Material \'"<<str<<"\' file load failed."<<ENDL;
		return false;
	}

	// get the root of xml
	TiXmlNode*	root = doc.RootElement();

	// parse materials
	TiXmlElement* material = root->FirstChildElement( "Material" );
	while( material )
	{
		// parse the material
		string name = material->Attribute( "name" );
		string type = material->Attribute( "type" );

		// check if there is a material with the specific name, crash if there is
		if( FindMaterial( name ) != 0 )
			LOG_ERROR<<"A material named \'"<<name<<"\' already exists in material system."<<CRASH;

		// create specific material
		Material* mat = CREATE_TYPE( type , Material );

		if( mat )
		{
			// set properties
			TiXmlElement* prop = material->FirstChildElement( "Property" );
			while( prop )
			{
				string attr_name = prop->Attribute( "name" );
				string attr_value = prop->Attribute( "value" );
				mat->SetProperty( attr_name , attr_value );
				prop = prop->NextSiblingElement( "Property" );
			}

			// set texture properties
			prop = material->FirstChildElement( "Texture" );
			while( prop )
			{
				string name = prop->Attribute( "name" );
				string type = prop->Attribute( "type" );
				Texture* tex = CREATE_TYPE(type,Texture);
				if( tex )
				{
					TiXmlElement* tex_prop = prop->FirstChildElement( "Property" );
					while( tex_prop )
					{
						string pn = tex_prop->Attribute( "name" );
						string pv = tex_prop->Attribute( "value" );
						tex->SetProperty( pn , pv );
						tex_prop = tex_prop->NextSiblingElement( "Property" );
					}
				}
				if( tex->IsValid() )
				{
					if( false == mat->SetProperty( name , tex ) )
						delete tex;
				}
				else
					delete tex;
				prop = prop->NextSiblingElement( "Texture" );
			}

			// push the material
			m_matPool.insert( make_pair( name , mat ) );
		}

		// parse the next material
		material = material->NextSiblingElement( "Material" );
	}

	return 0;
}

// get material number
unsigned MatManager::GetMatCount() const
{
	return m_matPool.size();
}