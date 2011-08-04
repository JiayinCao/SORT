/*
   FileName:      intersection.h

   Created Time:  2011-08-04 12:49:29

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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
