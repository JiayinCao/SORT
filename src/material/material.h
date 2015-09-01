/*
   FileName:      material.h

   Created Time:  2011-08-04 12:46:32

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MATERIAL
#define	SORT_MATERIAL

// include the cpp file
#include "utility/propertyset.h"
#include "utility/creator.h"
#include "spectrum/spectrum.h"
#include "utility/referencecount.h"
#include <vector>

// pre-declera classes
class Bsdf;
class Bxdf;
class Intersection;
class Merl;

enum Socket_Type
{
	Socket_Value,
	Socket_Node,
};

enum Material_Node_Type
{
	Material_Node_Bxdf,
	Material_Node_Others,
};

class MaterialNode;

class MaterialSocket
{
public:
	// default constructor
	MaterialSocket(){
		type = Socket_Value;
		node = 0;
	}

	// type of the socket input
	Socket_Type type;

	// temp solution
	Spectrum		value;
	string			str_value;
	MaterialNode*	node;
};

class MaterialNode : public PropertySet<MaterialNode>
{
public:
	// the material inputs
	std::vector<MaterialSocket>	inputs;

	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf );
};

class LambertNode : public MaterialNode
{
public:
	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf );
};

class MerlNode : public MaterialNode
{
public:
	// update bsdf
	virtual void UpdateBSDF( Bsdf* bsdf );

	Merl* merl;
};

///////////////////////////////////////////////////////////
// definition of material
class Material : public PropertySet<Material> , public ReferenceCount
{
// public method
public:
	Material();

	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;

	// set name
	void SetName( const string& n ) { name = n; }
	// get name of the material
	const string& GetName() const { return name; }

	// set root
	MaterialNode* GetRootNode() { return root; }

// private field
private:
	// the name for the material
	string			name;

	// the root node of the material
	MaterialNode*	root;
};

#endif