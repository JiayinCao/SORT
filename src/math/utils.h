/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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

#include <math.h>
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#define NOMINMAX
#  include <algorithm> // for std::min and std::max
#endif

#include "core/define.h"

#define PI              3.1415926f
#define TWO_PI          6.2831852f
#define FOUR_PI         12.5663704f
#define HALF_PI         1.5707963f
#define INV_PI          0.3183099f
#define INV_TWOPI       0.15915494f
#define INV_FOUR_PI     0.07957747f

#define SQR(x)      (Pow<2>(x))

#if defined(SORT_IN_LINUX)
    // This may not be 100% correct
    #define IsInf   std::isinf
    #define IsNan   std::isnan
#else
    #define IsInf   isinf
    #define IsNan   isnan
#endif

//! @brief  Power of a value.
//!
//! Instead of using the native one provided by C++, this involves O(ln(N)) multiplications
//! where 'N' is the number of power.
//!
//! @param  n   The number of power.
//! @param  x   Value to be evaluated.
//! @return     x^n
template<unsigned n>
SORT_FORCEINLINE float Pow( float x ){
    static_assert(n > 0, "Power can't be negative");
    const auto x2 = Pow<n / 2>(x);
    return x2 * x2 * Pow<n & 1>(x);
}

template<>
SORT_FORCEINLINE float Pow<0>( float x ){
    return 1.0f;
}

template<>
SORT_FORCEINLINE float Pow<1>( float x ){
    return x;
}

//! @brief  Clamp before calculating square root to avoid NaN.
//!
//! @param  x   Value to be evaluated.
//! @return     The square root of @param x.
SORT_FORCEINLINE float ssqrt( const float x ){
    return sqrt( std::max( 0.0f , x ) );
}

#define saturate(x)             std::max(0.0f,std::min(1.0f,x))
#define slerp( a , b , t  )     ( (a) * ( 1.0f - (t) ) + (b) * (t) )

//! @brief  Clamp a value
//!
//! @param  x   Value to be clampped.
//! @param  mi  Lower bound of the range.
//! @param  ma  Upper bound of the range.
//! @return     Clampped value.
SORT_FORCEINLINE float clamp( float x , float mi , float ma ){
    if( x > ma ) x = ma;
    if( x < mi ) x = mi;
    return x;
}
SORT_FORCEINLINE int clamp( int x , int mi , int ma ){
    if( x > ma ) x = ma;
    if( x < mi ) x = mi;
    return x;
}

//! @brief  Degree to radian.
//!
//! @param  deg Degree to be converted.
//! @return Cooresponding radian.
SORT_FORCEINLINE float Radians( float deg ) {
    return PI / 180.0f * deg;
}

//! @brief  Radian to degree.
//!
//! @param  rad     Value to be converted.
//! @return         Cooresponding degree.
SORT_FORCEINLINE float Degrees( float rad ) {
    return 180.0f * INV_PI * rad ;
}

//! @brief  Color from linear space to gamma space.
//!
//! @param  value   Linear space color.
//! @return         Gamma space color.
SORT_FORCEINLINE float LinearToGamma( float value ){
    if (value <= 0.0031308f) return 12.92f * value;
    return 1.055f * pow(value, (float)(1.f / 2.4f)) - 0.055f;
}

//! @brief  Color from gamma space to linear space.
//!
//! @param value    Gamma space color.
//! @return         Linear space color.
SORT_FORCEINLINE float GammaToLinear( float value ){
    if (value <= 0.04045f) return value * 1.f / 12.92f;
    return pow((value + 0.055f) * 1.f / 1.055f, (float)2.4f);
}