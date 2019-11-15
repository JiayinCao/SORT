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

//! @brief  Triangle4 is more of a simplified resolved data structure holds only barebone information of triangle.
/**
 * Triangle4 is used in QBVH to accelerate ray triangle intersection using SSE. Its sole purpose is to accelerate 
 * ray triangle intersection by using SSE. Meaning there is no need to provide sophisticated interface of the class.
 * And since it is quite performance sensitive code, everything is inlined and there is no polymorphisum to keep it
 * as simple as possible. However, since there will be extra data kept in the system, it will also significately 
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
    
}

#endif