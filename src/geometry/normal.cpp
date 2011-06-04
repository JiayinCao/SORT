/*
 * filename :	normal.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "normal.h"

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
