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
IMPLEMENT_CREATOR( CoatNode );

void LambertNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight ){
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(bc,baseColor);
	bsdf->AddBxdf( SORT_MALLOC(Lambert)(bc , weight , n) );
}

void LambertTransmissionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight ){
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(bc,baseColor);
    bsdf->AddBxdf( SORT_MALLOC(LambertTransmission)(bc, weight , n ) );
}

void OrenNayarNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(bc,baseColor);
    SORT_MATERIAL_GET_PROP_FLOAT(r,roughness);
    bsdf->AddBxdf( SORT_MALLOC(OrenNayar)(bc, r , weight , n ) );
}

void MicrofacetReflectionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(bc,baseColor);
    SORT_MATERIAL_GET_PROP_FLOAT(ru,roughnessU);
    SORT_MATERIAL_GET_PROP_FLOAT(rv,roughnessV);
    SORT_MATERIAL_GET_PROP_STR(type,mf_dist);
    SORT_MATERIAL_GET_PROP_COLOR(_eta,eta);
    SORT_MATERIAL_GET_PROP_COLOR(_k,k);
    
	MicroFacetDistribution* dist = nullptr;
	if( type == "Blinn" )
		dist = SORT_MALLOC(Blinn)( ru , rv );
	else if( type == "Beckmann" )
		dist = SORT_MALLOC(Beckmann)( ru , rv );
	else
		dist = SORT_MALLOC(GGX)( ru , rv );	// GGX is default

	const Fresnel* frenel = SORT_MALLOC( FresnelConductor )(_eta, _k);
	bsdf->AddBxdf( SORT_MALLOC(MicroFacetReflection)(bc , frenel , dist , weight , n ) );
}

void MicrofacetRefractionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(bc,baseColor);
    SORT_MATERIAL_GET_PROP_FLOAT(ru,roughnessU);
    SORT_MATERIAL_GET_PROP_FLOAT(rv,roughnessV);
    SORT_MATERIAL_GET_PROP_STR(type,mf_dist);
    SORT_MATERIAL_GET_PROP_FLOAT(in_eta,in_ior);    // index of refraction inside the surface
    SORT_MATERIAL_GET_PROP_FLOAT(ext_eta,ext_ior);  // index of refraction outside the surface
    
    MicroFacetDistribution* dist = nullptr;
    if( type == "Blinn" )
        dist = SORT_MALLOC(Blinn)( ru , rv );
    else if( type == "Beckmann" )
        dist = SORT_MALLOC(Beckmann)( ru , rv );
    else
        dist = SORT_MALLOC(GGX)( ru , rv );    // GGX is default

	bsdf->AddBxdf( SORT_MALLOC(MicroFacetRefraction)(bc, dist , ext_eta , in_eta , weight , n ) );
}

void AshikhmanShirleyNode::UpdateBSDF(Bsdf* bsdf, Spectrum weight)
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(specDiffuse,diffuse);
    SORT_MATERIAL_GET_PROP_FLOAT(specSpecular,specular);
    SORT_MATERIAL_GET_PROP_FLOAT(ru,roughnessU);
    SORT_MATERIAL_GET_PROP_FLOAT(rv,roughnessV);
    bsdf->AddBxdf(SORT_MALLOC(AshikhmanShirley)(specDiffuse, specSpecular, ru, rv , weight , n ));
}

void PhongNode::UpdateBSDF(Bsdf* bsdf, Spectrum weight)
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(specDiffuse,diffuse);
    SORT_MATERIAL_GET_PROP_COLOR(specSpecular,specular);
    SORT_MATERIAL_GET_PROP_FLOAT(p,power);
    SORT_MATERIAL_GET_PROP_FLOAT(r,diffRatio);
    
    if (specDiffuse.GetIntensity() + specSpecular.GetIntensity() <= 0.0f)
        return;
    bsdf->AddBxdf(SORT_MALLOC(Phong)(specDiffuse * r, specSpecular * ( 1.0f - r ), p, weight, n));
}

void MerlNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    bsdf->AddBxdf( SORT_MALLOC(Merl)( data , weight , n ) );
}

void MerlNode::PostProcess()
{
    if( m_post_processed )
        return;
    
    Bsdf* bsdf = nullptr;
    SORT_MATERIAL_GET_PROP_STR(file,merlfile);
    
    if( file.empty() == false )
        data.LoadData( file );
    BxdfNode::PostProcess();
}

void FourierBxdfNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    bsdf->AddBxdf( SORT_MALLOC(FourierBxdf)( fourierBxdfData , weight , n ) );
}

// post process
void FourierBxdfNode::PostProcess()
{
    if( m_post_processed )
        return;

    Bsdf* bsdf = nullptr;
    SORT_MATERIAL_GET_PROP_STR(file,fourierBxdfFile);
    
    if( file.empty() == false )
        fourierBxdfData.LoadData( file );
    BxdfNode::PostProcess();
}

void CoatNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(s,sigma);
    SORT_MATERIAL_GET_PROP_FLOAT(r,roughness);
    SORT_MATERIAL_GET_PROP_FLOAT(i,ior);
    SORT_MATERIAL_GET_PROP_FLOAT(t,thickness);
    
    Bsdf* bottom = SORT_MALLOC(Bsdf)( bsdf->GetIntersection() , true );
    bxdf.UpdateBsdf( bottom , weight );
    bsdf->AddBxdf( SORT_MALLOC(Coat)( t, i, r, s, bottom, weight, n ) );
}
