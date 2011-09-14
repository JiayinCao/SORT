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
#include "geometry/scene.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"

// sample a ray
Spectrum DistantLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& wi , float delta , float* pdf , Visibility& visibility ) const 
{
	// distant light direction
	Vector dir( light2world.matrix.m[1] , light2world.matrix.m[5] , light2world.matrix.m[9] );
	wi = -dir;
	if( pdf ) *pdf = 1.0f;

	visibility.ray = Ray( intersect.intersect , wi , 0 , delta );

	return intensity;
}

// sample a ray from light
Spectrum DistantLight::sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf ) const
{
	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
	r.m_Dir = Vector( -light2world.matrix.m[1] , -light2world.matrix.m[5] , -light2world.matrix.m[9] );
	
	const BBox& box = scene->GetBBox();
	Point center = ( box.m_Max + box.m_Min ) * 0.5f;
	Vector delta = box.m_Max - box.m_Min;
	float world_radius = delta.Length() * 0.5f;

	Vector v0 , v1;
	CoordinateSystem( r.m_Dir , v0 , v1 );
	float u , v;
	UniformSampleDisk( ls.u , ls.v , u , v );
	Point p = ( u * v0 + v * v1 ) * world_radius + center;
	r.m_Ori = p - r.m_Dir * world_radius * 2.0f;

	if( pdf ) *pdf = 1.0f / ( PI * world_radius * world_radius );

	n = r.m_Dir;

	return intensity;
}

// register all properties
void DistantLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "dir" , new DirProperty(this) );
}
