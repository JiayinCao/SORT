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
    REGISTER_MATERIALNODE_PROPERTY( "BaseColor" , basecolor );
    REGISTER_MATERIALNODE_PROPERTY( "SubSurface" , subsurface );
    REGISTER_MATERIALNODE_PROPERTY( "Metallic" , metallic );
    REGISTER_MATERIALNODE_PROPERTY( "Specular" , specular );
    REGISTER_MATERIALNODE_PROPERTY( "SpecularTint" , specularTint );
    REGISTER_MATERIALNODE_PROPERTY( "Roughness" , roughness );
    REGISTER_MATERIALNODE_PROPERTY( "Anisotropic" , anisotropic );
    REGISTER_MATERIALNODE_PROPERTY( "Sheen" , sheen );
    REGISTER_MATERIALNODE_PROPERTY( "SheenTint" , sheenTint );
    REGISTER_MATERIALNODE_PROPERTY( "Clearcoat" , clearcoat );
    REGISTER_MATERIALNODE_PROPERTY( "ClearcoatGloss" , clearcoatGloss );
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
        REGISTER_MATERIALNODE_PROPERTY( "Bxdf" + to_string(i) , bxdfs[i] );
        REGISTER_MATERIALNODE_PROPERTY( "Weight" + to_string(i) , weights[i] );
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
    REGISTER_MATERIALNODE_PROPERTY( "BaseColor" , baseColor );
    REGISTER_MATERIALNODE_PROPERTY( "RoughnessU" , roughnessU );
    REGISTER_MATERIALNODE_PROPERTY( "RoughnessV" , roughnessV );
    REGISTER_MATERIALNODE_PROPERTY( "Metallic" , metallic );
    REGISTER_MATERIALNODE_PROPERTY( "Specular" , specular );
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
    REGISTER_MATERIALNODE_PROPERTY( "BaseColor" , baseColor );
    REGISTER_MATERIALNODE_PROPERTY( "Roughness" , roughness );
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
    REGISTER_MATERIALNODE_PROPERTY( "Diffuse" , diffuse );
    REGISTER_MATERIALNODE_PROPERTY( "Specular" , specular );
    REGISTER_MATERIALNODE_PROPERTY( "Roughness" , roughness );
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
    REGISTER_MATERIALNODE_PROPERTY( "Type" , bxdfType );
    REGISTER_MATERIALNODE_PROPERTY( "Filename" , bxdfFilePath );
}

void MeasuredMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    const Vector n = normal.GetPropertyValue(bsdf).ToVector();
    if( bxdfType.str == "Fourier" )
        bsdf->AddBxdf( SORT_MALLOC(FourierBxdf)( fourierBxdfData , weight , n ) );
    else if( bxdfType.str == "MERL" )
        bsdf->AddBxdf( SORT_MALLOC(Merl)(merlData, weight , n) );
}

// post process
void MeasuredMaterialNode::PostProcess()
{
    if( m_post_processed || bxdfFilePath.str.empty() )
        return;
    
    if( bxdfType.str == "Fourier" )
        fourierBxdfData.LoadData( bxdfFilePath.str );
    else if( bxdfType.str == "MERL" )
        merlData.LoadData( bxdfFilePath.str );
    BxdfNode::PostProcess();
}

GlassMaterialNode::GlassMaterialNode()
{
    REGISTER_MATERIALNODE_PROPERTY( "Reflectance" , reflectance );
    REGISTER_MATERIALNODE_PROPERTY( "Transmittance" , transmittance );
    REGISTER_MATERIALNODE_PROPERTY( "Roughness" , roughness );
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
        // This is one of the rare cases where there is double faces for perfect inner reflection.
        bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)( r , fresnel , dist , weight , n , true ));
    }
    if( !t.IsBlack() )
        bsdf->AddBxdf(SORT_MALLOC(MicroFacetRefraction)( t , dist , 1.0f , 1.5f , weight , n ));
}

MirrorMaterialNode::MirrorMaterialNode(){
    REGISTER_MATERIALNODE_PROPERTY( "BaseColor" , basecolor );
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
