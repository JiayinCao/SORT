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

// #include <OSL/oslexec.h>
#include <math.h>
#include "math/utils.h"
#include "core/sassert.h"

//! @brief  Abstraction 3D vector class.
template< class T >
class Vector3{
public:
    //! @brief  Default constructor reset all value to be zero.
    Vector3() : x(0),y(0),z(0){}

    //! @brief  Constructor from three float values.
    //!
    //! @param  x   Value in x channel.
    //! @param  y   Value in y channel.
    //! @param  z   Value in z channel.
    Vector3( T x , T y , T z ) : x(x),y(y),z(z){}

    //! @brief  Constructor from one float value.
    //!
    //! @param  t   Value to be propergated to all channels.
    Vector3( T t ) : Vector3(t,t,t){}

    //! @brief  Copy constructor.
    //!
    //! @param  v   Value to copy from.
    Vector3( const Vector3<T>& v ) : Vector3( v.x , v.y , v.z ) {}

    //! @brief  Constructor from a point to three float value.
    Vector3( const T* const d ) : x(d[0]),y(d[1]),z(d[2]){}

    //! @brief  Constructor from OSL vector.
    //!
    //! @param  v   OSL type vector.
    // Vector3( OSL::Vec3 v ) : x( v.x ) , y( v.y ) , z(v.z) {}

    //! @brief  = operator.
    //!
    //! @brief v    Value to copy from.
    //! @return     Copied value.
    const Vector3<T>& operator = ( const Vector3<T>& v ){
        x = v.x; y = v.y; z = v.z;
        return *this;
    }

    //! @brief  Access value in a specific channel.
    //!
    //! It is up to the higher level code to make sure id is with the valid range.
    //!
    //! @param id   Index of channel of interest.
    //! @return     Value of interest.
    T operator[] ( unsigned id ) const{
        sAssert( id >= 0 && id < 3 , GENERAL );
        return data[id];
    }

    //! @brief  Access value in a specific channel.
    //!
    //! It is up to the higher level code to make sure id is with the valid range.
    //!
    //! @param id   Index of channel of interest.
    //! @return     Value of interest.
    T& operator[] ( unsigned id ){
        sAssert( id >= 0 && id < 3 , GENERAL );
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
        return x * x + y * y + z * z;
    }

    //! @brief  Normalize the vector.
    //!
    //! @return     Normalized vector.
    Vector3<T>& Normalize(){
        float len = Length();
        if( len != 0 )
            *this /= len;
        return *this;
    }

    union{
        struct{
            T x , y , z;
        };
        struct{
            T r , g , b;
        };
        struct{
            T data[3];
        };
    };

    static const Vector3<T> UP;
};

typedef Vector3<float>      Vector;     // Vector is 3-float vector by default
typedef Vector3<float>      Vector3f;
typedef Vector3<int>        Vector3i;
typedef Vector3<unsigned>   Vector3u;
typedef Vector3<double>     Vector3d;

extern const Vector DIR_UP;

