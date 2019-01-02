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

#include "material_node.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"
#include "bsdf/lambert.h"
#include "math/intersection.h"
#include "core/log.h"

// update bsdf, for layered brdf
void MaterialNodeProperty::UpdateBsdf( Bsdf* bsdf , Spectrum weight ){
    if( node )
        node->UpdateBSDF( bsdf , weight );
}

// post process
void MaterialNode::PostProcess()
{
	if( m_post_processed )
		return;

    for( auto node : m_props ){
		if( node->GetNode() )
			node->GetNode()->PostProcess();
	}

	m_post_processed = true;
}

// update bsdf
void MaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

    for( auto node : m_props ){
		if( node->GetNode() )
			node->GetNode()->UpdateBSDF(bsdf , weight);
	}
}

// update bsdf
void OutputNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	// return a default one for invalid material
	if( !m_node_valid || !output.GetNode() ){
		static const Spectrum default_spectrum( 0.5f , 0.1f , 0.1f );
		const Lambert* lambert = SORT_MALLOC(Lambert)( default_spectrum , weight , DIR_UP);
		bsdf->AddBxdf( lambert );
		return;
	}

	output.GetNode()->UpdateBSDF( bsdf );
}
