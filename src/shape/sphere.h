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

#include "shape.h"

//! @brief Sphere class defines the basic behavior of sphere.
/**
 * The sphere center is always at the origin of its local coordinate.
 */
class	Sphere : public Shape
{
public:
	//! @brief Sample a point on the surface of the shape given a shading point.
	//!
	//! Sample a position on the surface of the shape. This function is heavily
	//! used in area light, where it is necessary to take random sample on the surface to
	//! support multiple importance sampling.
	//!
	//! @param ls 		The light sample.
	//! @param p		The position of shading point to be lit.
	//! @param wi		The vector from shading point to sampled point, it is normalized.
	//! @param pdf 		The pdf w.r.t solid angle ( not surface area ) of picking the sampled point.
	//! @return			The sampled point on the surface of the shape.
    Point 			Sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n , float* pdf ) const override;

	//! @brief Sample a ray from the light source without a given shading point.
	//!
	//! Uniformly sample a random ray shooting from the surface of the shape.
	//!
	//! @param ls		The light sample.
	//! @param r		The ray randomly sampled, whose origin lies on the surface of the shape,
	//!					the direction of the ray will point outward depending on the normal.
	//! @param n		The normal at the surface where the ray shoots from.
	//! @param pdf		The pdf w.r.t solid angle of picking the ray.
    void 			Sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf ) const override;

	//! @brief		Get intersected point between the ray and the shape.
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
	//! @return			Whether the ray intersects the shape.
	bool			GetIntersect( const Ray& ray , Point& p , Intersection* inter = nullptr ) const override;

	//! @brief	Get the pdf w.r.t solid angle of picking a point on the surface where the ray intersects.
	//!
	//! By shooting a ray, starting from @param p along @param wi, we can calculate the intersection
	//! between the ray and the shape. This function returns the pdf w.r.t solid angle of picking such a
	//! point on the surface of the shape.
	//!
	//! @param p		Origin of the ray.
	//! @param wi		Direction of the ray.
	//! @return			PDF w.r.t the solid angle of picking this sample point on the surface of the shape.
	float 			Pdf( const Point& p , const Vector& wi ) const override;

	//! @brief		Get bounding box of the shape in world space.
	//!
	//!	Get the bounding box of the shape. Some shape may return a relatively conservative bounding
	//! box, which is also acceptable to all rest systems call this function.
	//!
	//! @return		The bounding box of the shape.
	const BBox&		GetBBox() const override;

	//! @brief		Get the surface area of the shape.
	//!
	//! Get the surface area of the shape. This function is heavily used in the case of picking a area light
	//! among lots of them, surface area is one of the singals telling us how strong the light is.
	//!
	//! @return		Surface area of the shape.
	float 			SurfaceArea() const override;
    
private:
	float radius = 1.0f;	/**< Radius of the sphere. */
};
