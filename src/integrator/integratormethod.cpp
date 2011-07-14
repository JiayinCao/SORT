/*
 * filename :	integratormethod.cpp
 *
 * programmer :	Cao Jiayin
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
	r.m_Dir.Normalize();
	r.m_Depth = r.m_Depth + 1;
	r.m_Ori = intersect->intersect + r.m_Dir ;

	return f * integrator->Li( scene , r );
}

// radiance along specular refraction
Spectrum	SpecularRefraction( const Scene& scene , const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator )
{
	// to be implemented
	return Spectrum();
}
