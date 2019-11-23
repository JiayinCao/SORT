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
#include "simd_wrapper.h"
#include "math/ray.h"

#if defined(SIMD_SSE_IMPLEMENTATION) && defined(SIMD_AVX_IMPLEMENTATION)
	static_assert( false , "More than one SIMD version is defined before including simd_bbox." );
#endif

#ifdef SSE_ENABLED

#ifdef SIMD_SSE_IMPLEMENTATION
simd_data_sse	ray_ori_dir_x( const Ray& ray ){
	return ray.m_ori_dir_x;
}
simd_data_sse	ray_ori_dir_y( const Ray& ray ){
	return ray.m_ori_dir_y;
}
simd_data_sse	ray_ori_dir_z( const Ray& ray ){
	return ray.m_ori_dir_z;
}
simd_data_sse	ray_rcp_dir_x( const Ray& ray ){
	return ray.m_rcp_dir_x;
}
simd_data_sse	ray_rcp_dir_y( const Ray& ray ){
	return ray.m_rcp_dir_y;
}
simd_data_sse	ray_rcp_dir_z( const Ray& ray ){
	return ray.m_rcp_dir_z;
}
#endif

#endif // SSE_ENABLED

#ifdef AVX_ENABLED

#ifdef SIMD_AVX_IMPLEMENTATION
simd_data_avx	ray_ori_dir_x( const Ray& ray ){
	return ray.m_ori_dir_x_avx;
}
simd_data_avx	ray_ori_dir_y( const Ray& ray ){
	return ray.m_ori_dir_y_avx;
}
simd_data_avx	ray_ori_dir_z( const Ray& ray ){
	return ray.m_ori_dir_z_avx;
}
simd_data_avx	ray_rcp_dir_x( const Ray& ray ){
	return ray.m_rcp_dir_x_avx;
}
simd_data_avx	ray_rcp_dir_y( const Ray& ray ){
	return ray.m_rcp_dir_y_avx;
}
simd_data_avx	ray_rcp_dir_z( const Ray& ray ){
	return ray.m_rcp_dir_z_avx;
}
#endif

#endif // AVX_ENABLED