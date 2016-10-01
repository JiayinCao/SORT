/*
   FileName:      material_node.cpp

   Created Time:  2015-09-04 20:22:00

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "material_node.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"
#include "bsdf/lambert.h"
#include "thirdparty/tinyxml/tinyxml.h"
#include "geometry/intersection.h"

// get node property
MaterialNodeProperty* MaterialNode::getProperty( const string& name )
{
	map< string , MaterialNodeProperty* >::const_iterator it = m_props.find( name );
	if( it != m_props.end() )
		return it->second;
	return 0;
}

// set node property
void MaterialNodeProperty::SetNodeProperty( const string& prop )
{
	string str = prop;
	string x = NextToken( str , ' ' );
	string y = NextToken( str , ' ' );
	string z = NextToken( str , ' ' );
	string w = NextToken( str , ' ' );

	value.x = (float)atof( x.c_str() );
	value.y = (float)atof( y.c_str() );
	value.z = (float)atof( z.c_str() );
	value.w = (float)atof( w.c_str() );
}

// get node property
MaterialPropertyValue MaterialNodeProperty::GetPropertyValue( Bsdf* bsdf )
{
	if( node )
		return node->GetNodeValue( bsdf );
	return value;
}

// set node property
void MaterialNodePropertyString::SetNodeProperty( const string& prop )
{
	str = prop;
}

// parse property or socket
void MaterialNode::ParseProperty( TiXmlElement* element , MaterialNode* node )
{
	TiXmlElement* prop = element->FirstChildElement( "Property" );
	while(prop)
	{
		string prop_name = prop->Attribute( "name" );
		string prop_type = prop->Attribute( "type" );

		MaterialNodeProperty* node_prop = node->getProperty( prop_name );
		if( node_prop == 0 )
		{
			// output error log
			LOG_WARNING<<"Node property "<<prop_name<<" is ignored."<<ENDL;

			// get next property
			prop = prop->NextSiblingElement( "Property" );

			// proceed to the next property
			continue;
		}

		// add socket input
		if( prop_type == "node" )
		{
			node_prop->node = ParseNode( prop , node );
		}else
		{
			string node_value = prop->Attribute( "value" );
			node_prop->SetNodeProperty( node_value );
		}
			
		// get next property
		prop = prop->NextSiblingElement( "Property" );
	}
}

// parse a new node
MaterialNode* MaterialNode::ParseNode( TiXmlElement* element , MaterialNode* node )
{
	string node_type = element->Attribute( "node" );
	// create new material node
	MaterialNode* mat_node = CREATE_TYPE( node_type , MaterialNode );
	if( mat_node == 0 )
	{
		LOG_WARNING<<"Node type "<<node_type<<" is unknown!!"<<ENDL;
		return mat_node;
	}

	// parse node properties
	ParseProperty( element , mat_node );

	return mat_node; 
}

// check validation
bool MaterialNode::CheckValidation()
{
	// get subtree node type
	getNodeType();

	m_node_valid = true;
	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		if( it->second->node )
			m_node_valid &= it->second->node->CheckValidation();
		++it;
	}

	return m_node_valid;
}

// get sub tree node type
MAT_NODE_TYPE MaterialNode::getNodeType()
{
	MAT_NODE_TYPE type = MAT_NODE_NONE;

	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		if( it->second->node )
			type |= it->second->node->getNodeType();
		++it;
	}

	// setup sub-tree type
	subtree_node_type = type;

	return type;
}

// post process
void MaterialNode::PostProcess()
{
	if( m_post_processed )
		return;

	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		if( it->second->node )
			it->second->node->PostProcess();
		++it;
	}

	m_post_processed = true;
}

// update bsdf
void MaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		if( it->second->node )
			it->second->node->UpdateBSDF(bsdf , weight);
		++it;
	}
}

MaterialNode::~MaterialNode()
{
	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		delete it->second->node;
		++it;
	}
}

OutputNode::OutputNode()
{
	// register node property
	m_props.insert( make_pair( "Surface" , &output ) );
}

// update bsdf
void OutputNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	// return a default one for invalid material
	if( !m_node_valid )
	{
		Spectrum default_spectrum( 0.3f , 0.0f , 0.0f );
		Lambert* lambert = SORT_MALLOC(Lambert)( default_spectrum );
		lambert->m_weight = weight;
		bsdf->AddBxdf( lambert );
		return;
	}

	if( output.node )
		output.node->UpdateBSDF( bsdf );
}

// check validation
bool OutputNode::CheckValidation()
{
	// it is invalid if there is no node attached
	if( output.node == 0 )
		return false;

	// get node type
	MAT_NODE_TYPE type = output.node->getNodeType();

	// make sure there is bxdf attached !!
	if( ( output.node == 0 ) || !(type & MAT_NODE_BXDF) )
	{
		m_node_valid = false;
		return false;
	}

	m_node_valid = MaterialNode::CheckValidation();

	return m_node_valid;
}
