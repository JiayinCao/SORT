/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2017 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef	SORT_SHAPE
#define	SORT_SHAPE

#include "math/point.h"
#include "math/transform.h"
#include "geometry/primitive.h"
#include "math/vector3.h"

class LightSample;
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
	Shape():Primitive(0,nullptr){ radius = 1.0f;}
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
	virtual Point sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n , float* pdf ) const = 0;
	
	// sample a ray from light
	// para 'ls'       : light sample
	// para 'r'       : the light vector
	// para 'pdf'      : the properbility density function
	virtual void sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf ) const = 0;

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
