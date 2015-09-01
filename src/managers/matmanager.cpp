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
#include "bsdf/merl.h"

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
		if( it->second->reference != 0 )
		{
			string isare = (it->second->reference>1)?"is":"are";
			string refer = (it->second->reference>1)?" reference":" references";
			LOG_ERROR<<"There "<<isare<<" still "<<it->second->reference<<refer<<" pointing to material \""<<it->first<<"\"."<<CRASH;
		}

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

// parse material nodes
MaterialNode* MatManager::_parseMaterialNode( TiXmlElement* element , const string& name )
{
	string node_type = element->Attribute( "node" );

	// create new material node
	MaterialNode* node = 0;
	if( node_type == "SORTLambertNode" )
		node = new LambertNode();
	else if( node_type == "SORTMerlNode" )
		node = new MerlNode();
	else
		node = new MaterialNode();

	TiXmlElement* prop = element->FirstChildElement( "Property" );
	while(prop)
	{
		string prop_name = prop->Attribute( "name" );
		string prop_type = prop->Attribute( "type" );

		// add socket input
		MaterialSocket socket;
		if( prop_type == "node" )
		{
			socket.type = Socket_Node;
			socket.node = _parseMaterialNode( prop , prop_name );
		}else
		{
			socket.type = Socket_Value;
			string prop_value = prop->Attribute( "value" );
			if( node_type == "SORTLambertNode" )
				socket.value = SpectrumFromStr( prop_value );
			else if( node_type == "SORTMerlNode" ){
				socket.str_value = prop_value;
				MerlNode* merlNode = (MerlNode*)node;
				merlNode->merl = new Merl(socket.str_value);
			}
		}
		node->inputs.push_back( socket );
			
		// get next property
		prop = element->NextSiblingElement( "Property" );
	}

	return node;
}

// parse material file and add the materials into the manager
unsigned MatManager::ParseMatFile( const string& str )
{
	// load the xml file
	const string& full_filename_path = GetFullPath(str).c_str();
	TiXmlDocument doc( full_filename_path.c_str() );
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
		//string type = material->Attribute( "type" );

		// check if there is a material with the specific name, crash if there is
		//if( FindMaterial( name ) != 0 )
		//	LOG_ERROR<<"A material named \'"<<name<<"\' already exists in material system."<<CRASH;

		Material* mat = new Material();
		mat->SetName(name);
		TiXmlElement* prop = material->FirstChildElement( "Property" );
		while(prop)
		{
			MaterialNode* root = mat->GetRootNode();
			string prop_name = prop->Attribute( "name" );
			string prop_type = prop->Attribute( "type" );

			// add socket input
			MaterialSocket socket;
			if( prop_type == "node" )
			{
				socket.node = _parseMaterialNode( prop , prop_name );
			}else
			{
				string prop_value = prop->Attribute( "value" );
				socket.value = SpectrumFromStr( prop_value );
			}
			root->inputs.push_back( socket );
			
			// get next property
			prop = material->NextSiblingElement( "Property" );
		}

		// push the material
		m_matPool.insert( make_pair( name , mat ) );

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