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

#pragma once

#include "material_node.h"

class OperatorNode : public MaterialNode
{
};

// Adding node
class AddNode : public OperatorNode
{
public:
	DEFINE_CREATOR( AddNode , MaterialNode , "SORTNodeAdd" );

	// constructor
	AddNode();

	// get property value
    MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) override;

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;
};

// Inverse node
class SORTNodeOneMinus : public OperatorNode
{
public:
    DEFINE_CREATOR( SORTNodeOneMinus , MaterialNode , "SORTNodeOneMinus" );
    
    // constructor
    SORTNodeOneMinus();
    
    // get property value
    MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) override;
    
private:
    MaterialNodeProperty    src;
};

// Lerp node
class LerpNode : public OperatorNode
{
public:
	DEFINE_CREATOR( LerpNode , MaterialNode , "SORTNodeLerp" );

	// constructor
	LerpNode();
	
	// get property value
    MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) override;

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;
	MaterialNodeProperty	factor;
};

// Blend node
class BlendNode : public OperatorNode
{
public:
	DEFINE_CREATOR( BlendNode , MaterialNode , "SORTNodeBlend" );

	// constructor
	BlendNode();
	
	// get property value
    MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) override;

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;
	MaterialNodeProperty	factor0;
	MaterialNodeProperty	factor1;
};

// Mutiply node
class MutiplyNode : public OperatorNode
{
public:
	DEFINE_CREATOR( MutiplyNode , MaterialNode , "SORTNodeMultiply" );

	// constructor
	MutiplyNode();
    
	// get property value
    MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) override;

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;
};

// Gamma correction Node
class GammaToLinearNode : public OperatorNode
{
public:
    DEFINE_CREATOR( GammaToLinearNode , MaterialNode , "SORTNodeGammaToLinear" );
    
    // constructor
    GammaToLinearNode();
    
    // get property value
    MaterialPropertyValue    GetNodeValue( Bsdf* bsdf ) override;
    
private:
    MaterialNodeProperty    src;
};

// Gamma correction Node
class LinearToGammaNode : public OperatorNode
{
public:
    DEFINE_CREATOR( LinearToGammaNode , MaterialNode , "SORTNodeLinearToGamma" );
    
    // constructor
    LinearToGammaNode();
    
    // get property value
    MaterialPropertyValue    GetNodeValue( Bsdf* bsdf ) override;
    
private:
    MaterialNodeProperty    src;
};

// Gamma correction Node
class NormalDecoderNode : public OperatorNode
{
public:
    DEFINE_CREATOR( NormalDecoderNode , MaterialNode , "SORTNodeDecodeNormal" );
    
    // constructor
    NormalDecoderNode();
    
    // get property value
    MaterialPropertyValue    GetNodeValue( Bsdf* bsdf ) override;
    
private:
    MaterialNodeProperty    src;
};
