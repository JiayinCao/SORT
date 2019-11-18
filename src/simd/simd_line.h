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

//! @brief  Like Triangle4, Line4 is the cooresponding version for line shape.
struct Line4{
    __m128  m_p0_x , m_p0_y , m_p0_z;   /**< Point at the end of the line. */
    __m128  m_p1_x , m_p1_y , m_p1_z;   /**< Point at the other end of the line. */

    __m128  m_w0 , m_w1;                /**< Half width of the line. */
    __m128  m_length;                   /**< Length of the line. */

    /**< Transformation from world space to line local space. */
    __m128  m_mat_00, m_mat_01, m_mat_02, m_mat_03;
    __m128  m_mat_10, m_mat_11, m_mat_12, m_mat_13;
    __m128  m_mat_20, m_mat_21, m_mat_22, m_mat_23;
//    __m128  m_mat_30, m_mat_31, m_mat_32, m_mat_33;

    __m128  m_mask;                     /**< Mask marks which line is valid. */

    /**< Pointers to original primitive. */
    const Line*         m_ori_line[4] = { nullptr };
    const Primitive*    m_ori_pri[4] = { nullptr };

    //! @brief  Push a line in the data structure.
    //!
	//! @param	pri		The original primitive.
    //! @return         Whether the data structure is full.
    bool PushLine( const Primitive* primitive ){
		const Line* line = dynamic_cast<const Line*>(primitive->GetShape());
        if( m_ori_pri[0] == nullptr ){
            m_ori_pri[0] = primitive;
			m_ori_line[0] = line;
            return false;
        }else if( m_ori_pri[1] == nullptr ){
            m_ori_pri[1] = primitive;
			m_ori_line[1] = line;
            return false;
        }else if( m_ori_pri[2] == nullptr ){
            m_ori_pri[2] = primitive;
			m_ori_line[2] = line;
            return false;
        }
        m_ori_pri[3] = primitive;
		m_ori_line[3] = line;
        return true;
    }

    //! @brief  Pack line information into SSE compatible data.
	//!
	//! @return		Whether there is valid line inside.
    bool PackData(){
		if( !m_ori_pri[0] )
			return false;

        float	mask[4] = { 1.0f , 1.0f , 1.0f , 1.0f };
        float   p0_x[4] , p0_y[4] , p0_z[4] , p1_x[4] , p1_y[4] , p1_z[4];
        float   w0[4] , w1[4] , length[4];
        float   mat_00[4] , mat_01[4] , mat_02[4] , mat_03[4];
        float   mat_10[4] , mat_11[4] , mat_12[4] , mat_13[4];
        float   mat_20[4] , mat_21[4] , mat_22[4] , mat_23[4];
        float   mat_30[4] , mat_31[4] , mat_32[4] , mat_33[4];
        for( auto i = 0 ; i < 4 && m_ori_pri[i] ; ++i ){
            const auto line = m_ori_line[i];

            p0_x[i] = line->m_p0.x;
            p0_y[i] = line->m_p0.y;
            p0_z[i] = line->m_p0.z;

            p1_x[i] = line->m_p1.x;
            p1_y[i] = line->m_p1.y;
            p1_z[i] = line->m_p1.z;

            w0[i] = line->m_w0;
            w1[i] = line->m_w1;

            length[i] = line->m_length;

            mat_00[i] = line->m_world2Line.matrix.m[0];
            mat_01[i] = line->m_world2Line.matrix.m[1];
            mat_02[i] = line->m_world2Line.matrix.m[2];
            mat_03[i] = line->m_world2Line.matrix.m[3];
            mat_10[i] = line->m_world2Line.matrix.m[4];
            mat_11[i] = line->m_world2Line.matrix.m[5];
            mat_12[i] = line->m_world2Line.matrix.m[6];
            mat_13[i] = line->m_world2Line.matrix.m[7];
            mat_20[i] = line->m_world2Line.matrix.m[8];
            mat_21[i] = line->m_world2Line.matrix.m[9];
            mat_22[i] = line->m_world2Line.matrix.m[10];
            mat_23[i] = line->m_world2Line.matrix.m[11];
//            mat_30[i] = line->m_world2Line.matrix.m[12];
//            mat_31[i] = line->m_world2Line.matrix.m[13];
//            mat_32[i] = line->m_world2Line.matrix.m[14];
//            mat_33[i] = line->m_world2Line.matrix.m[15];

            mask[i] = 0.0f;
        }

        m_p0_x = _mm_set_ps( p0_x[3] , p0_x[2] , p0_x[1] , p0_x[0] );
        m_p0_y = _mm_set_ps( p0_y[3] , p0_y[2] , p0_y[1] , p0_y[0] );
        m_p0_z = _mm_set_ps( p0_z[3] , p0_z[2] , p0_z[1] , p0_z[0] );
        m_p1_x = _mm_set_ps( p1_x[3] , p1_x[2] , p1_x[1] , p1_x[0] );
        m_p1_y = _mm_set_ps( p1_y[3] , p1_y[2] , p1_y[1] , p1_y[0] );
        m_p1_z = _mm_set_ps( p1_z[3] , p1_z[2] , p1_z[1] , p1_z[0] );
        m_w0 = _mm_set_ps( w0[3] , w0[2] , w0[1] , w0[0] );
        m_w1 = _mm_set_ps( w1[3] , w1[2] , w1[1] , w1[0] );
        m_length = _mm_set_ps( length[3] , length[2] , length[1] , length[0] );

        m_mat_00 = _mm_set_ps( mat_00[3] , mat_00[2] , mat_00[1] , mat_00[0] );
        m_mat_01 = _mm_set_ps( mat_01[3] , mat_01[2] , mat_01[1] , mat_01[0] );
        m_mat_02 = _mm_set_ps( mat_02[3] , mat_02[2] , mat_02[1] , mat_02[0] );
        m_mat_03 = _mm_set_ps( mat_03[3] , mat_03[2] , mat_03[1] , mat_03[0] );
        m_mat_10 = _mm_set_ps( mat_10[3] , mat_10[2] , mat_10[1] , mat_10[0] );
        m_mat_11 = _mm_set_ps( mat_11[3] , mat_11[2] , mat_11[1] , mat_11[0] );
        m_mat_12 = _mm_set_ps( mat_12[3] , mat_12[2] , mat_12[1] , mat_12[0] );
        m_mat_13 = _mm_set_ps( mat_13[3] , mat_13[2] , mat_13[1] , mat_13[0] );
        m_mat_20 = _mm_set_ps( mat_20[3] , mat_20[2] , mat_20[1] , mat_20[0] );
        m_mat_21 = _mm_set_ps( mat_21[3] , mat_21[2] , mat_21[1] , mat_21[0] );
        m_mat_22 = _mm_set_ps( mat_22[3] , mat_22[2] , mat_22[1] , mat_22[0] );
        m_mat_23 = _mm_set_ps( mat_23[3] , mat_23[2] , mat_23[1] , mat_23[0] );
//        m_mat_30 = _mm_set_ps( mat_30[3] , mat_30[2] , mat_30[1] , mat_30[0] );
//        m_mat_31 = _mm_set_ps( mat_31[3] , mat_31[2] , mat_31[1] , mat_31[0] );
//        m_mat_32 = _mm_set_ps( mat_32[3] , mat_32[2] , mat_32[1] , mat_32[0] );
//        m_mat_33 = _mm_set_ps( mat_33[3] , mat_33[2] , mat_33[1] , mat_33[0] );

        m_mask = _mm_cmpeq_ps( zeros , _mm_set_ps( mask[3] , mask[2] , mask[1] , mask[0] ) );

		return true;
    }

