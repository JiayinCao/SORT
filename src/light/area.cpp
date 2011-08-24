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

	shape = 0;
	radius = 1.0f;
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

	Point ps = shape->sample_l( *ls , intersect.intersect , wi , pdf );
	if( pdf && *pdf == 0.0f )
		return 0.0f;

	// get the delta
	Vector dlt = ps - intersect.intersect;
	// setup visibility tester
	visibility.ray = Ray( intersect.intersect , wi , 0 , delta , dlt.Length() );

	return intensity;
}

// the pdf of the direction
float AreaLight::Pdf( const Point& p , const Point& lp , const Vector& wi ) const
{
	Sort_Assert(shape!=0);
	return shape->Pdf( p , lp , wi );
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
	_registerProperty( "shape" , new ShapeProperty(this) );
	_registerProperty( "radius" , new RadiusProperty(this) );
}

// sample light density
Spectrum AreaLight::sample_l( const Intersection& intersect , const Vector& wo ) const
{
	if( Dot( wo , intersect.normal ) > 0.0f )
		return intensity;
	return 0.0f;
}

// get intersection between the light and the ray
bool AreaLight::Evaluate( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const
{
	Sort_Assert( shape != 0 );
	Sort_Assert( intersect != 0 );

	// transform the ray to local coordinate
	Ray r = light2world.invMatrix( ray );

	// get intersect
	bool result = shape->GetIntersect( r , intersect );

	// transform the intersection result back to world coordinate
	if( result )
	{
		intersect->light_id = (int)GetID();
		intersect->intersect = light2world(intersect->intersect);
		intersect->normal = light2world(intersect->normal);
		radiance = sample_l( *intersect , -ray.m_Dir );
	}

	return result;
}
