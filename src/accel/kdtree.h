/*
 * filename :	kdtree.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_KDTREE
#define	SORT_KDTREE

// include the header file
#include "accelerator.h"

// kd-tree node
struct Kd_Node
{
public:
	union
	{
		unsigned	flag;		// 11 leaf node , 00 x split , 01 y split , 02 z split
		unsigned	right;		// the index of right child
		unsigned	pri_offset;	// the index of triangle list
	};
	union
	{
		float		split;		// splitting information
		unsigned	trinum;		// triangle number in the leaf node
	};
	// default constructor
	Kd_Node():flag(0),trinum(0){}
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
	// the primitive id
	unsigned	id;

	Split(){pos = 0;type = Split_None;id=0;}
	Split( float p , Split_Type t , unsigned pid ){pos = p;type = t;id=pid;}
	bool operator < ( Split& split )
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
	Kd_Node*		m_root;
	// temporary buffer for marking triangles
	unsigned char*	m_temp;

	// initialize
	void _init();
	// split node
	void _splitNode( Kd_Node* node , Splits& splits , unsigned tri_num , const BBox& box );
	// malloc the memory
	void _mallocMemory();
	// free temporary memory
	void _deallocTmpMemory();
	// evaluate sah value for the kdtree node
	float _sah( unsigned l , unsigned r , unsigned axis , float split , const BBox& box );
};

#endif
