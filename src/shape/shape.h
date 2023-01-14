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

#include <memory>
#include "core/log.h"
#include "math/transform.h"
#include "math/bbox.h"
#include "math/ray.h"
#include "math/interaction.h"

#ifdef INTEL_EMBREE_ENABLED
    #include <embree3/rtcore.h>
    #include "accel/embree_util.h"
#endif

class LightSample;
class Embree;
struct RenderContext;

enum SHAPE_TYPE{
    SHAPE_TRIANGLE  = 0,
    SHAPE_LINE      = 1,
    SHAPE_DISK      = 2,
    SHAPE_QUAD      = 3,
    SHAPE_SPHERE    = 4,
};

//! @brief Shape class defines basic interface of shape.
/**
 * A shape class defines the very fundamental concept of shape supported in SORT.
 * It serves as the basic interface for different shapes supported in SORT. Shape is
 * purely a mathematical concept, which doesn't have any information about the
 * material attached to the shape or anything else.
 */
class Shape
{
public:
    //! @brief  Empty virtual destructor.
    virtual ~Shape(){}

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
    virtual Point   Sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n , float* pdf ) const = 0;

    //! @brief Sample a ray from the light source without a given shading point.
    //!
    //! Uniformly sample a random ray shooting from the surface of the shape.
    //!
    //! @param ls       The light sample.
    //! @param r        The ray randomly sampled, whose origin lies on the surface of the shape,
    //!                 the direction of the ray will point outward depending on the normal.
    //! @param n        The normal at the surface where the ray shoots from.
    //! @param pdf      The pdf w.r.t solid angle of picking the ray.
    virtual void    Sample_l( RenderContext& rc, const LightSample& ls , Ray& r , Vector& n , float* pdf ) const = 0;

    //! @brief  Get the pdf w.r.t solid angle of picking a point on the surface where the ray intersects.
    //!
    //! By shooting a ray, starting from @param p along @param wi, we can calculate the intersection
    //! between the ray and the shape. This function returns the pdf w.r.t solid angle of picking such a
    //! point on the surface of the shape.
    //!
    //! @param p        Origin of the ray.
    //! @param wi       Direction of the ray.
    //! @return         PDF w.r.t the solid angle of picking this sample point on the surface of the shape.
    virtual float   Pdf( const Point& p , const Vector& wi ) const{
        SurfaceInteraction inter;
        if( !GetIntersect( Ray( p , wi ) , &inter ) )
            return 0.0f;

        const auto delta = p - inter.intersect;
        const auto dot = satDot( normalize(delta) , inter.normal );
        if( dot <= 0.0f )
            return 0.0f;
        return delta.SquaredLength() / ( SurfaceArea() * dot );
    }

    //! @brief      Get intersected point between the ray and the shape.
    //!
    //! Get the intersection between a ray and the shape. This is a function that every child
    //! class needs to overwrite with their own behavior.
    //! Default implementation will simply crash the program. Any non-compound shape should
    //! implement the function with its own algorithm. All compound shape shouldn't overwrite
    //! this function because it will be flattened during spatial structure construction.
    //!
    //! @param ray      The ray to be tested against.
    //! @param inter    The intersection data to be filled. If it is nullptr, there is no detailed information
    //!                 for the intersection.
    //! @return         Whether the ray intersects the shape.
    virtual bool    GetIntersect( const Ray& ray , SurfaceInteraction* inter = nullptr ) const = 0;

    //! @brief Intersection test between the shape and a bounding box.
    //!
    //! The default implementation here is a fairly conservative one by returning true.
    //! To be changed once I have a better solution, maybe AABB against OBB intersection algorithm as default one.
    //! param box       Bounding box to be checked.
    virtual bool    GetIntersect( const BBox& box ) const { return true; }

    //! @brief      Get bounding box of the shape in world space.
    //!
    //! Get the bounding box of the shape. Some shape may return a relatively conservative bounding
    //! box, which is also acceptable to all rest systems call this function.
    //!
    //! @return     The bounding box of the shape.
    virtual const   BBox&   GetBBox() const = 0;

    //! @brief      Get the surface area of the shape.
    //!
    //! Get the surface area of the shape. This function is heavily used in the case of picking a area light
    //! among lots of them, surface area is one of the signals telling us how strong the light is.
    //!
    //! @return     Surface area of the shape.
    virtual float   SurfaceArea() const = 0;

    //! @brief      Set transform for the shape.
    //!
    //! @param transform    The new transform of the shape to be set.
    virtual void    SetTransform( const Transform& transform ) { m_transform = transform; }

    //! @brief      Get the type of the shape
    //!
    //! @return     The type of the shape.
    virtual SHAPE_TYPE GetShapeType() const = 0;

#if INTEL_EMBREE_ENABLED
    //! @brief      Construct instersection data from Embree intersection.
    //!
    //! @param ray_hit   Embree intersection data.
    //! @param inter     SORT intersection data.
    virtual void ConvertIntersection(const RTCRayHit& ray_hit, SurfaceInteraction& inter) const = 0;

    //! @brief  Process embree data.
    //!
    //! @param device   Embree device.
    //! @param geom     Embree geometry wrapper
    virtual EmbreeGeometry* BuildEmbreeGeometry(RTCDevice device, Embree& ebmree) const = 0;
#endif
    
protected:
    Transform                       m_transform;    /**< Transform of the shape from local space to world space. It is assumed there is no scaling in this matrix, the upper level code should handle it. */
    mutable std::unique_ptr<BBox>   m_bbox;         /**< Bounding box of the shape in world coordinate. */
};