    //! @brief  Reset the data for reuse
    void Reset(){
        m_ori_pri[0] = m_ori_pri[1] = m_ori_pri[2] = m_ori_pri[3] = nullptr;
		m_ori_line[0] = m_ori_line[1] = m_ori_line[2] = m_ori_line[3] = nullptr;
    }
};

//! @brief  With the power of SSE, this utility function helps intersect a ray with four lines at the cost of one.
//!
//! @param  ray     Ray to be tested against.
//! @param  line4   Data structure holds four lines.
//! @param  ret     The result of intersection.
//! @return         Whether there is any intersection that is valid.
SORT_FORCEINLINE bool intersectLine4( const Ray& ray , const Line4& line4 , Intersection* ret ){
#if 1
    bool found = false;
    for( int i = 0 ; i < 4 ; ++i ){
        if( line4.m_ori_pri[i] == nullptr )
            break;
        found |= line4.m_ori_pri[i]->GetIntersect( ray , ret );
    }
    return found;
#else
    __m128  mask = line4.m_mask;

    const __m128 ray_ori_x = _mm_add_ps( _mm_add_ps( _mm_mul_ps( line4.m_mat_00 , ray.m_ori_x ) , _mm_mul_ps( line4.m_mat_01 , ray.m_ori_y ) ) ,
                                   _mm_add_ps( _mm_mul_ps( line4.m_mat_02 , ray.m_ori_z ) , line4.m_mat_03 ) );
    const __m128 ray_ori_y = _mm_add_ps( _mm_add_ps( _mm_mul_ps( line4.m_mat_10 , ray.m_ori_x ) , _mm_mul_ps( line4.m_mat_11 , ray.m_ori_y ) ) ,
                                   _mm_add_ps( _mm_mul_ps( line4.m_mat_12 , ray.m_ori_z ) , line4.m_mat_13 ) );
    const __m128 ray_ori_z = _mm_add_ps( _mm_add_ps( _mm_mul_ps( line4.m_mat_20 , ray.m_ori_x ) , _mm_mul_ps( line4.m_mat_21 , ray.m_ori_y ) ) ,
                                   _mm_add_ps( _mm_mul_ps( line4.m_mat_22 , ray.m_ori_z ) , line4.m_mat_23 ) );
    
    const __m128 ray_dir_x = _mm_add_ps( _mm_add_ps( _mm_mul_ps( line4.m_mat_00 , ray.m_dir_x ) , _mm_mul_ps( line4.m_mat_01 , ray.m_dir_y ) ) ,
                                   _mm_mul_ps( line4.m_mat_02 , ray.m_dir_z ) );
    const __m128 ray_dir_y = _mm_add_ps( _mm_add_ps( _mm_mul_ps( line4.m_mat_10 , ray.m_dir_x ) , _mm_mul_ps( line4.m_mat_11 , ray.m_dir_y ) ) ,
                                   _mm_mul_ps( line4.m_mat_12 , ray.m_dir_z ) );
    const __m128 ray_dir_z = _mm_add_ps( _mm_add_ps( _mm_mul_ps( line4.m_mat_20 , ray.m_dir_x ) , _mm_mul_ps( line4.m_mat_21 , ray.m_dir_y ) ) ,
                                   _mm_mul_ps( line4.m_mat_22 , ray.m_dir_z ) );

    const __m128 tmp =  _mm_div_ps( _mm_sub_ps( line4.m_w1 , line4.m_w0 ) , line4.m_length );
    const __m128 tmp0 = _mm_add_ps( line4.m_w0 , _mm_mul_ps( ray_ori_y , tmp ) );
    const __m128 tmp1 = _mm_mul_ps( ray_dir_y , tmp );

    // The 2.0 factor is skipped because it is not needed and will be canceled out.
    const auto a = _mm_sub_ps( _mm_add_ps( _mm_sqr_ps( ray_dir_x ) , _mm_sqr_ps( ray_dir_z ) ) , _mm_sqr_ps( tmp1 ) );
    const auto b = _mm_sub_ps( _mm_add_ps( _mm_mul_ps( ray_dir_x , ray_ori_x ) , _mm_mul_ps( ray_dir_z , ray_dir_x ) ) , _mm_mul_ps( tmp0 , tmp1 ) );
    const auto c = _mm_sub_ps( _mm_add_ps( _mm_sqr_ps( ray_ori_x ) , _mm_sqr_ps( ray_ori_z ) ) , _mm_sqr_ps( tmp0 ) );

    const auto discriminant = _mm_sub_ps( _mm_sqr_ps( b ) , _mm_mul_ps( a , c ) );
    mask = _mm_and_ps( mask , _mm_cmplt_ps( discriminant , zeros ) );
    auto cm = _mm_movemask_ps(mask);
    if (0 == cm)
		return false;
    
    const auto sqrt_dist = _mm_sqrt_ps( discriminant );
    const auto t0 = _mm_div_ps( _mm_sub_ps( _mm_sub_ps( zeros , b ) , sqrt_dist ) , a );
    const auto inter_y_0 = _mm_add_ps( ray_ori_y , _mm_mul_ps( ray_dir_y , t0 ) );
    const auto mask0 = _mm_or_ps( _mm_cmpgt_ps( inter_y_0 , line4.m_length ) , _mm_cmplt_ps( inter_y_0 , zeros ) );

    const auto t1 = _mm_div_ps( _mm_sub_ps( sqrt_dist , b ) , a );
    const auto inter_y_1 = _mm_add_ps( ray_ori_y , _mm_mul_ps( ray_dir_y , t1 ) );
    const auto mask1 = _mm_or_ps( _mm_cmpgt_ps( inter_y_1 , line4.m_length ) , _mm_cmplt_ps( inter_y_1 , zeros ) );
    const auto mask2 = _mm_andnot_ps( mask1 , mask0 );

    auto t = _mm_or_ps( _mm_and_ps( mask2 , t1 ) , _mm_andnot_ps( mask0 , t0 ) );
    t = _mm_or_ps( _mm_and_ps( mask , t ) , _mm_andnot_ps( mask , infinites ) );

    const __m128 ray_min_t = _mm_set_ps1(ray.m_fMin);
	const __m128 ray_max_t = _mm_set_ps1(ray.m_fMax);
    mask = _mm_add_ps( mask , _mm_and_ps( _mm_cmpgt_ps( t , ray_min_t ) , _mm_cmplt_ps( t , ray_max_t ) ) );
    cm = _mm_movemask_ps(mask);
	if (0 == cm)
		return false;

    if( nullptr == ret )
        return true;
    
    mask = _mm_add_ps( mask , _mm_cmpge_ps( t , _mm_set_ps1(ret->t) ) );
    cm = _mm_movemask_ps(mask);
	if (0 == cm)
		return false;

    // find the closest result
    __m128 t_min = _mm_min_ps( t , _mm_shuffle_ps( t , t , _MM_SHUFFLE(2, 3, 0, 1) ) );
	t_min = _mm_min_ps( t_min , _mm_shuffle_ps(t_min, t_min, _MM_SHUFFLE(1, 0, 3, 2) ) );

    // get the index of the closest one
    const auto resolved_mask = _mm_movemask_ps( _mm_cmpeq_ps( t , t_min ) );
	const auto res_i = __bsf(resolved_mask);
    
    ret->intersect = ray( t[res_i] );

    return false;
#endif
}

#endif