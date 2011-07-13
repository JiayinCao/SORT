/*
 * filename :	integratormethod.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header files
#include "integratormethod.h"

// radiance along specular reflection
Spectrum	SpecularReflection( const Scene& scene , const Ray& ray , const Integrator* integrator )
{
	return integrator->Li( scene , ray );
}

// radiance along specular refraction
Spectrum	SpecularRefraction( const Scene& scene , const Ray& ray , const Integrator* integrator )
{
	// to be implemented
	return Spectrum();
}
