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
}

// release data
void AreaLight::_release()
{
}

// sample ray from light
Spectrum AreaLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& wi , float delta , float* pdf , Visibility& visibility ) const
{
	Sort_Assert( ls != 0 );

	// sample the primitive
	Primitive* pri = mesh->SamplePrimitive( ls->t , pdf );
	// sample a point on the primitive
	Point p = pri->Sample( ls->u , ls->v );

	// get light direction
	Vector vec = p - intersect.intersect;
	wi = Normalize( vec );
	float length = 0.0f;

	Ray inv_r( intersect.intersect , wi );
	Intersection ip;
	if( mesh->GetIntersect( inv_r , &ip ) )
		length = ( intersect.intersect - ip.intersect ).Length();
	else
		length = vec.Length();

	// setup visibility tester
	visibility.ray = Ray( intersect.intersect , wi , 0 , delta , length - delta );

	// update pdf
	if( pdf ) *pdf *= INV_PI * 0.25f / pri->SurfaceArea();

	return intensity;
}

// total power of the light
Spectrum AreaLight::Power() const
{
	Sort_Assert( mesh != 0 );
	return mesh->GetSurfaceArea() * intensity.GetIntensity() * 4 * PI;
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

// preprocess
void AreaLight::PreProcess()
{
	mesh->GenTriDistribution();
	mesh->BuildAccel("bvh");
}