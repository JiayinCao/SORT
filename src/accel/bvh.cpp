/*
 * filename :	bvh.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "bvh.h"
#include "geometry/ray.h"
#include "managers/logmanager.h"

// default construction
Bvh::Bvh()
{
}

// destructor
Bvh::~Bvh()
{
}

// get the intersection between the ray and the primitive set
bool Bvh::GetIntersect( const Ray& ray , Intersection* intersect ) const
{
	return Intersect( ray , m_BBox ) > 0.0f;
}

// build the acceleration structure
void Bvh::Build()
{
	// build bounding box
	_computeBBox();
}

// output log information
void Bvh::OutputLog() const
{
	LOG_HEADER( "Accelerator" );
	LOG<<"Accelerator Type :\tBounding Volumn Hierarchy"<<ENDL;
	LOG<<"Not implemented."<<ENDL;
}