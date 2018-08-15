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

#include "utility/propertyset.h"
#include "spectrum/spectrum.h"
#include <vector>
#include "utility/creator.h"

#include "spectrum/rgbspectrum.h"
#include "math/vector4.h"

class Bsdf;
class MaterialNode;
class TiXmlElement;

typedef Vector4<float> MaterialPropertyValue;

#define REGISTER_MATERIALNODE_PROPERTY( name , prop )    m_props.insert( make_pair( (name) , &(prop) ) )
#define GET_MATERIALNODE_PROPERTY( prop )                prop.GetPropertyValue(bsdf)
#define GET_MATERIALNODE_PROPERTY_FLOAT( prop )          prop.GetPropertyValue(bsdf).x
#define GET_MATERIALNODE_PROPERTY_SPECTRUM( prop )       prop.GetPropertyValue(bsdf).ToSpectrum()
#define GET_MATERIALNODE_PROPERTY_VECTOR( prop )         prop.GetPropertyValue(bsdf).ToVector()

class MaterialNodeProperty
{
public:
	MaterialNodeProperty():node(0){
	}

	// set node property
	virtual void SetNodeProperty( const string& prop );

	// get node property
	virtual MaterialPropertyValue GetPropertyValue( Bsdf* bsdf );
    
    // update bsdf, this is for bxdf wrappers like Blend , Coat or any other BXDF that can attach other BXDF as input
    void UpdateBsdf( Bsdf* bsdf , Spectrum weight = Spectrum( 1.0f ) );

	// sub node if it has value
	MaterialNode*	node;

	// value
	MaterialPropertyValue value;
};

class MaterialNodePropertyString : public MaterialNodeProperty
{
public:
	// set node property
	virtual void SetNodeProperty( const string& prop );

	// get node property
	virtual MaterialPropertyValue GetPropertyValue( Bsdf* bsdf ) { return MaterialPropertyValue(); }

	// color value
	string	str;
};

// base material node
class MaterialNode
{
public:
	virtual ~MaterialNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// parse property or socket
    bool ParseProperty( TiXmlElement* element , MaterialNode* node );

	// parse a new node
    MaterialNode* ParseNode( TiXmlElement* element , MaterialNode* node );

	// get property value, this should never be called
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) { return 0.0f; }

	// post process
	virtual void PostProcess();
    
    // whether the node is a bxdf node
    virtual bool IsBxdfNode() const { return false; }
    
    // whether the node is valid
    inline bool IsNodeValid() const { return m_node_valid; }

protected:
	// node properties
	std::unordered_map< string , MaterialNodeProperty * > m_props;

	// get node property
	MaterialNodeProperty*	getProperty( const string& name );

	// valid node
    bool m_node_valid = true;

	// already post processed
	bool m_post_processed = false;
};

// Mateiral output node
class OutputNode : public MaterialNode
{
public:
	OutputNode();

	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
	MaterialNodeProperty	output;
};
