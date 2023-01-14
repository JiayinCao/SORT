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
#include "math/ray.h"
#include "shape/line.h"
#include "core/primitive.h"

// Reference implementation is disabled by default, it is only for debugging purposes.
// #define SIMD_LINE_REFERENCE_IMPLEMENTATION

#if defined(SIMD_4WAY_IMPLEMENTATION) && defined(SIMD_8WAY_IMPLEMENTATION)
    static_assert( false , "More than one SIMD version is defined before including simd_line.h." );
#endif

#ifdef SIMD_BVH_IMPLEMENTATION

#ifdef SIMD_4WAY_IMPLEMENTATION
    #define Simd_Line   Line4
#endif

#ifdef SIMD_8WAY_IMPLEMENTATION
    #define Simd_Line   Line8
#endif

//! @brief  Like Triangle8, Line8 is the corresponding version for line shape.
struct alignas(SIMD_ALIGNMENT) Simd_Line{
    simd_data  m_p0_x , m_p0_y , m_p0_z;   /**< Point at the end of the line. */
    simd_data  m_p1_x , m_p1_y , m_p1_z;   /**< Point at the other end of the line. */

    simd_data  m_w0 , m_w1;                /**< Half width of the line. */
    simd_data  m_length;                   /**< Length of the line. */

    /**< Transformation from world space to line local space. */
    simd_data  m_mat_00, m_mat_01, m_mat_02, m_mat_03;
    simd_data  m_mat_10, m_mat_11, m_mat_12, m_mat_13;
    simd_data  m_mat_20, m_mat_21, m_mat_22, m_mat_23;

    simd_data  m_mask;                     /**< Mask marks which line is valid. */

    /**< Pointers to original primitive. */
    const Line*         m_ori_line[SIMD_CHANNEL] = { nullptr };
    const Primitive*    m_ori_pri[SIMD_CHANNEL] = { nullptr };

#ifdef SIMD_8WAY_IMPLEMENTATION
    char                padding[16];
#endif

    //! @brief  Push a line in the data structure.
    //!
    //! @param  pri     The original primitive.
    //! @return         Whether the data structure is full.
    bool PushLine( const Primitive* primitive ){
#ifdef SIMD_4WAY_IMPLEMENTATION
        const Line* line = dynamic_cast<const Line*>(primitive->GetShape());
        if(IS_PTR_INVALID(m_ori_pri[0])){
            m_ori_pri[0] = primitive;
            m_ori_line[0] = line;
            return false;
        }else if(IS_PTR_INVALID(m_ori_pri[1])){
            m_ori_pri[1] = primitive;
            m_ori_line[1] = line;
            return false;
        }else if(IS_PTR_INVALID(m_ori_pri[2])){
            m_ori_pri[2] = primitive;
            m_ori_line[2] = line;
            return false;
        }
        m_ori_pri[3] = primitive;
        m_ori_line[3] = line;
        return true;
#endif

#ifdef SIMD_8WAY_IMPLEMENTATION
        const Line* line = dynamic_cast<const Line*>(primitive->GetShape());
        if(IS_PTR_INVALID(m_ori_pri[0])){
            m_ori_pri[0] = primitive;
            m_ori_line[0] = line;
            return false;
        }else if(IS_PTR_INVALID(m_ori_pri[1])){
            m_ori_pri[1] = primitive;
            m_ori_line[1] = line;
            return false;
        }else if(IS_PTR_INVALID(m_ori_pri[2])){
            m_ori_pri[2] = primitive;
            m_ori_line[2] = line;
            return false;
        }else if(IS_PTR_INVALID(m_ori_pri[3])){
            m_ori_pri[3] = primitive;
            m_ori_line[3] = line;
            return false;
        }else if(IS_PTR_INVALID(m_ori_pri[4])){
            m_ori_pri[4] = primitive;
            m_ori_line[4] = line;
            return false;
        }else if(IS_PTR_INVALID(m_ori_pri[5])){
            m_ori_pri[5] = primitive;
            m_ori_line[5] = line;
            return false;
        }else if(IS_PTR_INVALID(m_ori_pri[6])){
            m_ori_pri[6] = primitive;
            m_ori_line[6] = line;
            return false;
        }
        m_ori_pri[7] = primitive;
        m_ori_line[7] = line;
        return true;
#endif
    }

