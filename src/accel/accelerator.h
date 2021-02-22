/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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

#include <vector>
#include "core/define.h"
#include "math/bbox.h"
#include "core/rtti.h"
#include "core/stats.h"
#include "core/profile.h"
#include "stream/stream.h"
#include "core/render_context.h"
#include "math/interaction.h"
#include "scatteringevent/bssrdf/bssrdf.h"
#include "medium/medium.h"
#include "core/primitive.h"

class Ray;

#ifdef ENABLE_TRANSPARENT_SHADOW
SORT_FORCEINLINE bool isShadowRay( const SurfaceInteraction* intersection ){
    return intersection->query_shadow;
}
#else
SORT_FORCEINLINE bool isShadowRay( const SurfaceInteraction* intersection ){
    return IS_PTR_INVALID( intersection );
}
#endif

//! @brief Spatial acceleration structure interface.
/**
 * Accelerator is an interface rather than a base class. There is no instance of it.
 * It is responsible for acceleration of intersection tests between ray and primitives.
 * Tracing a ray against the scene without a spatial acceleration structure is O(N) where
 * N is the number of primitives. Spatial acceleration structure can optimize the
 * algorithm so that it is O(lg(N)), a significant improvement over the naive brute
 * force ray tracing. Common spatial structures include KD-Tree, BVH and Uniform Grid.
 */
class   Accelerator : public SerializableObject{
public:
    //! @brief  Empty destructor.
    virtual ~Accelerator() = default;

    //! @brief Get intersection between the ray and the primitive set using spatial data structure.
    //!
    //! It will return true if there is intersection between the ray and the primitive set.
    //! In case of an intersection, it will fill the structure and return the nearest intersection.
    //! This intersection could possibly be a fully transparent intersection, it is up to the higher
    //! level logic to handle (semi)transparency later.
    //!
    //! @param r            The input ray to be tested.
    //! @param intersect    The intersection result. If a nullptr pointer is provided, it
    //!                     stops as long as it finds an intersection. It is faster than
    //!                     the one with intersection information data and suitable for
    //!                     shadow ray calculation.
    //! @return             It will return true if there is an intersection, otherwise
    //!                     it returns false.
    virtual bool GetIntersect( RenderContext& rc, const Ray& r , SurfaceInteraction& intersect) const = 0;

#ifndef ENABLE_TRANSPARENT_SHADOW
    //! @brief This is a dedicated interface for detecting shadow rays.
    //!
    //! Instead of merging the interface with 'GetIntersect', this is a separate interface purely for occlusion detection.
    //! There is a need for it so that we can achieve better performance. There will be less branch in this interfaces and
    //! most importantly the traversed node doesn't need to be sorted.
    //!
    //! @param r            The ray to be tested.
    //! @return             Whether the ray is occluded by anything.
    virtual bool IsOccluded( const Ray& r ) const = 0;
#else
    //! @brief  Evaluate attenuation along a ray segment.
    //!
    //! This function just returned the attenuation of the first intersection. It is not responsible for evaluating all attenuations
    //! along the ray.
    //!
    //! @param r            The ray to be tested. Its origin will be updated upon intersection.
    //! @param attenuation  The occlusion along the ray.
    //! @param ms           The medium stack used to evaluate shadow attenuation.
    //! @return             Whether there is an intersection along the ray.
    bool         GetAttenuation( Ray& r , Spectrum& attenuation , RenderContext& rc ,MediumStack* ms = nullptr ) const;
#endif

    //! @brief    Update medium stack.
    //!
    //! The only difference between this function and 'GetAttenuation' is there is no need to evaluate attenuation.
    //!
    //! @param    ray            The ray to be tested.
    //! @param    ms            The medium stack to be populated.
    //! @param  reversed    Reversing the order of updating medium stack.
    //! @return                Whether there is an intersection found.
    bool        UpdateMediumStack( Ray& r , MediumStack& ms , RenderContext& rc , const bool reversed = false ) const;

    //! @brief Get multiple intersections between the ray and the primitive set using spatial data structure.
    //!
    //! This is a specific interface designed for SSS during disk ray casting. Without this interface, the algorithm has to use the
    //! above one to acquire all intersections in a brute force way, which obviously introduces quite some duplicated work.
    //! The intersection returned doesn't guarantee the order of the intersection of the results, but it does guarantee to get the
    //! nearest N intersections.
    //! WARNING, it is quite possible to find an intersection that is fully transparent and still taking consideration of the light
    //! coming from that point. This is not strictly correct, but I would choose to live with it because it is not worth the extra
    //! performance overhead to fix the problem since it is very minor.
    //!
    //! @param  r           The input ray to be tested.
    //! @param  intersect   The intersection result that holds all intersection.
    //! @param  matID       We are only interested in intersection with the same material, whose material id should be set to matID.
    virtual void GetIntersect( const Ray& r , BSSRDFIntersections& intersect , RenderContext& rc , const StringID matID = INVALID_SID ) const = 0;

    //! @brief Build the acceleration structure.
    //!
    //! @param primitives       A vector holding all primitives.
    //! @param bbox             The bounding box of the scene.
    virtual void Build(const std::vector<const Primitive*>& primitives, const BBox& bbox) = 0;

    //! @brief Get the bounding box of the primitive set.
    //!
    //! @return Bounding box of the spatial acceleration structure.
    SORT_FORCEINLINE const BBox& GetBBox() const {
        return m_bbox;
    }

    //! @brief  Whether the spatial data structure is constructed.
    //!
    //! @return     Whether the spatial data structure is constructed.
    SORT_FORCEINLINE bool GetIsValid() const {
        return m_isValid;
    }

    //! @brief    Clone the accelerator.
    //!
    //! Only configuration will be cloned, not the data inside the accelerator, this is for primitives that has volumes attached.
    //!
    //! @return        Cloned accelerator.
    virtual std::unique_ptr<Accelerator>    Clone() const = 0;

protected:
    /**< The vector holding all primitive pointers. */
    const std::vector<const Primitive*>*    m_primitives = nullptr;
    /**< The bounding box of all primitives. */
    BBox                                    m_bbox;
    /**< Whether the spatial structure is constructed before. */
    bool                                    m_isValid = false;
};
