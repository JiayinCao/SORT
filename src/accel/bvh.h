/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
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
    // offset in primitive list
    unsigned	pri_offset = 0;
    
    Bvh_Node*   left = 0;
    Bvh_Node*   right = 0;
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
	// primitives in bvh
	Bvh_Primitive*	m_bvhpri = nullptr;
    // root node
    Bvh_Node*       m_root = nullptr;

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
	bool _traverseNode( const Bvh_Node* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const;
    
    // delete node
    void _delete( Bvh_Node* node );
};
