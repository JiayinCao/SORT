/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#include "utility/log.h"
#include "math/transform.h"
#include "geometry/bbox.h"
#include "geometry/ray.h"
#include "geometry/intersection.h"

class LightSample;

//! @brief Shape class defines basic interface of shape.
/**
 * A shape class defines the very fundermental concept of shape supported in SORT.
 * It serves as the basic interface for different shapes supported in SORT. Shape is 
 * purely a mathematical concept, which doesn't have any information about the 
 * material attached to the shape or anything else.
 */
class Shape
{
public:
	//! Empty vitual destructor
	virtual ~Shape(){}

	//! @brief Sample a point on shape given a shading point
	//!
	//! Uniformly sample a position on the surface of the shape. This function is heavily
	//! used in area light, where it is necessary to take random sample on the surface to
	//! support multiple importance sampling.
	//!
	//! @param ls 		The light sample.
	//! @param p		The position of shading point to be lit.
	//! @param wi		The vector from shading point to sampled point, it is normalized.
	//! @param pdf 		The pdf w.r.t solid angle ( not surface area ) of picking the sampled point.
	//! @return			The sampled point on the surface of the shape.
	virtual Point sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n , float* pdf ) const = 0;
	
	//! @brief Sample a ray from the light source without a given shading point
	//!
	//! Uniformly sample a random ray shooting from the surface of the shape.
	//!
	//! @param ls		The light sample.
	//! @param r		The ray randomly sampled, whose origin lies on the surface of the shape.
	//! @param n		The normal at the surface where the ray shoots from.
	//! @param pdf		The pdf w.r.t solid angle of picking the ray.
	virtual void sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf ) const = 0;

	//! @brief Set transform for the shape
	//!
	//! Set transform for the shape
	//!
	//! @param t	The new transform of the shape to be set.
	virtual void SetTransform( const Transform& t ) { transform = t; }

	//! @brief	Get the pdf w.r.t solid angle of picking a point on the surface where the ray intersects
	//!
	//! By shooting a ray, starting from @param p along @param wi, we can caculate the intersection
	//! between the ray and the shape. This function returns the pdf w.r.t solid angle of picking such a
	//! point on the surface of the shape.
	//!
	//! @param p		Origin of the ray.
	//! @param wi		Direction of the ray.
	//! @return			PDF w.r.t the solid angle of picking this sample point on the surface of the shape.
	virtual float Pdf( const Point& p , const Vector& wi ) const{
		Point lp = transform.invMatrix( p );
		Vector lwi = transform.invMatrix( wi );

		Point intersect;
		Intersection inter;
		if( getIntersect( Ray( lp , lwi ) , intersect , &inter ) < 0.0f )
			return 0.0f;

		Vector delta = inter.intersect - p;
		float dot = AbsDot( Normalize(delta) , inter.normal );
		return delta.SquaredLength() / ( SurfaceArea() * dot );
	}

	//! @brief	Get intersection between the shape and the ray
	//!
	//! Get intersection between ray and the shape. This is an inline function, the exact algorithm for
	//! intersection test is inside 'getIntersect' function, which could be overwritten by child class.
	//! The inline function here hides some common detail shared by all shapes, transforming the ray
	//! from world space to local space.
	//! @param ray			The ray we use to check intersection.
	//! @param intersect	The intersection to be filled. If this is nullptr, no information will be 
	//!						output in the case of an intersection.
	//! @return				'True' for a real intersection, 'False' for no intersection.
	inline bool GetIntersect( const Ray& ray , Intersection* intersect ) const {
		Ray local = transform.invMatrix( ray );
		Point p;
		if( intersect )
			return getIntersect( local , p , intersect ) > 0.0f;
		return getIntersect( local , p ) > 0.0f;
	}

	//! @brief		Get bounding box of the shape in world space.
	//!
	//!	Get the bounding box of the shape. Some shape may return a relatively conservative bounding
	//! box, which is also acceptable to all rest systems call this function.
	//!
	//! @return		The bounding box of the shape.
	virtual const BBox&	GetBBox() const = 0;

	//! @brief		Get the surface area of the shape
	//!
	//! Get the surface area of the shape. This function is heavily used in the case of picking a area light
	//! among lots of them, surface area is one of the singals telling us how strong the light is.
	//!
	//! @return		Surface area of the shape.
	virtual float	SurfaceArea() const = 0;

protected:
	Transform						transform;		/**< Transform of the shape from local space to world space. */
	mutable std::unique_ptr<BBox> 	m_bbox;			/**< Bounding box of the shape in world coordinate. */

	//! @brief		Get intersected point between the ray and the shape
	//!
	//!	Get the intersection between a ray and the shape. This is a function that every child 
	//! class needs to overwrite with their own behavior.
	//! Default implementation will simply crash the program. Any non-compound shape should
	//! implement the function with its own algorithm. All compound shape shouldn't overwrite
	//! this function because it will be flattened during spatial structure constructure.
	//!
	//! @param ray		The ray to be tested against.
	//! @param p		The intersected point in local space.
	//! @param inter	The intersection data to be filled. If it is nullptr, there is no detailed information
	//!					for the intersection.
	//! @return			The distance between the intersected point and the ray origin in the case of intersection.
	//!					Otherwise, negative value will be returned.
	virtual float getIntersect( const Ray& ray , Point& p , Intersection* inter = nullptr ) const{
		sAssertMsg( false , SHAPE , "There is no default implementation for shape::getIntersect !" );
	}
};
