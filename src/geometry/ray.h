/*
 * filename :	ray.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_RAY
#define SORT_RAY

// include the header
#include "vector.h"
#include "point.h"

////////////////////////////////////////////////////////////////////////////
class Ray
{
// public method
public:
	// default constructor
	Ray();
	// constructor from a point and a direction
	Ray( const Point& ori , const Vector& dir );
	// copy constructor
	Ray( const Ray& r );
	// destructor
	~Ray();

// the original point and direction are also public
	// original point of the ray
	Point	m_Ori;
	// direction , the direction of the ray
	Vector	m_Dir;
};
#endif
