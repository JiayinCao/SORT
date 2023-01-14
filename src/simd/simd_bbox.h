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

#pragma once

#include "core/define.h"
#include "math/bbox.h"

// Reference implementation is disabled by default, it is only for debugging purposes.
// #define SIMD_BBOX_REFERENCE_IMPLEMENTATION

#if defined(SIMD_4WAY_IMPLEMENTATION) && defined(SIMD_8WAY_IMPLEMENTATION)
    static_assert( false , "More than one SIMD version is defined before including simd_bbox." );
#endif

#ifdef SIMD_BVH_IMPLEMENTATION

#if defined(SIMD_8WAY_IMPLEMENTATION)
    #define Simd_BBox   BBox8
#endif

#if defined(SIMD_4WAY_IMPLEMENTATION)
    #define Simd_BBox   BBox4
#endif

//! @brief  SIMD version bounding box.
/**
 * This is basically 4/8 bounding box in a single data structure. For best performance, they are saved in
 * structure of arrays.
 * Since this data structure is only used in limited places, only very few interfaces are implemented for
 * simplicity.
 */
struct alignas(SIMD_ALIGNMENT) Simd_BBox{
public:
    simd_data   m_min_x;
    simd_data   m_min_y;
    simd_data   m_min_z;

    simd_data   m_max_x;
    simd_data   m_max_y;
    simd_data   m_max_z;

    simd_data   m_mask;
};

static_assert( sizeof( Simd_BBox ) % SIMD_ALIGNMENT == 0 , "Incorrect size of Simd_BBox." );

SORT_FORCEINLINE int IntersectBBox_SIMD(const Ray& ray, const Simd_Ray_Data& simd_ray , const Simd_BBox& bb, simd_data& f_min ) {
#ifndef SIMD_BBOX_REFERENCE_IMPLEMENTATION
    f_min = simd_set_ps1( ray.m_fMin );
    simd_data f_max = simd_set_ps1( ray.m_fMax );

    simd_data t1    = simd_add_ps( ray_ori_dir_x(simd_ray) , simd_mul_ps( ray_rcp_dir_x(simd_ray) , bb.m_max_x ) );
    simd_data t2    = simd_add_ps( ray_ori_dir_x(simd_ray) , simd_mul_ps( ray_rcp_dir_x(simd_ray) , bb.m_min_x ) );
    f_min           = simd_max_ps( f_min , simd_min_ps( t1 , t2 ) );
    f_max           = simd_min_ps( f_max , simd_max_ps( t1 , t2 ) );

    t1              = simd_add_ps( ray_ori_dir_y(simd_ray) , simd_mul_ps( ray_rcp_dir_y(simd_ray) , bb.m_max_y ) );
    t2              = simd_add_ps( ray_ori_dir_y(simd_ray) , simd_mul_ps( ray_rcp_dir_y(simd_ray) , bb.m_min_y ) );
    f_min           = simd_max_ps( f_min , simd_min_ps( t1 , t2 ) );
    f_max           = simd_min_ps( f_max , simd_max_ps( t1 , t2 ) );

    t1              = simd_add_ps( ray_ori_dir_z(simd_ray) , simd_mul_ps( ray_rcp_dir_z(simd_ray) , bb.m_max_z ) );
    t2              = simd_add_ps( ray_ori_dir_z(simd_ray) , simd_mul_ps( ray_rcp_dir_z(simd_ray) , bb.m_min_z ) );
    f_min           = simd_max_ps( f_min , simd_min_ps( t1 , t2 ) );
    f_max           = simd_min_ps( f_max , simd_max_ps( t1 , t2 ) );

    const simd_data mask = simd_and_ps( bb.m_mask , simd_cmple_ps( f_min , f_max ) );
    f_min = simd_pick_ps( mask , f_min , simd_neg_ones );

    return simd_movemask_ps( mask );
#else
    int ret = 0;
    for( auto i = 0u ; i < SIMD_CHANNEL ; ++i ){
        if( bb.m_min_x[i] > bb.m_max_x[i] ){
            f_min[i] = -1.0f;
            continue;
        }

        BBox bbox;
        bbox.m_Min[0] = bb.m_min_x[i];
        bbox.m_Min[1] = bb.m_min_y[i];
        bbox.m_Min[2] = bb.m_min_z[i];
        bbox.m_Max[0] = bb.m_max_x[i];
        bbox.m_Max[1] = bb.m_max_y[i];
        bbox.m_Max[2] = bb.m_max_z[i];

        f_min[i] = Intersect( ray , bbox );

        if( f_min[i] >= 0.0f )
            ret |= ( 1 << i );
    }
    return ret;
#endif
}
#endif