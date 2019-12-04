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

#include "core/define.h"
#include "core/memory.h"
#include "math/ray.h"
#include "math/intersection.h"
#include "math/point.h"
#include "scatteringevent/bssrdf/bssrdf.h"
#include "core/primitive.h"
#include "shape/triangle.h"
#include "entity/visual.h"

#if defined(SIMD_SSE_IMPLEMENTATION) && defined(SIMD_AVX_IMPLEMENTATION)
    static_assert( false , "More than one SIMD version is defined before including simd_triangle." );
#endif

#ifdef SIMD_BVH_IMPLEMENTATION

#ifdef SIMD_SSE_IMPLEMENTATION
    #define Simd_Triangle       Triangle4
#endif

#ifdef SIMD_AVX_IMPLEMENTATION
    #define Simd_Triangle       Triangle8
#endif

//! @brief  Simd_Triangle is more of a simplified resolved data structure holds only bare bone information of triangle.
/**
 * Simd_Triangle is used in OBVH/QBVH to accelerate ray triangle intersection using AVX/SSE. Its sole purpose is to accelerate 
 * ray triangle intersection by using AVX/SSE. Meaning there is no need to provide sophisticated interface of the class.
 * And since it is quite performance sensitive code, everything is inlined and there is no polymorphisms to keep it
 * as simple as possible. However, since there will be extra data kept in the system, it will also insignificantly 
 * incur more cost in term of memory usage.
 */
struct alignas(SIMD_ALIGNMENT) Simd_Triangle{
    simd_data  m_p0_x , m_p0_y , m_p0_z ;  /**< Position of point 0 of the triangle. */
    simd_data  m_p1_x , m_p1_y , m_p1_z ;  /**< Position of point 1 of the triangle. */
    simd_data  m_p2_x , m_p2_y , m_p2_z ;  /**< Position of point 2 of the triangle. */
    simd_data  m_mask;

    /**< Pointers to original primitives. */
    const Triangle*  m_ori_tri[SIMD_CHANNEL] = { nullptr };
    const Primitive* m_ori_pri[SIMD_CHANNEL] = { nullptr };

#ifdef SIMD_AVX_IMPLEMENTATION
    char padding[16];
#endif

    //! @brief  Push a triangle in the data structure.
    //!
    //! @param  pri     The original primitive.
    //! @return         Whether the data structure is full.
    bool PushTriangle( const Primitive* primitive ){
#ifdef SIMD_SSE_IMPLEMENTATION
        const Triangle* triangle = dynamic_cast<const Triangle*>(primitive->GetShape());
        if( m_ori_pri[0] == nullptr ){
            m_ori_pri[0] = primitive;
            m_ori_tri[0] = triangle;
            return false;
        }else if( m_ori_pri[1] == nullptr ){
            m_ori_pri[1] = primitive;
            m_ori_tri[1] = triangle;
            return false;
        }else if( m_ori_pri[2] == nullptr ){
            m_ori_pri[2] = primitive;
            m_ori_tri[2] = triangle;
            return false;
        }
        m_ori_pri[3] = primitive;
        m_ori_tri[3] = triangle;
        return true;
#endif

#ifdef SIMD_AVX_IMPLEMENTATION
        const Triangle* triangle = dynamic_cast<const Triangle*>(primitive->GetShape());
        if( m_ori_pri[0] == nullptr ){
            m_ori_pri[0] = primitive;
            m_ori_tri[0] = triangle;
            return false;
        }else if( m_ori_pri[1] == nullptr ){
            m_ori_pri[1] = primitive;
            m_ori_tri[1] = triangle;
            return false;
        }else if( m_ori_pri[2] == nullptr ){
            m_ori_pri[2] = primitive;
            m_ori_tri[2] = triangle;
            return false;
        }else if( m_ori_pri[3] == nullptr ){
            m_ori_pri[3] = primitive;
            m_ori_tri[3] = triangle;
            return false;
        }else if( m_ori_pri[4] == nullptr ){
            m_ori_pri[4] = primitive;
            m_ori_tri[4] = triangle;
            return false;
        }else if( m_ori_pri[5] == nullptr ){
            m_ori_pri[5] = primitive;
            m_ori_tri[5] = triangle;
            return false;
        }else if( m_ori_pri[6] == nullptr ){
            m_ori_pri[6] = primitive;
            m_ori_tri[6] = triangle;
            return false;
        }
        m_ori_pri[7] = primitive;
        m_ori_tri[7] = triangle;
        return true;
#endif
    }

