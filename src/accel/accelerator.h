/*
 * filename :	accelerator.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_ACCELERATOR
#define	SORT_ACCELERATOR

// include the header file
#include "sort.h"
#include <vector>

// pre-declera classes
class Primitive;
class Intersection;
class Ray;

////////////////////////////////////////////////////////////////////////////////
// definition of accelerator
// Accelerator is a kind of space-partitioning data structure for improving 
// the performance of ray and scene intersection test.
class	Accelerator
{
// public method
public:
	// default constructor
	Accelerator(){}
	// constructor from primitive list
	// para 'l' ; the primitive list
	Accelerator( vector<Primitive*>* l ) : primitives(l){}
	// destructor
	~Accelerator(){}

	// get the intersection between the ray and the primitive set
	// para 'r' : the ray
	// para 'intersect' : the intersection result
	// result   : 'true' if the ray pirece one of the triangle in the list
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const = 0;

	// build the acceleration structure
	virtual void Build() = 0;

// protected field
protected:
	// the vector storing primitive list
	vector<Primitive*>* primitives;
};

#endif