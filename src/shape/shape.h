/*
   FileName:      shape.h

   Created Time:  2011-08-19 21:45:25

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SHAPE
#define	SORT_SHAPE

#include "geometry/point.h"
#include "geometry/transform.h"

class LightSample;
class Vector;
class Ray;
class Intersection;

/////////////////////////////////////////////////////////////////////////////////////////
//	definition of shape class
//	Note : SORT doesn't make shape and primitive the same because of two reasons.
//		1. Shape is only binded to area light , while primitive is the basic unit of 
//		the scene geometry. The only primitive supported in SORT is triangle, supporting
//		the other geometry doesn't contribute much. There is no material attached to 
//		shape, because it's only emissive , doesn't reflect light in SORT.
//		2. Shape is simple to sample. The pdf of sampling shape is much more accurate
//		than sampling a triangle , let alone the triangle set , which is very hard to 
//		sample correctly.
class	Shape
{
// public method
public:
	// default constructor
	Shape(){ radius = 1.0f; }
	// destructor
	virtual ~Shape(){}

	// sample a point on shape
	// para 'ls': the light sample
	// para 'p'	: the target point for the light to lit.
	// para 'wi': the sampled vector ( output )
	// para 'pdf': the pdf of the light sample ( output )
	// result   : a sampled point from the light source
	virtual Point sample_l( const LightSample& ls , const Point& p , Vector& wi , float* pdf ) const = 0;

	// the surface area of the shape
	virtual float SurfaceArea() const = 0;
	
	// setup transformation for the shape
	virtual void SetTransform( const Transform& s ) { transform = s; }

	// get intersection between the light surface and the ray
	virtual bool GetIntersect( const Ray& ray , Intersection* intersect ) const = 0;

	// set the radius of the shape
	virtual void SetRadius( float r ) { radius = r; }

	// get the pdf of specific direction
	virtual float Pdf( const Point& p , const Point& lp , const Vector& wi ) const = 0;

// protected field
protected:
	// the transformation for the shape
	Transform	transform;
	// the radius of the shape
	float		radius;
};

#endif