    //! @brief  Pack triangle information into SSE/AVX compatible data.
    //!
    //! @return     Whether there is valid triangle inside.
    bool PackData(){
        if( !m_ori_pri[0] )
            return false;

#ifdef SIMD_SSE_IMPLEMENTATION
        float   mask[SIMD_CHANNEL] = { 1.0f , 1.0f , 1.0f , 1.0f };
#endif
#ifdef SIMD_AVX_IMPLEMENTATION
        float   mask[SIMD_CHANNEL] = { 1.0f , 1.0f , 1.0f , 1.0f , 1.0f , 1.0f , 1.0f , 1.0f };
#endif

        float   p0_x[SIMD_CHANNEL] , p0_y[SIMD_CHANNEL] , p0_z[SIMD_CHANNEL] , p1_x[SIMD_CHANNEL] , p1_y[SIMD_CHANNEL] , p1_z[SIMD_CHANNEL] , p2_x[SIMD_CHANNEL] , p2_y[SIMD_CHANNEL] , p2_z[SIMD_CHANNEL];
        for( auto i = 0 ; i < SIMD_CHANNEL && m_ori_pri[i] ; ++i ){
            const auto triangle = m_ori_tri[i];

            const auto& mem = triangle->m_meshVisual->m_memory;
            const auto id0 = triangle->m_index.m_id[0];
            const auto id1 = triangle->m_index.m_id[1];
            const auto id2 = triangle->m_index.m_id[2];

            const auto& mv0 = mem->m_vertices[id0];
            const auto& mv1 = mem->m_vertices[id1];
            const auto& mv2 = mem->m_vertices[id2];

            p0_x[i] = mv0.m_position.x;
            p0_y[i] = mv0.m_position.y;
            p0_z[i] = mv0.m_position.z;

            p1_x[i] = mv1.m_position.x;
            p1_y[i] = mv1.m_position.y;
            p1_z[i] = mv1.m_position.z;

            p2_x[i] = mv2.m_position.x;
            p2_y[i] = mv2.m_position.y;
            p2_z[i] = mv2.m_position.z;

            mask[i] = 0.0f;
        }

        m_p0_x = simd_set_ps( p0_x );
        m_p0_y = simd_set_ps( p0_y );
        m_p0_z = simd_set_ps( p0_z );
        m_p1_x = simd_set_ps( p1_x );
        m_p1_y = simd_set_ps( p1_y );
        m_p1_z = simd_set_ps( p1_z );
        m_p2_x = simd_set_ps( p2_x );
        m_p2_y = simd_set_ps( p2_y );
        m_p2_z = simd_set_ps( p2_z );
        m_mask = simd_cmpeq_ps( simd_zeros , simd_set_ps( mask ) );

        return true;
    }

    //! @brief  Reset the data for reuse
    void Reset(){
#ifdef SIMD_SSE_IMPLEMENTATION
        m_ori_pri[0] = m_ori_pri[1] = m_ori_pri[2] = m_ori_pri[3] = nullptr;
        m_ori_tri[0] = m_ori_tri[1] = m_ori_tri[2] = m_ori_tri[3] = nullptr;
#endif

#ifdef SIMD_AVX_IMPLEMENTATION
        m_ori_pri[0] = m_ori_pri[1] = m_ori_pri[2] = m_ori_pri[3] = m_ori_pri[4] = m_ori_pri[5] = m_ori_pri[6] = m_ori_pri[7] = nullptr;
        m_ori_tri[0] = m_ori_tri[1] = m_ori_tri[2] = m_ori_tri[3] = m_ori_tri[4] = m_ori_tri[5] = m_ori_tri[6] = m_ori_tri[7] = nullptr;
#endif
    }
};

static_assert( sizeof( Simd_Triangle ) % SIMD_ALIGNMENT == 0 , "Incorrect size of Triangle8." );

