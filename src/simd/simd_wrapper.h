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

#ifdef SSE_ENABLED
#include <nmmintrin.h>

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

#ifdef  SIMD_SSE_IMPLEMENTATION

static const __m128 sse_zeros       = _mm_set_ps1( 0.0f );
static const __m128 sse_infinites   = _mm_set_ps1( FLT_MAX );
static const __m128 sse_neg_ones	= _mm_set_ps1( -1.0f );
static const __m128 sse_ones		= _mm_set_ps1(1.0f);

#define simd_data       simd_data_sse
#define simd_ones       sse_ones
#define simd_zeros      sse_zeros
#define simd_neg_ones   sse_neg_ones
#define simd_infinites  sse_infinites

static SORT_FORCEINLINE simd_data   simd_set_ps1( const float d ){
    return _mm_set_ps1( d );
}
static SORT_FORCEINLINE simd_data   simd_set_ps( const float d[] ){
    return _mm_set_ps( d[3] , d[2] , d[1] , d[0] );
}
static SORT_FORCEINLINE simd_data   simd_add_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_add_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_sub_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_sub_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_mul_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_mul_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_div_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_div_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_sqr_ps( const simd_data& m ){
    return _mm_mul_ps( m.sse_data , m.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_rcp_ps( const simd_data& m ){
    return _mm_div_ps( sse_ones , m.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_mad_ps( const simd_data& a , const simd_data& b , const simd_data& c ){
    return _mm_add_ps( _mm_mul_ps( a.sse_data , b.sse_data ) , c.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_pick_ps( const simd_data& mask , const simd_data& a , const simd_data& b ){
    return _mm_or_ps( _mm_and_ps( mask.sse_data , a.sse_data ) , _mm_andnot_ps( mask.sse_data , b.sse_data ) );
}
static SORT_FORCEINLINE simd_data   simd_cmpeq_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmpeq_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_cmpneq_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmpneq_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_cmple_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmple_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_cmplt_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmplt_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_cmpge_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmpge_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_cmpgt_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_cmpgt_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_and_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_and_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_or_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_or_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE int         simd_movemask_ps( const simd_data& mask ){
    return _mm_movemask_ps( mask.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_min_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_min_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_max_ps( const simd_data& s0 , const simd_data& s1 ){
    return _mm_max_ps( s0.sse_data , s1.sse_data );
}
static SORT_FORCEINLINE simd_data   simd_minreduction_ps( const simd_data& s ){
    __m128 t_min = _mm_min_ps( s.sse_data , _mm_shuffle_ps( s.sse_data , s.sse_data , _MM_SHUFFLE(2, 3, 0, 1) ) );
	t_min = _mm_min_ps( t_min , _mm_shuffle_ps(t_min, t_min, _MM_SHUFFLE(1, 0, 3, 2) ) );
    return t_min;
}

static SORT_FORCEINLINE float sse_data( const simd_data& s , const int index ){
#ifdef SORT_IN_WINDOWS
	return s.sse_data.m128_f32[index];
#else
	return s.sse_data[index];
#endif
}

#endif // SIMD_SSE_IMPLEMENTATION
#endif // SSE_ENABLED

#ifdef  SIMD_AVX_IMPLEMENTATION

#endif