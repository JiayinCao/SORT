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

#include <math.h>
#include "core/define.h"
#include "core/sassert.h"

//! @brief  Abstraction 2D vector class.
template< class T >
class Vector2{
public:
    //! @brief  Default constructor reset all value to be zero.
    Vector2() : x(0),y(0){}

    //! @brief  Constructor from two float values.
    //!
    //! @param  x   Value in x channel.
    //! @param  y   Value in y channel.
    Vector2( T x , T y ) : x(x),y(y){}

    //! @brief  Constructor from one float value.
    //!
    //! @param  t   Value to be propergated to all channels.
    Vector2( T t ) : Vector2(t,t){}

    //! @brief  Copy constructor.
    //!
    //! @param  v   Value to copy from.
    Vector2( const Vector2<T>& v ) : Vector2( v.x , v.y ) {}

    //! @brief  Constructor from a point to two float value.
    Vector2( const T* const d ) : x(d[0]),y(d[1]){}

    //! @brief  = operator.
    //!
    //! @brief v    Value to copy from.
    //! @return     Copied value.
    const Vector2<T>& operator = ( const Vector2<T>& v ){
        x = v.x; y = v.y;
        return *this;
    }

    //! @brief  Access value in a specific channel.
    //!
    //! It is up to the higher level code to make sure id is with the valid range.
    //!
    //! @param id   Index of channel of interest.
    //! @return     Value of interest.
    T operator[] ( unsigned id ) const{
        sAssert( id >= 0 && id < 2 , GENERAL );
        return data[id];
    }

    //! @brief  Access value in a specific channel.
    //!
    //! It is up to the higher level code to make sure id is with the valid range.
    //!
    //! @param id   Index of channel of interest.
    //! @return     Value of interest.
    T& operator[] ( unsigned id ){
        sAssert( id >= 0 && id < 2 , GENERAL );
        return data[id];
    }

    //! @brief  Length of the vector.
    //!
    //! @return     The length of the vector.
    float Length() const{
        return sqrt( SquaredLength() );
    }

    //! @brief  The squared length of the vector.
    //!
    //! @return     The squared length of the vector.
    float SquaredLength() const{
        return x * x + y * y;
    }

    //! @brief  Normalize the vector.
    //!
    //! @return     Normalized vector.
    Vector2<T>& Normalize(){
        float len = Length();
        if( len != 0 )
            *this /= len;
        return *this;
    }

    union{
        struct{
            T x , y;
        };
        struct{
            T data[2];
        };
    };
};

typedef Vector2<float>      Vector2f;
typedef Vector2<int>        Vector2i;
typedef Vector2<unsigned>   Vector2u;
typedef Vector2<double>     Vector2d;

template<class T>
SORT_FORCEINLINE float dot( const Vector2<T>& v0 , const Vector2<T>& v1 ){
    return v0.x * v1.x + v0.y * v1.y;
}

template<class T>
SORT_FORCEINLINE float absDot( const Vector2<T>& v0 , const Vector2<T>& v1 ){
    float r = dot( v0 , v1 );
    return ( r < 0.0f )? -r : r;
}

template<class T>
SORT_FORCEINLINE float satDot( const Vector2<T>& v0 , const Vector2<T>& v1 ){
    return clamp( dot( v0 , v1 ) , 0.0f , 1.0f );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator+( const Vector2<T>& v0 , const Vector2<T>& v1 ){
    return Vector2<T>( v0[0] + v1[0] , v0[1] + v1[1] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator-( const Vector2<T>& v0 , const Vector2<T>& v1 ){
    return Vector2<T>( v0[0] - v1[0] , v0[1] - v1[1] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator*( const Vector2<T>& v0 , const Vector2<T>& v1 ){
    return Vector2<T>( v0[0] * v1[0] , v0[1] * v1[1] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator/( const Vector2<T>& v0 , const Vector2<T>& v1 ){
    return Vector2<T>( v0[0] / v1[0] , v0[1] / v1[1] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator+( const Vector2<T>& v , const T s ){
    return Vector2<T>( v[0] + s , v[1] + s );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator-( const Vector2<T>& v , const T s ){
    return Vector2<T>( v[0] - s , v[1] - s );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator*( const Vector2<T>& v , const T s ){
    return Vector2<T>( v[0] * s , v[1] * s );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator/( const Vector2<T>& v , const T s ){
    return Vector2<T>( v[0] / s , v[1] / s );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator+( const T s , const Vector2<T> v ){
    return Vector2<T>( s + v[0] , s + v[1] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator-( const T s , const Vector2<T>& v ){
    return Vector2<T>( s - v[0] , s - v[1] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator*( const T s , const Vector2<T>& v ){
    return Vector2<T>( s * v[0] , s * v[1] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector2<T> operator/( const T s , const Vector2<T>& v ){
    return Vector2<T>( s / v[0] , s / v[1] );
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector2<T>& operator+= ( Vector2<T>& t, const Vector2<T>& v ){
    t[0] += v[0]; t[1] += v[1];
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector2<T>& operator-= ( Vector2<T>& t, const Vector2<T>& v ){
    t[0] -= v[0]; t[1] -= v[1];
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector2<T>& operator*= ( Vector2<T>& t, const Vector2<T>& v ){
    t[0] *= v[0]; t[1] *= v[1];
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector2<T>& operator/= ( Vector2<T>& t, const Vector2<T>& v ){
    t[0] /= v[0]; t[1] /= v[1];
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector2<T>& operator+= ( Vector2<T>& t, const T s ){
    t[0] += s; t[1] += s;
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector2<T>& operator-= ( Vector2<T>& t, const T s ){
    t[0] -= s; t[1] -= s;
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector2<T>& operator*= ( Vector2<T>& t, const T s ){
    t[0] *= s; t[1] *= s;
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector2<T>& operator/= ( Vector2<T>& t, const T s ){
    t[0] /= s; t[1] /= s; 
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector2<T>& operator+ ( const Vector2<T>& v ){
    return v;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector2<T> operator- ( const Vector2<T>& v ){
    return Vector2<T>(-v[0],-v[1]);
}

template<class T>
SORT_STATIC_FORCEINLINE bool operator== ( const Vector2<T>& v0 , const Vector2<T>& v1 ){
    return ( v0[0] == v1[0] ) && ( v0[1] == v1[1] );
}

template<class T>
SORT_STATIC_FORCEINLINE bool operator!= ( const Vector2<T>& v0 , const Vector2<T>& v1 ){
    return ( v0[0] != v1[0] ) || ( v0[1] != v1[1] ) ;
}

template<class T>
SORT_STATIC_FORCEINLINE bool isZero( const Vector2<T>& v ){
    return ( v[0] == (T)0 ) && ( v[1] == (T)0 );
}