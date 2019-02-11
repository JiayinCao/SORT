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
#include "core/memory.h"
#include "bsdf/bsdf.h"
#include "bsdf/lambert.h"
#include "stream/stream.h"

void MaterialNode::LinkNode( const std::string& channel , MaterialNodeInputSocket* inputSocket ){
	bool found = m_outputs.count( channel ) != 0;
	sAssertMsg( found , MATERIAL , "Matrial node has no output socket named %s" , channel.c_str() );

	inputSocket->m_fromSocket = m_outputs[channel];
}

void MaterialNode::PostProcess(){
	if( m_post_processed )
		return;

    for( auto input : m_inputs ){
		if( input->GetConnectedNode() )
			input->GetConnectedNode()->PostProcess();
	}
	m_post_processed = true;
}

void MaterialNode::UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ){
	if( weight.IsBlack() )
		return;

    for( auto input : m_inputs ){
		sAssert( input , MATERIAL );
		input->UpdateBSDF(bsdf , weight);
	}
}

void OutputNode::UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ){
	// return a default one for invalid material
	if( !m_node_valid || !output.GetConnectedNode() ){
		static const Spectrum default_spectrum( 0.5f , 0.5f , 0.5f );
		const Lambert* lambert = SORT_MALLOC(Lambert)( default_spectrum , weight , DIR_UP);
		bsdf->AddBxdf( lambert );
		return;
	}

	static const Spectrum fullWeight( 1.0f );
	output.UpdateBSDF( bsdf , fullWeight );
}

void OutputNode::Serialize( IStreamBase& stream , MaterialNodeCache& cache ){
	output.Serialize( stream , cache );
}