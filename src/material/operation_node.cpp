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

#include "operation_node.h"

IMPLEMENT_CREATOR( SORTNodeOneMinus );
IMPLEMENT_CREATOR( AddNode );
IMPLEMENT_CREATOR( LerpNode );
IMPLEMENT_CREATOR( BlendNode );
IMPLEMENT_CREATOR( MutiplyNode );
IMPLEMENT_CREATOR( GammaToLinearNode );
IMPLEMENT_CREATOR( LinearToGammaNode );
IMPLEMENT_CREATOR( NormalDecoderNode );

// Adding node
AddNode::AddNode()
{
	REGISTER_MATERIALNODE_PROPERTY( "Color1" , src0 );
	REGISTER_MATERIALNODE_PROPERTY( "Color2" , src1 );
}

// get property value
MaterialPropertyValue AddNode::GetNodeValue( Bsdf* bsdf )
{
	return GET_MATERIALNODE_PROPERTY(src0) + GET_MATERIALNODE_PROPERTY(src1);
}

// inverse node
SORTNodeOneMinus::SORTNodeOneMinus()
{
    REGISTER_MATERIALNODE_PROPERTY( "Color" , src );
}

// get property value
MaterialPropertyValue SORTNodeOneMinus::GetNodeValue( Bsdf* bsdf )
{
    return MaterialPropertyValue(1.0f) - GET_MATERIALNODE_PROPERTY(src);
}

LerpNode::LerpNode()
{
	REGISTER_MATERIALNODE_PROPERTY( "Color1" , src0 );
	REGISTER_MATERIALNODE_PROPERTY( "Color2" , src1 );
	REGISTER_MATERIALNODE_PROPERTY( "Factor" , factor );
}

// get property value
MaterialPropertyValue LerpNode::GetNodeValue( Bsdf* bsdf )
{
	const float f = GET_MATERIALNODE_PROPERTY_FLOAT(factor);
    return lerp(GET_MATERIALNODE_PROPERTY(src0) , GET_MATERIALNODE_PROPERTY(src1), f);
}

BlendNode::BlendNode()
{
	REGISTER_MATERIALNODE_PROPERTY( "Color1" , src0 );
	REGISTER_MATERIALNODE_PROPERTY( "Color2" , src1 );
	REGISTER_MATERIALNODE_PROPERTY( "Factor1" , factor0 );
	REGISTER_MATERIALNODE_PROPERTY( "Factor2" , factor1 );
}

// get property value
MaterialPropertyValue BlendNode::GetNodeValue( Bsdf* bsdf )
{
	const float f0 = GET_MATERIALNODE_PROPERTY_FLOAT(factor0);
	const float f1 = GET_MATERIALNODE_PROPERTY_FLOAT(factor1);
	return GET_MATERIALNODE_PROPERTY(src0) * f0 + GET_MATERIALNODE_PROPERTY(src1) * f1;
}

MutiplyNode::MutiplyNode()
{
	REGISTER_MATERIALNODE_PROPERTY( "Color1" , src0 );
	REGISTER_MATERIALNODE_PROPERTY( "Color2" , src1 );
}

// get property value
MaterialPropertyValue MutiplyNode::GetNodeValue( Bsdf* bsdf )
{
	return GET_MATERIALNODE_PROPERTY(src0) * GET_MATERIALNODE_PROPERTY(src1);
}

GammaToLinearNode::GammaToLinearNode()
{
    REGISTER_MATERIALNODE_PROPERTY( "Color" , src );
}

// get property value
MaterialPropertyValue GammaToLinearNode::GetNodeValue( Bsdf* bsdf )
{
    auto tmp = GET_MATERIALNODE_PROPERTY(src);
    tmp.x = GammaToLinear(tmp.x);
    tmp.y = GammaToLinear(tmp.y);
    tmp.z = GammaToLinear(tmp.z);
    return tmp;
}

LinearToGammaNode::LinearToGammaNode()
{
    REGISTER_MATERIALNODE_PROPERTY( "Color" , src );
}

// get property value
MaterialPropertyValue LinearToGammaNode::GetNodeValue( Bsdf* bsdf )
{
    auto tmp = GET_MATERIALNODE_PROPERTY(src);
    tmp.x = LinearToGamma(tmp.x);
    tmp.y = LinearToGamma(tmp.y);
    tmp.z = LinearToGamma(tmp.z);
    return tmp;
}

NormalDecoderNode::NormalDecoderNode()
{
    REGISTER_MATERIALNODE_PROPERTY( "Color" , src );
}

// get property value
MaterialPropertyValue NormalDecoderNode::GetNodeValue( Bsdf* bsdf )
{
    const auto tmp = GET_MATERIALNODE_PROPERTY(src);
    return MaterialPropertyValue( 2.0f * tmp.x - 1.0f , tmp.z , 2.0f * tmp.y - 1.0f , 0.0f );
}
