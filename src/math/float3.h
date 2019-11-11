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

#include <stdio.h>
#include <nmmintrin.h>
#include "core/define.h"

//! @brief  float3 is a low level data structure that holds three floats.
/**
  * It heavily uses SSE operations to accelerate the computation process.
  * The SSE operations could also be disabled by the configuration of the project.
  * The catch of SSE version float3 implementation is that it is actually a 4-channel
  * float data structure instead of three. This is both for 128 bits alignment and
  * fitting the SSE data structure.
  */
struct float3{

    //! @brief  Default constructor initialize values with zeros.
    SORT_FORCEINLINE float3() : data(_mm_set_ps1(0.0f)) {}

    //! @brief  Constructor that takes only one value and populates it to all channels.
    //!
    //! @param v    Value to be populated in all channels of the internal float3.
    SORT_FORCEINLINE float3( const float v ) : data(_mm_set_ps(0.0f,v,v,v)) {}

    //! @brief  Constructor that takes three different values.
    //!
    //! @param v0   Value to fill the first channel of SSE data.
    //! @param v1   Value to fill the second channel of SSE data.
    //! @param v2   Value to fill the third channel of SSE data.
    SORT_FORCEINLINE float3( const float v0 , const float v1 , const float v2 ) : data( _mm_set_ps(0.0f,v2,v1,v0) ) {}

    //! @brief  Constructor that takes __m128.
    //!
    //! @param v    Value to be populated in all channels of the internal float3.
    SORT_FORCEINLINE float3( const __m128 v ) : data(v) {}

    //! @brief  = operator of float3.
    //!
    //! @param f0   Value to be copied to.
    //! @param f1   Value to be copied from.
    //! @return     Value of the second float3.
    SORT_FORCEINLINE float3&   operator =( const float3& f1 ){
        data = f1.data;
        return *this;
    }

    //! @brief  [] operator to retreive per channel data.
    //!
    //! There will be no safety checking for performance reason since this is low level code.
    //! It is up to whoever calls this function to make sure the index is alwasy valid.
    //!
    //! @param i    Channel to be retrieved, it has to be 0/1/2.
    //! @return     The data stored in the channel.
    SORT_FORCEINLINE float    operator []( const int i ) const{
        return f[i];
    }

    //! @brief  [] operator to retreive per channel data.
    //!
    //! There will be no safety checking for performance reason since this is low level code.
    //! It is up to whoever calls this function to make sure the index is alwasy valid.
    //!
    //! @param i    Channel to be retrieved, it has to be 0/1/2.
    //! @return     The data stored in the channel.
    SORT_FORCEINLINE float&    operator []( const int i ){
        return f[i];
    }

    //! @brief  Whether the float3 is ( 0.0f , 0.0f , 0.0f )
    //!
    //! @return     True if the float3 is ( 0.0f , 0.0f , 0.0f )
    SORT_FORCEINLINE bool       isZero() const{
        return _mm_movemask_ps(_mm_cmpeq_ps( data , _mm_set_ps1(0.0f) )) == 0xf;
    }

    union{
        __m128  data;   /**< Although it is a three channel floats, but SORT will add another implicit channel for better perforamnce. */
        struct { float x , y , z , w; };
        struct { float r , g , b , a; };
        float   f[4];
    };
};

// make sure float3 is 16 bytes
static_assert( sizeof( float3 ) == 16 , "Incorrect float3 size." );

SORT_FORCEINLINE float3   operator  +( const float3& f0 , const float3& f1 ) {
    return _mm_add_ps( f0.data , f1.data );
}

SORT_FORCEINLINE float3   operator  +( const float3& f0 , const float f1 ) {
    return _mm_add_ps( f0.data , _mm_set_ps( 0.0f , f1 , f1 , f1 ) );
}

SORT_FORCEINLINE float3   operator  +( float f0 , const float3& f1 ){
    return _mm_add_ps( f1.data , _mm_set_ps( 0.0f , f0 , f0 , f0 ) );
}

SORT_FORCEINLINE float3   operator  +=( float3& f0 , const float3& f1 ){
    return f0 = f0 + f1;
}

SORT_FORCEINLINE float3   operator  +=( float3& f0 , const float f1 ){
    return f0 = f0 + f1;
}

SORT_FORCEINLINE float3   operator  -( const float3& f ) {
    return _mm_sub_ps( _mm_set_ps1( 0.0f ) , f.data );
}

SORT_FORCEINLINE float3   operator  -( const float3& f0 , const float3& f1 ) {
    return _mm_sub_ps( f0.data , f1.data );
}

SORT_FORCEINLINE float3   operator  -( const float3& f0 , const float f1 ) {
    return _mm_sub_ps( f0.data , _mm_set_ps(0.0f, f1, f1, f1) );
}

SORT_FORCEINLINE float3   operator  -=( float3& f0 , const float3& f1 ){
    return f0 = f0 - f1;
}

SORT_FORCEINLINE float3   operator  -=( float3& f0 , const float f1 ) {
    return f0 = f0 - f1;
}

SORT_FORCEINLINE float3   operator  *( const float3& f0 , const float3& f1 ) {
    return _mm_mul_ps( f0.data , f1.data );
}

SORT_FORCEINLINE float3   operator  *( const float3& f0 , const float f1 ) {
    return _mm_mul_ps( f0.data , _mm_set_ps(0.0f, f1, f1, f1) );
}

SORT_FORCEINLINE float3   operator  *( const float f0 , const float3& f1 ) {
    return _mm_mul_ps( f1.data , _mm_set_ps(0.0f, f0, f0, f0) );
}

SORT_FORCEINLINE float3   operator  *=( const float f0 , const float3& f1 ) {
    return _mm_mul_ps( f1.data , _mm_set_ps(0.0f, f0, f0, f0) );
}

SORT_FORCEINLINE float3   operator  *=( float3& f0 , const float3& f1 ) {
    return f0 = f0 * f1;
}

SORT_FORCEINLINE float3   operator  *=( float3& f0 , const float f1 ) {
    return f0 = f0 * f1;
}

SORT_FORCEINLINE float3   operator  /( const float3& f0 , const float3& f1 ) {
    return _mm_div_ps( f0.data , f1.data );
}

SORT_FORCEINLINE float3   operator  /( const float3& f0 , const float f1 ) {
    return _mm_div_ps( f0.data , _mm_set_ps(0.0f, f1, f1, f1) );
}

SORT_FORCEINLINE float3   operator  /=( float3& f0 , const float3& f1 ) {
    return f0 = f0 / f1;
}

SORT_FORCEINLINE float3   operator  /=( float3& f0 , const float f1 ) {
    return f0 = f0 / f1;
}

SORT_FORCEINLINE bool     operator  ==( const float3& f0 , const float3& f1 ){
    return _mm_movemask_ps(_mm_cmpeq_ps( f0.data , f1.data )) == 0xf;
}

SORT_FORCEINLINE bool     operator  !=( const float3& f0 , const float3& f1 ){
    return _mm_movemask_ps(_mm_cmpeq_ps( f0.data , f1.data )) == 0x0;
}