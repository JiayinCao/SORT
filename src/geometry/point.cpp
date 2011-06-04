/*
 * filename:	point.cpp
 *
 * programmer:	Cao Jiayin
 */

// include the header file
#include "point.h"

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

// - operator
Vector Point::operator - ( const Point& p ) const
{
	return Vector( x - p.x , y - p.y , z - p.z );
}

// [] operator
float Point::operator [] ( unsigned id )
{
	if( id == 2 )
		return z;
	if( id == 1 )
		return y;
	if( id == 0 )
		return x;

	return 0;
}

// + operator
Point operator + ( const Vector& v , const Point& p )
{
	return p + v;
}


