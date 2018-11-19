/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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
#include "utility/define.h"

template< class T >
class Vector2
{
public:
	Vector2() :	x(0),y(0){
	}
	Vector2( T _x , T _y ) : x(_x),y(_y){
	}
	Vector2( const Vector2<T>& c ) : x(c.x), y(c.y){
	}
	Vector2( const T* const d ) : x(d[0]),y(d[1]){
	}

	Vector2<T>	operator+ ( const Vector2<T>& v) const {
		return Vector2<T>( x + v.x , y + v.y );
	}
	const Vector2<T>&	operator+= ( const Vector2<T>& v) {
		x += v.x; y += v.y;
		return *this;
	}
	Vector2<T>	operator- ( const Vector2<T>& v) const {
		return Vector2<T>( x - v.x , y - v.y );
	}
	const Vector2<T>&	operator-= ( const Vector2<T>& v ) {
		x -= v.x; y -= v.y;
		return *this;
	}
	Vector2<T>	operator* ( float s ) const {
		return Vector2<T>( (T)(x * s) , (T)(y * s) );
	}
	const Vector2<T>&	operator*= ( float s ){
		x *= s; y *= s;
		return *this;
	}
	Vector2<T>	operator/ ( float div ) const {
		if( div == 0.0f )
			return *this;
		div = 1.0f / div;
		return (*this) * div;
	}
	const Vector2<T>&	operator/= ( float div ) {
		if( div == 0.0f )
			return *this;
		div = 1.0f / div;
		return *this *= div;
	}
	const Vector2<T>& operator = ( const Vector2<T>& v ){
		x = v.x; y = v.y;
		return *this;
	}
	T operator[] ( unsigned id ) const{
		return data[id];
	}
	T& operator[] ( unsigned id ){
		return data[id];
	}
	Vector2<T> operator-() const{
		return Vector2<T>( -x , -y );
	}
	bool operator== ( const Vector2<T>& v ) const{
		return ( v.x == x ) && ( v.y == y ) ;
	}
	bool operator!= ( const Vector2<T>& v ) const{
		return ( v.x != x ) || ( v.y != y ) ;
	}
	bool IsZero() const{
		return x == 0 && y == 0 ;
	}

	float Length() const
	{
		return sqrt( SquaredLength() );
	}
	float SquaredLength() const
	{
		return x * x + y * y ;
	}
	Vector2<T>& Normalize()
	{
		float len = Length();
		if( len != 0 )
			*this /= len;
		return *this;
	}

public:
	// the vector data
	union
	{
		struct{
			T x , y;
		};
		struct{
			T data[2];
		};
	};
};

typedef Vector2<float>		Vector2f;
typedef Vector2<int>		Vector2i;
typedef Vector2<unsigned>	Vector2u;
typedef Vector2<double>		Vector2d;

template<class T>
inline Vector2<T> operator *( float f , const Vector2<T>& v0 )
{
	return v0 * f;
}

template<class T>
inline float Dot( const Vector2<T>& v0 , const Vector2<T>& v1 )
{
	return v0.x * v1.x + v0.y * v1.y;
}

template<class T>
inline float AbsDot( const Vector2<T>& v0 , const Vector2<T>& v1 )
{
	float r = Dot( v0 , v1 );
	return ( r < 0.0f )? -r : r;
}

template<class T>
inline float SatDot( const Vector2<T>& v0 , const Vector2<T>& v1 )
{
	return clamp( Dot( v0 , v1 ) , 0.0f , 1.0f );
}
