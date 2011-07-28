/*
 * filename :	distant.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header
#include "distant.h"

// sample a ray
Spectrum DistantLight::sample_f( const Intersection& intersect , Vector& wi , float* pdf ) const
{
	// distant light direction
	Vector dir( light2world.matrix.m[1] , light2world.matrix.m[5] , light2world.matrix.m[9] );
	wi = -dir;
	*pdf = 1.0f;
	return intensity;
}

// register all properties
void DistantLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "dir" , new DirProperty(this) );
}