    //! @brief  Pack line information into SIMD compatible data.
    //!
    //! @return     Whether there is valid line inside.
    bool PackData(){
        if( !m_ori_pri[0] )
            return false;

        bool    mask[SIMD_CHANNEL] = { false };
        float   p0_x[SIMD_CHANNEL] , p0_y[SIMD_CHANNEL] , p0_z[SIMD_CHANNEL] , p1_x[SIMD_CHANNEL] , p1_y[SIMD_CHANNEL] , p1_z[SIMD_CHANNEL];
        float   w0[SIMD_CHANNEL] , w1[SIMD_CHANNEL] , length[SIMD_CHANNEL];
        float   mat_00[SIMD_CHANNEL] , mat_01[SIMD_CHANNEL] , mat_02[SIMD_CHANNEL] , mat_03[SIMD_CHANNEL];
        float   mat_10[SIMD_CHANNEL] , mat_11[SIMD_CHANNEL] , mat_12[SIMD_CHANNEL] , mat_13[SIMD_CHANNEL];
        float   mat_20[SIMD_CHANNEL] , mat_21[SIMD_CHANNEL] , mat_22[SIMD_CHANNEL] , mat_23[SIMD_CHANNEL];
        for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
            if(IS_PTR_INVALID(m_ori_pri[i])){
                mask[i] = false;
                continue;
            }

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

            mask[i] = true;
        }

        m_p0_x = simd_set_ps( p0_x );
        m_p0_y = simd_set_ps( p0_y );
        m_p0_z = simd_set_ps( p0_z );
        m_p1_x = simd_set_ps( p1_x );
        m_p1_y = simd_set_ps( p1_y );
        m_p1_z = simd_set_ps( p1_z );
        m_w0 = simd_set_ps( w0 );
        m_w1 = simd_set_ps( w1 );
        m_length = simd_set_ps( length );

        m_mat_00 = simd_set_ps( mat_00 );
        m_mat_01 = simd_set_ps( mat_01 );
        m_mat_02 = simd_set_ps( mat_02 );
        m_mat_03 = simd_set_ps( mat_03 );
        m_mat_10 = simd_set_ps( mat_10 );
        m_mat_11 = simd_set_ps( mat_11 );
        m_mat_12 = simd_set_ps( mat_12 );
        m_mat_13 = simd_set_ps( mat_13 );
        m_mat_20 = simd_set_ps( mat_20 );
        m_mat_21 = simd_set_ps( mat_21 );
        m_mat_22 = simd_set_ps( mat_22 );
        m_mat_23 = simd_set_ps( mat_23 );

        m_mask = simd_set_mask( mask );

        return true;
    }

    //! @brief  Reset the data for reuse
    void Reset(){
#ifdef SIMD_4WAY_IMPLEMENTATION
        m_ori_pri[0] = m_ori_pri[1] = m_ori_pri[2] = m_ori_pri[3] = nullptr;
        m_ori_line[0] = m_ori_line[1] = m_ori_line[2] = m_ori_line[3] = nullptr;
#endif

#ifdef SIMD_8WAY_IMPLEMENTATION
        m_ori_pri[0] = m_ori_pri[1] = m_ori_pri[2] = m_ori_pri[3] = m_ori_pri[4] = m_ori_pri[5] = m_ori_pri[6] = m_ori_pri[7] = nullptr;
        m_ori_line[0] = m_ori_line[1] = m_ori_line[2] = m_ori_line[3] = m_ori_line[4] = m_ori_line[5] = m_ori_line[6] = m_ori_line[7] = nullptr;
#endif
    }
};

static_assert( sizeof( Simd_Line ) % SIMD_ALIGNMENT == 0 , "Incorrect size of Simd_Line." );

