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
#include "simd_utils.h"

#ifdef SSE_ENABLED

//! @brief  Triangle4 is more of a simplified resolved data structure holds only bare bone information of triangle.
/**
 * Triangle4 is used in QBVH to accelerate ray triangle intersection using SSE. Its sole purpose is to accelerate 
 * ray triangle intersection by using SSE. Meaning there is no need to provide sophisticated interface of the class.
 * And since it is quite performance sensitive code, everything is inlined and there is no polymorphisms to keep it
 * as simple as possible. However, since there will be extra data kept in the system, it will also insignificantly 
 * incur more cost in term of memory usage.
 */
struct Triangle4{
    simd_data  m_p0_x , m_p0_y , m_p0_z ;  /**< Position of point 0 of the triangle. */
    simd_data  m_p1_x , m_p1_y , m_p1_z ;  /**< Position of point 1 of the triangle. */
    simd_data  m_p2_x , m_p2_y , m_p2_z ;  /**< Position of point 2 of the triangle. */
    simd_data  m_mask;

    /**< Pointers to original primitives. */
    const Triangle*  m_ori_tri[4] = { nullptr };
	const Primitive* m_ori_pri[4] = { nullptr };

    //! @brief  Push a triangle in the data structure.
    //!
	//! @param	pri		The original primitive.
    //! @return         Whether the data structure is full.
    bool PushTriangle( const Primitive* primitive ){
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
    }

