/*
 * filename :	unigrid.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_UNIGRID
#define	SORT_UNIGRID

// include the header file
#include "accelerator.h"

////////////////////////////////////////////////////////////////////////////////
// definition of uniform grid
class UniGrid : public Accelerator
{
// public method
public:
	// default constructor
	UniGrid();
	// constructor from a primitive list
	// para 'l' : the primitive list
	UniGrid( vector<Primitive*>* l );
	// destructor
	~UniGrid();

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