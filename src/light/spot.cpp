/*
 * filename :	spot.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "spot.h"
#include "geometry/intersection.h"
#include "geometry/scene.h"

// sample ray from light
Spectrum SpotLight::sample_f( const Intersection& intersect , Vector& wi , float* pdf , const Scene& scene ) const
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
	if( d == 0.0f )
		return 0.0f;

	float len = delta.Length();
	Ray r( intersect.intersect , wi , 0 , 1.0f , len );
	bool occluded = scene.GetIntersect( r , 0 );
	if( occluded )
		return 0.0f;

	return intensity / delta.SquaredLength() * d * d;
}

// register property
void SpotLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "pos" , new PosProperty(this) );
	_registerProperty( "falloff_start" , new FalloffStartProperty(this) );
	_registerProperty( "range" , new RangeProperty(this) );
	_registerProperty( "dir" , new DirProperty(this) );
}

// initialize default value
void SpotLight::_init()
{
	cos_total_range = 0.0f;
	cos_falloff_start = 1.0f;
}
