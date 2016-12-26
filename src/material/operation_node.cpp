/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "operation_node.h"

IMPLEMENT_CREATOR( InverseNode );
IMPLEMENT_CREATOR( AddNode );
IMPLEMENT_CREATOR( LerpNode );
IMPLEMENT_CREATOR( BlendNode );
IMPLEMENT_CREATOR( MutiplyNode );

// Adding node
AddNode::AddNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
}

// check validation
bool AddNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();

	MAT_NODE_TYPE type0 = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	MAT_NODE_TYPE type1 = (src1.node)?src1.node->getNodeType():MAT_NODE_CONSTANT;

	// if one of the parameters is a bxdf, the other should be exactly the same
	if( ( type0 & MAT_NODE_BXDF ) != ( type1 & MAT_NODE_BXDF ) )
		m_node_valid = false;

	return m_node_valid;
}

// get property value
MaterialPropertyValue AddNode::GetNodeValue( Bsdf* bsdf )
{
	return src0.GetPropertyValue(bsdf) + src1.GetPropertyValue(bsdf);
}

// inverse node
InverseNode::InverseNode()
{
    m_props.insert( make_pair( "Color" , &src ) );
}

bool InverseNode::CheckValidation()
{
    m_node_valid = MaterialNode::CheckValidation();
    
    MAT_NODE_TYPE type = (src.node)?src.node->getNodeType():MAT_NODE_CONSTANT;
    
    // if one of the parameters is a bxdf, the other should be exactly the same
    if( ( type & MAT_NODE_CONSTANT ) == 0 )
        m_node_valid = false;
    
    return m_node_valid;
}

// get property value
MaterialPropertyValue InverseNode::GetNodeValue( Bsdf* bsdf )
{
    return MaterialPropertyValue(1.0f) - src.GetPropertyValue(bsdf);
}

LerpNode::LerpNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
	m_props.insert( make_pair( "Factor" , &factor ) );
}

// update bsdf
void LerpNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	float f = factor.GetPropertyValue(bsdf).x;

	if( src0.node )
		src0.node->UpdateBSDF( bsdf, weight * ( 1.0f - f ) );
	if( src1.node )
		src1.node->UpdateBSDF( bsdf, weight * f );
}

// get property value
MaterialPropertyValue LerpNode::GetNodeValue( Bsdf* bsdf )
{
	float f = factor.GetPropertyValue( bsdf ).x;
	return src0.GetPropertyValue(bsdf) * ( 1.0f - f ) + src1.GetPropertyValue(bsdf) * f;
}

// check validation
bool LerpNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();

	MAT_NODE_TYPE type0 = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	MAT_NODE_TYPE type1 = (src1.node)?src1.node->getNodeType():MAT_NODE_CONSTANT;

	// if one of the parameters is a bxdf, the other should be exactly the same
	if( ( type0 & MAT_NODE_BXDF ) != ( type1 & MAT_NODE_BXDF ) )
		m_node_valid = false;

	return m_node_valid;
}

BlendNode::BlendNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
	m_props.insert( make_pair( "Factor1" , &factor0 ) );
	m_props.insert( make_pair( "Factor2" , &factor1 ) );
}

// update bsdf
void BlendNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	float f0 = factor0.GetPropertyValue(bsdf).x;
	float f1 = factor1.GetPropertyValue(bsdf).y;
	if( src0.node )
		src0.node->UpdateBSDF( bsdf, weight * f0 );
	if( src1.node )
		src1.node->UpdateBSDF( bsdf, weight * f1 );
}

// get property value
MaterialPropertyValue BlendNode::GetNodeValue( Bsdf* bsdf )
{
	float f0 = factor0.GetPropertyValue( bsdf ).x;
	float f1 = factor1.GetPropertyValue( bsdf ).x;
	return src0.GetPropertyValue(bsdf) * f0 + src1.GetPropertyValue(bsdf) * f1;
}

// check validation
bool BlendNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();

	MAT_NODE_TYPE type0 = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	MAT_NODE_TYPE type1 = (src1.node)?src1.node->getNodeType():MAT_NODE_CONSTANT;

	// if one of the parameters is a bxdf, the other should be exactly the same
	if( ( type0 & MAT_NODE_BXDF ) != ( type1 & MAT_NODE_BXDF ) )
		m_node_valid = false;

	return m_node_valid;
}

MutiplyNode::MutiplyNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
}

// update bsdf
void MutiplyNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	MAT_NODE_TYPE type0 = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	MAT_NODE_TYPE type1 = (src1.node)?src1.node->getNodeType():MAT_NODE_CONSTANT;

	if( type0 & MAT_NODE_BXDF )
		src0.node->UpdateBSDF( bsdf , weight * src1.GetPropertyValue(bsdf).x );
	else if( type1 & MAT_NODE_BXDF )
		src1.node->UpdateBSDF( bsdf , weight * src0.GetPropertyValue(bsdf).x );
}

// get property value
MaterialPropertyValue MutiplyNode::GetNodeValue( Bsdf* bsdf )
{
	return src0.GetPropertyValue(bsdf) * src1.GetPropertyValue(bsdf);
}

// check validation
bool MutiplyNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();

	MAT_NODE_TYPE type0 = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	MAT_NODE_TYPE type1 = (src1.node)?src1.node->getNodeType():MAT_NODE_CONSTANT;

	// Can't multiply two bxdfs
	if( ( type0 & MAT_NODE_BXDF ) && ( type1 & MAT_NODE_BXDF ) )
		m_node_valid = false;

	return m_node_valid;
}
