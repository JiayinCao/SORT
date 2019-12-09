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

// WARNING, following instructions will results in different results in SIMD and non-SIMD version.
//  - 0.0f / 0.0f    ( SIMD, Nan 0x7fc00000 ) ( Non-SIMD, Nan 0xffc00000 )
//  - Nan != Nan     ( SIMD, 0xffffffff )     ( Non-SIMD, false )

#include <float.h>
#include "core/define.h"

#if defined(SIMD_SSE_IMPLEMENTATION) && defined(SIMD_AVX_IMPLEMENTATION)
    static_assert( false , "More than one SIMD version is defined before including the wrapper." );
#endif

const static unsigned mask_true_i = 0xffffffff;
const static float mask_true = *((float*)&( mask_true_i ));

#ifdef SSE_ENABLED
#include <nmmintrin.h>

#ifndef SORT_IN_WINDOWS
#define simd_data_sse   __m128
#else
// somehow this data structure can introduce huge performance problem on MacOS
// since it is purely for [] operator, it is only used on Windows, where there is no
// performance issue by using it.
struct simd_data_sse{
    union{
        __m128  sse_data;
        float   float_data[4];
    };

    SORT_FORCEINLINE simd_data_sse(){}
    SORT_FORCEINLINE simd_data_sse( const __m128& data ):sse_data(data){}
    
    SORT_FORCEINLINE float  operator []( const int i ) const{
        return float_data[i];
    }
    SORT_FORCEINLINE float& operator []( const int i ){
        return float_data[i];
    }
};
#endif

static SORT_FORCEINLINE __m128 get_sse_data( const simd_data_sse& d ){
#ifdef SORT_IN_WINDOWS
    return d.sse_data;
#else
    return d;
#endif
}

// zero tolerance in any extra size in this structure.
static_assert( sizeof( simd_data_sse ) == sizeof( __m128 ) , "Incorrect SSE data size." );

#ifdef  SIMD_SSE_IMPLEMENTATION

static const __m128 sse_zeros       = _mm_set_ps1( 0.0f );
static const __m128 sse_infinites   = _mm_set_ps1( FLT_MAX );
static const __m128 sse_neg_ones    = _mm_set_ps1( -1.0f );
static const __m128 sse_ones        = _mm_set_ps1(1.0f);

#define simd_data       simd_data_sse
#define simd_ones       sse_ones
#define simd_zeros      sse_zeros
#define simd_neg_ones   sse_neg_ones
#define simd_infinites  sse_infinites

#define SIMD_CHANNEL    4
#define SIMD_ALIGNMENT  16

