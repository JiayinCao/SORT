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
#include "simd_utils.h"

#ifdef SSE_ENABLED
	#include <nmmintrin.h>
#endif

#ifdef SSE_ENABLED

//! @brief	SIMD version bounding box.
/**
 * This is basically 4 bounding box in a single data structure. For best performance, they are saved in
 * structure of arrays.
 * Since this data structure is only used in limited places, only very few interfaces are implemented for
 * simplicity.
 */
struct BBox4{
public:
	__m128	m_min_x;
	__m128	m_min_y;
	__m128	m_min_z;

	__m128	m_max_x;
	__m128	m_max_y;
	__m128	m_max_z;
};

SORT_FORCEINLINE void IntersectBBox4(const Ray& ray, const BBox4& bb, __m128& f_min ) {
	f_min = _mm_set_ps1( ray.m_fMin );
	__m128 f_max = _mm_set_ps1( ray.m_fMax );

	__m128 t1	= _mm_add_ps( ray.m_ori_dir_x , _mm_mul_ps( ray.m_rcp_dir_x , bb.m_max_x ) );
	__m128 t2	= _mm_add_ps( ray.m_ori_dir_x , _mm_mul_ps( ray.m_rcp_dir_x , bb.m_min_x ) );
	f_min	    = _mm_max_ps( f_min , _mm_min_ps( t1 , t2 ) );
	f_max		= _mm_min_ps( f_max , _mm_max_ps( t1 , t2 ) );

    t1		    = _mm_add_ps( ray.m_ori_dir_y , _mm_mul_ps( ray.m_rcp_dir_y , bb.m_max_y ) );
	t2		    = _mm_add_ps( ray.m_ori_dir_y , _mm_mul_ps( ray.m_rcp_dir_y , bb.m_min_y ) );
	f_min	    = _mm_max_ps( f_min , _mm_min_ps( t1 , t2 ) );
	f_max	    = _mm_min_ps( f_max , _mm_max_ps( t1 , t2 ) );

    t1		    = _mm_add_ps( ray.m_ori_dir_z , _mm_mul_ps( ray.m_rcp_dir_z , bb.m_max_z ) );
	t2		    = _mm_add_ps( ray.m_ori_dir_z , _mm_mul_ps( ray.m_rcp_dir_z , bb.m_min_z ) );
	f_min	    = _mm_max_ps( f_min , _mm_min_ps( t1 , t2 ) );
	f_max	    = _mm_min_ps( f_max , _mm_max_ps( t1 , t2 ) );

	const __m128 mask = _mm_cmple_ps( f_min , f_max );
	f_min = _mm_pick_ps( mask , f_min , neg_ones );
}

#endif