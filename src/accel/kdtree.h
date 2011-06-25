/*
 * filename :	kdtree.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_KDTREE
#define	SORT_KDTREE

// include the header file
#include "accelerator.h"

////////////////////////////////////////////////////////////////////////////////
// definition of kd-tree
//  a k-d tree (short for k-dimensional tree) is a space-partitioning data 
//	structure for organizing points in a k-dimensional space. 
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

// private field
private:

};

#endif