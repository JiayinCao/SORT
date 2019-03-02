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

#include "bsdf_node.h"
#include "core/memory.h"
#include "bsdf/lambert.h"
#include "bsdf/merl.h"
#include "bsdf/orennayar.h"
#include "bsdf/microfacet.h"
#include "bsdf/disney.h"
#include "bsdf/dielectric.h"
#include "bsdf/bsdf.h"
#include "bsdf/blend.h"
#include "bsdf/doublesided.h"
#include "bsdf/hair.h"
#include "math/utils.h"

IMPLEMENT_RTTI( DisneyPrincipleNode );
IMPLEMENT_RTTI( PrincipleMaterialNode );
IMPLEMENT_RTTI( MatteMaterialNode );
IMPLEMENT_RTTI( PlasticMaterialNode );
IMPLEMENT_RTTI( GlassMaterialNode );
IMPLEMENT_RTTI( MirrorMaterialNode );
IMPLEMENT_RTTI( MeasuredMaterialNode );
IMPLEMENT_RTTI( BlendMaterialNode );
IMPLEMENT_RTTI( DoubleSidedMaterialNode );
IMPLEMENT_RTTI( HairMaterialNode );

IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( Result , DisneyPrincipleNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(bc,basecolor);
    SORT_MATERIAL_GET_PROP_FLOAT(ss,subsurface);
    SORT_MATERIAL_GET_PROP_FLOAT(m,metallic);
    SORT_MATERIAL_GET_PROP_FLOAT(s,specular);
    SORT_MATERIAL_GET_PROP_FLOAT(st,specularTint);
    SORT_MATERIAL_GET_PROP_FLOAT(r,roughness);
    SORT_MATERIAL_GET_PROP_FLOAT(a,anisotropic);
    SORT_MATERIAL_GET_PROP_FLOAT(sh,sheen);
    SORT_MATERIAL_GET_PROP_FLOAT(sht,sheenTint);
    SORT_MATERIAL_GET_PROP_FLOAT(cc,clearcoat);
    SORT_MATERIAL_GET_PROP_FLOAT(ccg,clearcoatGloss);
    
    bsdf->AddBxdf(SORT_MALLOC(DisneyBRDF)( bc , ss , m , s , st , r , a , sh , sht , cc , ccg , weight , n ));
IMPLEMENT_OUTPUT_BSDF_SOCKET_END

IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( Result , PrincipleMaterialNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(basecolor,baseColor);
    SORT_MATERIAL_GET_PROP_FLOAT(spec,specular);
    SORT_MATERIAL_GET_PROP_FLOAT(m,metallic);
    SORT_MATERIAL_GET_PROP_FLOAT(ru, roughnessU);
    SORT_MATERIAL_GET_PROP_FLOAT(rv, roughnessV);
    
    const auto eta = Spectrum( 0.37f , 0.37f , 0.37f );
    const Spectrum k( 2.82f );
    const auto dist = SORT_MALLOC(GGX)(ru , rv);
    
    const auto fresnel = SORT_MALLOC( FresnelConductor )( eta , k );
    
    bsdf->AddBxdf(SORT_MALLOC(Lambert)(basecolor , weight * (1 - m) * 0.92f , n));
    bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)(basecolor, fresnel, dist , weight * (m * 0.92f + 0.08f * spec) , n));
IMPLEMENT_OUTPUT_BSDF_SOCKET_END

IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( Result , MatteMaterialNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(basecolor,baseColor);
    SORT_MATERIAL_GET_PROP_FLOAT(rn,roughness);
    
    if( !basecolor.IsBlack() ){
        if( rn == 0.0f )
            bsdf->AddBxdf(SORT_MALLOC(Lambert)( basecolor , weight , n ));
        else
            bsdf->AddBxdf( SORT_MALLOC(OrenNayar)( basecolor , rn , weight , n ) );
    }
IMPLEMENT_OUTPUT_BSDF_SOCKET_END

IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( Result , PlasticMaterialNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(diff,diffuse);
    SORT_MATERIAL_GET_PROP_COLOR(spec,specular);
    SORT_MATERIAL_GET_PROP_FLOAT(rough,roughness);
    
    if( !diff.IsBlack() )
        bsdf->AddBxdf(SORT_MALLOC(Lambert)( diff , weight , n ));
    
    if( !spec.IsBlack() ){
        const auto fresnel = SORT_MALLOC(FresnelDielectric)(1.0f, 1.5f);
        const auto dist = SORT_MALLOC(GGX)( rough , rough );   // GGX
        bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)( spec , fresnel , dist , weight , n ));
    }