//! @brief  Helper function that implements the core algorithm of ray line intersection.
//!
//! @param  ray         Ray to be tested against.
//! @param  ray_simd    Resolved simd ray data.
//! @param  line_simd   The data structure holds four lines. Some of them may be invalid.
//! @param  mask        The mask of valid results.
//! @param  t_simd      Distance from the ray origin to the intersected point on the line.
//! @param  inter_x     Intersection in local line space.
//! @param  inter_y     Intersection in local line space.
//! @param  inter_z     Intersection in local line space.
//! @return             Whether there is intersection between the ray and the four lines.
SORT_FORCEINLINE bool intersectLine_Inner( const Ray& ray , const Simd_Ray_Data& ray_simd, const Simd_Line& line_simd , simd_data& mask , simd_data& t_simd , simd_data& inter_x , simd_data& inter_y , simd_data& inter_z ){
    mask = line_simd.m_mask;

    const simd_data _ray_ori_x = simd_add_ps( simd_mad_ps( line_simd.m_mat_02, ray_ori_z(ray_simd), simd_mad_ps( line_simd.m_mat_01, ray_ori_y(ray_simd), simd_mul_ps( line_simd.m_mat_00, ray_ori_x(ray_simd)) ) ) , line_simd.m_mat_03 );
    const simd_data _ray_ori_y = simd_add_ps( simd_mad_ps( line_simd.m_mat_12, ray_ori_z(ray_simd), simd_mad_ps( line_simd.m_mat_11, ray_ori_y(ray_simd), simd_mul_ps( line_simd.m_mat_10, ray_ori_x(ray_simd)) ) ) , line_simd.m_mat_13 );
    const simd_data _ray_ori_z = simd_add_ps( simd_mad_ps( line_simd.m_mat_22, ray_ori_z(ray_simd), simd_mad_ps( line_simd.m_mat_21, ray_ori_y(ray_simd), simd_mul_ps( line_simd.m_mat_20, ray_ori_x(ray_simd)) ) ) , line_simd.m_mat_23 );
    
    const simd_data _ray_dir_x = simd_mad_ps( line_simd.m_mat_02, ray_dir_z(ray_simd), simd_mad_ps( line_simd.m_mat_01, ray_dir_y(ray_simd), simd_mul_ps( line_simd.m_mat_00, ray_dir_x(ray_simd) )));
    const simd_data _ray_dir_y = simd_mad_ps( line_simd.m_mat_12, ray_dir_z(ray_simd), simd_mad_ps( line_simd.m_mat_11, ray_dir_y(ray_simd), simd_mul_ps( line_simd.m_mat_10, ray_dir_x(ray_simd) )));
    const simd_data _ray_dir_z = simd_mad_ps( line_simd.m_mat_22, ray_dir_z(ray_simd), simd_mad_ps( line_simd.m_mat_21, ray_dir_y(ray_simd), simd_mul_ps( line_simd.m_mat_20, ray_dir_x(ray_simd) )));

    const simd_data tmp =  simd_div_ps( simd_sub_ps( line_simd.m_w1 , line_simd.m_w0 ) , line_simd.m_length );
    const simd_data tmp0 = simd_mad_ps( _ray_ori_y, tmp, line_simd.m_w0 );
    const simd_data tmp1 = simd_mul_ps( _ray_dir_y, tmp);

    // The 2.0 factor is skipped because it is not needed and will be canceled out.
    const simd_data a = simd_sub_ps( simd_add_ps( simd_sqr_ps( _ray_dir_x ) , simd_sqr_ps( _ray_dir_z ) ) , simd_sqr_ps( tmp1 ) );
    const simd_data b = simd_sub_ps( simd_mad_ps( _ray_dir_x , _ray_ori_x , simd_mul_ps( _ray_dir_z , _ray_ori_z ) ) , simd_mul_ps( tmp0 , tmp1 ) );
    const simd_data c = simd_sub_ps( simd_add_ps( simd_sqr_ps( _ray_ori_x ) , simd_sqr_ps( _ray_ori_z ) ) , simd_sqr_ps( tmp0 ) );

    const simd_data zeros = simd_zero();
    const simd_data discriminant = simd_sub_ps( simd_sqr_ps( b ) , simd_mul_ps( a , c ) );
    mask = simd_and_ps( mask , simd_cmpgt_ps( discriminant , zeros ) );
    auto cm = simd_movemask_ps(mask);
    if (0 == cm)
        return false;
    
    const simd_data sqrt_dist = simd_sqrt_ps( discriminant );
    const simd_data t0 = simd_div_ps( simd_sub_ps( simd_sub_ps( zeros , b ) , sqrt_dist ) , a );
    const simd_data inter_y0 = simd_mad_ps( _ray_dir_y, t0, _ray_ori_y );
    const simd_data mask0 = simd_and_ps( simd_cmplt_ps( inter_y0 , line_simd.m_length ) , simd_cmpgt_ps( inter_y0 , zeros ) );

    const simd_data t1 = simd_div_ps( simd_sub_ps( sqrt_dist , b ) , a );
    const simd_data inter_y1 = simd_mad_ps( _ray_dir_y , t1 , _ray_ori_y );
    inter_y = simd_pick_ps( mask0 , inter_y0 , inter_y1 );
    const simd_data mask1 = simd_and_ps( simd_cmplt_ps( inter_y , line_simd.m_length ) , simd_cmpgt_ps( inter_y , zeros ) );
    mask = simd_and_ps( mask , mask1 );
    cm = simd_movemask_ps(mask);
    if (0 == cm)
        return false;

    t_simd = simd_pick_ps( mask0 , t0 , t1 );
    t_simd = simd_pick_ps( mask , t_simd , simd_infinites );

    const simd_data ray_min_t = simd_set_ps1(ray.m_fMin);
    const simd_data ray_max_t = simd_set_ps1(ray.m_fMax);
    mask = simd_and_ps( mask , simd_and_ps( simd_cmpgt_ps( t_simd , ray_min_t ) , simd_cmplt_ps( t_simd , ray_max_t ) ) );
    cm = simd_movemask_ps(mask);
    if (0 == cm)
        return false;
    
    inter_x = simd_pick_ps( mask0, simd_mad_ps(t0, _ray_dir_x, _ray_ori_x), simd_mad_ps(t1, _ray_dir_x, _ray_ori_x) );
    inter_z = simd_pick_ps( mask0, simd_mad_ps(t0, _ray_dir_z, _ray_ori_z), simd_mad_ps(t1, _ray_dir_z, _ray_ori_z) );

    return true;
}

