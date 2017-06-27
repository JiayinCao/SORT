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
	DEFINE_CREATOR(Sphere,"sphere");

	// default constructor
	Sphere(){}
	// destructor
	~Sphere(){}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// methods inheriting from Shape ( for light )

	// sample a point on shape
	// para 'ls': the light sample
	// para 'p'	: the target point for the light to lit.
	// para 'wi': the sampled vector ( output )
	// para 'pdf': the pdf of the light sample ( output )
	// result   : a sampled point from the light source
	virtual Point sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n , float* pdf ) const;

	// sample a ray from light
	// para 'ls'       : light sample
	// para 'r'       : the light vector
	// para 'pdf'      : the properbility density function
	virtual void sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf ) const;

	// get the pdf of specific direction
	virtual float Pdf( const Point& p , const Vector& wi ) const ;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// methods inheriting from Primitive ( for geometry )

	// get the bounding box of the primitive
	virtual const BBox&	GetBBox() const;

	// the surface area of the shape
	virtual float SurfaceArea() const;
    
// private method
private:
	// get intersection between a ray and the sphere
	float	_getIntersect( const Ray& ray , Point& p , float limit = FLT_MAX , Intersection* inter = 0 ) const;
};

#endif
