/*
 * filename :	pointlight.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header
#include "pointlight.h"
#include "geometry/intersection.h"
#include "geometry/scene.h"

// sample ray from light
Spectrum PointLight::sample_f( const Intersection& intersect , Vector& wi , float* pdf , const Scene& scene ) const
{
	Point pos( light2world.matrix.m[3] , light2world.matrix.m[7] , light2world.matrix.m[11] );
	Vector delta = pos - intersect.intersect;
	wi = Normalize( delta );
	*pdf = 1.0f;

	float len = delta.Length();
	Ray r( intersect.intersect , wi , 0 , 1.0f , len );
	bool occluded = scene.GetIntersect( r , 0 );
	if( occluded )
		return 0.0f;

	return intensity / ( len * len );
}

// register property
void PointLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "pos" , new PosProperty(this) );
}
