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

#if defined(SIMD_AVX_IMPLEMENTATION)
    static_assert( false , "No 8 lane width simd available on Arm." );
#endif

const static unsigned mask_true_i = 0xffffffff;
const static float mask_true = *((float*)&( mask_true_i ));

#ifdef SSE_ENABLED
#include<arm_neon.h>

#define __m128 float32x4_t

#ifndef SORT_IN_WINDOWS
#define simd_data_sse   float32x4_t
#else
// somehow this data structure can introduce huge performance problem on MacOS
// since it is purely for [] operator, it is only used on Windows, where there is no
// performance issue by using it.
struct simd_data_sse{
    union{
        float32x4_t     sse_data;
        float           float_data[4];
    };

    SORT_FORCEINLINE simd_data_sse(){}
    SORT_FORCEINLINE simd_data_sse( const float32x4_t& data ):sse_data(data){}
    
    SORT_FORCEINLINE float  operator []( const int i ) const{
        return float_data[i];
    }
    SORT_FORCEINLINE float& operator []( const int i ){
        return float_data[i];
    }
};
#endif

SORT_STATIC_FORCEINLINE float32x4_t get_sse_data( const simd_data_sse& d ){
#ifdef SORT_IN_WINDOWS
    return d.sse_data;
#else
    return d;
#endif
}

// zero tolerance in any extra size in this structure.
static_assert( sizeof( simd_data_sse ) == sizeof( float32x4_t ) , "Incorrect SSE data size." );

#ifdef  SIMD_SSE_IMPLEMENTATION

static const float32x4_t sse_zeros       = { 0.0f, 0.0f, 0.0f, 0.0f };
static const float32x4_t sse_infinites   = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
static const float32x4_t sse_neg_ones    = { -1.0f, -1.0f, -1.0f, -1.0f };
static const float32x4_t sse_ones        = { 1.0f, 1.0f, 1.0f, 1.0f };

#define simd_data       simd_data_sse
#define simd_ones       sse_ones
#define simd_zeros      sse_zeros
#define simd_neg_ones   sse_neg_ones
#define simd_infinites  sse_infinites

#define SIMD_CHANNEL    4
#define SIMD_ALIGNMENT  16

SORT_STATIC_FORCEINLINE simd_data   simd_zero(){
    // this is slower on my 2017 iMac somehow.
    // It could be a hardware issue, since MacOS is the major platform I used for development
    // this is more of a workaround to make it faster.
    // return _mm_setzero_ps();
    return { 0.0f, 0.0f, 0.0f, 0.0f };
}
SORT_STATIC_FORCEINLINE simd_data   simd_set_ps1( const float d ){
    return { d , d , d , d };
}
SORT_STATIC_FORCEINLINE simd_data   simd_set_ps( const float d[] ){
    return vld1q_f32( d );
}
SORT_STATIC_FORCEINLINE simd_data   simd_set_mask(const bool mask[]) {
#define MASK_TO_INT(m)  (m?mask_true:0)
    //return _mm_set_ps(MASK_TO_INT(mask[3]), MASK_TO_INT(mask[2]), MASK_TO_INT(mask[1]), MASK_TO_INT(mask[0]));
#undef MASK_TO_INT
}
SORT_STATIC_FORCEINLINE simd_data   simd_add_ps( const simd_data& s0 , const simd_data& s1 ){
 //   return _mm_add_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_sub_ps( const simd_data& s0 , const simd_data& s1 ){
  //  return _mm_sub_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_mul_ps( const simd_data& s0 , const simd_data& s1 ){
  //  return _mm_mul_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_div_ps( const simd_data& s0 , const simd_data& s1 ){
  //  return _mm_div_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_sqr_ps( const simd_data& m ){
  //  return _mm_mul_ps( get_sse_data(m) , get_sse_data(m) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_sqrt_ps( const simd_data& m ){
  //  return _mm_sqrt_ps( get_sse_data(m) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_rcp_ps( const simd_data& m ){
  //  return _mm_div_ps( sse_ones , get_sse_data(m) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_mad_ps( const simd_data& a , const simd_data& b , const simd_data& c ){
  //  return _mm_add_ps( _mm_mul_ps( get_sse_data(a) , get_sse_data(b) ) , get_sse_data(c) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_pick_ps( const simd_data& mask , const simd_data& a , const simd_data& b ){
  //  return _mm_blendv_ps( get_sse_data(b) , get_sse_data(a) , get_sse_data(mask) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmpeq_ps( const simd_data& s0 , const simd_data& s1 ){
   // return _mm_cmpeq_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmpneq_ps( const simd_data& s0 , const simd_data& s1 ){
   // return _mm_cmpneq_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmple_ps( const simd_data& s0 , const simd_data& s1 ){
  //  return _mm_cmple_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmplt_ps( const simd_data& s0 , const simd_data& s1 ){
   // return _mm_cmplt_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmpge_ps( const simd_data& s0 , const simd_data& s1 ){
  //  return _mm_cmpge_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_cmpgt_ps( const simd_data& s0 , const simd_data& s1 ){
  //  return _mm_cmpgt_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_and_ps( const simd_data& s0 , const simd_data& s1 ){
//    return _mm_and_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_or_ps( const simd_data& s0 , const simd_data& s1 ){
 //   return _mm_or_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE int         simd_movemask_ps( const simd_data& mask ){
  //  return _mm_movemask_ps( get_sse_data(mask) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_min_ps( const simd_data& s0 , const simd_data& s1 ){
 //   return _mm_min_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_max_ps( const simd_data& s0 , const simd_data& s1 ){
//    return _mm_max_ps( get_sse_data(s0) , get_sse_data(s1) );
}
SORT_STATIC_FORCEINLINE simd_data   simd_minreduction_ps( const simd_data& s ){
  //  const __m128 t_min = _mm_min_ps( get_sse_data(s) , _mm_shuffle_ps( get_sse_data(s) , get_sse_data(s) , _MM_SHUFFLE(2, 3, 0, 1) ) );
  //  return _mm_min_ps( t_min , _mm_shuffle_ps(t_min, t_min, _MM_SHUFFLE(1, 0, 3, 2) ) );
}

#endif // SIMD_SSE_IMPLEMENTATION
#endif // SSE_ENABLED

SORT_STATIC_FORCEINLINE int __bsf(int v) {
#ifdef SORT_IN_WINDOWS
    unsigned long r = 0;
    _BitScanForward(&r, v);
    return r;
#else
    return __builtin_ctz(v);
#endif
}