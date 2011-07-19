/*
 * filename :	bvh.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_BVH
#define	SORT_BVH

// include header
#include "accelerator.h"

/////////////////////////////////////////////////////////////////////////////////////
//	definition of bounding volumn hierarchy
class Bvh : public Accelerator
{
// public method
public:
	// default construction
	Bvh();
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

};

#endif