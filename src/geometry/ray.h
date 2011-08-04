/*
   FileName:      ray.h

   Created Time:  2011-08-04 12:50:15

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_RAY
#define SORT_RAY

// include the header
#include "vector.h"
#include "point.h"
#include "float.h"

////////////////////////////////////////////////////////////////////////////
class Ray
{
// public method
public:
	// default constructor
	Ray();
	// constructor from a point and a direction
	// para 'ori'   :	original point of the ray
	// para 'dir'   :	direction of the ray , it's the programmer's responsibility to normalize it
	// para 'depth' :	the depth of the curren ray , if not set , default value is 0
	// para 'fmin'  :	the minium range of the ray . It could be set a very small value to avoid false self intersection
	// para 'fmax'  :	the maxium range of the ray . A ray with 'fmax' not equal to 0 is actually a line segment, usually used for shadow ray.
	Ray( const Point& ori , const Vector& dir , unsigned depth = 0 , float fmin = 0.0f , float fmax = FLT_MAX );
	// copy constructor
	// para 'r' :	a ray to copy
	Ray( const Ray& r );
	// destructor
	~Ray();
	
	// operator to get a point from the ray
	// para 't' :	the distance from the retrive point if the direction of the ray is normalized.
	// reslut   :	A point ( o + t * d )
	Point operator()(float t) const;

// the original point and direction are also public
	// original point of the ray
	Point	m_Ori;
	// direction , the direction of the ray
	Vector	m_Dir;

	// the depth for the ray
	unsigned m_Depth;

	// the maxium and minium value in the ray
	float	m_fMin;
	float	m_fMax;
};
#endif


