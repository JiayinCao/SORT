/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2017 by Cao Jiayin - All rights reserved.
 
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
#include "texture/gridtexture.h"
#include "texture/checkboxtexture.h"
#include "texture/imagetexture.h"

// constant node
class ConstantNode : public MaterialNode
{
public:
    // get node type
    MAT_NODE_TYPE getNodeType() override { return MAT_NODE_CONSTANT | MaterialNode::getNodeType(); }
};

// Grid texture Node
class GridTexNode : public ConstantNode
{
public:
	DEFINE_CREATOR( GridTexNode , "SORTNodeGrid" );

	// constructor
	GridTexNode();

	// get property value
    MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) override;

	// post process
	void PostProcess() override;

	// check validation
	bool CheckValidation() override;

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;

	GridTexture grid_tex;
};

// Grid texture Node
class CheckBoxTexNode : public ConstantNode
{
public:
	DEFINE_CREATOR( CheckBoxTexNode , "SORTNodeCheckbox" );

	// constructor
	CheckBoxTexNode();

	// get property value
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf );

	// post process
	virtual void PostProcess();

	// check validation
	virtual bool CheckValidation();

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;

	CheckBoxTexture checkbox_tex;
};

// Grid texture Node
class ImageTexNode : public ConstantNode
{
public:
	DEFINE_CREATOR( ImageTexNode , "SORTNodeImage" );

	// constructor
	ImageTexNode();

	// get property value
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf );

	// post process
	virtual void PostProcess();

private:
	MaterialNodePropertyString	filename;

	ImageTexture image_tex;
};

// constant color node
class ConstantColorNode : public ConstantNode
{
public:
	DEFINE_CREATOR( ConstantColorNode , "SORTNodeConstant" );

	// constructor
	ConstantColorNode();

	// get property value
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf );

	// check validation
	virtual bool CheckValidation();

private:
	MaterialNodeProperty	src;
};
