/*
 * filename :	intersection.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_INTERSECTION
#define	SORT_INTERSECTION

// include the header
#include "point.h"

// pre-decleration class
class Primitive;

///////////////////////////////////////////////////////////////////////
//	definition of intersection
class	Intersection
{
// public method
public :
	// default constructor
	Intersection();
	// destructor
	~Intersection();

// public field
public:
	// the interesection point
	Point	intersect;
	// the normal
	Vector	normal;
	// tangent vector
	Vector	tangent;
	// the uv coordinate
	float	u , v;
	// the delta distance from the orginal point
	float	t;
	// the intersected primitive
	Primitive* primitive;
};

#endif
