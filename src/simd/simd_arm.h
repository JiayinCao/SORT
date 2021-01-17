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

// WARNING, following instructions will results in different results in SIMD and non-SIMD version.
//  - 0.0f / 0.0f    ( SIMD, Nan 0x7fc00000 ) ( Non-SIMD, Nan 0xffc00000 )
//  - Nan != Nan     ( SIMD, 0xffffffff )     ( Non-SIMD, false )

#include <float.h>
#include "core/define.h"

#if defined(SIMD_8WAY_IMPLEMENTATION)
    static_assert( false , "No 8 lane width simd available on Arm." );
#endif

const static unsigned mask_true_i = 0xffffffff;
const static float mask_true = *((float*)&( mask_true_i ));

#ifdef SIMD_4WAY_ENABLED
#include<arm_neon.h>

#ifndef SORT_IN_WINDOWS
struct simd_data_neon
{
    union{
        float32x4_t   neon_data_f4;
        uint32x4_t    neon_data_u4;
        float         neon_data_raw_f4[4];
    };

    SORT_FORCEINLINE simd_data_neon(const float32x4_t& data) : neon_data_f4(data){}
    SORT_FORCEINLINE simd_data_neon(const uint32x4_t& data) : neon_data_u4(data){}
    SORT_FORCEINLINE simd_data_neon() {}
    SORT_FORCEINLINE float  operator []( const int i ) const{
        return neon_data_f4[i];
    }

    SORT_FORCEINLINE float&  operator []( const int i ){
        return neon_data_raw_f4[i];
    }
};
#define simd_data_sse   simd_data_neon  // to be removed
#else
#error "Not implemented yet."
#endif

SORT_STATIC_FORCEINLINE float32x4_t get_neon_data( const simd_data_neon& d ){
#ifdef SORT_IN_WINDOWS
    #error "Not implemented yet."
#else
    return d.neon_data_f4;
#endif
}

// zero tolerance in any extra size in this structure.
static_assert( sizeof( simd_data_neon ) == sizeof( float32x4_t ) , "Incorrect Neon data size." );

#ifdef  SIMD_4WAY_IMPLEMENTATION

static const float32x4_t neon_zeros       = { 0.0f, 0.0f, 0.0f, 0.0f };
static const float32x4_t neon_infinites   = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
static const float32x4_t neon_neg_ones    = { -1.0f, -1.0f, -1.0f, -1.0f };
static const float32x4_t neon_ones        = { 1.0f, 1.0f, 1.0f, 1.0f };


#define simd_data       simd_data_neon
#define simd_ones       neon_ones
#define simd_zeros      neon_zeros
#define simd_neg_ones   neon_neg_ones
#define simd_infinites  neon_infinites

#define SIMD_CHANNEL    4
#define SIMD_ALIGNMENT  16

SORT_STATIC_FORCEINLINE simd_data   simd_zero(){
    // this is slower on my 2017 iMac somehow.
    // It could be a hardware issue, since MacOS is the major platform I used for development
    // this is more of a workaround to make it faster.
    // return _mm_setzero_ps();
    return (float32x4_t){ 0.0f, 0.0f, 0.0f, 0.0f };
}
SORT_STATIC_FORCEINLINE simd_data   simd_set_ps1( const float d ){
    return (float32x4_t){ d , d , d , d };
}
SORT_STATIC_FORCEINLINE simd_data   simd_set_ps( const float d[] ){
    return vld1q_f32( d );
}
SORT_STATIC_FORCEINLINE simd_data   simd_set_mask(const bool mask[]) {
#define MASK_TO_INT(m)  (m?mask_true:0)
    return (float32x4_t){MASK_TO_INT(mask[0]), MASK_TO_INT(mask[1]), MASK_TO_INT(mask[2]), MASK_TO_INT(mask[3])};
#undef MASK_TO_INT
}
SORT_STATIC_FORCEINLINE simd_data   simd_add_ps( const simd_data& s0 , const simd_data& s1 ){
    return vaddq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_sub_ps( const simd_data& s0 , const simd_data& s1 ){
    return vsubq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_mul_ps( const simd_data& s0 , const simd_data& s1 ){
    return vmulq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_div_ps( const simd_data& s0 , const simd_data& s1 ){
    return vdivq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_sqr_ps( const simd_data& m ){
    return simd_mul_ps( get_neon_data(m) , get_neon_data(m) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_sqrt_ps( const simd_data& m ){
    return vsqrtq_f32( get_neon_data(m) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_rcp_ps( const simd_data& m ){
    return vdivq_f32( neon_ones , get_neon_data(m) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_mad_ps( const simd_data& a , const simd_data& b , const simd_data& c ){
    return simd_add_ps( simd_mul_ps( get_neon_data(a) , get_neon_data(b) ) , get_neon_data(c) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_pick_ps( const simd_data& mask , const simd_data& a , const simd_data& b ){
    return vbslq_f32(mask.neon_data_u4, get_neon_data(a), get_neon_data(b));
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmpeq_ps( const simd_data& s0 , const simd_data& s1 ){
    return vceqq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmpneq_ps( const simd_data& s0 , const simd_data& s1 ){
    return vmvnq_u32(simd_cmpeq_ps( get_neon_data(s0) , get_neon_data(s1) ).neon_data_u4);
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmple_ps( const simd_data& s0 , const simd_data& s1 ){
    return vcleq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmplt_ps( const simd_data& s0 , const simd_data& s1 ){
    return vcltq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmpge_ps( const simd_data& s0 , const simd_data& s1 ){
    return vcgeq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmpgt_ps( const simd_data& s0 , const simd_data& s1 ){
    return vcgtq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_and_ps( const simd_data& s0 , const simd_data& s1 ){
    return vandq_u32( s0.neon_data_u4 , s1.neon_data_u4 );
}
SORT_STATIC_FORCEINLINE simd_data   simd_or_ps( const simd_data& s0 , const simd_data& s1 ){
    return vorrq_u32( s0.neon_data_u4 , s1.neon_data_u4 );
}
SORT_STATIC_FORCEINLINE int         simd_movemask_ps( const simd_data& mask ){
    const uint32x4_t conditions = mask.neon_data_u4;
    
    // reference implementation for now
    // I need to figure out a better way later.
    int ret = 0;
    if( mask.neon_data_u4[0] & 0x80000000 )
      ret |= 1;
    if( mask.neon_data_u4[1] & 0x80000000 )
      ret |= 2;
    if( mask.neon_data_u4[2] & 0x80000000 )
      ret |= 4;
    if( mask.neon_data_u4[3] & 0x80000000 )
      ret |= 8;
    return ret;
}

SORT_STATIC_FORCEINLINE simd_data   simd_min_ps( const simd_data& s0 , const simd_data& s1 ){
    return vminq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_max_ps( const simd_data& s0 , const simd_data& s1 ){
    return vmaxq_f32( get_neon_data(s0) , get_neon_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_minreduction_ps( const simd_data& s ){
    // there must be a better way!
    return simd_set_ps1( fmin( fmin( s[0], s[1] ) , fmin( s[2], s[3]) ) );
}

#endif // SIMD_4WAY_IMPLEMENTATION
#endif // SIMD_4WAY_ENABLED

SORT_STATIC_FORCEINLINE int __bsf(int v) {
#ifdef SORT_IN_WINDOWS
    unsigned long r = 0;
    _BitScanForward(&r, v);
    return r;
#else
    return __builtin_ctz(v);
#endif
}