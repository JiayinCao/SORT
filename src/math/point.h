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

#include "vector3.h"
#include "core/sassert.h"

//! @brief  Data structure that represent a point in 3D space. 
struct Point{
public:
    //! @brief  Default constructor that sets everything zero.
    Point():x(0.0f),y(0.0f),z(0.0f){}

    //! @brief  Constructor from three float values
    //!
    //! @param x    Value in x channel.
    //! @param y    Value in y channel.
    //! @param z    Value in z channel.
    Point( float x , float y , float z ):x(x),y(y),z(z){}

    //! @brief  Copy constructor
    //!
    //! @param  p   Point value to copy from.
    Point( const Point& p ):Point(p.x,p.y,p.z){}

    //! @brief  Constructor from a vector.
    //!
    //! @param  v   Vector value to copy from.
    Point( const Vector& v ):Point(v.x,v.y,v.z){}

    //! @brief  = operator
    //!
    //! @param  p   Value to copy from.
    Point& operator = ( const Point& p ){
        x = p.x; y = p.y; z = p.z;
        return *this;
    }

    //! @brief  Get the data of specific channel.
    //!
    //! @param  id      Index of channel of interest.
    //! @return         Value of channel of interest.
    float operator [] ( unsigned id ) const{
        return data[id];
    }

    //! @brief  Get the data of specific channel.
    //!
    //! @param  id      Index of channel of interest.
    //! @return         Value of channel of interest.
    float& operator[]( unsigned id ){
        return data[id];
    }

    //! @brief  Operator to convert a pointer to a vector.
    operator Vector3f() const{
        return Vector3f( x , y , z );
    }

    union{
        struct{
            float x , y , z;
        };
        float data[3];
    };
};

SORT_STATIC_FORCEINLINE Point operator + ( const Point& p , const Point& v ){
    return Point( p.x + v.x , p.y + v.y , p.z + v.z );
}

SORT_STATIC_FORCEINLINE Point operator + ( const Point& p , const Vector& v ){
    return Point( p.x + v.x , p.y + v.y , p.z + v.z );
}

SORT_STATIC_FORCEINLINE Point operator + ( const Vector& v , const Point& p ){
    return p + v;
}

SORT_STATIC_FORCEINLINE const Point& operator += ( Point& p , const Vector& v ){
    p.x += v.x; p.y += v.y; p.z += v.z;
    return p;
}

SORT_STATIC_FORCEINLINE Point operator - ( const Point& p , const Vector& v ){
    return Point( p.x - v.x , p.y - v.y , p.z - v.z );
}

SORT_STATIC_FORCEINLINE Vector operator - ( const Point& p0 , const Point& p1 ){
    return Vector( p0.x - p1.x , p0.y - p1.y , p0.z - p1.z );
}

SORT_STATIC_FORCEINLINE const Point& operator -= ( Point& p , const Vector& v ){
    p.x -= v.x; p.y -= v.y; p.z -= v.z;
    return p;
}

SORT_STATIC_FORCEINLINE Point operator * ( float f , const Point& p ){
    return Point( p.x * f , p.y * f , p.z * f );
}

SORT_STATIC_FORCEINLINE Point operator * ( const Point& p , float f ){
    return Point( f * p.x , f * p.y , f * p.z );
}

SORT_STATIC_FORCEINLINE Point& operator *= ( Point& p , float f ){
    p.x *= f; p.y *= f; p.z *= f;
    return p;
}

SORT_STATIC_FORCEINLINE Point operator / ( const Point& p , float f ){
    const auto t = 1.0f / f;
    return p * t;
}

SORT_STATIC_FORCEINLINE Point& operator /= ( Point& p , float f ){
    const auto t = 1.0f / f;
    p.x *= t; p.y *= t; p.z *= t;
    return p;
}

SORT_STATIC_FORCEINLINE float distance( const Point& p0 , const Point& p1 ){
    return (p0-p1).Length();
}

SORT_STATIC_FORCEINLINE bool operator == ( const Point& p0 , const Point& p1 ){
    return ( p0.x == p1.x ) && ( p0.y == p1.y ) && ( p0.z == p1.z );
}

SORT_STATIC_FORCEINLINE bool operator != ( const Point& p0 , const Point& p1 ){
    return ( p0.x != p1.x ) || ( p0.y != p1.y ) || ( p0.z != p1.z );
}