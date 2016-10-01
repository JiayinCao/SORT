/*
   FileName:      material_node.h

   Created Time:  2015-09-04 20:22:00

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_CONSTANT_NODE
#define	SORT_CONSTANT_NODE

#include "material_node.h"

// constant node
class ConstantNode : public MaterialNode
{
public:
    // get node type
    virtual MAT_NODE_TYPE getNodeType() { return MAT_NODE_CONSTANT | MaterialNode::getNodeType(); }
};

// Grid texture Node
class GridTexNode : public ConstantNode
{
public:
	DEFINE_CREATOR( GridTexNode , "SORTNodeGrid" );

	// constructor
	GridTexNode();

	// get property value
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf );

	// post process
	virtual void PostProcess();

	// check validation
	virtual bool CheckValidation();

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

#endif
