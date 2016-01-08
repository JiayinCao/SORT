/*
   FileName:      pointlight.cpp

   Created Time:  2011-08-04 12:48:32

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header
#include "pointlight.h"
#include "geometry/intersection.h"
#include "utility/samplemethod.h"
#include "sampler/sample.h"

IMPLEMENT_CREATOR( PointLight );

// sample ray from light
Spectrum PointLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const
{
    // Get light position
	Vector _dirToLight = light_pos - intersect.intersect;
    
    // Normalize vec
    float sqrLen = _dirToLight.SquaredLength();
    float len = sqrt(sqrLen);
	dirToLight = _dirToLight / len;
    
    // setup visibility ray
    const float delta = 0.01f;
	visibility.ray = Ray( light_pos , -dirToLight , 0 , delta , len - delta );

    // direction pdf from 'intersect' to light source w.r.t solid angle
    if( pdfw )
        *pdfw = sqrLen;
    
    if( cosAtLight )
        *cosAtLight = 1.0f;
    
    if( distance )
        *distance = len;
    
    // product of pdf of sampling a point w.r.t surface area and a direction w.r.t direction
    if( emissionPdf )
        *emissionPdf = UniformSpherePdf();
    
	return intensity;
}

// sample a ray from light
Spectrum PointLight::sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const
{
    // sample a new ray
	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
	r.m_Ori = light_pos;
	r.m_Dir = UniformSampleSphere( ls.u , ls.v );

    // product of pdf of sampling a point w.r.t surface area and a direction w.r.t direction
	if( pdfW )
        *pdfW = UniformSpherePdf();
    
    // pdf w.r.t surface area
	if( pdfA )
        *pdfA = 1.0f;
    
    if( cosAtLight )
        *cosAtLight = 1.0f;

	return intensity;
}

// register property
void PointLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "pos" , new PosProperty(this) );
}
