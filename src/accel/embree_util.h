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

#ifdef INTEL_EMBREE_ENABLED

// Intel embree API
#include <embree3/rtcore.h>
#include <vector>
#include "core/define.h"
#include "math/ray.h"

class Primitive;
struct SurfaceInteraction;
class Embree;

//! @brief  A wrapper of embree geometry
struct EmbreeGeometry{
    // Embree geometry object
    RTCGeometry    m_geometry;

    //! @brief  A list of primitives in this geometry
    //!
    //! This is clearly not the optimal solution in terms of memory usage.
    //! I'm not planning to use Embree for productinoal scene for now, so it
    //! should not be a big problem.
    std::vector<const Primitive*>   m_primitives;

    //! @brief  Make sure the geometry is destroyed
    ~EmbreeGeometry() {
        rtcReleaseGeometry(m_geometry);
    }
};

//! @brief  Construct an Embree ray from a SORT ray
SORT_FORCEINLINE void EmbreeRayFromSORT(const Ray& sort_ray, RTCRay& embree_ray) {
    embree_ray.org_x = sort_ray.m_Ori[0];
    embree_ray.org_y = sort_ray.m_Ori[1];
    embree_ray.org_z = sort_ray.m_Ori[2];
    embree_ray.dir_x = sort_ray.m_Dir[0];
    embree_ray.dir_y = sort_ray.m_Dir[1];
    embree_ray.dir_z = sort_ray.m_Dir[2];
    embree_ray.tnear = sort_ray.m_fMin;
    embree_ray.tfar = sort_ray.m_fMax;
    embree_ray.time = 0.0f;
    embree_ray.mask = 0xffffffff;
    embree_ray.flags = 0xffffffff;
    embree_ray.id = 0xffffffff;
}

//! @briec  Construct a SORT ray from an Embree ray
SORT_FORCEINLINE void SORTRayFromEmbree(const RTCRay& embree_ray, Ray& sort_ray) {
    sort_ray.m_Ori[0] = embree_ray.org_x;
    sort_ray.m_Ori[1] = embree_ray.org_y;
    sort_ray.m_Ori[2] = embree_ray.org_z;
    sort_ray.m_Dir[0] = embree_ray.dir_x;
    sort_ray.m_Dir[1] = embree_ray.dir_y;
    sort_ray.m_Dir[2] = embree_ray.dir_z;
    sort_ray.m_fMin = embree_ray.tnear;
    sort_ray.m_fMax = embree_ray.tfar;
}

template<class T>
class EmbreeShape {
protected:
    //! @brief  Process embree data.
    //!
    //! @param device   Embree device.
    EmbreeGeometry* buildEmbreeGeometry(RTCDevice device, Embree& ebmree, const T* shape) const;

private:
    // Embree geometry id
    mutable unsigned int m_rtc_geom_id;

    static void rtc_bounding_box(const struct RTCBoundsFunctionArguments* args);
    static void rtc_intersect(const RTCIntersectFunctionNArguments* args);
    static void rtc_occluded(const RTCOccludedFunctionNArguments* args);
};

#endif