/*
   FileName:      spot.cpp

   Created Time:  2011-08-04 12:48:39

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "spot.h"
#include "geometry/intersection.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"

// sample ray from light
Spectrum SpotLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& wi , float delta , float* pdf , Visibility& visibility ) const 
{
	Point pos( light2world.matrix.m[3] , light2world.matrix.m[7] , light2world.matrix.m[11] );
	Vector dir( light2world.matrix.m[1] , light2world.matrix.m[5] , light2world.matrix.m[9] );
	Vector vec = pos - intersect.intersect;
	wi = Normalize( vec );
	if( pdf ) *pdf = 1.0f;

	float falloff = SatDot( wi , -dir );
	if( falloff < cos_total_range )
		return 0.0f;
	if( falloff > cos_falloff_start )
		return intensity / vec.SquaredLength();
	float d = ( falloff - cos_total_range ) / ( cos_falloff_start - cos_total_range );
	if( d == 0.0f )
		return 0.0f;

	float len = vec.Length();
	visibility.ray = Ray( pos , -wi , 0 , delta , len - delta );

	return intensity / vec.SquaredLength() * d * d;
}

// sample a ray from light
void SpotLight::sample_l( const LightSample& ls , Ray& r , float* pdf ) const
{
	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
	r.m_Ori = Point( light2world.matrix.m[3] , light2world.matrix.m[7] , light2world.matrix.m[11] );
	r.m_Dir = UniformSampleCone( ls.u , ls.v , cos_total_range );
	r.m_Dir = light2world.matrix( r.m_Dir );

	if( pdf ) *pdf = UniformConePdf( cos_total_range );
}

// register property
void SpotLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "pos" , new PosProperty(this) );
	_registerProperty( "falloff_start" , new FalloffStartProperty(this) );
	_registerProperty( "range" , new RangeProperty(this) );
	_registerProperty( "dir" , new DirProperty(this) );
}

// initialize default value
void SpotLight::_init()
{
	cos_total_range = 0.0f;
	cos_falloff_start = 1.0f;
}
