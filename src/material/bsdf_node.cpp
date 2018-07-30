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
#include "bsdf/lambert.h"
#include "bsdf/merl.h"
#include "bsdf/orennayar.h"
#include "bsdf/microfacet.h"
#include "bsdf/disney.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"

IMPLEMENT_CREATOR( DisneyPrincipleNode );
IMPLEMENT_CREATOR( LayeredMaterialNode );
IMPLEMENT_CREATOR( PrincipleMaterialNode );
IMPLEMENT_CREATOR( MatteMaterialNode );
IMPLEMENT_CREATOR( PlasticMaterialNode );
IMPLEMENT_CREATOR( GlassMaterialNode );
IMPLEMENT_CREATOR( MirrorMaterialNode );
IMPLEMENT_CREATOR( MeasuredMaterialNode );

DisneyPrincipleNode::DisneyPrincipleNode()
{
    m_props.insert( make_pair( "BaseColor" , &basecolor ) );
    m_props.insert( make_pair( "SubSurface" , &subsurface ) );
    m_props.insert( make_pair( "Metallic" , &metallic ) );
    m_props.insert( make_pair( "Specular" , &specular ) );
    m_props.insert( make_pair( "SpecularTint" , &specularTint ) );
    m_props.insert( make_pair( "Roughness" , &roughness ) );
    m_props.insert( make_pair( "Anisotropic" , &anisotropic ) );
    m_props.insert( make_pair( "Sheen" , &sheen ) );
    m_props.insert( make_pair( "SheenTint" , &sheenTint ) );
    m_props.insert( make_pair( "Clearcoat" , &clearcoat ) );
    m_props.insert( make_pair( "ClearcoatGloss" , &clearcoatGloss ) );
}

void DisneyPrincipleNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector    n = normal.GetPropertyValue(bsdf).ToVector();
    const Spectrum  bc = basecolor.GetPropertyValue(bsdf).ToSpectrum();
    const float     ss = subsurface.GetPropertyValue(bsdf).x;
    const float     m = metallic.GetPropertyValue(bsdf).x;
    const float     s = specular.GetPropertyValue(bsdf).x;
    const float     st = specularTint.GetPropertyValue(bsdf).x;
    const float     r = roughness.GetPropertyValue(bsdf).x;
    const float     a = anisotropic.GetPropertyValue(bsdf).x;
    const float     sh = sheen.GetPropertyValue(bsdf).x;
    const float     sht = sheenTint.GetPropertyValue(bsdf).x;
    const float     cc = clearcoat.GetPropertyValue(bsdf).x;
    const float     ccg = clearcoatGloss.GetPropertyValue(bsdf).x;
    bsdf->AddBxdf(SORT_MALLOC(DisneyBRDF)( bc , ss , m , s , st , r , a , sh , sht , cc , ccg , weight , n ));
}

LayeredMaterialNode::LayeredMaterialNode(){
    for( int i = 0 ; i < MAX_BXDF_COUNT ; ++i ){
        m_props.insert( make_pair( "Bxdf" + to_string(i) , &bxdfs[i] ) );
        m_props.insert( make_pair( "Weight" + to_string(i) , &weights[i] ));
    }
}

bool LayeredMaterialNode::CheckValidation()
{
    for( auto bxdf_prop : bxdfs ){
        if( bxdf_prop.node ){
            // Only bxdf nodes are allowed to be connected to Layered Bxdf node
            MAT_NODE_TYPE sub_type = bxdf_prop.node->getNodeType();
            if( (sub_type & MAT_NODE_BXDF) == 0 )
                return false;
        }
    }
    
    for( auto weight_prop : weights ){
        if( weight_prop.node ){
            // Only constant node is allowed to be connected with Layered Bxdf weight node
            MAT_NODE_TYPE sub_type = weight_prop.node->getNodeType();
            if( (sub_type & MAT_NODE_CONSTANT) == 0 )
                return false;
        }
    }
    
    return MaterialNode::CheckValidation();
}

void LayeredMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    for( int i = 0 ; i < MAX_BXDF_COUNT ; ++i )
        bxdfs[i].UpdateBsdf(bsdf, weights[i].GetPropertyValue(bsdf).ToSpectrum() * weight );
}

PrincipleMaterialNode::PrincipleMaterialNode()
{
    m_props.insert( make_pair( "BaseColor" , &baseColor ) );
    m_props.insert( make_pair( "RoughnessU" , &roughnessU ) );
    m_props.insert( make_pair( "RoughnessV" , &roughnessV ) );
    m_props.insert( make_pair( "Metallic" , &metallic ) );
    m_props.insert( make_pair( "Specular" , &specular ) );
}

void PrincipleMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    const Spectrum  basecolor = baseColor.GetPropertyValue(bsdf).ToSpectrum();
    const float     spec = specular.GetPropertyValue(bsdf).x;
    
    const Spectrum eta = Spectrum( 0.37f , 0.37f , 0.37f );
    const Spectrum k( 2.82f );
    const MicroFacetDistribution* dist = SORT_MALLOC(GGX)( roughnessU.GetPropertyValue(bsdf).x , roughnessV.GetPropertyValue(bsdf).x );
    
    const Fresnel* fresnel = SORT_MALLOC( FresnelConductor )( eta , k );
    
    const float m = metallic.GetPropertyValue(bsdf).x;
    bsdf->AddBxdf(SORT_MALLOC(Lambert)(baseColor.GetPropertyValue(bsdf).ToSpectrum() , weight * (1 - m) * 0.92f , n));
    bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)(basecolor, fresnel, dist , weight * (m * 0.92f + 0.08f * spec) , n));
}

MatteMaterialNode::MatteMaterialNode()
{
    m_props.insert( make_pair( "BaseColor" , &baseColor ) );
    m_props.insert( make_pair( "Roughness" , &roughness ) );
}

void MatteMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    const Spectrum reflectance(baseColor.GetPropertyValue(bsdf).ToSpectrum());
    if( !reflectance.IsBlack() ){
        const float rn = roughness.GetPropertyValue(bsdf).x;
        if( rn == 0.0f )
            bsdf->AddBxdf(SORT_MALLOC(Lambert)( reflectance , weight , n ));
        else
            bsdf->AddBxdf( SORT_MALLOC(OrenNayar)( reflectance , rn , weight , n ) );
    }
}

PlasticMaterialNode::PlasticMaterialNode()
{
    m_props.insert( make_pair( "Diffuse" , &diffuse ) );
    m_props.insert( make_pair( "Specular" , &specular ) );
    m_props.insert( make_pair( "Roughness" , &roughness ) );
}

void PlasticMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    const Spectrum diff(diffuse.GetPropertyValue(bsdf).ToSpectrum());
    if( !diff.IsBlack() )
        bsdf->AddBxdf(SORT_MALLOC(Lambert)( diff , weight , n ));
    
    const Spectrum spec(specular.GetPropertyValue(bsdf).ToSpectrum());
    if( !spec.IsBlack() ){
        const Fresnel *fresnel = SORT_MALLOC(FresnelDielectric)(1.0f, 1.2f);
        const float rough = roughness.GetPropertyValue(bsdf).x;
        const MicroFacetDistribution* dist = SORT_MALLOC(GGX)( rough , rough );   // GGX
        bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)( spec , fresnel , dist , weight , n ));
    }
}

MeasuredMaterialNode::MeasuredMaterialNode()
{
    m_props.insert( make_pair( "Type" , &bxdfType ) );
    m_props.insert( make_pair( "Filename" , &bxdfFilePath ) );
}

void MeasuredMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    if( bxdfType.str == "Fourier" ){
        // This may not even be correct if I have multiple of this BXDF with different weight
        fourierBxdf.UpdateWeight( weight );
        bsdf->AddBxdf( &fourierBxdf );
    }else if( bxdfType.str == "MERL" ){
        // This may not even be correct if I have multiple of this BXDF with different weight
        merlBxdf.UpdateWeight( weight );
        bsdf->AddBxdf( &merlBxdf );
    }
}

// post process
void MeasuredMaterialNode::PostProcess()
{
    if( m_post_processed || bxdfFilePath.str.empty() )
        return;
    
    if( bxdfType.str == "Fourier" )
        fourierBxdf.LoadData( bxdfFilePath.str );
    else if( bxdfType.str == "MERL" )
        merlBxdf.LoadData( bxdfFilePath.str );
}

GlassMaterialNode::GlassMaterialNode()
{
    m_props.insert( make_pair( "Reflectance" , &reflectance ) );
    m_props.insert( make_pair( "Transmittance" , &transmittance ) );
    m_props.insert( make_pair( "Roughness" , &roughness ) );
}

void GlassMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    const float rough = roughness.GetPropertyValue(bsdf).x;
    const Spectrum r = reflectance.GetPropertyValue(bsdf).ToSpectrum();
    const Spectrum t = transmittance.GetPropertyValue(bsdf).ToSpectrum();
    if( r.IsBlack() && t.IsBlack() )
        return;
    
    const MicroFacetDistribution* dist = SORT_MALLOC(GGX)( rough , rough );   // GGX
    if( !r.IsBlack() ){
        const Fresnel* fresnel = SORT_MALLOC( FresnelDielectric )( 1.0f , 1.5f );
        bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)( r , fresnel , dist , weight , n ));
    }
    if( !t.IsBlack() )
        bsdf->AddBxdf(SORT_MALLOC(MicroFacetRefraction)( t , dist , 1.0f , 1.5f , weight , n ));
}

MirrorMaterialNode::MirrorMaterialNode(){
    m_props.insert( make_pair( "BaseColor" , &basecolor ) );
}

void MirrorMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    const Spectrum color = basecolor.GetPropertyValue(bsdf).ToSpectrum();
    if( color.IsBlack() ) return;
    
    // This is not perfect mirror reflection in term of physically based rendering, but it looks good enough so far.
    const MicroFacetDistribution* dist = SORT_MALLOC(GGX)( 0.0f , 0.0f );   // GGX
    if( !color.IsBlack() ){
        const Fresnel* fresnel = SORT_MALLOC( FresnelNo )();
        bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)( color , fresnel , dist , weight , n ));
    }
}
