/*
 * filename :	instancetri.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "instancetri.h"
#include "triangle.h"

// constructor from a triangle
InstanceTriangle::InstanceTriangle( const Triangle* tri , Transform* transform ):
Primitive( transform )
{
}

// destructor
InstanceTriangle::~InstanceTriangle()
{
}

// get the intersection
bool	InstanceTriangle::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	// to be modified

	return false;
}

// get the bounding box of the triangle
const BBox& InstanceTriangle::GetBBox()
{
	// if there is no bounding box , cache it
	if( !m_bbox )
	{
		// to be modified
		m_bbox = new BBox();
	}

	return *m_bbox;
}
