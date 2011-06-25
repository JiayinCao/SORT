/*
 * filename :	unigrid.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "unigrid.h"

// default constructor
UniGrid::UniGrid()
{
}

// constructor from a primitive list
UniGrid::UniGrid( vector<Primitive*>* l ) : Accelerator(l)
{
}

// destructor
UniGrid::~UniGrid()
{
}

// get the intersection between the ray and the primitive set
bool UniGrid::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	return true;
}

// build the acceleration structure
void UniGrid::Build()
{
}