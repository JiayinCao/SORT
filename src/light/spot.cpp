/*
 * filename :	spot.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "spot.h"
#include "geometry/intersection.h"

// sample ray from light
Spectrum SpotLight::sample_f( const Intersection& intersect , Vector& wi , float* pdf ) const
{
	Point pos( light2world.matrix.m[3] , light2world.matrix.m[7] , light2world.matrix.m[11] );
	Vector dir( light2world.matrix.m[1] , light2world.matrix.m[5] , light2world.matrix.m[9] );
	Vector delta = pos - intersect.intersect;
	wi = Normalize( delta );
	*pdf = 1.0f;

	float falloff = SatDot( wi , -dir );
	if( falloff < cos_total_range )
		return 0.0f;
	if( falloff > cos_falloff_start )
		return intensity / delta.SquaredLength();
	float d = ( falloff - cos_total_range ) / ( cos_falloff_start - cos_total_range );
	return intensity / delta.SquaredLength() * d * d;
}

// register property
void SpotLight::_registerAllProperty()
{
	_registerProperty( "intensity" , new IntensityProperty(this) );
	_registerProperty( "falloff_start" , new FalloffStartProperty(this) );
	_registerProperty( "range" , new RangeProperty(this) );
}

// initialize default value
void SpotLight::_init()
{
	cos_total_range = 0.0f;
	cos_falloff_start = 1.0f;
}