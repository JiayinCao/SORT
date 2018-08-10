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

#include "managers/memmanager.h"
#include "bxdf_node.h"
#include "bsdf/lambert.h"
#include "bsdf/merl.h"
#include "bsdf/orennayar.h"
#include "bsdf/microfacet.h"
#include "bsdf/fourierbxdf.h"
#include "bsdf/ashikhmanshirley.h"
#include "bsdf/phong.h"
#include "bsdf/bsdf.h"
#include "bsdf/coat.h"

IMPLEMENT_CREATOR( AshikhmanShirleyNode );
IMPLEMENT_CREATOR( PhongNode );
IMPLEMENT_CREATOR( LambertNode );
IMPLEMENT_CREATOR( LambertTransmissionNode );
IMPLEMENT_CREATOR( OrenNayarNode );
IMPLEMENT_CREATOR( MicrofacetReflectionNode );
IMPLEMENT_CREATOR( MicrofacetRefractionNode );
IMPLEMENT_CREATOR( FourierBxdfNode );
IMPLEMENT_CREATOR( MerlNode );
IMPLEMENT_CREATOR( CoateNode );

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
    const Vector n = GET_MATERIALNODE_PROPERTY_VECTOR(normal);
    const auto  bc = GET_MATERIALNODE_PROPERTY_SPECTRUM(baseColor);
	bsdf->AddBxdf( SORT_MALLOC(Lambert)(bc , weight , n ) );
}

LambertTransmissionNode::LambertTransmissionNode()
{
    REGISTER_MATERIALNODE_PROPERTY( "Diffuse" , baseColor );
}

void LambertTransmissionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight ){
    const Vector n = GET_MATERIALNODE_PROPERTY_VECTOR(normal);
    const auto  bc = GET_MATERIALNODE_PROPERTY_SPECTRUM(baseColor);
    bsdf->AddBxdf( SORT_MALLOC(LambertTransmission)(bc, weight , n ) );
}

OrenNayarNode::OrenNayarNode()
{
	REGISTER_MATERIALNODE_PROPERTY( "Diffuse" , baseColor );
	REGISTER_MATERIALNODE_PROPERTY( "Roughness" , roughness );
}

void OrenNayarNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = GET_MATERIALNODE_PROPERTY_VECTOR(normal);
    const auto bc = GET_MATERIALNODE_PROPERTY_SPECTRUM(baseColor);
    const auto r = GET_MATERIALNODE_PROPERTY_FLOAT(roughness);
    bsdf->AddBxdf( SORT_MALLOC(OrenNayar)(bc, r , weight , n ) );
}

MicrofacetReflectionNode::MicrofacetReflectionNode()
{
	REGISTER_MATERIALNODE_PROPERTY( "BaseColor" , baseColor );
	REGISTER_MATERIALNODE_PROPERTY( "MicroFacetDistribution" , mf_dist );
	REGISTER_MATERIALNODE_PROPERTY( "RoughnessU" , roughnessU );
    REGISTER_MATERIALNODE_PROPERTY( "RoughnessV" , roughnessV );
	REGISTER_MATERIALNODE_PROPERTY( "Interior_IOR" , eta );
	REGISTER_MATERIALNODE_PROPERTY( "Absorption_Coefficient" , k );
}

void MicrofacetReflectionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = GET_MATERIALNODE_PROPERTY_VECTOR(normal);
	float ru = saturate(GET_MATERIALNODE_PROPERTY_FLOAT(roughnessU));
    float rv = saturate(GET_MATERIALNODE_PROPERTY_FLOAT(roughnessV));
	MicroFacetDistribution* dist = nullptr;
	if( mf_dist.str == "Blinn" )
		dist = SORT_MALLOC(Blinn)( ru , rv );
	else if( mf_dist.str == "Beckmann" )
		dist = SORT_MALLOC(Beckmann)( ru , rv );
	else
		dist = SORT_MALLOC(GGX)( ru , rv );	// GGX is default

	const Fresnel* frenel = SORT_MALLOC( FresnelConductor )(GET_MATERIALNODE_PROPERTY_SPECTRUM(eta), GET_MATERIALNODE_PROPERTY_SPECTRUM(k));
    const auto bc = GET_MATERIALNODE_PROPERTY_SPECTRUM(baseColor);
	bsdf->AddBxdf( SORT_MALLOC(MicroFacetReflection)(bc , frenel , dist , weight , n ) );
}

MicrofacetRefractionNode::MicrofacetRefractionNode()
{
	REGISTER_MATERIALNODE_PROPERTY( "BaseColor" , baseColor );
	REGISTER_MATERIALNODE_PROPERTY( "MicroFacetDistribution" , mf_dist );
    REGISTER_MATERIALNODE_PROPERTY( "RoughnessU" , roughnessU );
    REGISTER_MATERIALNODE_PROPERTY( "RoughnessV" , roughnessV );
	REGISTER_MATERIALNODE_PROPERTY( "Interior_IOR" , in_ior );
	REGISTER_MATERIALNODE_PROPERTY( "Exterior_IOR" , ext_ior );
}

void MicrofacetRefractionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = GET_MATERIALNODE_PROPERTY_VECTOR(normal);
    const float ru = saturate(GET_MATERIALNODE_PROPERTY_FLOAT(roughnessU));
    const float rv = saturate(GET_MATERIALNODE_PROPERTY_FLOAT(roughnessV));
	MicroFacetDistribution* dist = nullptr;
	if( mf_dist.str == "Blinn" )
		dist = SORT_MALLOC(Blinn)( ru , rv );
	else if( mf_dist.str == "Beckmann" )
		dist = SORT_MALLOC(Beckmann)(ru, rv);
	else
		dist = SORT_MALLOC(GGX)(ru, rv);	// GGX is default

    const float in_eta = GET_MATERIALNODE_PROPERTY_FLOAT(in_ior);     // index of refraction inside the surface
	const float ext_eta = GET_MATERIALNODE_PROPERTY_FLOAT(ext_ior);   // index of refraction outside the surface
    const auto bc = GET_MATERIALNODE_PROPERTY_SPECTRUM(baseColor);    // Base color
	bsdf->AddBxdf( SORT_MALLOC(MicroFacetRefraction)(bc, dist , ext_eta , in_eta , weight , n ) );
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
    const Vector n = GET_MATERIALNODE_PROPERTY_VECTOR(normal);
    const float ru = saturate(GET_MATERIALNODE_PROPERTY_FLOAT(roughnessU));
    const float rv = saturate(GET_MATERIALNODE_PROPERTY_FLOAT(roughnessV));
    const auto specDiffuse = GET_MATERIALNODE_PROPERTY_SPECTRUM(diffuse);
    const auto specSpecular = GET_MATERIALNODE_PROPERTY_FLOAT(specular);
    if (specDiffuse.GetIntensity()<= 0.0f)
        return;
    bsdf->AddBxdf(SORT_MALLOC(AshikhmanShirley)(specDiffuse, specSpecular, ru, rv , weight , n ));
}

PhongNode::PhongNode()
{
    REGISTER_MATERIALNODE_PROPERTY("Diffuse", diffuse);
    REGISTER_MATERIALNODE_PROPERTY("Specular", specular);
    REGISTER_MATERIALNODE_PROPERTY("SpecularPower", power);
    REGISTER_MATERIALNODE_PROPERTY("DiffuseRatio", diffRatio);
}

void PhongNode::UpdateBSDF(Bsdf* bsdf, Spectrum weight)
{
    const Vector n = GET_MATERIALNODE_PROPERTY_VECTOR(normal);
    const float p = GET_MATERIALNODE_PROPERTY_FLOAT(power);
    const float r = saturate(GET_MATERIALNODE_PROPERTY_FLOAT(diffRatio));
    const Spectrum& specDiffuse = GET_MATERIALNODE_PROPERTY_SPECTRUM(diffuse);
    const Spectrum& specSpecular = GET_MATERIALNODE_PROPERTY_SPECTRUM(specular);
    if (specDiffuse.GetIntensity() + specSpecular.GetIntensity() <= 0.0f)
        return;
    bsdf->AddBxdf(SORT_MALLOC(Phong)(specDiffuse * r, specSpecular * ( 1.0f - r ), p, weight, n));
}

MerlNode::MerlNode()
{
    REGISTER_MATERIALNODE_PROPERTY( "Filename" , merlfile );
}

void MerlNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = GET_MATERIALNODE_PROPERTY_VECTOR(normal);
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
    const Vector n = GET_MATERIALNODE_PROPERTY_VECTOR(normal);
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

CoateNode::CoateNode()
{
    REGISTER_MATERIALNODE_PROPERTY( "BaseColor" , basecolor );
    REGISTER_MATERIALNODE_PROPERTY( "Thickness" , thickness );
    REGISTER_MATERIALNODE_PROPERTY( "Roughness" , roughness );
    REGISTER_MATERIALNODE_PROPERTY( "Sigma" , sigma );
    REGISTER_MATERIALNODE_PROPERTY( "IOR" , ior );
}

void CoateNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = GET_MATERIALNODE_PROPERTY_VECTOR(normal);
    const Spectrum bc = GET_MATERIALNODE_PROPERTY_SPECTRUM(basecolor);
    const Spectrum s = GET_MATERIALNODE_PROPERTY_SPECTRUM(sigma);
    const float t = GET_MATERIALNODE_PROPERTY_FLOAT(thickness);
    const float r = GET_MATERIALNODE_PROPERTY_FLOAT(roughness);
    const float i = GET_MATERIALNODE_PROPERTY_FLOAT(ior);
    bsdf->AddBxdf( SORT_MALLOC(Coat)( bc, t, i, r, s, weight, n ) );
}
