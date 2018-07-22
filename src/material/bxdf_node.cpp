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

#include "bxdf_node.h"
#include "bsdf/lambert.h"
#include "bsdf/merl.h"
#include "bsdf/orennayar.h"
#include "bsdf/microfacet.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"

IMPLEMENT_CREATOR( LambertNode );
IMPLEMENT_CREATOR( OrenNayarNode );
IMPLEMENT_CREATOR( MicrofacetReflectionNode );
IMPLEMENT_CREATOR( MicrofacetRefractionNode );

// check validation
bool BxdfNode::CheckValidation()
{
    for( auto prop : m_props ){
        MaterialNode* node = prop.second->node;
        if( node ){
            MAT_NODE_TYPE sub_type = node->getNodeType();
            
            // attaching bxdf result as an input of another bxdf doesn't make any sense at all
            if( sub_type & MAT_NODE_BXDF )
                return false;
        }
    }
    return MaterialNode::CheckValidation();
}

LambertNode::LambertNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
}

void LambertNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    Lambert* lambert = SORT_MALLOC(Lambert)( baseColor.GetPropertyValue(bsdf).ToSpectrum() );
	lambert->m_weight = weight;
	bsdf->AddBxdf( lambert );
}

OrenNayarNode::OrenNayarNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "Roughness" , &roughness ) );
}

void OrenNayarNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	OrenNayar* orennayar = SORT_MALLOC(OrenNayar)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , roughness.GetPropertyValue(bsdf).x );
	orennayar->m_weight = weight;
	bsdf->AddBxdf( orennayar );
}

MicrofacetReflectionNode::MicrofacetReflectionNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "MicroFacetDistribution" , &mf_dist ) );
	m_props.insert( make_pair( "RoughnessU" , &roughnessU ) );
    m_props.insert( make_pair( "RoughnessV" , &roughnessV ) );
	m_props.insert( make_pair( "eta" , &eta ) );
	m_props.insert( make_pair( "k" , &k ) );
}

void MicrofacetReflectionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	float ru = clamp( roughnessU.GetPropertyValue(bsdf).x , 0.001f , 1.0f );
    float rv = clamp( roughnessV.GetPropertyValue(bsdf).x , 0.001f , 1.0f );
	MicroFacetDistribution* dist = 0;
	if( mf_dist.str == "Blinn" )
		dist = SORT_MALLOC(Blinn)( ru , rv );
	else if( mf_dist.str == "Beckmann" )
		dist = SORT_MALLOC(Beckmann)( ru , rv );
	else
		dist = SORT_MALLOC(GGX)( ru , rv );	// GGX is default

	Fresnel* frenel = SORT_MALLOC( FresnelConductor )( eta.GetPropertyValue(bsdf).ToSpectrum() , k.GetPropertyValue(bsdf).ToSpectrum() );

	MicroFacetReflection* mf = SORT_MALLOC(MicroFacetReflection)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , frenel , dist );
	mf->m_weight = weight;
	bsdf->AddBxdf( mf );
}

MicrofacetRefractionNode::MicrofacetRefractionNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "MicroFacetDistribution" , &mf_dist ) );
	m_props.insert( make_pair( "Roughness" , &roughness ) );
	m_props.insert( make_pair( "in_ior" , &in_ior ) );
	m_props.insert( make_pair( "ext_ior" , &ext_ior ) );
}

void MicrofacetRefractionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	float rn = clamp( roughness.GetPropertyValue(bsdf).x , 0.05f , 1.0f );
	MicroFacetDistribution* dist = 0;
	if( mf_dist.str == "Blinn" )
		dist = SORT_MALLOC(Blinn)( rn , rn );
	else if( mf_dist.str == "Beckmann" )
		dist = SORT_MALLOC(Beckmann)( rn , rn );
	else
		dist = SORT_MALLOC(GGX)( rn , rn );	// GGX is default

	float in_eta = in_ior.GetPropertyValue(bsdf).x;     // index of refraction inside the material
	float ext_eta = ext_ior.GetPropertyValue(bsdf).x;   // index of refraction outside the material
	MicroFacetRefraction* mf = SORT_MALLOC(MicroFacetRefraction)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , dist , ext_eta , in_eta );
	mf->m_weight = weight;
	bsdf->AddBxdf( mf );
}
