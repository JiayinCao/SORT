/*
	FileName:      vector3.h

	Created Time:  2011-08-04 12:51:24

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
	'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
	modify or publish the source code. It's cross platform. You could compile the source code in 
	linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_VECTOR3
#define SORT_VECTOR3

#include <math.h>
#include "utility/define.h"

template< class T >
class Vector3
{
// public method
public:
	Vector3() :	x(0),y(0),z(0){
	}
	Vector3( T _x , T _y , T _z ) : x(_x),y(_y),z(_z){
	}
	Vector3( const Vector3<T>& c ) : x(c.x), y(c.y), z(c.z){
	}
	Vector3( const T* const d ) : x(d[0]),y(d[1]),z(d[2]){
	}

	Vector3<T>	operator+ ( const Vector3<T>& v) const {
		return Vector3<T>( x + v.x , y + v.y , z + v.z );
	}
	const Vector3<T>&	operator+= ( const Vector3<T>& v) {
		x += v.x; y += v.y; z += v.z;
		return *this;
	}
	Vector3<T>	operator- ( const Vector3<T>& v) const {
		return Vector3<T>( x - v.x , y - v.y , z - v.z );
	}
	const Vector3<T>&	operator-= ( const Vector3<T>& v ) {
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}
	Vector3<T>	operator* ( float s ) const {
		return Vector3<T>( x * s , y * s , z * s );
	}
	const Vector3<T>&	operator*= ( float s ){
		x *= s; y *= s; z *= s;
		return *this;
	}
	Vector3<T>	operator/ ( float div ) const {
		if( div == 0.0f )
			return *this;
		div = 1.0f / div;
		return (*this) * div;
	}
	const Vector3<T>&	operator/= ( float div ) {
		if( div == 0.0f )
			return *this;
		div = 1.0f / div;
		return *this *= div;
	}
	const Vector3<T>& operator = ( const Vector3<T>& v ){
		x = v.x; y = v.y; z = v.z;
		return *this;
	}
	T operator[] ( unsigned id ) const{
		return data[id];
	}
	T& operator[] ( unsigned id ){
		return data[id];
	}
	Vector3<T> operator-() const{
		return Vector3<T>( -x , -y , -z );
	}
	bool operator== ( const Vector3<T>& v ) const{
		return ( v.x == x ) && ( v.y == y ) && ( v.z == z );
	}
	bool operator!= ( const Vector3<T>& v ) const{
		return ( v.x != x ) || ( v.y != y ) || ( v.z != z );
	}
	bool IsZero() const{
		return x == 0 && y == 0 && z == 0;
	}

	float Length() const
	{
		return sqrt( SquaredLength() );
	}
	float SquaredLength() const
	{
		return x * x + y * y + z * z;
	}
	Vector3<T>& Normalize()
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
			T x , y , z;
		};
		struct{
			T r , g , b;
		};
		struct{
			T data[3];
		};
	};
};

typedef Vector3<float>		Vector;		// Vector is 3-float vector by default
typedef Vector3<float>		Vector3f;
typedef Vector3<int>		Vector3i;
typedef Vector3<unsigned>	Vector3u;
typedef Vector3<double>		Vector3d;

// * operator
// para 'f'  :	scaler
// para 'v0' :	the vector to scale
// result    :	a scaled vector
template<class T>
inline Vector3<T> operator *( float f , const Vector3<T>& v0 )
{
	return v0 * f;
}
// para 'v0' : 	a vector
// para 'v1' :	another vector
// result    :	the dot product of the two vectors
template<class T>
inline float Dot( const Vector3<T>& v0 , const Vector3<T>& v1 )
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}
// para 'v0' :	a vector
// para 'v1' :	another vector
// result    :	the absolute value of the dot product
template<class T>
inline float AbsDot( const Vector3<T>& v0 , const Vector3<T>& v1 )
{
	float r = Dot( v0 , v1 );
	return ( r < 0.0f )? -r : r;
}
// para 'v0' :	a vector
// para 'v1' :	another vector
// reulst    :	saturated dot product
template<class T>
inline float SatDot( const Vector3<T>& v0 , const Vector3<T>& v1 )
{
	return clamp( Dot( v0 , v1 ) , 0.0f , 1.0f );
}
// para 'v0' :	a vector
// para 'v1' :	another vector
// result    :	the cross product of the two vectors
template<class T>
inline Vector Cross( const Vector3<T>& v0 , const Vector3<T>& v1 )
{
	return Vector (	v0.y * v1.z - v0.z * v1.y,
		v0.z * v1.x - v0.x * v1.z,
		v0.x * v1.y - v0.y * v1.x );
}
// para 'v0' :	a vector
// para 'v1' :	another vector
// result    :	'true' if the two vector is facing towards the same direction, 'false' else
template<class T>
inline bool FaceForward( const Vector3<T>& v0 , const Vector3<T>& v1 )
{
	return Dot( v0 , v1 ) >= 0.0f;
}

// normalize a vector
template<class T>
inline Vector Normalize( const Vector3<T>& v )
{
	float len = v.Length();
	if( len == 0.0f )
		return v;
	return v / len;
}

// generate a coorindate system
template<class T>
inline void CoordinateSystem( const Vector3<T>& v0 , Vector3<T>& v1 , Vector3<T>& v2 )
{
	if( fabs( v0.x ) > fabs( v0.y ) )
	{
		float invLen = 1.0f / sqrtf( v0.x * v0.x + v0.z * v0.z );
		v1 = Vector( -v0.z * invLen , 0.0f , v0.x * invLen );
	}else
	{
		float invLen = 1.0f / sqrtf( v0.y * v0.y + v0.z * v0.z );
		v1 = Vector( 0.0f , v0.z * invLen , -v0.y * invLen );
	}
	v2 = Cross( v0 , v1 );
}

#endif