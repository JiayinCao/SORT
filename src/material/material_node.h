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

#ifndef	SORT_MATERIAL_NODE
#define	SORT_MATERIAL_NODE

#include "utility/propertyset.h"
#include "spectrum/spectrum.h"
#include <vector>
#include "utility/creator.h"
#include "bsdf/merl.h"
#include "texture/gridtexture.h"
#include "texture/checkboxtexture.h"
#include "texture/imagetexture.h"
#include "spectrum/rgbspectrum.h"

class Bsdf;
class MaterialNode;
class TiXmlElement;
class Fresnel;
class MicroFacetDistribution;

struct MaterialPropertyValue
{
	float x , y , z;

	MaterialPropertyValue():x(0.0f),y(0.0f),z(0.0f)
	{
	}
	MaterialPropertyValue( float value )
	{
		x = y = z = value;
	}
	MaterialPropertyValue( float _x , float _y , float _z )
	{
		x = _x;
		y = _y;
		z = _z;
	}
	MaterialPropertyValue( Spectrum& spectrum ): x(spectrum.GetR()) , y(spectrum.GetG()) , z(spectrum.GetB())
	{
	}
};

class MaterialNodeProperty
{
public:
	MaterialNodeProperty()
	{
		node = 0;
	}

	// set node property
	virtual void SetNodeProperty( const string& prop ) = 0;

	// get node property
	virtual MaterialPropertyValue GetPropertyValue( Bsdf* bsdf ) = 0;

	// sub node if it has value
	MaterialNode*	node;
};

class MaterialNodePropertyColor : public MaterialNodeProperty
{
public:
	// set node property
	virtual void SetNodeProperty( const string& prop );

	// get node property
	virtual MaterialPropertyValue GetPropertyValue( Bsdf* bsdf );

	// color value
	Spectrum		value;
};

class MaterialNodePropertyFloat : public MaterialNodeProperty
{
public:
	// set node property
	virtual void SetNodeProperty( const string& prop );

	// get node property
	virtual MaterialPropertyValue GetPropertyValue( Bsdf* bsdf );

	// color value
	float		value;
};

class MaterialNodePropertyFloat2 : public MaterialNodeProperty
{
public:
	// set node property
	virtual void SetNodeProperty( const string& prop );

	// get node property
	virtual MaterialPropertyValue GetPropertyValue( Bsdf* bsdf );

	// color value
	float	x , y;
};

class MaterialNodePropertyString : public MaterialNodeProperty
{
public:
	// set node property
	virtual void SetNodeProperty( const string& prop );

	// get node property
	virtual MaterialPropertyValue GetPropertyValue( Bsdf* bsdf ) { return 0.0f; }

	// color value
	string	str;
};

// base material node
class MaterialNode : public PropertySet<MaterialNode>
{
public:
	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// parse property or socket
	virtual void ParseProperty( TiXmlElement* element , MaterialNode* node );

	// parse a new node
	virtual MaterialNode* ParseNode( TiXmlElement* element , MaterialNode* node );

	// post process
	virtual void PostProcess();

	// get property value, this should never be called
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) { return 0.0f; }

protected:
	std::map< string , MaterialNodeProperty * > m_props;

	// get node property
	MaterialNodeProperty*	getProperty( const string& name );
};

// Mateiral output node
class OutputNode : public MaterialNode
{
public:
	OutputNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

private:
	MaterialNodePropertyColor	output;
};

// Lambert node
class LambertNode : public MaterialNode
{
public:
	DEFINE_CREATOR( LambertNode , "SORTNodeLambert" );

	// constructor
	LambertNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

private:
	MaterialNodePropertyColor	baseColor;
};

// Merl node
class MerlNode : public MaterialNode
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
class OrenNayarNode : public MaterialNode
{
public:
	DEFINE_CREATOR( OrenNayarNode , "SORTNodeOrenNayar" );

	// constructor
	OrenNayarNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

private:
	MaterialNodePropertyColor	baseColor;

	MaterialNodePropertyFloat	sigma;
};

// Microfacet node
class MicrofacetNode : public MaterialNode
{
public:
	DEFINE_CREATOR( MicrofacetNode , "SORTNodeMicrofacet" );

	// constructor
	MicrofacetNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// post process
	virtual void PostProcess();

private:
	MaterialNodePropertyColor	baseColor;
	MaterialNodePropertyString	fresnel;
	MaterialNodePropertyString	mf_dist;

	Fresnel*				pFresnel;
	MicroFacetDistribution*	pMFDist;
};

// Microfacet node
class ReflectionNode : public MaterialNode
{
public:
	DEFINE_CREATOR( ReflectionNode , "SORTNodeReflection" );

	// constructor
	ReflectionNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// post process
	virtual void PostProcess();

private:
	MaterialNodePropertyColor	baseColor;
	MaterialNodePropertyString	fresnel;

	Fresnel*				pFresnel;
};

// Microfacet node
class RefractionNode : public MaterialNode
{
public:
	DEFINE_CREATOR( RefractionNode , "SORTNodeRefraction" );

	// constructor
	RefractionNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// post process
	virtual void PostProcess();

private:
	MaterialNodePropertyColor	baseColor;
	MaterialNodePropertyString	fresnel;
	MaterialNodePropertyFloat	theta0;
	MaterialNodePropertyFloat	theta1;

	Fresnel*					pFresnel;
};

// Adding node
class AddNode : public MaterialNode
{
public:
	DEFINE_CREATOR( AddNode , "SORTNodeAdd" );

	// constructor
	AddNode();

private:
	MaterialNodePropertyColor	src0;
	MaterialNodePropertyColor	src1;
};

// Lerp node
class LerpNode : public MaterialNode
{
public:
	DEFINE_CREATOR( LerpNode , "SORTNodeLerp" );

	// constructor
	LerpNode();
	
	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

private:
	MaterialNodePropertyColor	src0;
	MaterialNodePropertyColor	src1;
	MaterialNodePropertyFloat	factor;
};

// Lerp node
class BlendNode : public MaterialNode
{
public:
	DEFINE_CREATOR( BlendNode , "SORTNodeBlend" );

	// constructor
	BlendNode();
	
	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

private:
	MaterialNodePropertyColor	src0;
	MaterialNodePropertyColor	src1;
	MaterialNodePropertyFloat	factor0;
	MaterialNodePropertyFloat	factor1;
};

// Grid texture Node
class GridTexNode : public MaterialNode
{
public:
	DEFINE_CREATOR( GridTexNode , "SORTNodeGrid" );

	// constructor
	GridTexNode();

	// get property value
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf );

	// post process
	virtual void PostProcess();

private:
	MaterialNodePropertyColor	src0;
	MaterialNodePropertyColor	src1;

	GridTexture grid_tex;
};

// Grid texture Node
class CheckBoxTexNode : public MaterialNode
{
public:
	DEFINE_CREATOR( CheckBoxTexNode , "SORTNodeCheckbox" );

	// constructor
	CheckBoxTexNode();

	// get property value
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf );

	// post process
	virtual void PostProcess();

private:
	MaterialNodePropertyColor	src0;
	MaterialNodePropertyColor	src1;

	CheckBoxTexture checkbox_tex;
};

// Grid texture Node
class ImageTexNode : public MaterialNode
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


#endif