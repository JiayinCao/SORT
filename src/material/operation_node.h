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
public:
	// get node type
	virtual MAT_NODE_TYPE getNodeType() override { return MAT_NODE_OPERATOR | MaterialNode::getNodeType(); }
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

	// check validation
	bool CheckValidation() override;

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;
};

// Inverse node
class InverseNode : public OperatorNode
{
public:
    DEFINE_CREATOR( InverseNode , MaterialNode , "SORTNodeInverse" );
    
    // constructor
    InverseNode();
    
    // get property value
    MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) override;
    
    // check validation
    bool CheckValidation() override;
    
private:
    MaterialNodeProperty    src;
};

// To be changed
// Lerp node
class LerpNode : public OperatorNode
{
public:
	DEFINE_CREATOR( LerpNode , MaterialNode , "SORTNodeLerp" );

	// constructor
	LerpNode();
	
	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

	// get property value
    MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) override;

	// check validation
    bool CheckValidation() override;

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
	
	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

	// get property value
    MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) override;

	// check validation
    bool CheckValidation() override;

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
	
	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

	// get property value
    MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) override;

	// check validation
	bool CheckValidation() override;

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;
};