template<class T>
SORT_FORCEINLINE float dot( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

template<class T>
SORT_FORCEINLINE float absDot( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    float r = dot( v0 , v1 );
    return ( r < 0.0f )? -r : r;
}

template<class T>
SORT_FORCEINLINE float satDot( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    return clamp( dot( v0 , v1 ) , 0.0f , 1.0f );
}

template<class T>
SORT_FORCEINLINE Vector cross( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    return Vector ( v0.y * v1.z - v0.z * v1.y,
        v0.z * v1.x - v0.x * v1.z,
        v0.x * v1.y - v0.y * v1.x );
}

template<class T>
SORT_FORCEINLINE bool faceForward( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    return dot( v0 , v1 ) >= 0.0f;
}

template<class T>
SORT_FORCEINLINE Vector normalize( const Vector3<T>& v ){
    float len = v.Length();
    if( len == 0.0f )
        return v;
    return v / len;
}

template<class T>
SORT_FORCEINLINE void coordinateSystem( const Vector3<T>& v0 , Vector3<T>& v1 , Vector3<T>& v2 ){
    if( fabs( v0.x ) > fabs( v0.y ) ){
        float invLen = 1.0f / sqrtf( v0.x * v0.x + v0.z * v0.z );
        v1 = Vector( -v0.z * invLen , 0.0f , v0.x * invLen );
    }else{
        float invLen = 1.0f / sqrtf( v0.y * v0.y + v0.z * v0.z );
        v1 = Vector( 0.0f , v0.z * invLen , -v0.y * invLen );
    }
    v2 = cross( v0 , v1 );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator+( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    return Vector3<T>( v0[0] + v1[0] , v0[1] + v1[1] , v0[2] + v1[2] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator-( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    return Vector3<T>( v0[0] - v1[0] , v0[1] - v1[1] , v0[2] - v1[2] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator*( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    return Vector3<T>( v0[0] * v1[0] , v0[1] * v1[1] , v0[2] * v1[2] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator/( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    return Vector3<T>( v0[0] / v1[0] , v0[1] / v1[1] , v0[2] / v1[2] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator+( const Vector3<T>& v , const T s ){
    return Vector3<T>( v[0] + s , v[1] + s , v[2] + s );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator-( const Vector3<T>& v , const T s ){
    return Vector3<T>( v[0] - s , v[1] - s , v[2] - s );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator*( const Vector3<T>& v , const T s ){
    return Vector3<T>( v[0] * s , v[1] * s , v[2] * s );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator/( const Vector3<T>& v , const T s ){
    return Vector3<T>( v[0] / s , v[1] / s , v[2] / s );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator+( const T s , const Vector3<T> v ){
    return Vector3<T>( s + v[0] , s + v[1] , s + v[2] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator-( const T s , const Vector3<T>& v ){
    return Vector3<T>( s - v[0] , s - v[1] , s - v[2] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator*( const T s , const Vector3<T>& v ){
    return Vector3<T>( s * v[0] , s * v[1] , s * v[2] );
}

template<class T>
SORT_STATIC_FORCEINLINE Vector3<T> operator/( const T s , const Vector3<T>& v ){
    return Vector3<T>( s / v[0] , s / v[1] , s / v[2] );
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector3<T>& operator+= ( Vector3<T>& t, const Vector3<T>& v ){
    t[0] += v[0]; t[1] += v[1]; t[2] += v[2];
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector3<T>& operator-= ( Vector3<T>& t, const Vector3<T>& v ){
    t[0] -= v[0]; t[1] -= v[1]; t[2] -= v[2];
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector3<T>& operator*= ( Vector3<T>& t, const Vector3<T>& v ){
    t[0] *= v[0]; t[1] *= v[1]; t[2] *= v[2];
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector3<T>& operator/= ( Vector3<T>& t, const Vector3<T>& v ){
    t[0] /= v[0]; t[1] /= v[1]; t[2] /= v[2];
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector3<T>& operator+= ( Vector3<T>& t, const T s ){
    t[0] += s; t[1] += s; t[2] += s;
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector3<T>& operator-= ( Vector3<T>& t, const T s ){
    t[0] -= s; t[1] -= s; t[2] -= s;
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector3<T>& operator*= ( Vector3<T>& t, const T s ){
    t[0] *= s; t[1] *= s; t[2] *= s;
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector3<T>& operator/= ( Vector3<T>& t, const T s ){
    t[0] /= s; t[1] /= s; t[2] /= s;
    return t;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector3<T>& operator+ ( const Vector3<T>& v ){
    return v;
}

template<class T>
SORT_STATIC_FORCEINLINE const Vector3<T> operator- ( const Vector3<T>& v ){
    return Vector3<T>(-v[0],-v[1],-v[2]);
}

template<class T>
SORT_STATIC_FORCEINLINE bool operator== ( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    return ( v0[0] == v1[0] ) && ( v0[1] == v1[1] ) && ( v0[2] == v1[2] );
}

template<class T>
SORT_STATIC_FORCEINLINE bool operator!= ( const Vector3<T>& v0 , const Vector3<T>& v1 ){
    return ( v0[0] != v1[0] ) || ( v0[1] != v1[1] ) || ( v0[2] != v1[2] );
}

template<class T>
SORT_STATIC_FORCEINLINE bool isZero( const Vector3<T>& v ){
    return ( v[0] == (T)0 ) && ( v[1] == (T)0 ) && ( v[2] == (T)0 );
}