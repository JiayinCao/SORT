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
#include <nmmintrin.h>

//! @brief  Triangle4 is more of a simplified resolved data structure holds only bare bone information of triangle.
/**
 * Triangle4 is used in QBVH to accelerate ray triangle intersection using SSE. Its sole purpose is to accelerate 
 * ray triangle intersection by using SSE. Meaning there is no need to provide sophisticated interface of the class.
 * And since it is quite performance sensitive code, everything is inlined and there is no polymorphisms to keep it
 * as simple as possible. However, since there will be extra data kept in the system, it will also insignificantly 
 * incur more cost in term of memory usage.
 */
struct Triangle4{
    __m128  m_p0_x , m_p0_y , m_p0_z ;  /**< Position of point 0 of the triangle. */
    __m128  m_p1_x , m_p1_y , m_p1_z ;  /**< Position of point 1 of the triangle. */
    __m128  m_p2_x , m_p2_y , m_p2_z ;  /**< Position of point 2 of the triangle. */
    __m128  m_mask;

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

        m_p0_x = _mm_set_ps( p0_x[3] , p0_x[2] , p0_x[1] , p0_x[0] );
        m_p0_y = _mm_set_ps( p0_y[3] , p0_y[2] , p0_y[1] , p0_y[0] );
        m_p0_z = _mm_set_ps( p0_z[3] , p0_z[2] , p0_z[1] , p0_z[0] );
        m_p1_x = _mm_set_ps( p1_x[3] , p1_x[2] , p1_x[1] , p1_x[0] );
        m_p1_y = _mm_set_ps( p1_y[3] , p1_y[2] , p1_y[1] , p1_y[0] );
        m_p1_z = _mm_set_ps( p1_z[3] , p1_z[2] , p1_z[1] , p1_z[0] );
        m_p2_x = _mm_set_ps( p2_x[3] , p2_x[2] , p2_x[1] , p2_x[0] );
        m_p2_y = _mm_set_ps( p2_y[3] , p2_y[2] , p2_y[1] , p2_y[0] );
        m_p2_z = _mm_set_ps( p2_z[3] , p2_z[2] , p2_z[1] , p2_z[0] );
        m_mask = _mm_cmpeq_ps( zeros , _mm_set_ps( mask[3] , mask[2] , mask[1] , mask[0] ) );

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
SORT_FORCEINLINE bool intersectTriangle4Inner(const Ray& ray, const Triangle4& tri4, const bool quick_quit, const float maxt, __m128& t4, __m128& u4, __m128& v4, __m128& mask) {
	mask = tri4.m_mask;

	// step 0 : translate the vertices to ray coordinate system
	__m128 p0[3], p1[3], p2[3];
	p0[0] = _mm_sub_ps(tri4.m_p0_x, ray.m_ori_x);
	p0[1] = _mm_sub_ps(tri4.m_p0_y, ray.m_ori_y);
	p0[2] = _mm_sub_ps(tri4.m_p0_z, ray.m_ori_z);

	p1[0] = _mm_sub_ps(tri4.m_p1_x, ray.m_ori_x);
	p1[1] = _mm_sub_ps(tri4.m_p1_y, ray.m_ori_y);
	p1[2] = _mm_sub_ps(tri4.m_p1_z, ray.m_ori_z);

	p2[0] = _mm_sub_ps(tri4.m_p2_x, ray.m_ori_x);
	p2[1] = _mm_sub_ps(tri4.m_p2_y, ray.m_ori_y);
	p2[2] = _mm_sub_ps(tri4.m_p2_z, ray.m_ori_z);

	// step 1 : pick the major axis to avoid dividing by zero in the sheering pass.
	//          by picking the major axis, we can also make sure we sheer as little as possible
	__m128 p0_x = p0[ray.m_local_x];
	__m128 p0_y = p0[ray.m_local_y];
	__m128 p0_z = p0[ray.m_local_z];

	__m128 p1_x = p1[ray.m_local_x];
	__m128 p1_y = p1[ray.m_local_y];
	__m128 p1_z = p1[ray.m_local_z];

	__m128 p2_x = p2[ray.m_local_x];
	__m128 p2_y = p2[ray.m_local_y];
	__m128 p2_z = p2[ray.m_local_z];

	// step 2 : sheer the vertices so that the ray direction points to ( 0 , 1 , 0 )
	p0_x = _mm_add_ps(p0_x, _mm_mul_ps(p0_y, ray.m_sse_scale_x));
	p0_z = _mm_add_ps(p0_z, _mm_mul_ps(p0_y, ray.m_sse_scale_z));
	p1_x = _mm_add_ps(p1_x, _mm_mul_ps(p1_y, ray.m_sse_scale_x));
	p1_z = _mm_add_ps(p1_z, _mm_mul_ps(p1_y, ray.m_sse_scale_z));
	p2_x = _mm_add_ps(p2_x, _mm_mul_ps(p2_y, ray.m_sse_scale_x));
	p2_z = _mm_add_ps(p2_z, _mm_mul_ps(p2_y, ray.m_sse_scale_z));

	// compute the edge functions
	const __m128 e0 = _mm_sub_ps(_mm_mul_ps(p1_x, p2_z), _mm_mul_ps(p1_z, p2_x));
	const __m128 e1 = _mm_sub_ps(_mm_mul_ps(p2_x, p0_z), _mm_mul_ps(p2_z, p0_x));
	const __m128 e2 = _mm_sub_ps(_mm_mul_ps(p0_x, p1_z), _mm_mul_ps(p0_z, p1_x));

	const __m128 c0 = _mm_and_ps(_mm_and_ps(_mm_cmpge_ps(e0, zeros), _mm_cmpge_ps(e1, zeros)), _mm_cmpge_ps(e2, zeros));
	const __m128 c1 = _mm_and_ps(_mm_and_ps(_mm_cmple_ps(e0, zeros), _mm_cmple_ps(e1, zeros)), _mm_cmple_ps(e2, zeros));
	mask = _mm_and_ps(mask, _mm_or_ps(c0, c1));
	auto c = _mm_movemask_ps(mask);
	if (0 == c)
		return false;

	const __m128 det = _mm_add_ps(e0, _mm_add_ps(e1, e2));
	mask = _mm_and_ps(mask, _mm_cmpneq_ps(det, zeros));
	c = _mm_movemask_ps(mask);
	if (0 == c)
		return false;

	const __m128 rcp_det = _mm_rcpa_ps(det);

	p0_y = _mm_mul_ps(p0_y, ray.m_sse_scale_y);
	p1_y = _mm_mul_ps(p1_y, ray.m_sse_scale_y);
	p2_y = _mm_mul_ps(p2_y, ray.m_sse_scale_y);

	t4 = _mm_mul_ps(e0, p0_y);
	t4 = _mm_add_ps(t4, _mm_mul_ps(e1, p1_y));
	t4 = _mm_add_ps(t4, _mm_mul_ps(e2, p2_y));
	t4 = _mm_mul_ps(t4, rcp_det);

	const __m128 ray_min_t = _mm_set_ps1(ray.m_fMin);
	const __m128 ray_max_t = _mm_set_ps1(ray.m_fMax);
	mask = _mm_and_ps(_mm_and_ps(mask, _mm_cmpgt_ps(t4, ray_min_t)), _mm_cmple_ps(t4, ray_max_t));
	c = _mm_movemask_ps(mask);
	if (0 == c)
		return false;

	if (quick_quit)
		return true;

	mask = _mm_and_ps(_mm_and_ps(mask, _mm_cmpgt_ps(t4, zeros)), _mm_cmplt_ps(t4, _mm_set_ps1(maxt)));
	c = _mm_movemask_ps(mask);
	if (0 == c)
		return false;

	// mask out the invalid values
	t4 = _mm_or_ps(_mm_and_ps(mask, t4), _mm_andnot_ps(mask, infinites));

	u4 = _mm_mul_ps(e1, rcp_det);
	v4 = _mm_mul_ps(e2, rcp_det);

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
SORT_FORCEINLINE void setupIntersection(const Triangle4& tri4, const Ray& ray, const __m128& t4, const __m128& u4, const __m128& v4, const int id, Intersection* intersection) {
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
//! @param  ret     The result of intersection.
//! @return         Whether there is any intersection that is valid.
SORT_FORCEINLINE bool intersectTriangle4( const Ray& ray , const Triangle4& tri4 , Intersection* ret ){
	__m128	u4, v4, t4, mask;
	const auto quick_quit = (nullptr == ret);
	const auto maxt = ret ? ret->t : FLT_MAX;
	const auto intersected = intersectTriangle4Inner(ray, tri4, quick_quit, maxt, t4, u4, v4, mask);
	if (!intersected)
		return false;
	if (intersected && quick_quit)
		return true;

    // find the closest result
    __m128 t0 = _mm_min_ps( t4 , _mm_shuffle_ps( t4 , t4 , _MM_SHUFFLE(2, 3, 0, 1) ) );
	t0 = _mm_min_ps( t0 , _mm_shuffle_ps(t0, t0, _MM_SHUFFLE(1, 0, 3, 2) ) );

    // get the index of the closest one
    const auto resolved_mask = _mm_movemask_ps( _mm_cmpeq_ps( t4 , t0 ) );
	const auto res_i = __bsf(resolved_mask);

    sAssert( resolved_mask > 0 && resolved_mask < 16 , SPATIAL_ACCELERATOR );
    sAssert( res_i >= 0 && res_i < 4 , SPATIAL_ACCELERATOR );
    
	setupIntersection(tri4, ray, t4, u4, v4, res_i, ret);

    return true;
}

//! @brief  Unlike the above function, this helper function will populate all results in the BSSRDFIntersection data structure.
//!         It is for BSSRDF intersection tests.
//!
//! @param  ray     Ray to be tested against.
//! @param  tri4    Data structure holds four triangles.
//! @param  ret     The result of intersection.
SORT_FORCEINLINE void intersectTriangle4(const Ray& ray, const Triangle4& tri4, const StringID matID , BSSRDFIntersections& intersections) {
	__m128	u4, v4, t4, mask;
	const auto maxt = intersections.maxt;
	const auto intersected = intersectTriangle4Inner(ray, tri4, false, maxt, t4, u4, v4, mask);
	if (!intersected)
		return;

	auto resolved_mask = _mm_movemask_ps(mask);
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
