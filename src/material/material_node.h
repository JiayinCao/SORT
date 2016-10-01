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
#include "math/vector4.h"

class Bsdf;
class MaterialNode;
class TiXmlElement;
class Fresnel;
class MicroFacetDistribution;
class VisTerm;

#define MAT_NODE_TYPE unsigned int
#define MAT_NODE_CONSTANT   0x1
#define MAT_NODE_INPUT      0x2
#define MAT_NODE_BXDF       0x4
#define MAT_NODE_OPERATOR   0x8
#define MAT_NODE_OUTPUT     0x10
#define MAT_NODE_NONE       0x0

typedef Vector4<float> MaterialPropertyValue;

class MaterialNodeProperty
{
public:
	MaterialNodeProperty():node(0){
	}

	// set node property
	virtual void SetNodeProperty( const string& prop );

	// get node property
	virtual MaterialPropertyValue GetPropertyValue( Bsdf* bsdf );

	// sub node if it has value
	MaterialNode*	node;

	// value
	MaterialPropertyValue value;
};

class MaterialNodePropertyString : public MaterialNodeProperty
{
public:
	// set node property
	virtual void SetNodeProperty( const string& prop );

	// get node property
	virtual MaterialPropertyValue GetPropertyValue( Bsdf* bsdf ) { return MaterialPropertyValue(); }

	// color value
	string	str;
};

// base material node
class MaterialNode
{
public:
	MaterialNode(){
		subtree_node_type = MAT_NODE_NONE;
		m_node_valid = true;
		m_post_processed = false;
	}
	virtual ~MaterialNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// parse property or socket
	virtual void ParseProperty( TiXmlElement* element , MaterialNode* node );

	// parse a new node
	virtual MaterialNode* ParseNode( TiXmlElement* element , MaterialNode* node );

	// get property value, this should never be called
	virtual MaterialPropertyValue	GetNodeValue( Bsdf* bsdf ) { return 0.0f; }

	// post process
	virtual void PostProcess();

	// check validation
	virtual bool CheckValidation();

	// get node type
	virtual MAT_NODE_TYPE getNodeType();

protected:
	// node properties
	std::map< string , MaterialNodeProperty * > m_props;

	// get node property
	MaterialNodeProperty*	getProperty( const string& name );

	// node type of this sub-tree
	MAT_NODE_TYPE subtree_node_type;

	// valid node
	bool m_node_valid;

	// already post processed
	bool m_post_processed;
};

// Mateiral output node
class OutputNode : public MaterialNode
{
public:
	OutputNode();

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );

	// get node type
	virtual MAT_NODE_TYPE getNodeType() { return MAT_NODE_OUTPUT | MaterialNode::getNodeType(); }

	// check validation
	virtual bool CheckValidation();

private:
	MaterialNodeProperty	output;
};

#endif
