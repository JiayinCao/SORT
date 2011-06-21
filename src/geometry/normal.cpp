/*
 * filename :	normal.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "normal.h"
#include "transform.h"

// default constructor
Normal::Normal()
{
}
// constructor from three float
Normal::Normal( float _x , float _y , float _z )
{
	x = _x;
	y = _y;
	z = _z;
}
// copy constructor
Normal::Normal( const Normal& n )
{
	x = n.x;
	y = n.y;
	z = n.z;
}
// destructor
Normal::~Normal()
{
}

// transform matrix
Vector Normal::_transform( const Transform& t ) const
{
	float _x = x * t.invMatrix.m[0] + y * t.invMatrix.m[4] + z * t.invMatrix.m[8];
	float _y = x * t.invMatrix.m[1] + y * t.invMatrix.m[5] + z * t.invMatrix.m[9];
	float _z = x * t.invMatrix.m[2] + y * t.invMatrix.m[6] + z * t.invMatrix.m[10];

	return Vector( _x , _y , _z );
}