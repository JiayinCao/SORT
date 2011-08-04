/*
   FileName:      integratormethod.cpp

   Created Time:  2011-08-04 12:48:57

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header files
#include "integratormethod.h"
#include "geometry/ray.h"
#include "geometry/intersection.h"
#include "bsdf/bsdf.h"

// radiance along specular reflection
Spectrum	SpecularReflection( const Scene& scene , const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator )
{
	Ray r;
	Spectrum f = bsdf->sample_f( -ray.m_Dir , r.m_Dir , 0 , BXDF_REFLECTION );
	if( f.IsBlack() || r.m_Dir.IsZero() )
		return 0.0f;

	r.m_Depth = ray.m_Depth + 1;
	r.m_Ori = intersect->intersect + r.m_Dir ;

	float density = AbsDot( r.m_Dir , intersect->normal );

	return f * integrator->Li( scene , r ) * density;
}

// radiance along specular refraction
Spectrum	SpecularRefraction( const Scene& scene , const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator )
{
	Ray r;
	Spectrum f = bsdf->sample_f( -ray.m_Dir , r.m_Dir , 0 , BXDF_TRANSMISSION );
	if( f.IsBlack() || r.m_Dir.IsZero() )
		return 0.0f;

	r.m_Depth = ray.m_Depth + 1;
	r.m_Ori = intersect->intersect + r.m_Dir;

	float density = AbsDot( r.m_Dir , intersect->normal ) ;

	return f * integrator->Li( scene , r ) * density;
}
