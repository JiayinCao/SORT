/*
   FileName:      distant.cpp

   Created Time:  2011-08-04 12:48:23

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header
#include "distant.h"
#include "geometry/intersection.h"

// sample a ray
Spectrum DistantLight::sample_f( const Intersection& intersect , Vector& wi , float delta , float* pdf , Visibility& visibility ) const 
{
	// distant light direction
	Vector dir( light2world.matrix.m[1] , light2world.matrix.m[5] , light2world.matrix.m[9] );
	wi = -dir;
	if( pdf ) *pdf = 1.0f;

	visibility.ray = Ray( intersect.intersect , wi , 0 , delta );

	return intensity;
}

// register all properties
void DistantLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "dir" , new DirProperty(this) );
}
