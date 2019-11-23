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
#include "math/vector3.h"
#include "math/point.h"
#include "float.h"
#include "spectrum/spectrum.h"

#ifdef SSE_ENABLED
	#include <nmmintrin.h>
#endif

enum RAY_PREPARE_FLAG{
    RESOLVE_SSE_DATA = 0X01,
    RESOLVE_AVX_DATA = 0x02,
    RESOLVE_CPU_DATA = 0x03,
    RESOLVE_NONE_DATA = 0x00
};

//! @brief  Data structure representing a ray.
class Ray{
public:
    // default constructor
    Ray();
    // constructor from a point and a direction
    // para 'ori'   :   original point of the ray
    // para 'dir'   :   direction of the ray , it's the programmer's responsibility to normalize it
    // para 'depth' :   the depth of the curren ray , if not set , default value is 0
    // para 'fmin'  :   the minium range of the ray . It could be set a very small value to avoid false self intersection
    // para 'fmax'  :   the maxium range of the ray . A ray with 'fmax' not equal to 0 is actually a line segment, usually used for shadow ray.
    Ray( const Point& ori , const Vector& dir , unsigned depth = 0 , float fmin = 0.0f , float fmax = FLT_MAX );
    // copy constructor
    // para 'r' :   a ray to copy
    Ray( const Ray& r );

    // operator to get a point from the ray
    // para 't' :   the distance from the retrive point if the direction of the ray is normalized.
    // reslut   :   A point ( o + t * d )
    SORT_FORCEINLINE    Point operator ()( float t ) const{
        return m_Ori + t * m_Dir;
    }

    //! @brief  Pre-calculate some cached data for better performance. Only call this function after all ray data is prepared.
    void    Prepare( const RAY_PREPARE_FLAG flag = RESOLVE_CPU_DATA ) const;

// the original point and direction are also public
    // original point of the ray
    Point   m_Ori;
    // direction , the direction of the ray
    Vector  m_Dir;

    // the depth for the ray
    int     m_Depth;

    // the maximum and minimum value in the ray
    float   m_fMin;
    float   m_fMax;

    float   m_fPdfW;
    float   m_fPdfA;
    float   m_fCosAtCamera;

	// importance value of the ray
	Spectrum m_we;

    // some data are pre-calculated so that it won't be redo later multiple times
#ifdef SSE_ENABLED
    mutable __m128  m_ori_dir_x;    /**< -Ori.x/Dir.x , this is used in ray AABB intersection. */
    mutable __m128  m_ori_dir_y;    /**< -Ori.y/Dir.y , this is used in ray AABB intersection. */
    mutable __m128  m_ori_dir_z;    /**< -Ori.z/Dir.z , this is used in ray AABB intersection. */
    mutable __m128  m_rcp_dir_x;    /**< 1.0/Dir.x , this is used in ray AABB intersection. */
    mutable __m128  m_rcp_dir_y;    /**< 1.0/Dir.y , this is used in ray AABB intersection. */
    mutable __m128  m_rcp_dir_z;    /**< 1.0/Dir.z , this is used in ray AABB intersection. */
	mutable __m128	m_ori_x;		/**< Ori.x , this is used in ray Triangle&Line intersection. */
	mutable __m128	m_ori_y;		/**< Ori.y , this is used in ray Triangle&Line intersection. */
	mutable __m128	m_ori_z;		/**< Ori.z , this is used in ray Triangle&Line intersection. */
    mutable __m128  m_dir_x;        /**< Dir.x , this is used in ray Line intersection. */
    mutable __m128  m_dir_y;        /**< Dir.x , this is used in ray Line intersection. */
    mutable __m128  m_dir_z;        /**< Dir.x , this is used in ray Line intersection. */
	mutable __m128	m_sse_scale_x;	/**< Scaling along each axis in local coordinate. */
	mutable __m128	m_sse_scale_y;	/**< Scaling along each axis in local coordinate. */
	mutable __m128  m_sse_scale_z;	/**< Scaling along each axis in local coordinate. */
#endif

#ifdef AVX_ENABLED
    mutable __m256  m_ori_dir_x_avx;    /**< -Ori.x/Dir.x , this is used in ray AABB intersection. */
    mutable __m256  m_ori_dir_y_avx;    /**< -Ori.y/Dir.y , this is used in ray AABB intersection. */
    mutable __m256  m_ori_dir_z_avx;    /**< -Ori.z/Dir.z , this is used in ray AABB intersection. */
    mutable __m256  m_rcp_dir_x_avx;    /**< 1.0/Dir.x , this is used in ray AABB intersection. */
    mutable __m256  m_rcp_dir_y_avx;    /**< 1.0/Dir.y , this is used in ray AABB intersection. */
    mutable __m256  m_rcp_dir_z_avx;    /**< 1.0/Dir.z , this is used in ray AABB intersection. */
	mutable __m256	m_ori_x_avx;		/**< Ori.x , this is used in ray Triangle&Line intersection. */
	mutable __m256	m_ori_y_avx;		/**< Ori.y , this is used in ray Triangle&Line intersection. */
	mutable __m256	m_ori_z_avx;		/**< Ori.z , this is used in ray Triangle&Line intersection. */
    mutable __m256  m_dir_x_avx;        /**< Dir.x , this is used in ray Line intersection. */
    mutable __m256  m_dir_y_avx;        /**< Dir.x , this is used in ray Line intersection. */
    mutable __m256  m_dir_z_avx;        /**< Dir.x , this is used in ray Line intersection. */
	mutable __m256	m_sse_scale_x_avx;	/**< Scaling along each axis in local coordinate. */
	mutable __m256	m_sse_scale_y_avx;	/**< Scaling along each axis in local coordinate. */
	mutable __m256  m_sse_scale_z_avx;	/**< Scaling along each axis in local coordinate. */
#endif

	mutable int		m_local_x , m_local_y , m_local_z;	/**< Id used to identify axis in local coordinate. */
	mutable float	m_scale_x , m_scale_y , m_scale_z;	/**< Scaling along each axis in local coordinate. */
};
