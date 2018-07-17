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
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"

IMPLEMENT_CREATOR( LayeredMaterialNode );
IMPLEMENT_CREATOR( PrincipleMaterialNode );
IMPLEMENT_CREATOR( MatteMaterialNode );
IMPLEMENT_CREATOR( PlasticMaterialNode );
IMPLEMENT_CREATOR( GlassMaterialNode );
IMPLEMENT_CREATOR( MeasuredMaterialNode );

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
    Lambert* lambert = SORT_MALLOC(Lambert)( baseColor.GetPropertyValue(bsdf).ToSpectrum() );
    
    Spectrum basecolor = baseColor.GetPropertyValue(bsdf).ToSpectrum();
    
    Spectrum eta = Spectrum( 0.37f , 0.37f , 0.37f );
    Spectrum k( 2.82f );
    MicroFacetDistribution* dist = SORT_MALLOC(GGX)( roughnessU.GetPropertyValue(bsdf).x , roughnessV.GetPropertyValue(bsdf).x );
    
    // visibility function to be removed.
    VisTerm* vis = SORT_MALLOC(VisCookTorrance)();
    
    Fresnel* fresnel = SORT_MALLOC( FresnelConductor )( eta , k );
    MicroFacetReflection* mf = SORT_MALLOC(MicroFacetReflection)( basecolor , fresnel , dist , vis);
    
    const float m = metallic.GetPropertyValue(bsdf).x;
    mf->m_weight = weight * ( m * 0.92f + 0.08f );
    lambert->m_weight = weight * ( 1 - m ) * 0.92f;
    
    bsdf->AddBxdf(lambert);
    bsdf->AddBxdf(mf);
}

MatteMaterialNode::MatteMaterialNode()
{
    m_props.insert( make_pair( "BaseColor" , &baseColor ) );
    m_props.insert( make_pair( "Roughness" , &roughness ) );
}

void MatteMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    Spectrum reflectance(baseColor.GetPropertyValue(bsdf).ToSpectrum());
    if( !reflectance.IsBlack() ){
        float rn = roughness.GetPropertyValue(bsdf).x;
        if( rn == 0.0f ){
            Lambert* lambert = SORT_MALLOC(Lambert)( reflectance );
            lambert->m_weight = weight;
            bsdf->AddBxdf(lambert);
        }else{
            OrenNayar* orennayar = SORT_MALLOC(OrenNayar)( reflectance , rn );
            orennayar->m_weight = weight;
            bsdf->AddBxdf( orennayar );
        }
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
    Spectrum diff(diffuse.GetPropertyValue(bsdf).ToSpectrum());
    if( !diff.IsBlack() ){
        Lambert* lambert = SORT_MALLOC(Lambert)( diff );
        lambert->m_weight = weight;
        bsdf->AddBxdf(lambert);
    }
    
    Spectrum spec(specular.GetPropertyValue(bsdf).ToSpectrum());
    if( !spec.IsBlack() ){
        Fresnel *fresnel = SORT_MALLOC(FresnelDielectric)(1.2f, 1.f);
        float rough = roughness.GetPropertyValue(bsdf).x;
        MicroFacetDistribution* dist = SORT_MALLOC(GGX)( rough , rough );   // GGX
        VisTerm* vis = SORT_MALLOC(VisSmith)( rough );                      // Smith
        MicroFacetReflection* mf = SORT_MALLOC(MicroFacetReflection)( spec , fresnel , dist , vis);
        mf->m_weight = weight;
        bsdf->AddBxdf(mf);
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
        fourierBxdf.m_weight = weight;
        bsdf->AddBxdf( &fourierBxdf );
    }else if( bxdfType.str == "MERL" ){
        merlBxdf.m_weight = weight;
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
    float rough = roughness.GetPropertyValue(bsdf).x;
    Spectrum r = reflectance.GetPropertyValue(bsdf).ToSpectrum();
    Spectrum t = transmittance.GetPropertyValue(bsdf).ToSpectrum();
    if( r.IsBlack() && t.IsBlack() )
        return;
    
    MicroFacetDistribution* dist = SORT_MALLOC(GGX)( rough , rough );   // GGX
    VisTerm* vis = SORT_MALLOC(VisSmith)( rough );                      // Smith
    Fresnel* fresnel = SORT_MALLOC( FresnelDielectric )( 1.0f , 1.5f );
    
    if( !r.IsBlack() ){
        MicroFacetReflection* mf = SORT_MALLOC(MicroFacetReflection)( r , fresnel , dist , vis);
        mf->m_weight = weight;
        bsdf->AddBxdf(mf);
    }
    if( !t.IsBlack() ){
        MicroFacetRefraction* mr = SORT_MALLOC(MicroFacetRefraction)( t , fresnel , dist , vis , 1.5f , 1.0f );
        mr->m_weight = weight;
        bsdf->AddBxdf(mr);
    }
}