//! @brief  Core algorithm of ray triangle intersection.
//!
//! @param  ray         The ray to be tested.
//! @param  tri_simd    4/8 Triangles to be tested.
//! @param  quick_quit  Whether to quit as long as an intersection is found, this is usually true for shadow ray.
//! @param  maxt        The maximum distance we are interested. Any intersection behind it is ignored.
//! @param  t_simd      Output, the distances from ray origin to triangles. It will be FLT_MAX if there is no intersection.
//! @param  u_simd      Blending factor.
//! @param  v_simd      Blending factor.
SORT_FORCEINLINE bool intersectTriangleInner_SIMD(const Ray& ray, const Simd_Ray_Data& ray_simd, const Simd_Triangle& tri_simd, const bool quick_quit, const float maxt, simd_data& t_simd, simd_data& u_simd, simd_data& v_simd, simd_data& mask) {
    mask = tri_simd.m_mask;

    // step 0 : translate the vertices to ray coordinate system
    simd_data p0[3], p1[3], p2[3];
    p0[0] = simd_sub_ps(tri_simd.m_p0_x, ray_ori_x(ray_simd));
    p0[1] = simd_sub_ps(tri_simd.m_p0_y, ray_ori_y(ray_simd));
    p0[2] = simd_sub_ps(tri_simd.m_p0_z, ray_ori_z(ray_simd));

    p1[0] = simd_sub_ps(tri_simd.m_p1_x, ray_ori_x(ray_simd));
    p1[1] = simd_sub_ps(tri_simd.m_p1_y, ray_ori_y(ray_simd));
    p1[2] = simd_sub_ps(tri_simd.m_p1_z, ray_ori_z(ray_simd));

    p2[0] = simd_sub_ps(tri_simd.m_p2_x, ray_ori_x(ray_simd));
    p2[1] = simd_sub_ps(tri_simd.m_p2_y, ray_ori_y(ray_simd));
    p2[2] = simd_sub_ps(tri_simd.m_p2_z, ray_ori_z(ray_simd));

    // step 1 : pick the major axis to avoid dividing by zero in the sheering pass.
    //          by picking the major axis, we can also make sure we sheer as little as possible
    simd_data& p0_x = p0[ray.m_local_x];
    simd_data& p0_y = p0[ray.m_local_y];
    simd_data& p0_z = p0[ray.m_local_z];

    simd_data& p1_x = p1[ray.m_local_x];
    simd_data& p1_y = p1[ray.m_local_y];
    simd_data& p1_z = p1[ray.m_local_z];

    simd_data& p2_x = p2[ray.m_local_x];
    simd_data& p2_y = p2[ray.m_local_y];
    simd_data& p2_z = p2[ray.m_local_z];

    // step 2 : sheer the vertices so that the ray direction points to ( 0 , 1 , 0 )
    p0_x = simd_mad_ps(p0_y, ray_scale_x(ray_simd), p0_x);
    p0_z = simd_mad_ps(p0_y, ray_scale_z(ray_simd), p0_z);
    p1_x = simd_mad_ps(p1_y, ray_scale_x(ray_simd), p1_x);
    p1_z = simd_mad_ps(p1_y, ray_scale_z(ray_simd), p1_z);
    p2_x = simd_mad_ps(p2_y, ray_scale_x(ray_simd), p2_x);
    p2_z = simd_mad_ps(p2_y, ray_scale_z(ray_simd), p2_z);

    // compute the edge functions
    const simd_data e0 = simd_sub_ps(simd_mul_ps(p1_x, p2_z), simd_mul_ps(p1_z, p2_x));
    const simd_data e1 = simd_sub_ps(simd_mul_ps(p2_x, p0_z), simd_mul_ps(p2_z, p0_x));
    const simd_data e2 = simd_sub_ps(simd_mul_ps(p0_x, p1_z), simd_mul_ps(p0_z, p1_x));

    const simd_data c0 = simd_and_ps(simd_and_ps(simd_cmpge_ps(e0, simd_zeros), simd_cmpge_ps(e1, simd_zeros)), simd_cmpge_ps(e2, simd_zeros));
    const simd_data c1 = simd_and_ps(simd_and_ps(simd_cmple_ps(e0, simd_zeros), simd_cmple_ps(e1, simd_zeros)), simd_cmple_ps(e2, simd_zeros));
    mask = simd_and_ps(mask, simd_or_ps(c0, c1));
    auto c = simd_movemask_ps(mask);
    if (0 == c)
        return false;

    const simd_data det = simd_add_ps(e0, simd_add_ps(e1, e2));
    mask = simd_and_ps(mask, simd_cmpneq_ps(det, simd_zeros));
    c = simd_movemask_ps(mask);
    if (0 == c)
        return false;

    const simd_data rcp_det = simd_rcp_ps(det);

    p0_y = simd_mul_ps(p0_y, ray_scale_y(ray_simd));
    p1_y = simd_mul_ps(p1_y, ray_scale_y(ray_simd));
    p2_y = simd_mul_ps(p2_y, ray_scale_y(ray_simd));

    t_simd = simd_mul_ps(e0, p0_y);
    t_simd = simd_mad_ps(e1, p1_y, t_simd);
    t_simd = simd_mad_ps(e2, p2_y, t_simd);
    t_simd = simd_mul_ps(t_simd, rcp_det);

    const simd_data ray_min_t = simd_set_ps1(ray.m_fMin);
    const simd_data ray_max_t = simd_set_ps1(ray.m_fMax);
    mask = simd_and_ps(simd_and_ps(mask, simd_cmpgt_ps(t_simd, ray_min_t)), simd_cmple_ps(t_simd, ray_max_t));
    c = simd_movemask_ps(mask);
    if (0 == c)
        return false;

    if (quick_quit)
        return true;

    mask = simd_and_ps(simd_and_ps(mask, simd_cmpgt_ps(t_simd, simd_zeros)), simd_cmplt_ps(t_simd, simd_set_ps1(maxt)));
    c = simd_movemask_ps(mask);
    if (0 == c)
        return false;

    // mask out the invalid values
    t_simd = simd_pick_ps( mask, t_simd, simd_infinites);

    u_simd = simd_mul_ps(e1, rcp_det);
    v_simd = simd_mul_ps(e2, rcp_det);

    return true;
}