    //! @brief  Pack triangle information into SSE compatible data.
	//!
	//! @return		Whether there is valid triangle inside.
    bool PackData(){
		if( !m_ori_pri[0] )
			return false;

        float	mask[4] = { 1.0f , 1.0f , 1.0f , 1.0f };
        float   p0_x[4] , p0_y[4] , p0_z[4] , p1_x[4] , p1_y[4] , p1_z[4] , p2_x[4] , p2_y[4] , p2_z[4];
        for( auto i = 0 ; i < 4 && m_ori_pri[i] ; ++i ){
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
        m_ori_pri[0] = m_ori_pri[1] = m_ori_pri[2] = m_ori_pri[3] = nullptr;
		m_ori_tri[0] = m_ori_tri[1] = m_ori_tri[2] = m_ori_tri[3] = nullptr;
    }
};

//! @brief	Core algorithm of ray triangle intersection.
//!
//! @param	ray			The ray to be tested.
//! @param	tri4		4 Triangles to be tested.
//! @param	quick_quit	Whether to quit as long as an intersection is found, this is usually true for shadow ray.
//! @param	maxt		The maximum distance we are interested. Any intersection behind it is ignored.
//! @param	t4			Output, the distances from ray origin to triangles. It will be FLT_MAX if there is no intersection.
//! @param	u4			Blending factor.
//! @param	v4			Blending factor.
SORT_FORCEINLINE bool intersectTriangle4Inner(const Ray& ray, const Triangle4& tri4, const bool quick_quit, const float maxt, simd_data& t4, simd_data& u4, simd_data& v4, simd_data& mask) {
	mask = tri4.m_mask;

	// step 0 : translate the vertices to ray coordinate system
	simd_data p0[3], p1[3], p2[3];
	p0[0] = simd_sub_ps(tri4.m_p0_x, ray.m_ori_x);
	p0[1] = simd_sub_ps(tri4.m_p0_y, ray.m_ori_y);
	p0[2] = simd_sub_ps(tri4.m_p0_z, ray.m_ori_z);

	p1[0] = simd_sub_ps(tri4.m_p1_x, ray.m_ori_x);
	p1[1] = simd_sub_ps(tri4.m_p1_y, ray.m_ori_y);
	p1[2] = simd_sub_ps(tri4.m_p1_z, ray.m_ori_z);

	p2[0] = simd_sub_ps(tri4.m_p2_x, ray.m_ori_x);
	p2[1] = simd_sub_ps(tri4.m_p2_y, ray.m_ori_y);
	p2[2] = simd_sub_ps(tri4.m_p2_z, ray.m_ori_z);

	// step 1 : pick the major axis to avoid dividing by zero in the sheering pass.
	//          by picking the major axis, we can also make sure we sheer as little as possible
	simd_data p0_x = p0[ray.m_local_x];
	simd_data p0_y = p0[ray.m_local_y];
	simd_data p0_z = p0[ray.m_local_z];

	simd_data p1_x = p1[ray.m_local_x];
	simd_data p1_y = p1[ray.m_local_y];
	simd_data p1_z = p1[ray.m_local_z];

	simd_data p2_x = p2[ray.m_local_x];
	simd_data p2_y = p2[ray.m_local_y];
	simd_data p2_z = p2[ray.m_local_z];

	// step 2 : sheer the vertices so that the ray direction points to ( 0 , 1 , 0 )
	p0_x = simd_mad_ps(p0_y, ray.m_sse_scale_x, p0_x);
	p0_z = simd_mad_ps(p0_y, ray.m_sse_scale_z, p0_z);
	p1_x = simd_mad_ps(p1_y, ray.m_sse_scale_x, p1_x);
	p1_z = simd_mad_ps(p1_y, ray.m_sse_scale_z, p1_z);
	p2_x = simd_mad_ps(p2_y, ray.m_sse_scale_x, p2_x);
	p2_z = simd_mad_ps(p2_y, ray.m_sse_scale_z, p2_z);

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

	p0_y = simd_mul_ps(p0_y, ray.m_sse_scale_y);
	p1_y = simd_mul_ps(p1_y, ray.m_sse_scale_y);
	p2_y = simd_mul_ps(p2_y, ray.m_sse_scale_y);

	t4 = simd_mul_ps(e0, p0_y);
	t4 = simd_mad_ps(e1, p1_y, t4);
	t4 = simd_mad_ps(e2, p2_y, t4);
	t4 = simd_mul_ps(t4, rcp_det);

	const simd_data ray_min_t = simd_set_ps1(ray.m_fMin);
	const simd_data ray_max_t = simd_set_ps1(ray.m_fMax);
	mask = simd_and_ps(simd_and_ps(mask, simd_cmpgt_ps(t4, ray_min_t)), simd_cmple_ps(t4, ray_max_t));
	c = simd_movemask_ps(mask);
	if (0 == c)
		return false;

	if (quick_quit)
		return true;

	mask = simd_and_ps(simd_and_ps(mask, simd_cmpgt_ps(t4, simd_zeros)), simd_cmplt_ps(t4, simd_set_ps1(maxt)));
	c = simd_movemask_ps(mask);
	if (0 == c)
		return false;

	// mask out the invalid values
	t4 = simd_pick_ps( mask, t4, simd_infinites);

	u4 = simd_mul_ps(e1, rcp_det);
	v4 = simd_mul_ps(e2, rcp_det);

	return true;
}

//! @brief	A helper function setup the result of intersection.
//!
//! @param	tri4          The triangle4 data structure that has four triangles.
//! @param  ray           Ray that we used to tested.
//! @param	t4	          Output, the distances from ray origin to triangles. It will be FLT_MAX if there is no intersection.
//! @param	u4		      Blending factor.
//! @param	v4		      Blending factor.
//! @param  id            Index of the intersection of our interest.
//! @param  intersection  The pointer to the result to be filled. It can't be nullptr.
SORT_FORCEINLINE void setupIntersection(const Triangle4& tri4, const Ray& ray, const simd_data& t4, const simd_data& u4, const simd_data& v4, const int id, Intersection* intersection) {
	const auto* triangle = tri4.m_ori_tri[id];

	const auto u = sse_data(u4, id);
	const auto v = sse_data(v4, id);
	const auto w = 1 - u - v;

	const auto& mem = triangle->m_meshVisual->m_memory;
	const auto id0 = triangle->m_index.m_id[0];
	const auto id1 = triangle->m_index.m_id[1];
	const auto id2 = triangle->m_index.m_id[2];

	const auto& mv0 = mem->m_vertices[id0];
	const auto& mv1 = mem->m_vertices[id1];
	const auto& mv2 = mem->m_vertices[id2];

	const auto res_t = sse_data(t4, id);
	intersection->intersect = ray(sse_data(t4, id));
	intersection->t = res_t;

	intersection->gnormal = Normalize(Cross((mv2.m_position - mv0.m_position), (mv1.m_position - mv0.m_position)));
	intersection->normal = (w * mv0.m_normal + u * mv1.m_normal + v * mv2.m_normal).Normalize();
	intersection->tangent = (w * mv0.m_tangent + u * mv1.m_tangent + v * mv2.m_tangent).Normalize();
	intersection->view = -ray.m_Dir;

	const auto uv = w * mv0.m_texCoord + u * mv1.m_texCoord + v * mv2.m_texCoord;
	intersection->u = uv.x;
	intersection->v = uv.y;

	intersection->primitive = tri4.m_ori_pri[id];
}

//! @brief  With the power of SSE, this utility function helps intersect a ray with four triangles at the cost of one.
//!
//! @param  ray     Ray to be tested against.
//! @param  tri4    Data structure holds four triangles.
//! @param  ret     The result of intersection. It can't be nullptr.
//! @return         Whether there is any intersection that is valid.
SORT_FORCEINLINE bool intersectTriangle4( const Ray& ray , const Triangle4& tri4 , Intersection* ret ){
	sAssert( nullptr != ret , SPATIAL_ACCELERATOR );

	simd_data	u4, v4, t4, mask;
	const auto maxt = ret->t;
	const auto intersected = intersectTriangle4Inner(ray, tri4, false, maxt, t4, u4, v4, mask);
	if (!intersected)
		return false;

    // find the closest result
	simd_data t0 = simd_minreduction_ps( t4 );

    // get the index of the closest one
    const auto resolved_mask = simd_movemask_ps( simd_cmpeq_ps( t4 , t0 ) );
	const auto res_i = __bsf(resolved_mask);

    sAssert( resolved_mask > 0 && resolved_mask < 16 , SPATIAL_ACCELERATOR );
    sAssert( res_i >= 0 && res_i < 4 , SPATIAL_ACCELERATOR );
    
	setupIntersection(tri4, ray, t4, u4, v4, res_i, ret);

    return true;
}

//! @brief  With the power of SSE, this utility function helps intersect a ray with four triangles at the cost of one.
//!
//! This function stops as long as there is an intersection, it is for shadow ray occlusion detection.
//!
//! @param  ray     Ray to be tested against.
//! @param  tri4    Data structure holds four triangles.
//! @return         Whether there is any intersection that is valid.
SORT_FORCEINLINE bool intersectTriangle4Fast(const Ray& ray, const Triangle4& tri4) {
	// please optimize these value, compiler.
	simd_data	dummy_u4, dummy_v4, dummy_t4, mask;
	return intersectTriangle4Inner(ray, tri4, true, FLT_MAX, dummy_t4, dummy_u4, dummy_v4, mask);
}

//! @brief  Unlike the above function, this helper function will populate all results in the BSSRDFIntersection data structure.
//!         It is for BSSRDF intersection tests.
//!
//! @param  ray     Ray to be tested against.
//! @param  tri4    Data structure holds four triangles.
//! @param  ret     The result of intersection.
SORT_FORCEINLINE void intersectTriangle4Multi(const Ray& ray, const Triangle4& tri4, const StringID matID , BSSRDFIntersections& intersections) {
	simd_data	u4, v4, t4, mask;
	const auto maxt = intersections.maxt;
	const auto intersected = intersectTriangle4Inner(ray, tri4, false, maxt, t4, u4, v4, mask);
	if (!intersected)
		return;

	auto resolved_mask = simd_movemask_ps(mask);
	sAssert(resolved_mask > 0 && resolved_mask < 16, SPATIAL_ACCELERATOR);

	// A better approach would be to sort the intersection before populating it into the results to avoid some unnecessary setup.
	// However, the overhead to sort the result may out-weight the gain we expect.
	while (resolved_mask) {
		const auto res_i = __bsf(resolved_mask);
		resolved_mask = resolved_mask & (resolved_mask - 1);

		const auto primitive = tri4.m_ori_pri[res_i];
		if (matID != primitive->GetMaterial()->GetID())
			continue;

		if (intersections.cnt < TOTAL_SSS_INTERSECTION_CNT) {
			intersections.intersections[intersections.cnt] = SORT_MALLOC(BSSRDFIntersection)();
			setupIntersection(tri4, ray, t4, u4, v4, res_i, &intersections.intersections[intersections.cnt++]->intersection);
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
				setupIntersection(tri4, ray, t4, u4, v4, res_i, &intersections.intersections[picked_i]->intersection);

			intersections.ResolveMaxDepth();
		}
	}
}

#endif