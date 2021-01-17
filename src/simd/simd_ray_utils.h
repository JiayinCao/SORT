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

#include "core/define.h"
#include "simd_wrapper.h"
#include "math/ray.h"

#if defined(SIMD_4WAY_IMPLEMENTATION) && defined(SIMD_8WAY_IMPLEMENTATION)
    static_assert( false , "More than one SIMD version is defined before including simd_bbox." );
#endif

#if defined(SIMD_4WAY_ENABLED) || defined(SIMD_8WAY_ENABLED)
#ifdef SIMD_BVH_IMPLEMENTATION

#ifdef SIMD_4WAY_IMPLEMENTATION
    #define Simd_Ray_Data   Ray4_Data
#endif
#ifdef SIMD_8WAY_IMPLEMENTATION
    #define Simd_Ray_Data   Ray8_Data
#endif

SORT_STATIC_FORCEINLINE float sign( const float x ){
    return x < 0.0f ? -1.0f : 1.0f;
}

struct alignas(SIMD_ALIGNMENT) Simd_Ray_Data{
    simd_data  ori_dir_x;    /**< -Ori.x/Dir.x , this is used in ray AABB intersection. */
	simd_data  ori_dir_y;    /**< -Ori.y/Dir.y , this is used in ray AABB intersection. */
	simd_data  ori_dir_z;    /**< -Ori.z/Dir.z , this is used in ray AABB intersection. */
	simd_data  rcp_dir_x;    /**< 1.0/Dir.x , this is used in ray AABB intersection. */
	simd_data  rcp_dir_y;    /**< 1.0/Dir.y , this is used in ray AABB intersection. */
	simd_data  rcp_dir_z;    /**< 1.0/Dir.z , this is used in ray AABB intersection. */
	simd_data  ori_x;        /**< Ori.x , this is used in ray Triangle&Line intersection. */
	simd_data  ori_y;        /**< Ori.y , this is used in ray Triangle&Line intersection. */
	simd_data  ori_z;        /**< Ori.z , this is used in ray Triangle&Line intersection. */
	simd_data  dir_x;        /**< Dir.x , this is used in ray Line intersection. */
	simd_data  dir_y;        /**< Dir.x , this is used in ray Line intersection. */
	simd_data  dir_z;        /**< Dir.x , this is used in ray Line intersection. */
	simd_data  scale_x;      /**< Scaling along each axis in local coordinate. */
	simd_data  scale_y;      /**< Scaling along each axis in local coordinate. */
	simd_data  scale_z;      /**< Scaling along each axis in local coordinate. */
};

void resolveRayData( const Ray& ray , Simd_Ray_Data& simd_ray_data ){
    constexpr float delta = 0.00001f;
    const auto dir_x = fabs(ray.m_Dir[0]) < delta ? sign(ray.m_Dir[0]) * delta : ray.m_Dir[0];
    const auto dir_y = fabs(ray.m_Dir[1]) < delta ? sign(ray.m_Dir[1]) * delta : ray.m_Dir[1];
    const auto dir_z = fabs(ray.m_Dir[2]) < delta ? sign(ray.m_Dir[2]) * delta : ray.m_Dir[2];
    simd_ray_data.rcp_dir_x = simd_set_ps1( 1.0f/dir_x );
    simd_ray_data.rcp_dir_y = simd_set_ps1( 1.0f/dir_y );
    simd_ray_data.rcp_dir_z = simd_set_ps1( 1.0f/dir_z );
    simd_ray_data.ori_dir_x = simd_set_ps1( -ray.m_Ori[0]/dir_x );
    simd_ray_data.ori_dir_y = simd_set_ps1( -ray.m_Ori[1]/dir_y ); 
    simd_ray_data.ori_dir_z = simd_set_ps1( -ray.m_Ori[2]/dir_z ); 

    simd_ray_data.ori_x = simd_set_ps1( ray.m_Ori.x );
    simd_ray_data.ori_y = simd_set_ps1( ray.m_Ori.y );
    simd_ray_data.ori_z = simd_set_ps1( ray.m_Ori.z );

    simd_ray_data.dir_x = simd_set_ps1( ray.m_Dir.x );
    simd_ray_data.dir_y = simd_set_ps1( ray.m_Dir.y );
    simd_ray_data.dir_z = simd_set_ps1( ray.m_Dir.z );

    simd_ray_data.scale_x = simd_set_ps1( ray.m_scale_x );
    simd_ray_data.scale_y = simd_set_ps1( ray.m_scale_y );
    simd_ray_data.scale_z = simd_set_ps1( ray.m_scale_z );
}

SORT_STATIC_FORCEINLINE simd_data   ray_ori_dir_x( const Simd_Ray_Data& ray ){
    return ray.ori_dir_x;
}
SORT_STATIC_FORCEINLINE simd_data   ray_ori_dir_y( const Simd_Ray_Data& ray ){
    return ray.ori_dir_y;
}
SORT_STATIC_FORCEINLINE simd_data   ray_ori_dir_z( const Simd_Ray_Data& ray ){
    return ray.ori_dir_z;
}
SORT_STATIC_FORCEINLINE simd_data   ray_ori_x( const Simd_Ray_Data& ray ){
    return ray.ori_x;
}
SORT_STATIC_FORCEINLINE simd_data   ray_ori_y( const Simd_Ray_Data& ray ){
    return ray.ori_y;
}
SORT_STATIC_FORCEINLINE simd_data   ray_ori_z( const Simd_Ray_Data& ray ){
    return ray.ori_z;
}
SORT_STATIC_FORCEINLINE simd_data   ray_dir_x( const Simd_Ray_Data& ray ){
    return ray.dir_x;
}
SORT_STATIC_FORCEINLINE simd_data   ray_dir_y( const Simd_Ray_Data& ray ){
    return ray.dir_y;
}
SORT_STATIC_FORCEINLINE simd_data   ray_dir_z( const Simd_Ray_Data& ray ){
    return ray.dir_z;
}
SORT_STATIC_FORCEINLINE simd_data   ray_rcp_dir_x( const Simd_Ray_Data& ray ){
    return ray.rcp_dir_x;
}
SORT_STATIC_FORCEINLINE simd_data   ray_rcp_dir_y( const Simd_Ray_Data& ray ){
    return ray.rcp_dir_y;
}
SORT_STATIC_FORCEINLINE simd_data   ray_rcp_dir_z( const Simd_Ray_Data& ray ){
    return ray.rcp_dir_z;
}
SORT_STATIC_FORCEINLINE simd_data   ray_scale_x( const Simd_Ray_Data& ray ){
    return ray.scale_x;
}
SORT_STATIC_FORCEINLINE simd_data   ray_scale_y( const Simd_Ray_Data& ray ){
    return ray.scale_y;
}
SORT_STATIC_FORCEINLINE simd_data   ray_scale_z( const Simd_Ray_Data& ray ){
    return ray.scale_z;
}
#endif

#endif