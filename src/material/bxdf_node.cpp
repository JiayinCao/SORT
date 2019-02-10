/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "core/memory.h"
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

IMPLEMENT_RTTI( AshikhmanShirleyNode );
IMPLEMENT_RTTI( PhongNode );
IMPLEMENT_RTTI( LambertNode );
IMPLEMENT_RTTI( LambertTransmissionNode );
IMPLEMENT_RTTI( OrenNayarNode );
IMPLEMENT_RTTI( MicrofacetReflectionNode );
IMPLEMENT_RTTI( MicrofacetRefractionNode );
IMPLEMENT_RTTI( FourierBxdfNode );
IMPLEMENT_RTTI( MerlNode );
IMPLEMENT_RTTI( CoatNode );

IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , LambertNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(bc,baseColor);
	bsdf->AddBxdf( SORT_MALLOC(Lambert)(bc , weight , n) );
IMPLEMENT_OUTPUT_CHANNEL_END

IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , LambertTransmissionNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(bc,baseColor);
    bsdf->AddBxdf( SORT_MALLOC(LambertTransmission)(bc, weight , n ) );
IMPLEMENT_OUTPUT_CHANNEL_END

IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , OrenNayarNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(bc,baseColor);
    SORT_MATERIAL_GET_PROP_FLOAT(r,roughness);
    bsdf->AddBxdf( SORT_MALLOC(OrenNayar)(bc, r , weight , n ) );
IMPLEMENT_OUTPUT_CHANNEL_END

IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , MicrofacetReflectionNode )
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

	const auto frenel = SORT_MALLOC( FresnelConductor )(_eta, _k);
	bsdf->AddBxdf( SORT_MALLOC(MicroFacetReflection)(bc , frenel , dist , weight , n ) );
IMPLEMENT_OUTPUT_CHANNEL_END

IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , MicrofacetRefractionNode )
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
IMPLEMENT_OUTPUT_CHANNEL_END

IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , AshikhmanShirleyNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(specDiffuse,diffuse);
    SORT_MATERIAL_GET_PROP_FLOAT(specSpecular,specular);
    SORT_MATERIAL_GET_PROP_FLOAT(ru,roughnessU);
    SORT_MATERIAL_GET_PROP_FLOAT(rv,roughnessV);
    bsdf->AddBxdf(SORT_MALLOC(AshikhmanShirley)(specDiffuse, specSpecular, ru, rv , weight , n ));
IMPLEMENT_OUTPUT_CHANNEL_END

IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , PhongNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(specDiffuse,diffuse);
    SORT_MATERIAL_GET_PROP_COLOR(specSpecular,specular);
    SORT_MATERIAL_GET_PROP_FLOAT(p,power);
    SORT_MATERIAL_GET_PROP_FLOAT(r,diffRatio);
    
    if (specDiffuse.GetIntensity() + specSpecular.GetIntensity() <= 0.0f)
        return;
    bsdf->AddBxdf(SORT_MALLOC(Phong)(specDiffuse * r, specSpecular * ( 1.0f - r ), p, weight, n));
IMPLEMENT_OUTPUT_CHANNEL_END

IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , MerlNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    bsdf->AddBxdf( SORT_MALLOC(Merl)( node->data , weight , n ) );
IMPLEMENT_OUTPUT_CHANNEL_END

void MerlNode::PostProcess(){
    if( m_post_processed )
        return;
    
    Bsdf* bsdf = nullptr;
    SORT_MATERIAL_GET_PROP_STR_TMP(file,merlfile);
    
    if( file.empty() == false )
        data.LoadData( file );
    BxdfNode::PostProcess();
}

IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , FourierBxdfNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    bsdf->AddBxdf( SORT_MALLOC(FourierBxdf)( node->fourierBxdfData , weight , n ) );
IMPLEMENT_OUTPUT_CHANNEL_END

// post process
void FourierBxdfNode::PostProcess(){
    if( m_post_processed )
        return;

    Bsdf* bsdf = nullptr;
    SORT_MATERIAL_GET_PROP_STR_TMP(file,fourierBxdfFile);
    
    if( file.empty() == false )
        fourierBxdfData.LoadData( file );
    BxdfNode::PostProcess();
}

IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , CoatNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(s,sigma);
    SORT_MATERIAL_GET_PROP_FLOAT(r,roughness);
    SORT_MATERIAL_GET_PROP_FLOAT(i,ior);
    SORT_MATERIAL_GET_PROP_FLOAT(t,thickness);
    
    Bsdf* bottom = SORT_MALLOC(Bsdf)( bsdf->GetIntersection() , true );
    node->bxdf.UpdateBsdf( bottom , weight );
    bsdf->AddBxdf( SORT_MALLOC(Coat)( t, i, r, s, bottom, weight, n ) );
IMPLEMENT_OUTPUT_CHANNEL_END
