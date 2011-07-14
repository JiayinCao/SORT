/*
 * filename :	integratormethod.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_INTEGRATORMETHOD
#define	SORT_INTEGRATORMETHOD

// include the header
#include "integrator.h"

// pre-decleration
class Bsdf;
class Intersection;

// radiance along specular reflective direction
Spectrum	SpecularReflection( const Scene& scene , const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator );

// radiance along specular refractive direction
Spectrum	SpecularRefraction( const Scene& scene , const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator );

#endif
