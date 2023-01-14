/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifdef INTEL_EMBREE_ENABLED

#include "embree_util.h"
#include "embree.h"
#include "shape/quad.h"
#include "shape/disk.h"
#include "shape/line.h"
#include "shape/sphere.h"

template<class T>
EmbreeGeometry* EmbreeShape<T>::buildEmbreeGeometry(RTCDevice device, Embree& embree, const T* shape) const {
    auto geometry = std::make_unique<EmbreeGeometry>();
    auto ret = geometry.get();

    geometry->m_geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_USER);

    rtcSetGeometryUserPrimitiveCount(geometry->m_geometry, 1);
    rtcSetGeometryUserData(geometry->m_geometry, (void*)shape);

    rtcSetGeometryBoundsFunction(geometry->m_geometry, EmbreeShape<T>::rtc_bounding_box, nullptr);
    rtcSetGeometryIntersectFunction(geometry->m_geometry, EmbreeShape<T>::rtc_intersect);
    rtcSetGeometryOccludedFunction(geometry->m_geometry, EmbreeShape<T>::rtc_occluded);

    rtcCommitGeometry(geometry->m_geometry);

    m_rtc_geom_id = embree.PushGeometry(std::move(geometry));

    return ret;
}

template<class T>
void EmbreeShape<T>::rtc_bounding_box(const struct RTCBoundsFunctionArguments* args) {
    const T* shape = (const T*)args->geometryUserPtr;
    const auto& aabb = shape->GetBBox();

    RTCBounds* bounds_o = args->bounds_o;
    bounds_o->lower_x = aabb.m_Min.x;
    bounds_o->lower_y = aabb.m_Min.y;
    bounds_o->lower_z = aabb.m_Min.z;
    bounds_o->upper_x = aabb.m_Max.x;
    bounds_o->upper_y = aabb.m_Max.y;
    bounds_o->upper_z = aabb.m_Max.z;
}

template<class T>
void EmbreeShape<T>::rtc_intersect(const RTCIntersectFunctionNArguments* args) {
    int* valid = args->valid;
    if (!valid[0])
        return;

    auto ptr = args->geometryUserPtr;
    auto ray_hit = (RTCRayHit*)args->rayhit;
    auto hit = &ray_hit->hit;
    auto ray = &ray_hit->ray;

    auto shape = (const T*)args->geometryUserPtr;

    // construct a sort ray
    Ray sort_ray;
    SORTRayFromEmbree(*ray, sort_ray);

    SurfaceInteraction surface_intersection;
    const auto intersected = shape->GetIntersect(sort_ray, &surface_intersection);
    if (!intersected)
        return;

    // make sure geometry id is updated.
    hit->geomID = shape->m_rtc_geom_id;
    hit->instID[0] = 0;              // instance is not supported yet.
    hit->primID = 0;                 // there is always one single primitive in this type of class.

    hit->u = surface_intersection.u;
    hit->v = surface_intersection.v;
    hit->Ng_x = surface_intersection.normal.x;
    hit->Ng_y = surface_intersection.normal.y;
    hit->Ng_z = surface_intersection.normal.z;

    // update the far end of the ray
    ray->tfar = surface_intersection.t;
}

template<class T>
void EmbreeShape<T>::rtc_occluded(const RTCOccludedFunctionNArguments* args) {
    const auto valid = args->valid;
    if (!valid[0])
        return;

    auto ray = (RTCRay*)args->ray;
    auto shape = (const T*)args->geometryUserPtr;

    // construct a sort ray
    Ray sort_ray;
    SORTRayFromEmbree(*ray, sort_ray);

    // check for intersection
    const auto occuluded = shape->GetIntersect(sort_ray);
    if (occuluded)
        ray->tfar = -FLT_MAX;
}

#define EMBREE_SHAPE_INSTANTIATION(T)       template class EmbreeShape<T>;

EMBREE_SHAPE_INSTANTIATION(Quad);
EMBREE_SHAPE_INSTANTIATION(Disk);
EMBREE_SHAPE_INSTANTIATION(Line);
EMBREE_SHAPE_INSTANTIATION(Sphere);

#endif