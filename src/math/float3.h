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
#endif

//! @brief  float3 is a low level data structure that holds three floats.
/**
  * It heavily uses SSE operations to accelerate the computation process.
  * The SSE operations could also be disabled by the configuration of the project.
  * The catch of SSE version float3 implementation is that it is actually a 4-channel
  * float data structure instead of three. This is both for 128 bits alignment and
  * fitting the SSE data structure.
  * One minor and important detail is that float3 doesn't gurrantee correctness
  * of the implicit forth channel, meaning it is totally possible we will ahve 'nan'
  * in it. However, as long as it doesn't crash the system, it is not a threat.
  */
struct float3{

    //! @brief  Default constructor initialize values with zeros.
    SORT_FORCEINLINE float3() : 
    #ifdef SSE_ENABLED
        data(_mm_set_ps1(0.0f))
    #else
        x(0.0f), y(0.0f), z(0.0f)
    #endif
    {}

    //! @brief  Constructor that takes only one value and populates it to all channels.
    //!
    //! @param v    Value to be populated in all channels of the internal float3.
    SORT_FORCEINLINE float3( const float v ) : 
    #ifdef SSE_ENABLED
        data(_mm_set_ps1(v))
    #else
        x(v),y(v),z(v)
    #endif
    {}

    //! @brief  Constructor that takes three different values.
    //!
    //! @param v0   Value to fill the first channel of SSE data.
    //! @param v1   Value to fill the second channel of SSE data.
    //! @param v2   Value to fill the third channel of SSE data.
    SORT_FORCEINLINE float3( const float v0 , const float v1 , const float v2 ) : 
    #ifdef SSE_ENABLED
        data( _mm_set_ps(0.0f,v2,v1,v0) )
    #else
        x(v0),y(v1),z(v2)
    #endif
    {}

    #ifdef SSE_ENABLED
    //! @brief  Constructor that takes __m128.
    //!
    //! @param v    Value to be populated in all channels of the internal float3.
    SORT_FORCEINLINE float3( const __m128 v ) : data(v) {}
    #endif

    //! @brief  = operator of float3.
    //!
    //! @param v    Value to be copied from.
    //! @return     Value of the second float3.
    SORT_FORCEINLINE float3&   operator =( const float3& v ){
        #ifdef SSE_ENABLED
            data = v.data;
        #else
            x = v.x; y = v.y; z = v.z;
        #endif
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
        #ifdef SSE_ENABLED
            return _mm_movemask_ps(_mm_cmpeq_ps( data , _mm_set_ps1(0.0f) )) == 0xf;
        #else
            return ( x == 0.0f ) && ( y == 0.0f ) && ( z == 0.0f );
        #endif
    }

    union{
#ifdef SSE_ENABLED
        __m128  data;   /**< Although it is a three channel floats, but SORT will add another implicit channel for better perforamnce. */
#endif
        struct { float x , y , z ; };
        struct { float r , g , b ; };
        float   f[3];
    };
};

#ifdef SSE_ENABLED
    static_assert( sizeof( float3 ) == 16 , "Incorrect float3 size." );
#else
    static_assert( sizeof( float3 ) == 12 , "Incorrect float3 size." );
#endif

SORT_FORCEINLINE float3   operator  +( const float3& f0 , const float3& f1 ) {
#ifdef SSE_ENABLED
    return _mm_add_ps( f0.data , f1.data );
#else
    return float3( f0.x + f1.x , f0.y + f1.y , f0.z + f1.z );
#endif
}

SORT_FORCEINLINE float3   operator  +( const float3& f0 , const float f1 ) {
#ifdef SSE_ENABLED
    return _mm_add_ps( f0.data , _mm_set_ps1( f1 ) );
#else
    return float3( f0.x + f1 , f0.y + f1 , f0.z + f1 );
#endif
}

SORT_FORCEINLINE float3   operator  +( float f0 , const float3& f1 ){
#ifdef SSE_ENABLED
    return _mm_add_ps( f1.data , _mm_set_ps1( f0 ) );
#else
    return float3( f1.x + f0 , f1.y + f0 , f1.z + f0 );
#endif
}

SORT_FORCEINLINE float3   operator  +=( float3& f0 , const float3& f1 ){
    return f0 = f0 + f1;
}

SORT_FORCEINLINE float3   operator  +=( float3& f0 , const float f1 ){
    return f0 = f0 + f1;
}