//! @brief  With the power of SIMD, this utility function helps intersect a ray with four lines at the cost of one.
//!
//! @param  ray         Ray to be tested against.
//! @param  ray_simd    Resolved simd ray data.
//! @param  line_simd   Data structure holds four lines.
//! @param  ret         The result of intersection.
//! @return             Whether there is any intersection that is valid.
SORT_FORCEINLINE bool intersectLine_SIMD( const Ray& ray , const Simd_Ray_Data& ray_simd, const Simd_Line& line_simd , SurfaceInteraction* ret ){
#ifndef SIMD_LINE_REFERENCE_IMPLEMENTATION
    sAssert(IS_PTR_VALID(ret), SPATIAL_ACCELERATOR );

    simd_data  mask, t_simd , inter_x , inter_y , inter_z ;
    const auto intersected = intersectLine_Inner( ray , ray_simd, line_simd , mask , t_simd , inter_x , inter_y , inter_z );
    if( !intersected )
        return false;
    
    mask = simd_and_ps( mask , simd_cmplt_ps( t_simd , simd_set_ps1(ret->t) ) );
    const auto cm = simd_movemask_ps(mask);
    if (0 == cm)
        return false;

    // find the closest result
    simd_data t_min = simd_minreduction_ps( t_simd );

    // get the index of the closest one
    const auto resolved_mask = simd_movemask_ps( simd_cmpeq_ps( t_simd , t_min ) );
    const auto res_i = __bsf(resolved_mask);
    const auto line = line_simd.m_ori_line[res_i];

    ret->intersect = ray( t_simd[res_i] );

    if( inter_y[res_i] == line->m_length ){
        // A corner case where the tip of the line is being intersected.
        ret->gnormal = normalize( line->m_world2Line.GetInversed().TransformVector( Vector( 0.0f , 1.0f , 0.0f ) ) );
        ret->normal = ret->gnormal;
        ret->tangent = normalize( line->m_world2Line.GetInversed().TransformVector( Vector( 1.0f , 0.0f , 0.0f ) ) );
    }else{
        // This may not be physically correct, but it should be fine for a pixel width line.
        ret->gnormal = normalize(line->m_world2Line.GetInversed().TransformVector( Vector( inter_x[res_i], 0.0f , inter_z[res_i] ) ) );
        ret->normal = ret->gnormal;
        ret->tangent = normalize( line->m_gp1 - line->m_gp0 );

        ret->view = -ray.m_Dir;
    }

    ret->u = 1.0f;
    ret->v = slerp( line->m_v0 , line->m_v1 , inter_y[res_i] / line->m_length );
    ret->t = t_simd[res_i];

    ret->primitive = line_simd.m_ori_pri[res_i];

    return true;
#else
    bool ret_val = false;
    for( auto i = 0u ; i < SIMD_CHANNEL && IS_PTR_VALID(line_simd.m_ori_pri[i]) ; ++i )
        ret_val |= line_simd.m_ori_pri[i]->GetIntersect( ray , ret );
    return ret_val;
#endif
}

//! @brief  With the power of SIMD, this utility function helps intersect a ray with four lines at the cost of one.
//!
//! @param  ray         Ray to be tested against.
//! @param  ray_simd    Resolved simd ray data.
//! @param  line_simd   Data structure holds four lines.
//! @return             Whether there is any intersection that is valid.
SORT_FORCEINLINE bool intersectLineFast_SIMD( const Ray& ray , const Simd_Ray_Data& ray_simd, const Simd_Line& line_simd ){
#ifndef SIMD_LINE_REFERENCE_IMPLEMENTATION
    simd_data dummy_mask , dummy_t , dummy_inter_x , dummy_inter_y , dummy_inter_z;
    return intersectLine_Inner( ray , ray_simd , line_simd , dummy_mask , dummy_t , dummy_inter_x , dummy_inter_y , dummy_inter_z );
#else
    bool ret = false;
    for( auto i = 0u ; i < SIMD_CHANNEL && IS_PTR_VALID(line_simd.m_ori_pri[i]) && !ret ; ++i )
        ret |= line_simd.m_ori_pri[i]->GetIntersect( ray , nullptr );
    return ret;
#endif
}

#endif // SIMD_4WAY_IMPLEMENTATION || SIMD_8WAY_IMPLEMENTATION
