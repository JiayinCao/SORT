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
#include "math/ray.h"
#ifdef SSE_ENABLED
    #include <nmmintrin.h>
#endif

#ifdef  SSE_ENABLED

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
};

//! @brief  With the power of SSE, this utility function helps intersect a ray with four triangles at the cost of one.
//!
//! @param  tri4    Data structure holds four triangles.
//! @param  ray     Ray to be tested against.
//! @param  ret     The result of intersection.
SORT_FORCEINLINE void intersectTriangle4( const Triangle4& tri4 , const Ray& ray , Intersection& ret ){
	// step 0 : translate the vertices to ray coordinate system
	__m128 p0[3] , p1[3] , p2[3];
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

	__m128 p2_x = p1[ray.m_local_x];
	__m128 p2_y = p1[ray.m_local_y];
	__m128 p2_z = p1[ray.m_local_z];

	// step 2 : sheer the vertices so that the ray direction points to ( 0 , 1 , 0 )
	p0_x = _mm_add_ps(p0_x, _mm_mul_ps(p0_y, ray.m_sse_scale_x));
	p0_z = _mm_add_ps(p0_z, _mm_mul_ps(p0_y, ray.m_sse_scale_z));
	p1_x = _mm_add_ps(p1_x, _mm_mul_ps(p1_y, ray.m_sse_scale_x));
	p1_z = _mm_add_ps(p1_z, _mm_mul_ps(p1_y, ray.m_sse_scale_z));
	p2_x = _mm_add_ps(p2_x, _mm_mul_ps(p2_y, ray.m_sse_scale_x));
	p2_z = _mm_add_ps(p2_z, _mm_mul_ps(p2_y, ray.m_sse_scale_z));

	// compute the edge functions
	__m128 e0 = _mm_sub_ps( _mm_mul_ps( p1_x , p2_z ) , _mm_mul_ps( p1_z , p2_x ) );
	__m128 e1 = _mm_sub_ps( _mm_mul_ps( p2_x , p0_z ) , _mm_mul_ps( p2_z , p0_x ) );
	__m128 e2 = _mm_sub_ps( _mm_mul_ps( p0_x , p1_z ) , _mm_mul_ps( p0_z , p1_x ) );
}

#endif