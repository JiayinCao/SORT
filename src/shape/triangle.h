/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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

#include "core/define.h"
#include "shape.h"
#include "simd/simd_wrapper.h"

class   MeshVisual;
struct  MeshFaceIndex;

//! @brief Triangle class defines the basic behavior of triangle.
/**
 * Triangle is the most common shape that is used in a ray tracer.
 */
class   Triangle : public Shape{
public:
    //! @brief Constructor
    //!
    //! @param mesh         The triangle mesh it belongs to
    //! @param index        The index buffer
    Triangle( const class MeshVisual* mesh , const struct MeshFaceIndex& index ): m_meshVisual(mesh) , m_index(index) {}

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
        // to be implemented
        return Point();
    }

    //! @brief Sample a ray from the light source without a given shading point.
    //!
    //! Uniformly sample a random ray shooting from the surface of the shape.
    //!
    //! @param ls       The light sample.
    //! @param r        The ray randomly sampled, whose origin lies on the surface of the shape,
    //!                 the direction of the ray will point outward depending on the normal.
    //! @param n        The normal at the surface where the ray shoots from.
    //! @param pdf      The pdf w.r.t solid angle of picking the ray.
    void            Sample_l( RenderContext& rc, const LightSample& ls , Ray& r , Vector& n , float* pdf ) const override{
        // to be implemented.
    }

    //! @brief      Get intersected point between the ray and the shape.
    //!
    //! SORT implements a watertight ray triangle intersection for better precision.
    //! The detail algorithm could be found in this paper,
    //! <a href="http://jcgt.org/published/0002/01/05/paper.pdf">Watertight Ray/Triangle Intersection</a>.
    //!
    //! @param ray      The ray to be tested against.
    //! @param inter    The intersection data to be filled. If it is nullptr, there is no detailed information
    //!                 for the intersection.
    //! @return         Whether the ray intersects the shape.
    bool            GetIntersect( const Ray& ray , SurfaceInteraction* inter = nullptr ) const override;

    //! @brief Intersection test between the shape and a bounding box.
    //!
    //! Detail algorithm of triangle and bounding box intersection comes from this paper,
    //! <a href="http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox_tam.pdf">Fast 3D Triangle-Box Overlap Testing</a>.
    //!
    //! param box       Bounding box to be checked.
    bool            GetIntersect( const BBox& box ) const override;

    //! @brief      Get bounding box of the shape in world space.
    //!
    //! Get the bounding box of the shape. Some shape may return a relatively conservative bounding
    //! box, which is also acceptable to all rest systems call this function.
    //!
    //! @return     The bounding box of the shape.
    const BBox&     GetBBox() const override;

    //! @brief      Get the surface area of the shape.
    //!
    //! Get the surface area of the shape. This function is heavily used in the case of picking a area light
    //! among lots of them, surface area is one of the signals telling us how strong the light is.
    //!
    //! @return     Surface area of the shape.
    float           SurfaceArea() const override;

    //! @brief      Get the type of the shape
    //!
    //! @return     The type of the shape.
    SHAPE_TYPE GetShapeType() const override{
        return SHAPE_TRIANGLE;
    }

// This is very weird, somehow friend struct doesn't work on Linux and Mac since this change below
// https://github.com/JiayinCao/SORT/commit/9e22f5bc62bfe5a3bdf29cc0cf1ffcb06ac96cd3
// This is low priority since I only have limited time to support Neon, I'll leave it this way for now.
public:
    const MeshVisual*        m_meshVisual = nullptr;     /**< Visual holding the vertex buffer. */
    const MeshFaceIndex&     m_index;                    /**< Index buffer points to the index of this triangle. */

#ifdef SIMD_4WAY_ENABLED
    struct Triangle4;
    friend SORT_FORCEINLINE void setupIntersection(const Triangle4& tri4, const Ray& ray, const simd_data_sse& t4, const simd_data_sse& u4, const simd_data_sse& v4, const int id, SurfaceInteraction* intersection);
#endif

#ifdef SIMD_8WAY_ENABLED
    struct Triangle8;
    friend SORT_FORCEINLINE void setupIntersection(const Triangle8& tri4, const Ray& ray, const simd_data_avx& t8, const simd_data_avx& u8, const simd_data_avx& v8, const int id, SurfaceInteraction* intersection);
#endif
};