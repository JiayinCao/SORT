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

IMPLEMENT_CREATOR( PathTracing );

// return the radiance of a specific direction
// note : there are one factor makes the method biased.
//		there is a limitation on the number of vertexes in the path
Spectrum PathTracing::Li( const Ray& ray , const PixelSample& ps ) const
{
	Spectrum L = 0.0f;

	bool		intersect_diffuse = false;
	bool		specular = true;
	Spectrum	path_weight = 1.0f;
	unsigned	bounces = 0;
	Ray	r = ray;
	while(true)
	{
		Intersection inter;

		// get the intersection between the ray and the scene
		// if it's a light , accumulate the radiance and break
		if( false == scene.GetIntersect( r , &inter ) )
		{
			if( bounces == 0 )
				return scene.Le( r );
			else if( specular ) 
				L+=scene.Le(r)*path_weight;
			break;
		}

		if( bounces == 0 ) L+=inter.Le(-r.m_Dir);
		else if( specular ) L+=inter.Le(-r.m_Dir)*path_weight;

		// make sure there is intersected primitive
		Sort_Assert( inter.primitive != 0 );

		// evaluate the light
		Bsdf*			bsdf = inter.primitive->GetMaterial()->GetBsdf(&inter);
		float			light_pdf = 0.0f;
		LightSample		light_sample = (bounces==0)?ps.light_sample[0]:LightSample(true);
		BsdfSample		bsdf_sample = (bounces==0)?ps.bsdf_sample[0]:BsdfSample(true);
		const Light*	light = scene.SampleLight( light_sample.t , &light_pdf );
		if( light_pdf > 0.0f )
			L += path_weight * EvaluateDirect(	r  , scene , light , inter , light_sample , 
												bsdf_sample , BXDF_TYPE(BXDF_ALL&~BXDF_SPECULAR) ) / light_pdf;

		// sample the next direction using bsdf
		float		path_pdf;
		Vector		wi;
		BXDF_TYPE	bxdf_type;
		Spectrum f;
		BsdfSample	_bsdf_sample = (bounces==0)?ps.bsdf_sample[1]:BsdfSample(true);
		f = bsdf->sample_f( -r.m_Dir , wi , _bsdf_sample , &path_pdf , BXDF_ALL , &bxdf_type );
		if( f.IsBlack() || path_pdf == 0.0f )
			break;
		specular = (bxdf_type & BXDF_SPECULAR)!=0;
		if( !specular ) intersect_diffuse = true;

		// update path weight
		path_weight *= f * SatDot( wi , inter.normal ) / path_pdf;

		if( path_weight.GetIntensity() == 0.0f )
			break;
		if( bounces > 4 && intersect_diffuse )
		{
			float continueProperbility = min( 0.5f , path_weight.GetIntensity() );
			if( sort_canonical() > continueProperbility )
				break;
			path_weight /= continueProperbility;
		}

		r.m_Ori = inter.intersect + wi * 0.001f;
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
		ps[i].bsdf_sample = new BsdfSample[ 2 ];
		ps[i].light_sample = new LightSample[ 1 ];
	}

	ps[0].data = new float[ ps_num * 3 ];
}

// generate samples
void PathTracing::GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const
{
	Integrator::GenerateSample( sampler , samples , ps , scene );

	if( sampler->RoundSize( ps ) == ps )
	{
		float* data_1d = samples[0].data;
		float* data_2d = samples[0].data + ps;

		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		for( unsigned k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*k;
			samples[k].bsdf_sample[0].t = data_1d[k];
			samples[k].bsdf_sample[0].u = data_2d[two_k];
			samples[k].bsdf_sample[0].v = data_2d[two_k+1];
		}

		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		for( unsigned k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*k;
			samples[k].bsdf_sample[1].t = data_1d[k];
			samples[k].bsdf_sample[1].u = data_2d[two_k];
			samples[k].bsdf_sample[1].v = data_2d[two_k+1];
		}

		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		for( unsigned k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*k;
			samples[k].light_sample[0].t = data_1d[k];
			samples[k].light_sample[0].u = data_2d[two_k];
			samples[k].light_sample[0].v = data_2d[two_k+1];
		}
	}else
	{
		for (unsigned k = 0; k < ps; ++k) 
		{
			samples[k].bsdf_sample[0] = BsdfSample(true);
			samples[k].bsdf_sample[1] = BsdfSample(true);
			samples[k].light_sample[0] = LightSample(true);
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
	LOG<<"While it requires much more samples to reduce the noise to an acceptable level."<<ENDL<<ENDL;
}

// register property
void PathTracing::_registerAllProperty()
{
	_registerProperty( "max_path" , new MaxPathProperty(this) );
}