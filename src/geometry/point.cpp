/*
   FileName:      point.cpp

   Created Time:  2011-08-04 12:49:45

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "point.h"
#include "utility/sassert.h"

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

// constructor from a vector
Point::Point( const Vector& v )
{
	x = v.x;
	y = v.y;
	z = v.z;
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
	Sort_Assert( f != 0.0f );

	float t = 1.0f / f;

	return (*this) * t;
}

// /= operator
Point& Point::operator /= ( float f )
{
	Sort_Assert( f != 0.0f );

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
	Sort_Assert( id < 3 );

	if( id == 2 )
		return z;
	if( id == 1 )
		return y;
	if( id == 0 )
		return x;

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

// mix point
Point Point::operator + ( const Point& v ) const
{
	return Point( x + v.x , y + v.y , z + v.z );
}

// para 'p' :   point to copy
Point& Point::operator = ( const Point& p )
{
	x = p.x;
	y = p.y;
	z = p.z;

	return *this;
}
