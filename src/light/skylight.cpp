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
#include "utility/samplemethod.h"

IMPLEMENT_CREATOR( SkyLight );

// sample ray from light
Spectrum SkyLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& wi , float delta , float* pdf , Visibility& visibility ) const
{
	Sort_Assert( sky );

	// sample a ray
	wi = sky->sample_v( ls->u , ls->v , pdf , 0 );
	if( pdf && *pdf == 0.0f )
		return 0.0f;

	// setup visibility tester
	visibility.ray = Ray( intersect.intersect , wi , 0 , delta , FLT_MAX );

	return sky->Evaluate( wi );
}

// sample light density
Spectrum SkyLight::Le( const Intersection& intersect , const Vector& wo ) const
{ 
	Sort_Assert( sky != 0 );
	return sky->Evaluate( -wo ); 
}

// sample a ray from light
Spectrum SkyLight::sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf , float* area_pdf ) const
{
	Sort_Assert( sky != 0 );

	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
	r.m_Dir = -sky->sample_v( ls.u , ls.v , pdf , area_pdf );

	const BBox& box = scene->GetBBox();
	Point center = ( box.m_Max + box.m_Min ) * 0.5f;
	Vector delta = box.m_Max - box.m_Min;
	float world_radius = delta.Length() * 0.5f;
	r.m_Ori = center - r.m_Dir * world_radius * 3.0f;
	n = r.m_Dir;

	return sky->Evaluate( -r.m_Dir );
}

// total power of the light
Spectrum SkyLight::Power() const
{
	Sort_Assert( scene!=0 );
	BBox box = scene->GetBBox();
	float radius = (box.m_Max - box.m_Min).Length() * 0.5f;

	return radius * radius * PI * sky->GetAverage();
}

// get intersection between the light and the ray
bool SkyLight::Le( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const
{
	Sort_Assert( sky != 0 );

	if( intersect && intersect->t != FLT_MAX )
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
	_registerProperty( "transform" , new TransformProperty( this ) );
}

// the pdf for specific sampled directioin
float SkyLight::Pdf( const Point& p , const Vector& wi ) const
{
	Sort_Assert( sky );
	return sky->Pdf( p , wi );
}