IMPLEMENT_OUTPUT_BSDF_SOCKET_END

IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( Result , GlassMaterialNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(r,reflectance);
    SORT_MATERIAL_GET_PROP_COLOR(t,transmittance);
    SORT_MATERIAL_GET_PROP_FLOAT(roughU,roughnessU);
    SORT_MATERIAL_GET_PROP_FLOAT(roughV,roughnessV);
    
    if( r.IsBlack() && t.IsBlack() ) return;
    const auto dist = SORT_MALLOC(GGX)(roughU, roughV);
    bsdf->AddBxdf(SORT_MALLOC(Dielectric)(r, t, dist, 1.0f, 1.5f, weight, n));
IMPLEMENT_OUTPUT_BSDF_SOCKET_END

IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( Result , MirrorMaterialNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(color,basecolor);
    if( color.IsBlack() ) return;
    
    // This is not perfect mirror reflection in term of physically based rendering, but it looks good enough so far.
    const auto dist = SORT_MALLOC(GGX)( 0.0f , 0.0f );   // GGX
    if( !color.IsBlack() ){
        const auto fresnel = SORT_MALLOC( FresnelNo )();
        bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)( color , fresnel , dist , weight , n ));
    }
IMPLEMENT_OUTPUT_BSDF_SOCKET_END

IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( Result , MeasuredMaterialNode )
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_STR(type,bxdfType);
    
    if( type == "Fourier" )
        bsdf->AddBxdf( SORT_MALLOC(FourierBxdf)( node->m_fourierBxdfData , weight , n ) );
    else if( type == "MERL" )
        bsdf->AddBxdf( SORT_MALLOC(Merl)(node->m_merlData, weight , n) );
IMPLEMENT_OUTPUT_BSDF_SOCKET_END

void MeasuredMaterialNode::PostProcess(){
    if( m_post_processed )
        return;
    
    Bsdf* bsdf = nullptr;
    SORT_MATERIAL_GET_PROP_STR_TMP(type,bxdfType);
    SORT_MATERIAL_GET_PROP_STR_TMP(file,bxdfFilePath);
    
    if( file.empty() )
        return;

    if( type == "Fourier" )
        m_fourierBxdfData.LoadData( file );
    else if( type == "MERL" )
        m_merlData.LoadData( file );
    BxdfNode::PostProcess();
}

IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( Result , BlendMaterialNode )
    SORT_MATERIAL_GET_PROP_FLOAT(t,factor);
    
    auto bsdf0 = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
    auto bsdf1 = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
    Spectrum fullWeight(1.0f);
    node->bxdf0.UpdateBSDF(bsdf0,fullWeight);
    node->bxdf1.UpdateBSDF(bsdf1,fullWeight);
    bsdf->AddBxdf(SORT_MALLOC(Blend)(bsdf0, bsdf1, t, weight));
IMPLEMENT_OUTPUT_BSDF_SOCKET_END

IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( Result , DoubleSidedMaterialNode )
    auto bsdf0 = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
    auto bsdf1 = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
    Spectrum fullWeight(1.0f);
    node->bxdf0.UpdateBSDF(bsdf0,fullWeight);
    node->bxdf1.UpdateBSDF(bsdf1,fullWeight);
    bsdf->AddBxdf(SORT_MALLOC(DoubleSided)(bsdf0, bsdf1, weight));
IMPLEMENT_OUTPUT_BSDF_SOCKET_END

IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( Result , HairMaterialNode )
    SORT_MATERIAL_GET_PROP_COLOR(hc,hairColor);
    SORT_MATERIAL_GET_PROP_FLOAT(lr,longtitudinalRoughness);
    SORT_MATERIAL_GET_PROP_FLOAT(ar,azimuthalRoughness);
    SORT_MATERIAL_GET_PROP_FLOAT(ior,indexOfRefraction);

    // A Practical and Controllable Hair and Fur Model for Production Path Tracing
    // https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/147/asset/siggraph2015Fur.pdf
    const auto inv = 1.0f / (5.969f - 0.215f * ar + 2.532f * Pow<2>(ar) - 10.73f * Pow<3>(ar) + 5.574f * Pow<4>(ar) + 0.245f * Pow<5>(ar));
    const auto func = [=]( const float x ) { return SQR( std::log(x) * inv ); };
    Spectrum sigma( func( hc.GetR() ) , func( hc.GetG() ) , func( hc.GetB() ) );

    Spectrum fullWeight(1.0f);
	bsdf->AddBxdf( SORT_MALLOC(Hair)(sigma, lr, ar, ior, fullWeight) );
IMPLEMENT_OUTPUT_BSDF_SOCKET_END