/*
   FileName:      operation_node.h

   Created Time:  2015-09-04 20:22:00

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_OPERATION_NODE
#define	SORT_OPERATION_NODE

#include "material_node.h"

class OperatorNode : public MaterialNode
{
public:
	// get node type
	virtual MAT_NODE_TYPE getNodeType() { return MAT_NODE_OPERATOR | MaterialNode::getNodeType(); }
};

// Adding node
class AddNode : public OperatorNode
{
public:
	DEFINE_CREATOR( AddNode , "SORTNodeAdd" );

	// constructor
	AddNode();

	// get property value
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf );

	// check validation
	virtual bool CheckValidation();

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;
};

// To be changed
// Lerp node
class LerpNode : public OperatorNode
{
public:
	DEFINE_CREATOR( LerpNode , "SORTNodeLerp" );

	// constructor
	LerpNode();
	
	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// get property value
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf );

	// check validation
	virtual bool CheckValidation();

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;
	MaterialNodeProperty	factor;
};

// Blend node
class BlendNode : public OperatorNode
{
public:
	DEFINE_CREATOR( BlendNode , "SORTNodeBlend" );

	// constructor
	BlendNode();
	
	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// get property value
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf );

	// check validation
	virtual bool CheckValidation();

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
	DEFINE_CREATOR( MutiplyNode , "SORTNodeMultiply" );

	// constructor
	MutiplyNode();
	
	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// get property value
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf );

	// check validation
	virtual bool CheckValidation();

private:
	MaterialNodeProperty	src0;
	MaterialNodeProperty	src1;
};

#endif
