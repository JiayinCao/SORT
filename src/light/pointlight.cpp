/*
 * filename :	pointlight.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header
#include "pointlight.h"
#include "geometry/intersection.h"

// sample ray from light
Spectrum PointLight::sample_f( const Intersection& intersect , Vector& wi , float* pdf , Visibility& visibility ) const
{
	Point pos( light2world.matrix.m[3] , light2world.matrix.m[7] , light2world.matrix.m[11] );
	Vector delta = pos - intersect.intersect;
	wi = Normalize( delta );
	*pdf = 1.0f;

	float len = delta.Length();
	visibility.ray = Ray( intersect.intersect , wi , 0 , 1.0f , len );

	return intensity / ( len * len );
}

// register property
void PointLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "pos" , new PosProperty(this) );
}