static SORT_FORCEINLINE simd_data   simd_zero(){
    // this is slower on my 2017 iMac somehow.
    // It could be a hardware issue, since MacOS is the major platform I used for development
    // this is more of a workaround to make it faster.
    // return _mm_setzero_ps();
    
    return _mm_set_ps1( 0.0f );
}
static SORT_FORCEINLINE simd_data   simd_set_ps1( const float d ){
    return _mm_set_ps1( d );
}
static SORT_FORCEINLINE simd_data   simd_set_ps( const float d[] ){
    return _mm_set_ps( d[3] , d[2] , d[1] , d[0] );
}
static SORT_FORCEINLINE simd_data   simd_set_mask(const bool mask[]) {
#define MASK_TO_INT(m)  (m?mask_true:0)
    return _mm_set_ps(MASK_TO_INT(mask[3]), MASK_TO_INT(mask[2]), MASK_TO_INT(mask[1]), MASK_TO_INT(mask[0]));
#undef MASK_TO_INT
}
static SORT_FORCEINLINE simd_data   simd_add_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_add_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_sub_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_sub_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_mul_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_mul_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_div_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_div_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_sqr_ps( const simd_data& m ){
    return _mm_mul_ps( get_sse_data(m) , get_sse_data(m) );
}
static SORT_FORCEINLINE simd_data   simd_sqrt_ps( const simd_data& m ){
    return _mm_sqrt_ps( get_sse_data(m) );
}
static SORT_FORCEINLINE simd_data   simd_rcp_ps( const simd_data& m ){
    return _mm_div_ps( sse_ones , get_sse_data(m) );
}
static SORT_FORCEINLINE simd_data   simd_mad_ps( const simd_data& a , const simd_data& b , const simd_data& c ){
    return _mm_add_ps( _mm_mul_ps( get_sse_data(a) , get_sse_data(b) ) , get_sse_data(c) );
}
static SORT_FORCEINLINE simd_data   simd_pick_ps( const simd_data& mask , const simd_data& a , const simd_data& b ){
    return _mm_blendv_ps( get_sse_data(b) , get_sse_data(a) , mask );
}
static SORT_FORCEINLINE simd_data   simd_cmpeq_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmpeq_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_cmpneq_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmpneq_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_cmple_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmple_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_cmplt_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmplt_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_cmpge_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmpge_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_cmpgt_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmpgt_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_and_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_and_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_or_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_or_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE int         simd_movemask_ps( const simd_data& mask ){
    return _mm_movemask_ps( get_sse_data(mask) );
}
static SORT_FORCEINLINE simd_data   simd_min_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_min_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_max_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_max_ps( get_sse_data(s0) , get_sse_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_minreduction_ps( const simd_data& s ){
    const __m128 t_min = _mm_min_ps( get_sse_data(s) , _mm_shuffle_ps( get_sse_data(s) , get_sse_data(s) , _MM_SHUFFLE(2, 3, 0, 1) ) );
    return _mm_min_ps( t_min , _mm_shuffle_ps(t_min, t_min, _MM_SHUFFLE(1, 0, 3, 2) ) );
}

#endif // SIMD_SSE_IMPLEMENTATION
#endif // SSE_ENABLED

#ifdef  AVX_ENABLED

#include <immintrin.h>

#ifndef SORT_IN_WINDOWS
#define simd_data_avx   __m256
#else
struct simd_data_avx {
    union {
        __m256  avx_data;
        float   float_data[8];
    };

    SORT_FORCEINLINE simd_data_avx() {}
    SORT_FORCEINLINE simd_data_avx(const __m256& data) :avx_data(data) {}

    SORT_FORCEINLINE float  operator [](const int i) const {
        return float_data[i];
    }
    SORT_FORCEINLINE float& operator [](const int i) {
        return float_data[i];
    }
};
#endif

static SORT_FORCEINLINE __m256 get_avx_data( const simd_data_avx& d ){
#ifdef SORT_IN_WINDOWS
    return d.avx_data;
#else
    return d;
#endif
}

// zero tolerance in any extra size in this structure.
static_assert(sizeof(simd_data_avx) == sizeof(__m256), "Incorrect AVX data size.");

#ifdef SIMD_AVX_IMPLEMENTATION

static const __m256 avx_zeros       = _mm256_set_ps(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
static const __m256 avx_infinites   = _mm256_set_ps(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX );
static const __m256 avx_neg_ones    = _mm256_set_ps(-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f);
static const __m256 avx_ones        = _mm256_set_ps(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

#define simd_data       simd_data_avx
#define simd_ones       avx_ones
#define simd_zeros      avx_zeros
#define simd_neg_ones   avx_neg_ones
#define simd_infinites  avx_infinites

#define SIMD_CHANNEL    8
#define SIMD_ALIGNMENT  32

static SORT_FORCEINLINE simd_data   simd_zero(){
    return _mm256_setzero_ps();
}
static SORT_FORCEINLINE simd_data   simd_set_ps1( const float f ){
    return _mm256_set_ps(f,f,f,f,f,f,f,f);
}
static SORT_FORCEINLINE simd_data   simd_set_ps( const float d[] ){
    return _mm256_set_ps( d[7] , d[6] , d[5] , d[4] , d[3] , d[2] , d[1] , d[0] );
}
static SORT_FORCEINLINE simd_data   simd_set_mask(const bool mask[]) {
#define MASK_TO_INT(m)  (m?mask_true:0.0f)
    return _mm256_set_ps( MASK_TO_INT( mask[7] ) , MASK_TO_INT( mask[6] ) , MASK_TO_INT( mask[5] ) , MASK_TO_INT( mask[4] ) , MASK_TO_INT( mask[3] ) , MASK_TO_INT( mask[2] ) , MASK_TO_INT( mask[1] ) , MASK_TO_INT( mask[0] ) );
#undef MASK_TO_INT
}
static SORT_FORCEINLINE simd_data   simd_add_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_add_ps( get_avx_data(s0) , get_avx_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_sub_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_sub_ps( get_avx_data(s0) , get_avx_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_mul_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_mul_ps( get_avx_data(s0) , get_avx_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_div_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_div_ps( get_avx_data(s0) , get_avx_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_sqr_ps( const simd_data& m ){
    return _mm256_mul_ps( get_avx_data(m) , get_avx_data(m) );
}
static SORT_FORCEINLINE simd_data   simd_sqrt_ps( const simd_data& m ){
    return _mm256_sqrt_ps( get_avx_data(m) );
}
static SORT_FORCEINLINE simd_data   simd_rcp_ps( const simd_data& m ){
    return _mm256_div_ps( avx_ones , get_avx_data(m) );
}
static SORT_FORCEINLINE simd_data   simd_mad_ps( const simd_data& a , const simd_data& b , const simd_data& c ){
    return _mm256_add_ps( _mm256_mul_ps( get_avx_data(a) , get_avx_data(b) ) , get_avx_data(c) );
}
static SORT_FORCEINLINE simd_data   simd_pick_ps( const simd_data& mask , const simd_data& a , const simd_data& b ){
    return _mm256_blendv_ps( get_avx_data(b) , get_avx_data(a) , mask );
}
static SORT_FORCEINLINE simd_data   simd_cmpeq_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_cmp_ps( get_avx_data(s0) , get_avx_data(s1) , _CMP_EQ_OQ );
}
static SORT_FORCEINLINE simd_data   simd_cmpneq_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_cmp_ps( get_avx_data(s0) , get_avx_data(s1) , _CMP_NEQ_OQ );
}
static SORT_FORCEINLINE simd_data   simd_cmple_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_cmp_ps( get_avx_data(s0) , get_avx_data(s1) , _CMP_LE_OQ );
}
static SORT_FORCEINLINE simd_data   simd_cmplt_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_cmp_ps( get_avx_data(s0) , get_avx_data(s1) , _CMP_LT_OQ );
}
static SORT_FORCEINLINE simd_data   simd_cmpge_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_cmp_ps( get_avx_data(s0) , get_avx_data(s1) , _CMP_GE_OQ );
}
static SORT_FORCEINLINE simd_data   simd_cmpgt_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_cmp_ps( get_avx_data(s0) , get_avx_data(s1) , _CMP_GT_OQ );
}
static SORT_FORCEINLINE simd_data   simd_and_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_and_ps( get_avx_data(s0) , get_avx_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_or_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_or_ps( get_avx_data(s0) , get_avx_data(s1) );
}
static SORT_FORCEINLINE int         simd_movemask_ps( const simd_data& mask ){
    return _mm256_movemask_ps( get_avx_data(mask) );
}
static SORT_FORCEINLINE simd_data   simd_min_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_min_ps( get_avx_data(s0) , get_avx_data(s1) );
}
static SORT_FORCEINLINE simd_data   simd_max_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm256_max_ps( get_avx_data(s0) , get_avx_data(s1) );
}

static SORT_FORCEINLINE simd_data   simd_minreduction_ps( const simd_data& s ){
    const static __m256i shuffle_mask0 = _mm256_set_epi32(1, 0, 3, 2, 5, 4, 7, 6);
    const static __m256i shuffle_mask1 = _mm256_set_epi32(2, 3, 0, 1, 6, 7, 4, 5);

    __m256 avx_data = get_avx_data(s);
    __m256 shuffled = _mm256_permutevar_ps(avx_data, shuffle_mask1);
    avx_data = _mm256_min_ps(shuffled, avx_data);
    shuffled = _mm256_permutevar_ps(avx_data, shuffle_mask0);
    simd_data_avx reduced_data = _mm256_min_ps(shuffled, avx_data);
    return simd_set_ps1( reduced_data[0] < reduced_data[4] ? reduced_data[0] : reduced_data[4] );
}

#endif

#endif

static SORT_FORCEINLINE int __bsf(int v) {
#ifdef SORT_IN_WINDOWS
    unsigned long r = 0;
    _BitScanForward(&r, v);
    return r;
#else
    return __builtin_ctz(v);
#endif
}