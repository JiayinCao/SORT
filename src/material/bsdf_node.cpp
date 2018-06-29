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
        bxdfs[i].UpdateBsdf(bsdf, weights[i].GetPropertyValue(bsdf).ToSpectrum());
}

PrincipleMaterialNode::PrincipleMaterialNode()
{
    m_props.insert( make_pair( "BaseColor" , &baseColor ) );
    m_props.insert( make_pair( "Roughness" , &roughness ) );
    m_props.insert( make_pair( "Metallic" , &metallic ) );
    m_props.insert( make_pair( "Specular" , &specular ) );
}

void PrincipleMaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    Lambert* lambert = SORT_MALLOC(Lambert)( baseColor.GetPropertyValue(bsdf).ToSpectrum() );
    
    Spectrum eta = Spectrum( 0.37f , 0.4f , 0.37f );
    Spectrum k( 2.82f );
    float rn = clamp( roughness.GetPropertyValue(bsdf).x , 0.001f , 1.0f );
    MicroFacetDistribution* dist = SORT_MALLOC(GGX)( rn );      // GGX
    VisTerm* vis = SORT_MALLOC(VisSmith)( rn );                 // Smith
    Fresnel* frenel = SORT_MALLOC( FresnelConductor )( eta , k );
    MicroFacetReflection* mf = SORT_MALLOC(MicroFacetReflection)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , frenel , dist , vis);
    
    const float m = metallic.GetPropertyValue(bsdf).x;
    mf->m_weight = weight * ( m * 0.92f + 0.08f );
    lambert->m_weight = weight * ( 1 - m ) * 0.92;
    
    Bsdf* localbsdf = SORT_MALLOC(Bsdf)( bsdf->GetIntersection() , false );
    localbsdf->AddBxdf( lambert );
    localbsdf->AddBxdf( mf );
    
    localbsdf->m_weight = weight;
    bsdf->AddBxdf(localbsdf);
}
