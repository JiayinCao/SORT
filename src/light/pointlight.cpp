/*
   FileName:      pointlight.cpp

   Created Time:  2011-08-04 12:48:32

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header
#include "pointlight.h"
#include "geometry/intersection.h"

// sample ray from light
Spectrum PointLight::sample_f( const Intersection& intersect , Vector& wi , float delta , float* pdf , Visibility& visibility ) const 
{
	Point pos( light2world.matrix.m[3] , light2world.matrix.m[7] , light2world.matrix.m[11] );
	Vector vec = pos - intersect.intersect;
	wi = Normalize( vec );
	*pdf = 1.0f;

	float len = vec.Length();
	visibility.ray = Ray( intersect.intersect , wi , 0 , delta , len );

	return intensity / ( len * len );
}

// register property
void PointLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "pos" , new PosProperty(this) );
}
