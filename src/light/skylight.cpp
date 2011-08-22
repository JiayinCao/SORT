/*
   FileName:      skylight.cpp

   Created Time:  2011-08-04 17:41:00

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "skylight.h"
#include "sampler/sample.h"
#include "bsdf/bsdf.h"

// sample ray from light
Spectrum SkyLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& wi , float delta , float* pdf , Visibility& visibility ) const
{
	Sort_Assert( sky );

	// sample a ray
	wi = sky->sample_v( ls->u , ls->v , pdf );

	// setup visibility tester
	visibility.ray = Ray( intersect.intersect , wi , 0 , delta , FLT_MAX );

	// the 10.0f factor will be removed later
	return 10.0f * sky->Evaluate( wi );
}

// sample light density
Spectrum SkyLight::sample_l( const Intersection& intersect , const Vector& wo ) const
{ 
	Sort_Assert( sky != 0 );
	return sky->Evaluate( -wo ); 
}

// total power of the light
Spectrum SkyLight::Power() const
{
	Sort_Assert( scene!=0 );
	BBox box = scene->GetBBox();
	float radius = (box.m_Max - box.m_Min).Length() * 0.5f;

	// the 10.0f factor will be removed later
	return radius * radius * PI * sky->GetAverage() * 10.0f;
}

// get intersection between the light and the ray
bool SkyLight::Evaluate( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const
{
	Sort_Assert( sky != 0 );
	Sort_Assert( intersect != 0 );

	if( intersect->t != FLT_MAX )
		return false;

	radiance = sky->Evaluate( ray.m_Dir );
	return true;
}

// initialize default value
void SkyLight::_init()
{
	sky = 0;
	_registerAllProperty();
}

// release
void SkyLight::_release()
{
	SAFE_DELETE( sky );
}

// register all properties
void SkyLight::_registerAllProperty()
{
	_registerProperty( "type" , new TypeProperty( this ) );
	_registerProperty( "up" , new PropertyPasser( this ) );
	_registerProperty( "down" , new PropertyPasser( this ) );
	_registerProperty( "left" , new PropertyPasser( this ) );
	_registerProperty( "right" , new PropertyPasser( this ) );
	_registerProperty( "front" , new PropertyPasser( this ) );
	_registerProperty( "back" , new PropertyPasser( this ) );
	_registerProperty( "image" , new PropertyPasser( this ) );
}
