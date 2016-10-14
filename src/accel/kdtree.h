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

// include the header file
#include "accelerator.h"

class Primitive;

// kd-tree node
struct Kd_Node
{
public:
	Kd_Node*					leftChild = nullptr;	// pointer to the left child
	Kd_Node*					rightChild = nullptr;	// pointer to the right child
	BBox						bbox;                   // bounding box for kd-tree node
	vector<const Primitive*>	trilist;                // triangle list in the leaf node
	unsigned					flag = 0;               // 11 leaf node , 00 x split , 01 y split , 02 z split
	float						split = 0.0f;           // split position

    Kd_Node( const BBox& bb ):bbox(bb){}
};

// split type
enum Split_Type
{
	Split_None = 0,
	Split_End = 1 ,
	Split_Flat = 2 ,
	Split_Start = 4,
};

// split candidates
struct Split
{
public:
	// the position of the split
	float		pos = 0.0f;
	// the type of the split , start or end
	Split_Type	type = Split_None;
	// the id in the triangle list
	unsigned	id = 0;
	// the primitive pointer
	Primitive*	primitive = nullptr;

    Split( float po = 0.0f , Split_Type t = Split_None, unsigned pid = 0, Primitive* p = nullptr): pos(po) , type(t) , id(pid) , primitive(p){
	}
	bool operator < ( const Split& split ) const
	{
		if( pos != split.pos )
			return pos<split.pos;
		return type < split.type ;
	}
};
struct Splits
{
    Split*		split[3] = { nullptr , nullptr , nullptr };
    unsigned	split_c[3] = { 0 , 0 , 0 };
	void Release()
	{
		SAFE_DELETE_ARRAY(split[0]);
		SAFE_DELETE_ARRAY(split[1]);
		SAFE_DELETE_ARRAY(split[2]);
	}
};

////////////////////////////////////////////////////////////////////////////////
// definition of kd-tree
//  a k-d tree (short for k-dimensional tree) is a space-partitioning data 
//	structure for organizing points in a k-dimensional space. 
//	the algorithm of kd-tree construction works in O(N*logN).
//	To see the detail description of the algorithm, check the paper :
//	"On building fast kd-Trees for Ray Tracing, and on doing that in O(N log N)"
class KDTree : public Accelerator
{
// public method
public:
	DEFINE_CREATOR( KDTree , "kd_tree" );

	// destructor
	~KDTree();

	// get the intersection between the ray and the primitive set
	// para 'r' : the ray
	// para 'intersect' : the intersection result
	// result   : 'true' if the ray pirece one of the triangle in the list
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const;

	// build the acceleration structure
	virtual void Build();

	// Output log information
	virtual void OutputLog() const;

// private field
private:
	// the root of kd-tree
	Kd_Node*		m_root = nullptr;
	// temporary buffer for marking triangles
	unsigned char*	m_temp = nullptr;

	// maxmium depth of kd-tree
	const unsigned	m_maxDepth = 28;
	// maxmium number of triangles in a leaf node
	const unsigned	m_maxTriInLeaf = 32;

	// total node number
	unsigned	m_total = 0;
	// leaf node number
	unsigned	m_leaf = 0;
	// average triangle number in leaf
	float		m_fAvgLeafTri = 0;
	// depth of kd-tree
	unsigned	m_depth = 0;
	// maxium number of triangle in a leaf
	unsigned	m_MaxLeafTri = 0;

	// split node
	// para 'node'   : node to be split
	// para 'splits' : splitting candidates
	// para 'depth'  : depth of the node
	void _splitNode( Kd_Node* node , Splits& splits , unsigned tri_num , unsigned depth );
	
	// evaluate sah value for the kdtree node
	// para 'l' : the number of primitives on the left of the splitting plane
	// para 'r' : the number of primitives on the right of the splitting plane
	// para 'f' : the number of flat primitives lying on the splitting plane
	// para 'axis' : the axis id of splitting
	// para 'split' : splitting position
	// para 'box'  : the bounding box of the kd-tree node
	// para 'left' : whether the flat primitives lying on the left of the splitting plane
	// result  : the sah value for the splitting
	float _sah( unsigned l , unsigned r , unsigned f , unsigned axis , float split , const BBox& box , bool& left );
	
	// pick best splitting
	// para 'splits' : the splitting candidates
	// para 'tri_num': triangle number
	// para 'box'    : bounding box of the node
	// para 'splitaxis': axis of splitting
	// para 'split_pos': splitting position
	// para 'left'     : whether flat primitives are lying on the left child node
	// result          : sah value with the specific splitting
	float _pickSplitting( const Splits& splits , unsigned tri_num , const BBox& box , 
						 unsigned& splitAxis , float& split_pos , bool& left );
	
	// make leaf
	// para 'node'   : the node to be leaf
	// para 'splits' : the splitting candidates
	// para 'tri_num': triangle number
	void _makeLeaf( Kd_Node* node , Splits& splits , unsigned tri_num );
	
	// tranverse kd-tree node
	bool _traverse( const Kd_Node* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const;

	// delete kd-tree node
	void deleteKdNode( Kd_Node* node );
};
