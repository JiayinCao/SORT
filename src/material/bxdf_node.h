/*
   FileName:      constant_node.h

   Created Time:  2015-09-04 20:22:00

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_BXDF_NODE
#define	SORT_BXDF_NODE

#include "material_node.h"

// Bxdf node
class BxdfNode : public MaterialNode
{
protected:
	// get node type
	virtual MAT_NODE_TYPE getNodeType() { return MAT_NODE_BXDF | MaterialNode::getNodeType(); }

	// check validation
	virtual bool CheckValidation();
};

// Lambert node
class LambertNode : public BxdfNode
{
public:
	DEFINE_CREATOR( LambertNode , "SORTNodeLambert" );

	// constructor
	LambertNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

private:
	MaterialNodeProperty	baseColor;
};

// Merl node
class MerlNode : public BxdfNode
{
public:
	DEFINE_CREATOR( MerlNode , "SORTNodeMerl" );

	// constructor
	MerlNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// post process
	virtual void PostProcess();

private:
	MaterialNodePropertyString	merlfile;

	// the merl data
	Merl merl;
};

// Oren nayar node
class OrenNayarNode : public BxdfNode
{
public:
	DEFINE_CREATOR( OrenNayarNode , "SORTNodeOrenNayar" );

	// constructor
	OrenNayarNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

private:
	MaterialNodeProperty	baseColor;
	MaterialNodeProperty	roughness;
};

// Microfacet node
class MicrofacetReflectionNode : public BxdfNode
{
public:
	DEFINE_CREATOR( MicrofacetReflectionNode , "SORTNodeMicrofacetReflection" );

	// constructor
	MicrofacetReflectionNode();
	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

private:
	MaterialNodeProperty	baseColor;
	MaterialNodeProperty	roughness;
	MaterialNodeProperty	eta;
	MaterialNodeProperty	k;
	MaterialNodePropertyString	mf_dist;
	MaterialNodePropertyString	mf_vis;
};

// Microfacet node
class MicrofacetRefractionNode : public BxdfNode
{
public:
	DEFINE_CREATOR( MicrofacetRefractionNode , "SORTNodeMicrofacetRefraction" );

	// constructor
	MicrofacetRefractionNode();
	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

private:
	MaterialNodeProperty	baseColor;
	MaterialNodeProperty	roughness;
	MaterialNodeProperty	in_ior;
	MaterialNodeProperty	ext_ior;
	MaterialNodePropertyString	mf_dist;
	MaterialNodePropertyString	mf_vis;
};

#endif
