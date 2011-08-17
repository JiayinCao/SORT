/*
   FileName:      integratormethod.h

   Created Time:  2011-08-04 12:48:59

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_INTEGRATORMETHOD
#define	SORT_INTEGRATORMETHOD

// include the header
#include "integrator.h"

// pre-decleration
class Bsdf;
class Intersection;

// radiance along specular reflective direction
Spectrum	SpecularReflection( const Scene& scene , const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator , const PixelSample& ps );

// radiance along specular refractive direction
Spectrum	SpecularRefraction( const Scene& scene , const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator , const PixelSample& ps );

// evaluate direct lighting
Spectrum	EvaluateDirect( const Ray& r , const Scene& scene , const Intersection& ip , const PixelSample& ps );

#endif
