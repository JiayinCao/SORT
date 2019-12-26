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

// The following logic is shared across different vector like data structure, like Vector, Point and RGBSpectrum.

#define VECTOR3_COMMON_DEFINE(T,D) \
SORT_STATIC_FORCEINLINE T            operator+( const T& v0 , const T& v1 ){\
    return T( v0[0] + v1[0] , v0[1] + v1[1] , v0[2] + v1[2] );\
}\
SORT_STATIC_FORCEINLINE T            operator-( const T& v0 , const T& v1 ){\
    return T( v0[0] - v1[0] , v0[1] - v1[1] , v0[2] - v1[2] );\
}\
SORT_STATIC_FORCEINLINE T            operator*( const T& v0 , const T& v1 ){\
    return T( v0[0] * v1[0] , v0[1] * v1[1] , v0[2] * v1[2] );\
}\
SORT_STATIC_FORCEINLINE T            operator/( const T& v0 , const T& v1 ){\
    return T( v0[0] / v1[0] , v0[1] / v1[1] , v0[2] / v1[2] );\
}\
SORT_STATIC_FORCEINLINE T            operator+( const T& v , const D s ){\
    return T( v[0] + s , v[1] + s , v[2] + s );\
}\
SORT_STATIC_FORCEINLINE T            operator-( const T& v , const D s ){\
    return T( v[0] - s , v[1] - s , v[2] - s );\
}\
SORT_STATIC_FORCEINLINE T            operator*( const T& v , const D s ){\
    return T( v[0] * s , v[1] * s , v[2] * s );\
}\
SORT_STATIC_FORCEINLINE T            operator/( const T& v , const D s ){\
    return T( v[0] / s , v[1] / s , v[2] / s );\
}\
SORT_STATIC_FORCEINLINE T            operator+( const D s , const T& v ){\
    return T( s + v[0] , s + v[1] , s + v[2] );\
}\
SORT_STATIC_FORCEINLINE T            operator-( const D s , const T& v ){\
    return T( s - v[0] , s - v[1] , s - v[2] );\
}\
SORT_STATIC_FORCEINLINE T            operator*( const D s , const T& v ){\
    return T( s * v[0] , s * v[1] , s * v[2] );\
}\
SORT_STATIC_FORCEINLINE T            operator/( const D s , const T& v ){\
    return T( s / v[0] , s / v[1] , s / v[2] );\
}\
SORT_STATIC_FORCEINLINE const T&     operator+= ( T& t, const T& v ){\
    t[0] += v[0]; t[1] += v[1]; t[2] += v[2];\
    return t;\
}\
SORT_STATIC_FORCEINLINE const T&     operator-= ( T& t, const T& v ){\
    t[0] -= v[0]; t[1] -= v[1]; t[2] -= v[2];\
    return t;\
}\
SORT_STATIC_FORCEINLINE const T&     operator*= ( T& t, const T& v ){\
    t[0] *= v[0]; t[1] *= v[1]; t[2] *= v[2];\
    return t;\
}\
SORT_STATIC_FORCEINLINE const T&     operator/= ( T& t, const T& v ){\
    t[0] /= v[0]; t[1] /= v[1]; t[2] /= v[2];\
    return t;\
}\
SORT_STATIC_FORCEINLINE const T&     operator+= ( T& t, const D s ){\
    t[0] += s; t[1] += s; t[2] += s;\
    return t;\
}\
SORT_STATIC_FORCEINLINE const T&     operator-= ( T& t, const D s ){\
    t[0] -= s; t[1] -= s; t[2] -= s;\
    return t;\
}\
SORT_STATIC_FORCEINLINE const T&     operator*= ( T& t, const D s ){\
    t[0] *= s; t[1] *= s; t[2] *= s;\
    return t;\
}\
SORT_STATIC_FORCEINLINE const T&     operator/= ( T& t, const D s ){\
    t[0] /= s; t[1] /= s; t[2] /= s;\
    return t;\
}\
SORT_STATIC_FORCEINLINE const T&     operator+ ( const T& v ){\
    return v;\
}\
SORT_STATIC_FORCEINLINE const T      operator- ( const T& v ){\
    return T(-v[0],-v[1],-v[2]);\
}\
SORT_STATIC_FORCEINLINE bool            operator== ( const T& v0 , const T& v1 ){\
    return ( v0[0] == v1[0] ) && ( v0[1] == v1[1] ) && ( v0[2] == v1[2] );\
}\
SORT_STATIC_FORCEINLINE bool            operator!= ( const T& v0 , const T& v1 ){\
    return ( v0[0] != v1[0] ) || ( v0[1] != v1[1] ) && ( v0[2] != v1[2] );\
}\
SORT_STATIC_FORCEINLINE bool            isZero( const T& v ){\
    return ( v[0] == (D)0 ) && ( v[1] == (D)0 ) && ( v[2] == (D)0 );\
}