//! @brief  A helper function setup the result of intersection.
//!
//! @param  tri_simd      The triangle data structure that has 4/8 triangles.
//! @param  ray           Ray that we used to tested.
//! @param  t_simd        Output, the distances from ray origin to triangles. It will be FLT_MAX if there is no intersection.
//! @param  u_simd        Blending factor.
//! @param  v_simd        Blending factor.
//! @param  id            Index of the intersection of our interest.
//! @param  intersection  The pointer to the result to be filled. It can't be nullptr.
SORT_FORCEINLINE void setupIntersection(const Simd_Triangle& tri_simd, const Ray& ray, const simd_data& t_simd, const simd_data& u_simd, const simd_data& v_simd, const int id, Intersection* intersection) {
    const auto* triangle = tri_simd.m_ori_tri[id];

    const auto u = u_simd[id];
    const auto v = v_simd[id];
    const auto w = 1 - u - v;

    const auto& mem = triangle->m_meshVisual->m_memory;
    const auto id0 = triangle->m_index.m_id[0];
    const auto id1 = triangle->m_index.m_id[1];
    const auto id2 = triangle->m_index.m_id[2];

    const auto& mv0 = mem->m_vertices[id0];
    const auto& mv1 = mem->m_vertices[id1];
    const auto& mv2 = mem->m_vertices[id2];

    const auto res_t = t_simd[id];
    intersection->intersect = ray(res_t);
    intersection->t = res_t;

    intersection->gnormal = Normalize(Cross((mv2.m_position - mv0.m_position), (mv1.m_position - mv0.m_position)));
    intersection->normal = (w * mv0.m_normal + u * mv1.m_normal + v * mv2.m_normal).Normalize();
    intersection->tangent = (w * mv0.m_tangent + u * mv1.m_tangent + v * mv2.m_tangent).Normalize();
    intersection->view = -ray.m_Dir;

    const auto uv = w * mv0.m_texCoord + u * mv1.m_texCoord + v * mv2.m_texCoord;
    intersection->u = uv.x;
    intersection->v = uv.y;

    intersection->primitive = tri_simd.m_ori_pri[id];
}

