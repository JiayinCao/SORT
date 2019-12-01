/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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

#ifdef SSE_ENABLED
struct Line4;
#endif

#ifdef AVX_ENABLED
struct Line8;
#endif

//! @brief  Line is a common type for hair or fur rendering.
/**
 * Although being called line, this shape is essentially open cylinder. Other choose is to represent line
 * with billboard like faces that always face the ray, which is not physically accurate and can easily lead
 * problems like self shadowing.
 * With cylinder as the line shape, it represents a surface in 3D space that doesn't depend on anything else.
 * This is more robust in term of ray-shape intersection and leads way less problem than billboard solution,
 * which may work well if the radius is small enough, but it is still buggy.
 */
class   Line : public Shape{
public:
    //! @brief Constructor
    //!
    //! @param  p0      A point on one side of the line.
    //! @param  p1      A point on the other side of the line.
    //! @param  v0      V coordinate of @param p0.
    //! @param  v1      V coordinate of @param p1.
    //! @param  w0      Width at one side of the line.
    //! @param  w1      Width at the other side of the line.
    //! @param  matId   Material id of the line.
    Line( const Point& p0 , const Point& p1 , float v0 , float v1 , float w0 , float w1 , int matId ) :
        m_p0(p0), m_p1(p1), m_gp0(p0), m_gp1(p1), m_v0(v0), m_v1(v1), m_w0(w0), m_w1(w1), m_matId(matId) {
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
    //! @param ls       The light sample.
    //! @param p        The position of shading point to be lit.
    //! @param wi       The vector from shading point to sampled point, it is normalized.
    //! @param pdf      The pdf w.r.t solid angle ( not surface area ) of picking the sampled point.
    //! @return         The sampled point on the surface of the shape.
    Point           Sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n, float* pdf ) const override{
        sAssertMsg( false , LIGHT , "Using line as a area light source shape.");
        return Point();
    }

    //! @brief Sample a ray from the light source without a given shading point.
    //!
    //! This function should not be called, meaning line should not be used as a light source.
    //!
    //! @param ls       The light sample.
    //! @param r        The ray randomly sampled, whose origin lies on the surface of the shape,
    //!                 the direction of the ray will point outward depending on the normal.
    //! @param n        The normal at the surface where the ray shoots from.
    //! @param pdf      The pdf w.r.t solid angle of picking the ray.
    void            Sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf ) const override{
        sAssertMsg( false , LIGHT , "Using line as a area light source shape.");
    }

    //! @brief      Get intersected point between the ray and the shape.
    //!
    //! @param ray      The ray to be tested against.
    //! @param inter    The intersection data to be filled. If it is nullptr, there is no detailed information
    //!                 for the intersection.
    //! @return         Whether the ray intersects the shape.
    bool            GetIntersect( const Ray& ray , Intersection* inter = nullptr ) const override;

    //! @brief Intersection test between the shape and a bounding box.
    //!
    //! Because the accurate intersection test depends also on the viewing angle, which is not available
    //! as an input here, this is more of a conservative solution.
    //!
    //! param box       Bounding box to be checked.
    bool            GetIntersect( const BBox& box ) const override;

    //! @brief      Get bounding box of the shape in world space.
    //!
    //! This is also a conservative solution by expending the AABB by half width, whichever is larger on
    //! either side.
    //!
    //! @return     The bounding box of the shape.
    const BBox&     GetBBox() const override;

    //! @brief      Get the surface area of the shape.
    //!
    //! @return     Surface area of the shape.
    float           SurfaceArea() const override;

    //! @brief      Get the material id.
    //!
    //! @return     Material id of the shape.
    int             GetMaterialId() const {
        return m_matId;
    }

    //! @brief Set transform for the shape.
    //!
    //! Vertices of line in local space is pre-transformed into world space. The purpose of doing this is not
    //! to avoid run-time ray transformation during intersection test, where there is still one. It is for tackling
    //! situation where the matrix is not uniformly transformed, causing the shape of the line stretch along one
    //! direction, which is not desired. Pre-tranforming the vertices into world space will ignore the transformation
    //! on shapes, leading us a more unified way of defining width of the line, no matter what the transformation is.
    //!
    //! @param transform    The new transform of the shape to be set.
    void    SetTransform( const Transform& transform ) override;

    //! @brief      Get the type of the shape
    //!
    //! @return     The type of the shape.
    SHAPE_TYPE GetShapeType() const override{
        return SHAPE_LINE;
    }

private:
    /**< Point at the end of the line. */
    const Point     m_p0;
    /**< Point at the other side of the line. */
    const Point     m_p1;
    /**< Point at the end of the line in global space. */
    Point           m_gp0;
    /**< Point at the other side of the line in global space. */
    Point           m_gp1;
    /**< Half width (radius) of the line at 'm_p0', it should always be positive. */
    const float     m_w0;
    /**< Half width (radius) of the line at 'm_p1', it should always be positive. */
    const float     m_w1;
    /**< V coordinate at point p0. */
    const float     m_v0;
    /**< V coordinate at point p1. */
    const float     m_v1;
    /**< Length of the line segment. */
    float           m_length;
    /**< Material index of the line segment. */
    const int       m_matId;
    /**< Transformation from world space to line space, where the line points from origin upward,
     * there is no scaling in the matrix. */
    Transform       m_world2Line;

#ifdef SSE_ENABLED
    friend struct Line4;
    friend SORT_FORCEINLINE bool intersectLine_SIMD( const Ray& ray , const Line4& line_simd , Intersection* ret );
#endif

#ifdef AVX_ENABLED
    friend struct Line8;
    friend SORT_FORCEINLINE bool intersectLine_SIMD( const Ray& ray , const Line8& line_simd , Intersection* ret );
#endif
};
