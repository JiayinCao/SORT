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

IMPLEMENT_CREATOR( DistantLight );

// sample a ray
Spectrum DistantLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const
{
	// distant light direction
	dirToLight = -light_dir;
    
	if( pdfw )
        *pdfw = 1.0f;
    
    if( distance )
        *distance = 1e6f;
    
    if( emissionPdf )
    {
        const BBox& box = scene->GetBBox();
        Vector delta = box.m_Max - box.m_Min;
        *emissionPdf = 0.25f * INV_PI / delta.SquaredLength();
    }
    
    if( cosAtLight )
        *cosAtLight = 1.0f;

    const float delta = 0.01f;
	visibility.ray = Ray( intersect.intersect , dirToLight , 0 , delta );

	return intensity;
}

// sample a ray from light
Spectrum DistantLight::sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const
{
	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
    r.m_Dir = light_dir;
	
	const BBox& box = scene->GetBBox();
	const Point center = ( box.m_Max + box.m_Min ) * 0.5f;
	const Vector delta = box.m_Max - center;
	const float world_radius = delta.Length();

	Vector v0 , v1;
	CoordinateSystem( r.m_Dir , v0 , v1 );
	float u , v;
	UniformSampleDisk( ls.u , ls.v , u , v );
	const Point p = ( u * v0 + v * v1 ) * world_radius + center;
	r.m_Ori = p - r.m_Dir * world_radius * 3.0f;

    const float pdf = 1.0f / ( PI * world_radius * world_radius );
	if( pdfW ) *pdfW = pdf;
	if( pdfA ) *pdfA = pdf;
    if( cosAtLight ) *cosAtLight = 1.0f;
    
	return intensity;
}

// register all properties
void DistantLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "dir" , new DirProperty(this) );
}
