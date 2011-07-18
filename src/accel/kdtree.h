/*
 * filename :	kdtree.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_KDTREE
#define	SORT_KDTREE

// include the header file
#include "accelerator.h"

class Primitive;

// kd-tree node
struct Kd_Node
{
public:
	union
	{
		unsigned	flag;		// 11 leaf node , 00 x split , 01 y split , 02 z split
		unsigned	right;		// the index of right child
		unsigned	trinum;		// the number of triangles in the leaf node
	};
	union
	{
		float		split;		// splitting information
		unsigned	offset;		// offset in triangle list
	};
	// default constructor
	Kd_Node():flag(0),offset(0){}
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
	float		pos;
	// the type of the split , start or end
	Split_Type	type;
	// the id in the triangle list
	unsigned	id;
	// the primitive pointer
	Primitive*	primitive;

	Split(){pos = 0;type = Split_None;id=0;primitive=0;}
	Split( float po , Split_Type t , unsigned pid , Primitive* p = 0)
	{
		pos = po;type = t;id=pid;
		primitive = p;
	}
	Split( const Split& split )
	{
		pos = split.pos;
		type = split.type;
		id = split.id;
		primitive = split.primitive;
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
	Split*		split[3];
	unsigned	split_c[3];
	Splits()
	{
		split_c[0] = 0;
		split_c[1] = 0;
		split_c[2] = 0;
	}
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
class KDTree : public Accelerator
{
// public method
public:
	// default constructor
	KDTree();
	// constructor from a primitive list
	// para 'l' : the primitive list
	KDTree( vector<Primitive*>* l );
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
	Kd_Node*		m_nodes;
	// the primitive list
	Primitive**		m_prilist;
	// temporary buffer for marking triangles
	unsigned char*	m_temp;

	// initialize
	void _init();
	
	// split node
	// para 'node'   : node to be split
	// para 'splits' : splitting candidates
	// para 'box'    : bouding box of the node
	void _splitNode( Kd_Node* node , Splits& splits , unsigned tri_num , const BBox& box );
	
	// malloc the memory
	void _mallocMemory();
	
	// free temporary memory
	void _deallocTmpMemory();
	
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
	void _pickSplitting( const Splits& splits , unsigned tri_num , const BBox& box , 
						 unsigned& splitAxis , float& split_pos , bool& left );
	
	// make leaf
	// para 'node'   : the node to be leaf
	// para 'splits' : the splitting candidates
	// para 'tri_num': triangle number
	void _makeLeaf( Kd_Node* node , Splits& splits , unsigned tri_num );
	
	// tranverse kd-tree node
	bool _traverse( Kd_Node* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const;
};

#endif
