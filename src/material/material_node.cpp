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

#include "material_node.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"
#include "bsdf/lambert.h"
#include "thirdparty/tinyxml/tinyxml.h"
#include "geometry/intersection.h"
#include "utility/log.h"

// get node property
MaterialNodeProperty* MaterialNode::getProperty( const std::string& name )
{
    return m_props.count(name) ? m_props[name] : nullptr;
}

// update bsdf, for layered brdf
void MaterialNodeProperty::UpdateBsdf( Bsdf* bsdf , Spectrum weight ){
    if( node )
        node->UpdateBSDF( bsdf , weight );
}

// parse property or socket
bool MaterialNode::ParseProperty( TiXmlElement* element , MaterialNode* node )
{
	TiXmlElement* prop = element->FirstChildElement( "Property" );
	while(prop)
	{
		std::string prop_name = prop->Attribute( "name" );

		MaterialNodeProperty* node_prop = node->getProperty( prop_name );
		if( node_prop == nullptr )
		{
			// output error log
            slog( WARNING , MATERIAL , stringFormat("Node property %s is ignored." , prop_name.c_str() ) );

			// get next property
			prop = prop->NextSiblingElement( "Property" );

			// proceed to the next property
			continue;
		}

		// add socket input
		if( prop->Attribute( "node" ) )
		{
			node_prop->node = ParseNode( prop , node );
            
            if( node_prop->node && node_prop->GetNodeReturnType() != node_prop->node->GetNodeReturnType() ){
                m_node_valid = false;
                return false;
            }
		}else
		{
			std::string node_value = prop->Attribute( "value" );
			node_prop->SetNodeProperty( node_value );
		}
			
		// get next property
		prop = prop->NextSiblingElement( "Property" );
	}
    
    return true;
}

// parse a new node
MaterialNode* MaterialNode::ParseNode( TiXmlElement* element , MaterialNode* node )
{
	std::string node_type = element->Attribute( "node" );
	// create new material node
	MaterialNode* mat_node = CREATE_TYPE( node_type , MaterialNode );
	if( mat_node == nullptr )
	{
        slog( WARNING , MATERIAL , stringFormat("Node type %s is undefined." , node_type.c_str() ) );
		return mat_node;
	}

	// parse node properties
	m_node_valid = ParseProperty( element , mat_node );
    
	return mat_node; 
}

// post process
void MaterialNode::PostProcess()
{
	if( m_post_processed )
		return;

    for( auto it : m_props ){
		if( it.second->node )
			it.second->node->PostProcess();
	}

	m_post_processed = true;
}

// update bsdf
void MaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

    for( auto it : m_props ){
		if( it.second->node )
			it.second->node->UpdateBSDF(bsdf , weight);
	}
}

MaterialNode::~MaterialNode()
{
    for( auto it : m_props )
		delete it.second->node;
}

// update bsdf
void OutputNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	// return a default one for invalid material
	if( !m_node_valid || !output.node ){
		static const Spectrum default_spectrum( 0.5f , 0.1f , 0.1f );
		const Lambert* lambert = SORT_MALLOC(Lambert)( default_spectrum , weight , DIR_UP);
		bsdf->AddBxdf( lambert );
		return;
	}

	output.node->UpdateBSDF( bsdf );
}
