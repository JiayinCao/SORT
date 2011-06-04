/*
 *
 * filename:	vector.cpp
 *
 * programmer:	Cao Jiayin
 */

// include the header files
#include "vector.h"
#include <math.h>

// default constructor, all of the components are set zero
Vector::Vector()
{
	x = 0;
	y = 0;
	z = 0;
}

// copy constructor
Vector::Vector( const Vector& vector )
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

// constructor from three float data
Vector::Vector( float _x , float _y , float _z )
{
	x = _x;
	y = _y;
	z = _z;
}

// destructor does nothing here
Vector::~Vector()
{
}

// + operator
Vector Vector::operator +( const Vector& v ) const
{
	return Vector( x + v.x , y + v.y , z + v.z );
}

// += operator
Vector&	Vector::operator +=( const Vector& v )
{
	x += v.x;
	y += v.y;
	z += v.z;

	return *this;
}

// - operator
Vector	Vector::operator -( const Vector& v ) const
{
	return Vector( x - v.x , y - v.y , z - v.z );
}

// -= operator
Vector&	Vector::operator -=( const Vector& v )
{
	x -= v.x;
	y -= v.y;
	z -= v.z;

	return *this;
}

// * operator with a float
Vector Vector::operator *( float f ) const
{
	return Vector( x * f , y * f , z * f );
}

// *= operator with a float
Vector&	Vector::operator *=( float f )
{
	x *= f;
	y *= f;
	z *= f;

	return *this;
}

// global * operator for Vector
Vector operator * ( float f , const Vector& v )
{
	return Vector( v.x * f , v.y * f , v.z * f );
}

// / operator
Vector Vector::operator / ( float f ) const
{
	if( f == 0 )
	{
		// output the error here , to be modified
		return Vector();
	}

	float t = 1.0f / f;
	
	return (*this) * t;
}

// /= operator
Vector& Vector::operator /= ( float f )
{
	if( f == 0 )
	{
		// output the error here , to be modified
	}

	float t = 1.0f / f;

	x *= t;
	y *= t;
	z *= t;

	return *this;
}

// [] operator
float Vector::operator [] ( unsigned id ) const
{
	if( id == 0 )
		return x;
	if( id == 1 )
		return y;
	if( id == 2 )
		return z;

	return 0;
}

// - operator
Vector Vector::operator - () const 
{
	return Vector( -x , -y , -z );
}

// sqaured length
float Vector::SquaredLength() const 
{
	return x * x + y * y + z * z;
}

// length
float Vector::Length() const
{
	return sqrt( SquaredLength() );
}

// normalize the vector
Vector& Vector::Normalize()
{
	// get the length of the vector
	float len = Length();

	// divide the vector
	*this /= len;

	return *this;
}

// the dot product
float	dot( const Vector& v0 , const Vector& v1 )
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

// the cross product , we use left handed coordinate here
Vector	cross( const Vector& v0 , const Vector& v1 )
{
	Vector v;
	v.x = v0.y * v1.z - v0.z * v1.y ;
	v.y = v0.z * v1.x - v0.x * v1.z ;
	v.z = v0.x * v1.y - v0.y * v1.x ;

	return v;
}
