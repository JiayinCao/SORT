/*
   FileName:      spot.cpp

   Created Time:  2011-08-04 12:48:39

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "spot.h"
#include "geometry/intersection.h"

// sample ray from light
Spectrum SpotLight::sample_f( const Intersection& intersect , Vector& wi , float* pdf , Visibility& visibility ) const
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
	visibility.ray = Ray( intersect.intersect , wi , 0 , 1.0f , len );

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
