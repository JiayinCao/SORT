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

// initialize default value
void AreaLight::_init()
{
	_registerAllProperty();

	mesh = 0;
	distribution = 0;
}

// release data
void AreaLight::_release()
{
	SAFE_DELETE(distribution);
}

// sample ray from light
Spectrum AreaLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& wi , float delta , float* pdf , Visibility& visibility ) const
{
	Sort_Assert( ls != 0 );

	// get the sampled primitive
	float _pdf;
	unsigned pri_id = distribution->SampleDiscrete( ls->t , &_pdf );
	Primitive* pri = mesh->GetPrimitive( pri_id );
	
	Point p = pri->Sample( ls->u , ls->v );

	Vector vec = p - intersect.intersect;
	wi = Normalize( vec );

	if( pdf ) *pdf = _pdf * INV_PI * 0.25;

	visibility.ray = Ray( intersect.intersect , wi , 0 , delta , vec.Length() );
	return intensity;
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

// sample light density
Spectrum AreaLight::sample_l( const Intersection& intersect , const Vector& wo ) const
{
	if( Dot( wo , intersect.normal ) > 0.0f )
		return intensity;
	return 0.0f;
}
