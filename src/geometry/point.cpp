/*
 * filename:	point.cpp
 *
 * programmer:	Cao Jiayin
 */

// include the header file
#include "point.h"
#include "../managers/logmanager.h"

// default constructor
Point::Point()
{
	x = 0;
	y = 0;
	z = 0;
}

// copy constructor
Point::Point( const Point& p )
{
	x = p.x;
	y = p.y;
	z = p.z;
}

// constructor from three floats
Point::Point( float _x , float _y , float _z )
{
	x = _x;
	y = _y;
	z = _z;
}

// destructor
Point::~Point()
{
}

// + operator
Point Point::operator + ( const Vector& v ) const
{
	return Point( x + v.x , y + v.y , z + v.z );
}

// += operator
Point& Point::operator += ( const Vector& v )
{
	x += v.x;
	y += v.y;
	z += v.z;

	return *this;
}

// - operator
Point Point::operator - ( const Vector& v ) const 
{
	return Point( x - v.x , y - v.y , z - v.z );
}

// -= operator
Point& Point::operator -= ( const Vector& v )
{
	x -= v.x;
	y -= v.y;
	z -= v.z;

	return *this;
}

// * operator
Point Point::operator * ( float f ) const
{
	return Point( f * x , f * y , f * z );
}

// *= operator
Point& Point::operator *= ( float f )
{
	x *= f;
	y *= f;
	z *= f;

	return *this;
}

// / operator
Point Point::operator / ( float f ) const
{
	if( f == 0.0f )
		LOG_ERROR<<"point divided by zero!!!"<<CRASH;

	float t = 1.0f / f;

	return (*this) * t;
}

// /= operator
Point& Point::operator /= ( float f )
{
	if( f == 0.0f )
		LOG_ERROR<<"point divided by zero!!!"<<CRASH;

	float t = 1.0f / f;

	x *= t;
	y *= t;
	z *= t;

	return *this;
}

// - operator
Vector Point::operator - ( const Point& p ) const
{
	return Vector( x - p.x , y - p.y , z - p.z );
}

// [] operator
float Point::operator [] ( unsigned id ) const
{
	if( id == 2 )
		return z;
	if( id == 1 )
		return y;
	if( id == 0 )
		return x;

	LOG_ERROR<<"index out of range!!!"<<CRASH;

	return 0;
}

// [] operator
float& Point::operator[]( unsigned id )
{
	if( id == 2 )
		return z;
	if( id == 1 )
		return y;
	if( id != 0 )
		LOG_ERROR<<"index out of range!!!"<<CRASH;

	return x;
}

// == operator
bool Point::operator == ( const Point& p ) const
{
	if( p.x == x && p.y == y && p.z == z )
		return false;

	return true;
}

// != operator
bool Point::operator != ( const Point& p ) const
{
	if( p.x == x && p.y == y && p.z == z )
		return true;

	return false;
}


