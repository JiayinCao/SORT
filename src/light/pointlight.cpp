/*
 * filename :	pointlight.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header
#include "pointlight.h"
#include "geometry/intersection.h"

// sample ray from light
Spectrum PointLight::sample_f( const Intersection& intersect , Vector& wi , float* pdf ) const
{
	Vector delta = pos - intersect.intersect;
	wi = Normalize( delta );
	*pdf = 1.0f;
	return intensity / delta.SquaredLength();
}

// register property
void PointLight::_registerAllProperty()
{
	_registerProperty( "intensity" , new IntensityProperty(this) );
}