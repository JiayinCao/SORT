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

#include <numeric>
#include <string.h>
#include "embree.h"
#include "entity/visual.h"
#include "shape/triangle.h"
#include "core/primitive.h"
#include "math/interaction.h"
#include "core/log.h"
#include "core/sassert.h"
#include "core/profile.h"
#include "scatteringevent/scatteringevent.h"
#include "core/memory.h"
#include "core/scene.h"

#ifdef INTEL_EMBREE_ENABLED

SORT_STATS_COUNTER("Spatial-Structure(Embree)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(Embree)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(Embree)", "Intersection Test", sIntersectionTest);
SORT_STATS_AVG_COUNT("Spatial-Structure(Embree)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

Embree::Embree() {
    m_rtc_device = rtcNewDevice(nullptr);
}

Embree::~Embree() {
    m_geometries.clear();

    rtcReleaseDevice(m_rtc_device);
}

void Embree::Build(const Scene& scene){
    SORT_PROFILE("Build BVH using Embree");

    // create a new scene
    m_rtc_scene = rtcNewScene(m_rtc_device);

    // if there is no triangle, just bail
    const auto prim_cnt = scene.GetPrimitiveCount();
    if (!prim_cnt)
        return;

    SceneVisualIterator iter(scene);
    while(auto visual = iter.Next())
        visual->BuildEmbreeGeometry(m_rtc_device, *this);

    // we are done here, build the spatial structure now.
    rtcCommitScene(m_rtc_scene);
}

bool Embree::GetIntersect( RenderContext& rc, const Ray& r , SurfaceInteraction& intersect ) const{
    SORT_PROFILE("Traverse Embree bvh");
    SORT_STATS(++sRayCount);

#ifdef ENABLE_TRANSPARENT_SHADOW
    SORT_STATS(sShadowRayCount += intersect.query_shadow);
#endif

    RTCIntersectContext context;
    rtcInitIntersectContext(&context);

#ifndef ENABLE_TRANSPARENT_SHADOW
    // Only do this if transparent shadow is not enabled
    if (intersect.query_shadow) {
        RTCRay rtc_ray;
        EmbreeRayFromSORT(r, rtc_ray);
        rtcOccluded1(m_rtc_scene, &context, &rtc_ray);

        return rtc_ray.tfar < 0.0f;
    }
#endif

    RTCRayHit ray_hit;
    memset(&ray_hit, 0, sizeof(ray_hit));
    EmbreeRayFromSORT(r, ray_hit.ray);
    ray_hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    ray_hit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
    ray_hit.hit.primID = RTC_INVALID_GEOMETRY_ID;

    /* intersect ray with scene */
    rtcIntersect1(m_rtc_scene, &context, &ray_hit);

    // If there is not a hit, bail.
    if (ray_hit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
        return false;

    // make sure we have a valid hit, we should not hit this assert wrong.
    sAssert(ray_hit.ray.tnear <= ray_hit.ray.tfar, SPATIAL_ACCELERATOR);

    // get the geometry of the hit
    sAssert(ray_hit.hit.geomID < m_geometries.size(), SPATIAL_ACCELERATOR);
    const auto& geom = m_geometries[ray_hit.hit.geomID];

    // get the corresponding primitive
    sAssert(ray_hit.hit.primID < geom->m_primitives.size(), SPATIAL_ACCELERATOR);
    const auto& prim = geom->m_primitives[ray_hit.hit.primID];

    // convert the intersection
    prim->ConvertIntersection(ray_hit, intersect);

    // we have a valid hit now
    return true;
}

#ifndef ENABLE_TRANSPARENT_SHADOW
bool Embree::IsOccluded( const Ray& r ) const{
    RTCRay rtc_ray;
    EmbreeRayFromSORT(r, ray);
    rtcOccluded1(m_rtc_scene, &context, &rtc_ray);

    return rtc_ray.tfar < 0.0f;
}
#endif

std::unique_ptr<Accelerator> Embree::Clone() const {
    return std::make_unique<Embree>();
}

unsigned int Embree::PushGeometry(std::unique_ptr<EmbreeGeometry> geometry) {
    // the geometry id
    const auto geom_id = (unsigned int)m_geometries.size();

    // attach it to the scene
    rtcAttachGeometryByID(m_rtc_scene, geometry->m_geometry, geom_id);

    // keep track of geometry in a list for querying usage
    m_geometries.push_back(std::move(geometry));

    return geom_id;
}

#endif