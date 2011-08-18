/*
   FileName:      area.cpp

   Created Time:  2011-08-18 09:51:52

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "area.h"

// sample ray from light
Spectrum AreaLight::sample_f( const Intersection& intersect , Vector& wi , float delta , float* pdf , Visibility& visibility ) const
{
	return 0.0f;
}

// total power of the light
Spectrum AreaLight::Power() const
{
	return 0.0f;
}

// register property
void AreaLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "mesh" , new MeshProperty( this ) );
}