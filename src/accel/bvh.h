/*
   FileName:      bvh.h

   Created Time:  2011-08-04 12:53:05

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_BVH
#define	SORT_BVH

// include header
#include "accelerator.h"
#include "geometry/primitive.h"

// node for bvh
struct Bvh_Node
{
	// bounding box for the node
	BBox		bbox;
	// primitive number
	unsigned 	pri_num = 0;
	union
	{
		// offset in primitive list
		unsigned	pri_offset = 0;
		// index of right child
		Bvh_Node*	right_child;
	};
};

// bvh primitives
struct Bvh_Primitive
{
	Primitive*	primitive;
	Point		m_centroid;

	Bvh_Primitive( Primitive* p ):primitive(p)
	{m_centroid = ( p->GetBBox().m_Max + p->GetBBox().m_Min ) * 0.5f;}

	const BBox& GetBBox()
	{return primitive->GetBBox();}
};

/////////////////////////////////////////////////////////////////////////////////////
//	definition of bounding volume hierarchy
//	This is a simple version for "On face Construction of SAH-based Bounding Volume
//	Hierarchies"
class Bvh : public Accelerator
{
// public method
public:
	DEFINE_CREATOR( Bvh , "bvh" );

	// destructor
	~Bvh();

	// get the intersection between the ray and the primitive set
	// para 'r' : the ray
	// para 'intersect' : the intersection result
	// result   : 'true' if the ray pirece one of the triangle in the list
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const;

	// build the acceleration structure
	virtual void Build();

	// output log information
	void OutputLog() const;

// private field
private:
	// the nodes for bvh
	Bvh_Node*		m_nodes = nullptr;
	// primitives in bvh
	Bvh_Primitive*	m_bvhpri = nullptr;

	// maxmium primtive number in each leaf node
	unsigned	m_maxPriInLeaf = 8;

	// bvh information
	unsigned	m_totalNode = 0;
	// leaf node
	unsigned	m_leafNode = 0;
	// depth
	unsigned	m_bvhDepth = 0;
	// maxmium triangle number in leaf
	unsigned	m_maxLeafTriNum = 0;

	// malloc the memory
	void _mallocMemory();

	// dealloc memory
	void _deallocMemory();

	// recursive split node
	void _splitNode( Bvh_Node* node , unsigned _start , unsigned _end , unsigned depth );

	// make the node as a leaf
	void _makeLeaf( Bvh_Node* node , unsigned _start , unsigned _end );

	// evaluate sah value
	float _sah( unsigned left , unsigned right , const BBox& lbox , const BBox& rbox , const BBox& box );

	// pick best split plane
	float _pickBestSplit( unsigned& axis , float& split_pos , Bvh_Node* node , unsigned _start , unsigned _end );

	// traverse node
	bool _traverseNode( Bvh_Node* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const;
};

#endif