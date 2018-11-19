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
#include "vector2.h"
#include "utility/define.h"
#include "spectrum/spectrum.h"

template< class T >
class Vector4
{
public:
	Vector4() :	x(0),y(0),z(0),w(0){
	}
	Vector4(T t) : x(t),y(t),z(t),w(t){
	}
	Vector4( T _x , T _y , T _z , T _w ):x(_x),y(_y),z(_z),w(_w) {
	}
	Vector4( const Vector4<T>& c ):x(c.x),y(c.y),z(c.z),w(c.w) {
	}
	Vector4( const T* const d ) : x(d[0]),y(d[1]),z(d[2]),w(d[3]){
	}

	Vector4<T>	operator+ ( const Vector4<T>& v) const {
		return Vector4<T>( x + v.x , y + v.y , z + v.z , w + v.w);
	}
	const Vector4<T>&	operator+= ( const Vector4<T>& v) {
		x += v.x; y += v.y; z += v.z; w += v.w;
		return *this;
	}
	Vector4<T>	operator- ( const Vector4<T>& v) const {
		return Vector4<T>( x - v.x , y - v.y , z - v.z , w - v.w);
	}
	const Vector4<T>&	operator-= ( const Vector4<T>& v ) {
		x -= v.x; y -= v.y; z -= v.z; w -= v.w;
		return *this;
	}
	Vector4<T>	operator* ( const Vector4<T>& v ) const {
		return Vector4<T>( v.x * x , v.y * y , v.z * z , v.w * w );
	}
	Vector4<T>	operator* ( float s ) const {
		return Vector4<T>( x * s , y * s , z * s , w * s);
	}
	const Vector4<T>&	operator*= ( float s ){
		x *= s; y *= s; z *= s; w *= s;
		return *this;
	}
	Vector4<T>	operator/ ( float div ) const {
		if( div == 0.0f )
			return *this;
		div = 1.0f / div;
		return (*this) * div;
	}
	const Vector4<T>&	operator/= ( float div ) {
		if( div == 0.0f )
			return *this;
		div = 1.0f / div;
		return *this *= div;
	}
	const Vector4<T>& operator = ( const Vector4<T>& v ){
		x = v.x; y = v.y; z = v.z;
		return *this;
	}
	T operator[] ( unsigned id ) const{
		return data[id];
	}
	T& operator[] ( unsigned id ){
		return data[id];
	}
	Vector4<T> operator-() const{
		return Vector3<T>( -x , -y , -z , -w );
	}
	bool operator== ( const Vector4<T>& v ) const{
		return ( v.x == x ) && ( v.y == y ) && ( v.z == z ) && ( v.w == w );
	}
	bool operator!= ( const Vector4<T>& v ) const{
		return ( v.x != x ) || ( v.y != y ) || ( v.z != z ) || ( v.w != w );
	}
	bool IsZero() const{
		return x == 0 && y == 0 && z == 0 && w == 0 ;
	}

	float Length() const
	{
		return sqrt( SquaredLength() );
	}
	float SquaredLength() const
	{
		return x * x + y * y + z * z + w * w;
	}
	Vector4<T>& Normalize()
	{
		float len = Length();
		if( len != 0 )
			*this /= len;
		return *this;
	}

	// to spectrum
	Spectrum ToSpectrum() const{
		return Spectrum( x , y , z );
	}
    // to vector
    Vector   ToVector() const{
        return Vector( x , y , z );
    }

public:
	// the vector data
	union
	{
		struct{
			T x , y , z , w;
		};
		struct{
			T r , g , b , a;
		};
		struct{
			T data[4];
		};
	};
};

typedef Vector4<float>		Vector4f;
typedef Vector4<int>		Vector4i;
typedef Vector4<unsigned>	Vector4u;
typedef Vector4<double>		Vector4d;

template<class T>
inline Vector4<T> operator *( float f , const Vector4<T>& v0 )
{
	return v0 * f;
}

template<class T>
inline float Dot( const Vector4<T>& v0 , const Vector4<T>& v1 )
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
}

template<class T>
inline float AbsDot( const Vector4<T>& v0 , const Vector4<T>& v1 )
{
	float r = Dot( v0 , v1 );
	return ( r < 0.0f )? -r : r;
}

template<class T>
inline float SatDot( const Vector4<T>& v0 , const Vector4<T>& v1 )
{
	return clamp( Dot( v0 , v1 ) , 0.0f , 1.0f );
}

// normalize a vector
template<class T>
inline Vector4<T> Normalize( const Vector4<T>& v )
{
	float len = v.Length();
	if( len == 0.0f )
		return v;
	return v / len;
}

inline Vector4f FromSpectrum( const Spectrum& rgb )
{
	return Vector4f( rgb.GetR() , rgb.GetG() , rgb.GetB() , 0.0f );
}
