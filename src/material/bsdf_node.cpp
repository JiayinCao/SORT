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

#include "bsdf_node.h"
#include "managers/memmanager.h"
#include "bsdf/lambert.h"
#include "bsdf/merl.h"
#include "bsdf/orennayar.h"
#include "bsdf/microfacet.h"
#include "bsdf/disney.h"
#include "bsdf/dielectric.h"
#include "bsdf/bsdf.h"
#include "bsdf/blend.h"
#include "bsdf/doublesided.h"

IMPLEMENT_CREATOR( DisneyPrincipleNode );
IMPLEMENT_CREATOR( PrincipleMaterialNode );
IMPLEMENT_CREATOR( MatteMaterialNode );
IMPLEMENT_CREATOR( PlasticMaterialNode );
IMPLEMENT_CREATOR( GlassMaterialNode );
IMPLEMENT_CREATOR( MirrorMaterialNode );
IMPLEMENT_CREATOR( MeasuredMaterialNode );
IMPLEMENT_CREATOR( BlendMaterialNode );
IMPLEMENT_CREATOR( DoubleSidedMaterialNode );

void DisneyPrincipleNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
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
}

void PrincipleMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(basecolor,baseColor);
    SORT_MATERIAL_GET_PROP_FLOAT(spec,specular);
    SORT_MATERIAL_GET_PROP_FLOAT(m,metallic);
    SORT_MATERIAL_GET_PROP_FLOAT(ru, roughnessU);
    SORT_MATERIAL_GET_PROP_FLOAT(rv, roughnessV);
    
    const Spectrum eta = Spectrum( 0.37f , 0.37f , 0.37f );
    const Spectrum k( 2.82f );
    const MicroFacetDistribution* dist = SORT_MALLOC(GGX)(ru , rv);
    
    const Fresnel* fresnel = SORT_MALLOC( FresnelConductor )( eta , k );
    
    bsdf->AddBxdf(SORT_MALLOC(Lambert)(basecolor , weight * (1 - m) * 0.92f , n));
    bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)(basecolor, fresnel, dist , weight * (m * 0.92f + 0.08f * spec) , n));
}

void MatteMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(basecolor,baseColor);
    SORT_MATERIAL_GET_PROP_FLOAT(rn,roughness);
    
    if( !basecolor.IsBlack() ){
        if( rn == 0.0f )
            bsdf->AddBxdf(SORT_MALLOC(Lambert)( basecolor , weight , n ));
        else
            bsdf->AddBxdf( SORT_MALLOC(OrenNayar)( basecolor , rn , weight , n ) );
    }
}

void PlasticMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(diff,diffuse);
    SORT_MATERIAL_GET_PROP_COLOR(spec,specular);
    SORT_MATERIAL_GET_PROP_FLOAT(rough,roughness);
    
    if( !diff.IsBlack() )
        bsdf->AddBxdf(SORT_MALLOC(Lambert)( diff , weight , n ));
    
    if( !spec.IsBlack() ){
        const Fresnel *fresnel = SORT_MALLOC(FresnelDielectric)(1.0f, 1.5f);
        const MicroFacetDistribution* dist = SORT_MALLOC(GGX)( rough , rough );   // GGX
        bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)( spec , fresnel , dist , weight , n ));
    }
}

void GlassMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(r,reflectance);
    SORT_MATERIAL_GET_PROP_COLOR(t,transmittance);
    SORT_MATERIAL_GET_PROP_FLOAT(roughU,roughnessU);
    SORT_MATERIAL_GET_PROP_FLOAT(roughV,roughnessV);
    
    if( r.IsBlack() && t.IsBlack() ) return;
    const MicroFacetDistribution* dist = SORT_MALLOC(GGX)(roughU, roughV);
    bsdf->AddBxdf(SORT_MALLOC(Dielectric)(r, t, dist, 1.0f, 1.5f, weight, n));
}

void MirrorMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_COLOR(color,basecolor);
    if( color.IsBlack() ) return;
    
    // This is not perfect mirror reflection in term of physically based rendering, but it looks good enough so far.
    const MicroFacetDistribution* dist = SORT_MALLOC(GGX)( 0.0f , 0.0f );   // GGX
    if( !color.IsBlack() ){
        const Fresnel* fresnel = SORT_MALLOC( FresnelNo )();
        bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)( color , fresnel , dist , weight , n ));
    }
}

void MeasuredMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    SORT_MATERIAL_GET_PROP_VECTOR(n,normal);
    SORT_MATERIAL_GET_PROP_STR(type,bxdfType);
    
    if( type == "Fourier" )
        bsdf->AddBxdf( SORT_MALLOC(FourierBxdf)( fourierBxdfData , weight , n ) );
    else if( type == "MERL" )
        bsdf->AddBxdf( SORT_MALLOC(Merl)(merlData, weight , n) );
}

// post process
void MeasuredMaterialNode::PostProcess()
{
    if( m_post_processed || bxdfFilePath.str.empty() )
        return;
    
    Bsdf* bsdf = nullptr;
    SORT_MATERIAL_GET_PROP_STR(type,bxdfType);
    SORT_MATERIAL_GET_PROP_STR(file,bxdfFilePath);
    
    if( type == "Fourier" )
        fourierBxdfData.LoadData( file );
    else if( type == "MERL" )
        merlData.LoadData( file );
    BxdfNode::PostProcess();
}

void BlendMaterialNode::UpdateBSDF(Bsdf* bsdf, Spectrum weight)
{
    SORT_MATERIAL_GET_PROP_FLOAT(t,factor);
    
    Bsdf* bsdf0 = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
    Bsdf* bsdf1 = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
    bxdf0.UpdateBsdf(bsdf0);
    bxdf1.UpdateBsdf(bsdf1);
    bsdf->AddBxdf(SORT_MALLOC(Blend)(bsdf0, bsdf1, t, weight));
}

void DoubleSidedMaterialNode::UpdateBSDF(Bsdf* bsdf, Spectrum weight)
{
    Bsdf* bsdf0 = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
    Bsdf* bsdf1 = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
    bxdf0.UpdateBsdf(bsdf0);
    bxdf1.UpdateBsdf(bsdf1);
    bsdf->AddBxdf(SORT_MALLOC(DoubleSided)(bsdf0, bsdf1, weight));
}
