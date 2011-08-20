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
#include "sampler/sample.h"

// temp
#include "shape/disk.h"

// initialize default value
void AreaLight::_init()
{
	_registerAllProperty();

	shape = new Disk();
}

// release data
void AreaLight::_release()
{
	SAFE_DELETE( shape );
}

// sample ray from light
Spectrum AreaLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& wi , float delta , float* pdf , Visibility& visibility ) const
{
	Sort_Assert( ls != 0 );
	Sort_Assert( shape != 0 );

	Vector n;
	Point ps = shape->sample_l( *ls , intersect.intersect , wi , pdf , n );

	Vector dlt = ps - intersect.intersect;
	
	// setup visibility tester
	visibility.ray = Ray( intersect.intersect , wi , 0 , delta , dlt.Length() - delta );

	return intensity;
}

// total power of the light
Spectrum AreaLight::Power() const
{
	Sort_Assert( shape != 0 );
	return shape->SurfaceArea() * intensity.GetIntensity() * TWO_PI;
}

// register property
void AreaLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "pos" , new PosProperty(this) );
	_registerProperty( "dir" , new DirProperty(this) );
}

// sample light density
Spectrum AreaLight::sample_l( const Intersection& intersect , const Vector& wo ) const
{
	if( Dot( wo , intersect.normal ) > 0.0f )
		return intensity;
	return 0.0f;
}
