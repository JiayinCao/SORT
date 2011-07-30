/*
 * filename :	distant.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header
#include "distant.h"
#include "geometry/intersection.h"

// sample a ray
Spectrum DistantLight::sample_f( const Intersection& intersect , Vector& wi , float* pdf , const Scene& scene ) const
{
	// distant light direction
	Vector dir( light2world.matrix.m[1] , light2world.matrix.m[5] , light2world.matrix.m[9] );
	wi = -dir;
	*pdf = 1.0f;

	Ray r( intersect.intersect , wi , 0 , 10.0f );
	bool occluded = scene.GetIntersect( r , 0 );
	if( occluded )
		return 0.0f;
	return intensity;
}

// register all properties
void DistantLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "dir" , new DirProperty(this) );
}
