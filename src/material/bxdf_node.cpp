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
#include "bsdf/fourierbxdf.h"
#include "bsdf/ashikhmanshirley.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"

IMPLEMENT_CREATOR( AshikhmanShirleyNode );
IMPLEMENT_CREATOR( LambertNode );
IMPLEMENT_CREATOR( LambertTransmissionNode );
IMPLEMENT_CREATOR( OrenNayarNode );
IMPLEMENT_CREATOR( MicrofacetReflectionNode );
IMPLEMENT_CREATOR( MicrofacetRefractionNode );
IMPLEMENT_CREATOR( FourierBxdfNode );
IMPLEMENT_CREATOR( MerlNode );

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
	m_props.insert( make_pair( "Diffuse" , &baseColor ) );
}

void LambertNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight ){
	bsdf->AddBxdf( SORT_MALLOC(Lambert)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , weight ) );
}

LambertTransmissionNode::LambertTransmissionNode()
{
    m_props.insert( make_pair( "Diffuse" , &baseColor ) );
}

void LambertTransmissionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight ){
    bsdf->AddBxdf( SORT_MALLOC(LambertTransmission)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , weight ) );
}

OrenNayarNode::OrenNayarNode()
{
	m_props.insert( make_pair( "Diffuse" , &baseColor ) );
	m_props.insert( make_pair( "Roughness" , &roughness ) );
}

void OrenNayarNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	const OrenNayar* orennayar = SORT_MALLOC(OrenNayar)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , roughness.GetPropertyValue(bsdf).x , weight );
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
	float ru = saturate( roughnessU.GetPropertyValue(bsdf).x );
    float rv = saturate( roughnessV.GetPropertyValue(bsdf).x );
	MicroFacetDistribution* dist = 0;
	if( mf_dist.str == "Blinn" )
		dist = SORT_MALLOC(Blinn)( ru , rv );
	else if( mf_dist.str == "Beckmann" )
		dist = SORT_MALLOC(Beckmann)( ru , rv );
	else
		dist = SORT_MALLOC(GGX)( ru , rv );	// GGX is default

	Fresnel* frenel = SORT_MALLOC( FresnelConductor )( eta.GetPropertyValue(bsdf).ToSpectrum() , k.GetPropertyValue(bsdf).ToSpectrum() );
	bsdf->AddBxdf( SORT_MALLOC(MicroFacetReflection)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , frenel , dist , weight ) );
}

MicrofacetRefractionNode::MicrofacetRefractionNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "MicroFacetDistribution" , &mf_dist ) );
    m_props.insert( make_pair( "RoughnessU" , &roughnessU ) );
    m_props.insert( make_pair( "RoughnessV" , &roughnessV ) );
	m_props.insert( make_pair( "in_ior" , &in_ior ) );
	m_props.insert( make_pair( "ext_ior" , &ext_ior ) );
}

void MicrofacetRefractionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	float ru = saturate( roughnessU.GetPropertyValue(bsdf).x );
    float rv = saturate( roughnessV.GetPropertyValue(bsdf).x );
	MicroFacetDistribution* dist = 0;
	if( mf_dist.str == "Blinn" )
		dist = SORT_MALLOC(Blinn)( ru , rv );
	else if( mf_dist.str == "Beckmann" )
		dist = SORT_MALLOC(Beckmann)(ru, rv);
	else
		dist = SORT_MALLOC(GGX)(ru, rv);	// GGX is default

	float in_eta = in_ior.GetPropertyValue(bsdf).x;     // index of refraction inside the material
	float ext_eta = ext_ior.GetPropertyValue(bsdf).x;   // index of refraction outside the material
	bsdf->AddBxdf( SORT_MALLOC(MicroFacetRefraction)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , dist , ext_eta , in_eta , weight ) );
}

AshikhmanShirleyNode::AshikhmanShirleyNode()
{
    m_props.insert(make_pair("Diffuse", &diffuse));
    m_props.insert(make_pair("Specular", &specular));
    m_props.insert(make_pair("RoughnessU", &roughnessU));
    m_props.insert(make_pair("RoughnessV", &roughnessV));
}

void AshikhmanShirleyNode::UpdateBSDF(Bsdf* bsdf, Spectrum weight)
{
    const float ru = saturate(roughnessU.GetPropertyValue(bsdf).x);
    const float rv = saturate(roughnessV.GetPropertyValue(bsdf).x);
    const auto specDiffuse = diffuse.GetPropertyValue(bsdf).ToSpectrum();
    const auto specSpecular = specular.GetPropertyValue(bsdf).x;

    bsdf->AddBxdf(SORT_MALLOC(AshikhmanShirley)(specDiffuse, specSpecular, ru, rv , weight ));
}

MerlNode::MerlNode()
{
    m_props.insert( make_pair( "Filename" , &merlfile ) );
}

void MerlNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    merl.m_weight = weight;
    bsdf->AddBxdf( &merl );
}

void MerlNode::PostProcess()
{
    if( m_post_processed )
        return;
    
    if( merlfile.str.empty() == false )
        merl.LoadData( merlfile.str );
}

FourierBxdfNode::FourierBxdfNode()
{
    m_props.insert( make_pair( "Filename" , &fourierBxdfFile ) );
}

void FourierBxdfNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    fourierBxdf.m_weight = weight;
    bsdf->AddBxdf( &fourierBxdf );
}

// post process
void FourierBxdfNode::PostProcess()
{
    if( m_post_processed )
        return;

    if( fourierBxdfFile.str.empty() == false )
        fourierBxdf.LoadData( fourierBxdfFile.str );
}
