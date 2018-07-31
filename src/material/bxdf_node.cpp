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
	REGISTER_MATERIALNODE_PROPERTY( "Diffuse" , baseColor );
}

void LambertNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight ){
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
	bsdf->AddBxdf( SORT_MALLOC(Lambert)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , weight , n ) );
}

LambertTransmissionNode::LambertTransmissionNode()
{
    REGISTER_MATERIALNODE_PROPERTY( "Diffuse" , baseColor );
}

void LambertTransmissionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight ){
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    bsdf->AddBxdf( SORT_MALLOC(LambertTransmission)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , weight , n ) );
}

OrenNayarNode::OrenNayarNode()
{
	REGISTER_MATERIALNODE_PROPERTY( "Diffuse" , baseColor );
	REGISTER_MATERIALNODE_PROPERTY( "Roughness" , roughness );
}

void OrenNayarNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    bsdf->AddBxdf( SORT_MALLOC(OrenNayar)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , roughness.GetPropertyValue(bsdf).x , weight , n ) );
}

MicrofacetReflectionNode::MicrofacetReflectionNode()
{
	REGISTER_MATERIALNODE_PROPERTY( "BaseColor" , baseColor );
	REGISTER_MATERIALNODE_PROPERTY( "MicroFacetDistribution" , mf_dist );
	REGISTER_MATERIALNODE_PROPERTY( "RoughnessU" , roughnessU );
    REGISTER_MATERIALNODE_PROPERTY( "RoughnessV" , roughnessV );
	REGISTER_MATERIALNODE_PROPERTY( "eta" , eta );
	REGISTER_MATERIALNODE_PROPERTY( "k" , k );
}

void MicrofacetReflectionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
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
	bsdf->AddBxdf( SORT_MALLOC(MicroFacetReflection)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , frenel , dist , weight , n ) );
}

MicrofacetRefractionNode::MicrofacetRefractionNode()
{
	REGISTER_MATERIALNODE_PROPERTY( "BaseColor" , baseColor );
	REGISTER_MATERIALNODE_PROPERTY( "MicroFacetDistribution" , mf_dist );
    REGISTER_MATERIALNODE_PROPERTY( "RoughnessU" , roughnessU );
    REGISTER_MATERIALNODE_PROPERTY( "RoughnessV" , roughnessV );
	REGISTER_MATERIALNODE_PROPERTY( "in_ior" , in_ior );
	REGISTER_MATERIALNODE_PROPERTY( "ext_ior" , ext_ior );
}

void MicrofacetRefractionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
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
	bsdf->AddBxdf( SORT_MALLOC(MicroFacetRefraction)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , dist , ext_eta , in_eta , weight , n ) );
}

AshikhmanShirleyNode::AshikhmanShirleyNode()
{
    REGISTER_MATERIALNODE_PROPERTY("Diffuse", diffuse);
    REGISTER_MATERIALNODE_PROPERTY("Specular", specular);
    REGISTER_MATERIALNODE_PROPERTY("RoughnessU", roughnessU);
    REGISTER_MATERIALNODE_PROPERTY("RoughnessV", roughnessV);
}

void AshikhmanShirleyNode::UpdateBSDF(Bsdf* bsdf, Spectrum weight)
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    const float ru = saturate(roughnessU.GetPropertyValue(bsdf).x);
    const float rv = saturate(roughnessV.GetPropertyValue(bsdf).x);
    const auto specDiffuse = diffuse.GetPropertyValue(bsdf).ToSpectrum();
    const auto specSpecular = specular.GetPropertyValue(bsdf).x;

    bsdf->AddBxdf(SORT_MALLOC(AshikhmanShirley)(specDiffuse, specSpecular, ru, rv , weight , n ));
}

MerlNode::MerlNode()
{
    REGISTER_MATERIALNODE_PROPERTY( "Filename" , merlfile );
}

void MerlNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    bsdf->AddBxdf( SORT_MALLOC(Merl)( data , weight , n ) );
}

void MerlNode::PostProcess()
{
    if( m_post_processed )
        return;
    
    if( merlfile.str.empty() == false )
        data.LoadData( merlfile.str );
    BxdfNode::PostProcess();
}

FourierBxdfNode::FourierBxdfNode()
{
    REGISTER_MATERIALNODE_PROPERTY( "Filename" , fourierBxdfFile );
}

void FourierBxdfNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    bsdf->AddBxdf( SORT_MALLOC(FourierBxdf)( fourierBxdfData , weight , n ) );
}

// post process
void FourierBxdfNode::PostProcess()
{
    if( m_post_processed )
        return;

    if( fourierBxdfFile.str.empty() == false )
        fourierBxdfData.LoadData( fourierBxdfFile.str );
    BxdfNode::PostProcess();
}