//! @brief  With the power of SSE/AVX, this utility function helps intersect a ray with four/eight triangles at the cost of one.
//!
//! @param  ray         Ray to be tested against.
//! @param  simd_ray    Resolved simd ray data.
//! @param  tri_simd    Data structure holds four/eight triangles.
//! @param  ret         The result of intersection. It can't be nullptr.
//! @return             Whether there is any intersection that is valid.
SORT_FORCEINLINE bool intersectTriangle_SIMD( const Ray& ray , const Simd_Ray_Data& ray_simd , const Simd_Triangle& tri_simd , Intersection* ret ){
    sAssert( nullptr != ret , SPATIAL_ACCELERATOR );

    simd_data   u_simd, v_simd, t_simd, mask;
    const auto maxt = ret->t;
    const auto intersected = intersectTriangleInner_SIMD(ray, ray_simd , tri_simd, false, maxt, t_simd, u_simd, v_simd, mask);
    if (!intersected)
        return false;

    // find the closest result
    simd_data t0 = simd_minreduction_ps( t_simd );

    // get the index of the closest one
    const auto resolved_mask = simd_movemask_ps( simd_cmpeq_ps( t_simd , t0 ) );
    const auto res_i = __bsf(resolved_mask);

    sAssert( resolved_mask > 0 && resolved_mask < pow(2,SIMD_CHANNEL) , SPATIAL_ACCELERATOR );
    sAssert( res_i >= 0 && res_i < SIMD_CHANNEL , SPATIAL_ACCELERATOR );
    
    setupIntersection(tri_simd, ray, t_simd, u_simd, v_simd, res_i, ret);

    return true;
}

//! @brief  With the power of SSE/AVX, this utility function helps intersect a ray with four triangles at the cost of one.
//!
//! This function stops as long as there is an intersection, it is for shadow ray occlusion detection.
//!
//! @param  ray         Ray to be tested against.
//! @param  simd_ray    Resolved simd ray data.
//! @param  tri_simd    Data structure holds four/eight triangles.
//! @return             Whether there is any intersection that is valid.
SORT_FORCEINLINE bool intersectTriangleFast_SIMD(const Ray& ray, const Simd_Ray_Data& ray_simd , const Simd_Triangle& tri_simd) {
    // please optimize these value, compiler.
    simd_data   dummy_u, dummy_v, dummy_t, mask;
    return intersectTriangleInner_SIMD(ray, ray_simd, tri_simd, true, FLT_MAX, dummy_t, dummy_u, dummy_v, mask);
}

//! @brief  Unlike the above function, this helper function will populate all results in the BSSRDFIntersection data structure.
//!         It is for BSSRDF intersection tests.
//!
//! @param  ray         Ray to be tested against.
//! @param  tri_simd    Data structure holds four/eight triangles.
//! @param  ret         The result of intersection.
SORT_FORCEINLINE void intersectTriangleMulti_SIMD(const Ray& ray, const Simd_Ray_Data& ray_simd, const Simd_Triangle& tri_simd, const StringID matID , BSSRDFIntersections& intersections) {
    simd_data   u_simd, v_simd, t_simd, mask;
    const auto maxt = intersections.maxt;
    const auto intersected = intersectTriangleInner_SIMD(ray, ray_simd, tri_simd, false, maxt, t_simd, u_simd, v_simd, mask);
    if (!intersected)
        return;

    auto resolved_mask = simd_movemask_ps(mask);
    sAssert(resolved_mask > 0 && resolved_mask < pow(2,SIMD_CHANNEL), SPATIAL_ACCELERATOR);

    // A better approach would be to sort the intersection before populating it into the results to avoid some unnecessary setup.
    // However, the overhead to sort the result may out-weight the gain we expect.
    while (resolved_mask) {
        const auto res_i = __bsf(resolved_mask);
        resolved_mask = resolved_mask & (resolved_mask - 1);

        const auto primitive = tri_simd.m_ori_pri[res_i];
        if (matID != primitive->GetMaterial()->GetID())
            continue;

        if (intersections.cnt < TOTAL_SSS_INTERSECTION_CNT) {
            intersections.intersections[intersections.cnt] = SORT_MALLOC(BSSRDFIntersection)();
            setupIntersection(tri_simd, ray, t_simd, u_simd, v_simd, res_i, &intersections.intersections[intersections.cnt++]->intersection);
        } else {
            auto picked_i = -1;
            auto t = 0.0f;
            for (auto i = 0; i < TOTAL_SSS_INTERSECTION_CNT; ++i) {
                if (t < intersections.intersections[i]->intersection.t) {
                    t = intersections.intersections[i]->intersection.t;
                    picked_i = i;
                }
            }
            if( picked_i >= 0 )
                setupIntersection(tri_simd, ray, t_simd, u_simd, v_simd, res_i, &intersections.intersections[picked_i]->intersection);

            intersections.ResolveMaxDepth();
        }
    }
}

#endif // SIMD_SSE_IMPLEMENTATION || SIMD_AVX_IMPLEMENTATION