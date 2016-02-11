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

IMPLEMENT_CREATOR( SpotLight );

// sample ray from light
Spectrum SpotLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const
{
    // direction to light
	const Vector _dirToLight = light_pos - intersect.intersect;
    
    // Normalize vec
    const float sqrLen = _dirToLight.SquaredLength();
    const float len = sqrt(sqrLen);
    dirToLight = _dirToLight / len;
    
    // direction pdf from 'intersect' to light source w.r.t solid angle
	if( pdfw )
        *pdfw = sqrLen;
    
    // emission pdf from light source w.r.t solid angle
    if( emissionPdf )
        *emissionPdf = UniformConePdf( cos_total_range );
    
    if( cosAtLight )
        *cosAtLight = 1.0f;
    
    if( distance )
        *distance = len;
    
    // update visility
    const float delta = 0.01f;
    visibility.ray = Ray( light_pos , -dirToLight , 0 , delta , len - delta );
    
	const float falloff = SatDot( dirToLight , -light_dir );
	if( falloff <= cos_total_range )
		return 0.0f;
	if( falloff >= cos_falloff_start )
		return intensity ;
	const float d = ( falloff - cos_total_range ) / ( cos_falloff_start - cos_total_range );
	if( d == 0.0f )
		return 0.0f;

	return intensity * d * d;
}

// sample a ray from light
Spectrum SpotLight::sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const
{
    // udpate ray
	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
	r.m_Ori = light_pos;
    
    // sample a light direction
	const Vector local_dir = UniformSampleCone( ls.u , ls.v , cos_total_range );
	r.m_Dir = light2world.matrix( local_dir );

    // product of pdf of sampling a point w.r.t surface area and a direction w.r.t direction
	if( pdfW )
    {
		*pdfW = UniformConePdf( cos_total_range );
		Sort_Assert( *pdfW != 0.0f );
	}
    // pdf w.r.t surface area
	if( pdfA )
        *pdfA = 1.0f;
    if( cosAtLight )
        *cosAtLight = 1.0f;
	
	const float falloff = SatDot( r.m_Dir , light_dir );
	if( falloff <= cos_total_range )
		return 0.0f;
	if( falloff >= cos_falloff_start )
		return intensity;
	const float d = ( falloff - cos_total_range ) / ( cos_falloff_start - cos_total_range );
	if( d == 0.0f )
		return 0.0f;

	return intensity * d * d;
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
