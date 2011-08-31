/*
   FileName:      pathtracing.cpp

   Created Time:  2011-08-31 09:34:33

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "pathtracing.h"
#include "geometry/intersection.h"
#include "bsdf/bsdf.h"
#include "geometry/scene.h"
#include "integratormethod.h"

// return the radiance of a specific direction
Spectrum PathTracing::Li( const Scene& scene , const Ray& ray , const PixelSample& ps ) const
{
	Spectrum L = 0.0f;

	Spectrum	path_weight = 1.0f;
	unsigned	bounces = 0;
	Ray	r = ray;
	while(true)
	{
		Intersection inter;

		// get the intersection between the ray and the scene
		// if it's a light , accumulate the radiance and break
		// note that light don't reflect radiance themselves
		Spectrum li = scene.EvaluateLight( r , &inter );
		if( false == scene.GetIntersect( r , &inter ) )
		{
			if( bounces == 0 ) L+=li;
			break;
		}

		// make sure there is intersected primitive
		Sort_Assert( inter.primitive != 0 );

		// evaluate the light
		Bsdf*			bsdf = inter.primitive->GetMaterial()->GetBsdf(&inter);
		float			light_pdf = 0.0f;
		const Light*	light = scene.SampleLight( ps.light_sample[bounces].t , &light_pdf );
		if( light_pdf > 0.0f )
			L += path_weight * EvaluateDirect(	r  , scene , light , inter , ps.light_sample[bounces] , 
												ps.bsdf_sample[bounces] ) / light_pdf;

		// sample the next direct using bsdf
		float		path_pdf;
		Vector		wi;
		Spectrum f = bsdf->sample_f( -r.m_Dir , wi , ps.bsdf_sample[path_per_pixel+bounces] , &path_pdf );
		if( f.IsBlack() || path_pdf == 0.0f )
			break;

		// update path weight
		path_weight *= f * AbsDot( wi , inter.normal ) / path_pdf;

		if( path_weight.GetIntensity() == 0.0f )
			break;
		if( bounces > 3 )
		{
			float continueProperbility = min( 0.5f , path_weight.GetIntensity() );
			if( sort_canonical() > continueProperbility )
				break;
			path_weight /= continueProperbility;
		}

		r.m_Ori = inter.intersect + wi * 0.1f;
		r.m_Dir = wi;

		++bounces;

		// note :	the following code makes the method biased
		//			'path_per_pixel' could be set very large to reduce the side-effect.
		if( bounces >= path_per_pixel )
			break;
	}

	return L;
}

// request samples
void PathTracing::RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num )
{
	for( unsigned i = 0 ; i < ps_num ; i++ )
	{
		// the first half samples are used to sample bsdf for shading
		// the second half samples are used to sample bsdf for direction
		ps[i].bsdf_sample = new BsdfSample[ path_per_pixel * 2 ];
		ps[i].light_sample = new LightSample[ path_per_pixel ];
	}
}

// generate samples
void PathTracing::GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const
{
	Integrator::GenerateSample( sampler , samples , ps , scene );

	// the sampler is not used here , to be improved
	for (int k = 0; k < ps; ++k) {
		for (int i = 0; i < path_per_pixel; ++i) {
			samples[k].bsdf_sample[i] = BsdfSample(true);
			samples[k].bsdf_sample[path_per_pixel+i] = BsdfSample(true);
			samples[k].light_sample[i] = LightSample(true);
		}
	}
}

// output log information
void PathTracing::OutputLog() const
{
	LOG_HEADER( "Integrator" );
	LOG<<"Integrator algorithm : Path Tracing."<<ENDL;
	LOG<<"It supports all of the features in direct lighting algorithm."<<ENDL;
	LOG<<"Some global illumination effect is also supported in path tracing."<<ENDL;
	LOG<<"While it requires much more samples to reduce the noise to an acceptable level."<<ENDL;
}