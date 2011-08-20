/*
   FileName:      sphere.h

   Created Time:  2011-08-19 22:01:01

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SPHERE
#define	SORT_SPHERE

#include "shape.h"
#include "utility/creator.h"

///////////////////////////////////////////////////////////////////////////////
//	definition of sphere
class	Sphere : public Shape
{
// public method
public:
	DEFINE_CREATOR(Sphere);

	// default constructor
	Sphere(){}
	// destructor
	~Sphere(){}

	// sample a point on shape
	// para 'ls': the light sample
	// para 'p'	: the target point for the light to lit.
	// para 'wi': the sampled vector ( output )
	// para 'pdf': the pdf of the light sample ( output )
	// result   : a sampled point from the light source
	virtual Point sample_l( const LightSample& ls , const Point& p , Vector& wi , float* pdf ) const;

	// the surface area of the shape
	virtual float SurfaceArea() const;

	// get intersection between the light surface and the ray
	virtual bool GetIntersect( const Ray& ray , Intersection* intersect ) const;

// private method
private:
	// get intersection between a ray and the sphere
	float _getIntersect( const Ray& ray , float limit , Point& p ) const;
};

#endif
