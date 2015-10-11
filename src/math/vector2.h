/*
	FileName:      vector2.h

	Created Time:  2011-08-04 12:51:24

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
	'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
	modify or publish the source code. It's cross platform. You could compile the source code in 
	linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_VECTOR2
#define SORT_VECTOR2

#include <math.h>
#include "utility/define.h"

template< class T >
class Vector2
{
// public method
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


// private field
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

#endif