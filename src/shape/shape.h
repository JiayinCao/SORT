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
#include "geometry/primitive.h"

class LightSample;
class Vector;
class Ray;
class Intersection;
class Light;

/////////////////////////////////////////////////////////////////////////////////////////
//	definition of shape class
class	Shape : public Primitive
{
// public method
public:
	// default constructor
	Shape():Primitive(0,0){ radius = 1.0f;}
	// destructor
	virtual ~Shape(){}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// methods for Shape ( for light )

	// sample a point on shape
	// para 'ls': the light sample
	// para 'p'	: the target point for the light to lit.
	// para 'wi': the sampled vector ( output )
	// para 'pdf': the pdf of the light sample ( output )
	// result   : a sampled point from the light source
	virtual Point sample_l( const LightSample& ls , const Point& p , Vector& wi , float* pdf ) const = 0;
	
	// sample a ray from light
	// para 'ls'       : light sample
	// para 'r'       : the light vector
	// para 'pdf'      : the properbility density function
	virtual void sample_l( const LightSample& ls , Ray& r , float* pdf ) const = 0;

	// setup transformation for the shape
	virtual void SetTransform( const Transform& s ) { transform = s; }

	// set the radius of the shape
	virtual void SetRadius( float r ) { radius = r; }

	// get the pdf of specific direction
	virtual float Pdf( const Point& p , const Vector& wi ) const;

	// bind light
	void	BindLight( Light* l ) { light = l; }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// methods inheriting from Primitive ( for geometry )
	//
	// get intersection between the light surface and the ray
	virtual bool GetIntersect( const Ray& ray , Intersection* intersect ) const;
	//
	// get the bounding box of the primitive
	virtual const BBox&	GetBBox() const = 0;
	//
	// get surface area of the primitive
	virtual float	SurfaceArea() const = 0;

// protected field
protected:
	
	// the transformation for the shape
	Transform	transform;
	// the radius of the shape
	float		radius;

	// get intersected point between the ray and the shape
	virtual float _getIntersect( const Ray& ray , Point& p , float limit = FLT_MAX , Intersection* inter = 0 ) const = 0;
};

#endif
