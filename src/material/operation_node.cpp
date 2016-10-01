/*
   FileName:      material_node.cpp

   Created Time:  2015-09-04 20:22:00

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "operation_node.h"

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
