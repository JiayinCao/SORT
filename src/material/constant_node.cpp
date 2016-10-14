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

#include "constant_node.h"
#include "geometry/intersection.h"
#include "bsdf/bsdf.h"

IMPLEMENT_CREATOR( GridTexNode );
IMPLEMENT_CREATOR( CheckBoxTexNode );
IMPLEMENT_CREATOR( ImageTexNode );
IMPLEMENT_CREATOR( ConstantColorNode );

GridTexNode::GridTexNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
}

// get property value
MaterialPropertyValue GridTexNode::GetNodeValue( Bsdf* bsdf )
{
	// get intersection
	const Intersection* intesection = bsdf->GetIntersection();
	return FromSpectrum( grid_tex.GetColorFromUV( intesection->u * 10.0f , intesection->v * 10.0f ) );
}

// post process
void GridTexNode::PostProcess()
{
	if( m_post_processed )
		return;

	// set grid texture
	grid_tex.SetGridColor( src0.GetPropertyValue(0).ToSpectrum(), src1.GetPropertyValue(0).ToSpectrum());

	MaterialNode::PostProcess();
}

// check validation
bool GridTexNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();
	
	MAT_NODE_TYPE type = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	if( type & MAT_NODE_BXDF )
		m_node_valid = false;

	return m_node_valid;
}

CheckBoxTexNode::CheckBoxTexNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
}

// get property value
MaterialPropertyValue CheckBoxTexNode::GetNodeValue( Bsdf* bsdf )
{
	// get intersection
	const Intersection* intesection = bsdf->GetIntersection();
	return FromSpectrum( checkbox_tex.GetColorFromUV( intesection->u * 10.0f , intesection->v * 10.0f ) );
}

// post process
void CheckBoxTexNode::PostProcess()
{
	if( m_post_processed )
		return;

	// set grid texture
	checkbox_tex.SetCheckBoxColor( src0.GetPropertyValue(0).ToSpectrum() , src1.GetPropertyValue(0).ToSpectrum() );

	MaterialNode::PostProcess();
}

// check validation
bool CheckBoxTexNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();
	
	MAT_NODE_TYPE type = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	if( type & MAT_NODE_BXDF )
		m_node_valid = false;

	return m_node_valid;
}

ImageTexNode::ImageTexNode()
{
	m_props.insert( make_pair( "Filename" , &filename ) );
}

// get property value
MaterialPropertyValue ImageTexNode::GetNodeValue( Bsdf* bsdf )
{
	// get intersection
	const Intersection* intesection = bsdf->GetIntersection();
	return FromSpectrum( image_tex.GetColorFromUV( intesection->u , intesection->v ) );
}

// post process
void ImageTexNode::PostProcess()
{
	if( m_post_processed )
		return;

	// set grid texture
	image_tex.LoadImageFromFile( filename.str );
}

ConstantColorNode::ConstantColorNode()
{
	m_props.insert( make_pair( "Color" , &src ) );
}

// get property value
MaterialPropertyValue ConstantColorNode::GetNodeValue( Bsdf* bsdf )
{
	// get intersection
	return src.GetPropertyValue(bsdf);
}

// check validation
bool ConstantColorNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();

	MAT_NODE_TYPE type = (src.node)?src.node->getNodeType():MAT_NODE_CONSTANT;
	if( type & MAT_NODE_BXDF )
		m_node_valid = false;

	return m_node_valid;
}