SORT_FORCEINLINE float3   operator  -( const float3& f ) {
#ifdef SSE_ENABLED
    return _mm_sub_ps( _mm_set_ps1( 0.0f ) , f.data );
#else
    return float3( -f.x , -f.y , -f.z );
#endif
}

SORT_FORCEINLINE float3   operator  -( const float3& f0 , const float3& f1 ) {
#ifdef SSE_ENABLED
    return _mm_sub_ps( f0.data , f1.data );
#else
    return float3( f0.x - f1.x , f0.y - f1.y , f0.z - f1.z );
#endif
}

SORT_FORCEINLINE float3   operator  -( const float3& f0 , const float f1 ) {
#ifdef SSE_ENABLED
    return _mm_sub_ps( f0.data , _mm_set_ps1(f1) );
#else
    return float3( f0.x - f1 , f0.y - f1 , f0.z - f1 );
#endif
}

SORT_FORCEINLINE float3   operator  -=( float3& f0 , const float3& f1 ){
    return f0 = f0 - f1;
}

SORT_FORCEINLINE float3   operator  -=( float3& f0 , const float f1 ) {
    return f0 = f0 - f1;
}

SORT_FORCEINLINE float3   operator  *( const float3& f0 , const float3& f1 ) {
#ifdef SSE_ENABLED
    return _mm_mul_ps( f0.data , f1.data );
#else
    return float3( f0.x * f1.x , f0.y * f1.y , f0.z * f1.z );
#endif
}

SORT_FORCEINLINE float3   operator  *( const float3& f0 , const float f1 ) {
#ifdef SSE_ENABLED
    return _mm_mul_ps( f0.data , _mm_set_ps1(f1) );
#else
    return float3( f0.x * f1 , f0.y * f1 , f0.z * f1 );
#endif
}

SORT_FORCEINLINE float3   operator  *( const float f0 , const float3& f1 ) {
#ifdef SSE_ENABLED
    return _mm_mul_ps( f1.data , _mm_set_ps1(f0) );
#else
    return float3( f1.x * f0 , f1.y * f0 , f1.z * f0 );
#endif
}

SORT_FORCEINLINE float3   operator  *=( float3& f0 , const float3& f1 ) {
    return f0 = f0 * f1;
}

SORT_FORCEINLINE float3   operator  *=( float3& f0 , const float f1 ) {
    return f0 = f0 * f1;
}

SORT_FORCEINLINE float3   operator  /( const float3& f0 , const float3& f1 ) {
#ifdef SSE_ENABLED
    return _mm_div_ps( f0.data , f1.data );
#else
    return float3( f0.x / f1.x , f0.y / f1.y , f0.z / f1.z );
#endif
}

SORT_FORCEINLINE float3   operator  /( const float3& f0 , const float f1 ) {
#ifdef SSE_ENABLED
    return _mm_div_ps( f0.data , _mm_set_ps1(f1) );
#else
    return float3( f0.x / f1 , f0.y / f1 , f0.z / f1 );
#endif
}

SORT_FORCEINLINE float3   operator  /( const float f0 , const float3& f1 ) {
#ifdef SSE_ENABLED
    return _mm_div_ps( _mm_set_ps1(f0) , f1.data );
#else
    return float3( f0 / f1.x , f0 / f1.y, f0 / f1.z );
#endif
}

SORT_FORCEINLINE float3   operator  /=( float3& f0 , const float3& f1 ) {
    return f0 = f0 / f1;
}

SORT_FORCEINLINE float3   operator  /=( float3& f0 , const float f1 ) {
    return f0 = f0 / f1;
}

SORT_FORCEINLINE bool     operator  ==( const float3& f0 , const float3& f1 ){
#ifdef SSE_ENABLED
    return ( _mm_movemask_ps(_mm_cmpeq_ps( f0.data , f1.data )) & 0x7 ) == 0x7;
#else
    return ( f0.x == f1.x ) && ( f0.y == f1.y ) && ( f0.z == f1.z );
#endif
}

SORT_FORCEINLINE bool     operator  !=( const float3& f0 , const float3& f1 ){
#ifdef SSE_ENABLED
    return _mm_movemask_ps(_mm_cmpeq_ps( f0.data , f1.data )) == 0x0;
#else
    return ( f0.x != f1.x ) || ( f0.y != f1.y ) || ( f0.z != f1.z );
#endif
}