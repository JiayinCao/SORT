/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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

//! @brief 	Line is a common type for hair or fur rendering.
/**
 * Unlike line rendering in rasterization, which has a screen-based width regardless
 * of how far away the viewer is, line in ray tracing represents a real line with some
 * width. The 'face' of line always faces towards the viewer.
 * Though being called line, this shape is actually line segment, which has a finite
 * length instead of infinite length.
 * Technically speaking, there is no such a thing like line in reality. All lines have
 * a potential radius, occupying some volume, they may look like cylinder when zooming 
 * in. It is possible to setup a scene where different algorithms result into slightly
 * different result.
 * Although there is not a pure phsically based shape, line shape is especially helpful
 * in rendering things like fur, which is why SORT supports this kind of shape.
 */
class	Line : public Shape{
public:
	//! @brief Constructor
	//!
	//! @param	p0		A point on one side of the line.
	//! @param	p1		A point on the other side of the line.
	//! @param	w0		Width at one side of the line.
	//! @param	w1		Width at the other side of the line.
	//! @param	matId	Material id of the line.
    Line( const Point& p0 , const Point& p1 , float w0 , float w1 , int matId ) : 
		m_p0(p0), m_p1(p1), m_w0(w0), m_w1(w1), m_matId(matId) {
		sAssert( m_w0 >= 0.0f , GENERAL );
		sAssert( m_w1 >= 0.0f , GENERAL );
		m_length = Distance( p0 , p1 );
	}

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
    Point 			Sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n, float* pdf ) const override{
		sAssertMsg( false , LIGHT , "Using line as a area light source shape.");
		return Point();
	}

	//! @brief Sample a ray from the light source without a given shading point.
	//!
	//! This function should not be called, meaning line should not be used as a light source.
	//!
	//! @param ls		The light sample.
	//! @param r		The ray randomly sampled, whose origin lies on the surface of the shape,
	//!					the direction of the ray will point outward depending on the normal.
	//! @param n		The normal at the surface where the ray shoots from.
	//! @param pdf		The pdf w.r.t solid angle of picking the ray.
	void 			Sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf ) const override{
		sAssertMsg( false , LIGHT , "Using line as a area light source shape.");
	}

	//! @brief		Get intersected point between the ray and the shape.
	//!
	//! @param ray		The ray to be tested against.
	//! @param inter	The intersection data to be filled. If it is nullptr, there is no detailed information
	//!					for the intersection.
	//! @return			Whether the ray intersects the shape.
	bool 			GetIntersect( const Ray& ray , Intersection* inter = nullptr ) const override;

	//! @brief Intersection test between the shape and a bounding box.
	//!
	//! Because the accurate intersection test depends also on the viewing angle, which is not available
	//! as an input here, this is more of a conservative solution.
	//!
	//! param box 		Bounding box to be checked.
	bool 			GetIntersect( const BBox& box ) const override;

	//! @brief		Get bounding box of the shape in world space.
	//!
	//! This is also a conservative solution by expending the AABB by half width, whichever is larger on
	//! either side.
	//!
	//! @return		The bounding box of the shape.
	const BBox&		GetBBox() const override;

	//! @brief		Get the surface area of the shape.
	//!
	//! @return		Surface area of the shape.
	float 			SurfaceArea() const override;

	//! @brief		Get the material id.
	//!
	//! @return		Material id of the shape.
	int				GetMaterialId() const {
		return m_matId;
	}
	
private:
	/**< Point at the end of the line. */
	const Point		m_p0;
	/**< Point at the other side of the line. */
	const Point		m_p1;
	/**< Width of the line at 'm_p0', it should always be positive. */
	const float		m_w0;
	/**< Width of the line at 'm_p1', it should always be positive. */
	const float		m_w1;
	/**< Length of the line segment. */
	float			m_length;
	/**< Material index of the line segment. */
	const int		m_matId;
};
