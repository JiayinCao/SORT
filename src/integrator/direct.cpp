/*
   FileName:      direct.cpp

   Created Time:  2011-08-12 12:52:41

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "direct.h"
#include "integratormethod.h"
#include "geometry/intersection.h"
#include "geometry/scene.h"
#include "bsdf/bsdf.h"
#include "light/light.h"
#include "managers/memmanager.h"
#include "sampler/sampler.h"

IMPLEMENT_CREATOR( DirectLight );

// initialize default value
void DirectLight::_init()
{
	light_sample_offsets = 0;
	bsdf_sample_offsets = 0;
	total_samples = 0;
	ls_per_light = 16;	// default sample per light is 16

	_registerAllProperty();	// register all the properties
}

// release data
void DirectLight::_release()
{
	SAFE_DELETE_ARRAY(light_sample_offsets);
	SAFE_DELETE_ARRAY(bsdf_sample_offsets);
}

// radiance along a specific ray direction
Spectrum DirectLight::Li( const Ray& r , const PixelSample& ps ) const
{
	if( r.m_Depth > max_recursive_depth )
		return 0.0f;

	// get the intersection between the ray and the scene
	Intersection ip;
	// evaluate light directly
	if( false == scene.GetIntersect( r , &ip ) )
		return scene.Le( r );

	Spectrum t = ip.Le( -r.m_Dir );

	// eavluate direct light
	unsigned light_num = scene.LightNum();
	for( unsigned i = 0 ; i < light_num ; ++i )
	{
		const Light* light = scene.GetLight(i);
		Spectrum ld;
		unsigned sample_num = light_sample_offsets[i].num;
		for( unsigned k = 0 ; k < sample_num ; ++k )
			ld += EvaluateDirect(	r  , scene , light , ip , ps.light_sample[light_sample_offsets[i].offset+k] , 
									ps.bsdf_sample[bsdf_sample_offsets[i].offset+k] , BXDF_TYPE( BXDF_ALL & ~BXDF_SPECULAR ) );
		if( sample_num != 0 )
			t += ld / (float)sample_num;
	}

	// evaluate specular reflection or refraction
	Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );
	if( bsdf->NumComponents( BXDF_SPECULAR_REFLECTION ) > 0 )
		t += SpecularReflection( r , &ip , bsdf , this , ps );
	if( bsdf->NumComponents( BXDF_SPECULAR_TRANSMISSION ) > 0 )
		t += SpecularRefraction( r , &ip , bsdf , this , ps );

	return t;
}

// output log information
void DirectLight::OutputLog() const
{
	LOG_HEADER( "Integrator" );
	LOG<<"Integrator algorithm : Direct Light Integrator."<<ENDL;
	LOG<<"It supports direct lighting , specular reflection and specular refraction."<<ENDL;
	LOG<<"Soft shadow and area light are also supported in the algorithm."<<ENDL;
	LOG<<"Indirect lighting , like color bleeding , caustics , is not supported."<<ENDL<<ENDL;
}

// request samples
void DirectLight::RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num )
{
	SAFE_DELETE_ARRAY(light_sample_offsets);
	SAFE_DELETE_ARRAY(bsdf_sample_offsets);

	unsigned light_num = scene.LightNum();
	unsigned total_light_num = ls_per_light * light_num;
	light_sample_offsets = new SampleOffset[light_num];
	bsdf_sample_offsets = new SampleOffset[light_num];

	total_samples = 0;
	for( unsigned i = 0 ; i < ps_num ; i++ )
	{
		for( unsigned k = 0 ; k < light_num ; k++ )
		{
			float properbility = scene.LightProperbility(k);
			unsigned lsn = (unsigned)( properbility * total_light_num + 0.5f );

			const Light* light = scene.GetLight(k);
			if( light->IsDelta() ) lsn = 1;

			if( i == 0 )
			{
				light_sample_offsets[k].num = sampler->RoundSize( lsn );
				light_sample_offsets[k].offset = ps[i].RequestMoreLightSample( light_sample_offsets[i].num );
				bsdf_sample_offsets[k].num = light_sample_offsets[k].num;
				bsdf_sample_offsets[k].offset = ps[i].RequestMoreBsdfSample( bsdf_sample_offsets[i].num );
				total_samples += bsdf_sample_offsets[k].num;
			}else
			{
				ps[i].RequestMoreLightSample( light_sample_offsets[i].num );
				ps[i].RequestMoreLightSample( light_sample_offsets[i].num );
			}
		}
		ps[i].bsdf_sample = new BsdfSample[ total_samples ];
		ps[i].light_sample = new LightSample[ total_samples ];
	}

	// allocate the data to be used
	ps[0].data = new float[ total_samples * 5 ];
}

// generate samples
void DirectLight::GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const
{
	Integrator::GenerateSample( sampler , samples , ps , scene );

	unsigned light_num = scene.LightNum();
	for( unsigned i = 0 ; i < ps ; ++i )
	{
		unsigned offset = 0;

		float* light_2d = samples[0].data;
		float* bsdf_1d = samples[0].data + total_samples * 2;
		float* bsdf_2d = samples[0].data + total_samples ;
		for( unsigned k = 0 ; k < light_num ; ++k )
		{
			sampler->Generate2D( light_2d + 2 * offset , light_sample_offsets[k].num );
			sampler->Generate1D( bsdf_1d + offset , light_sample_offsets[k].num );
			sampler->Generate2D( bsdf_2d + 2 * offset , bsdf_sample_offsets[k].num );
			offset += bsdf_sample_offsets[k].num;
		}

		for( unsigned k = 0 ; k < total_samples ; k++ )
		{
			samples[i].light_sample[k].u = light_2d[2*k];
			samples[i].light_sample[k].v = light_2d[2*k+1];
			samples[i].bsdf_sample[k].t = bsdf_1d[k];
			samples[i].bsdf_sample[k].u = bsdf_2d[2*k];
			samples[i].bsdf_sample[k].v = bsdf_2d[2*k+1];
		}
	}
}

void DirectLight::_registerAllProperty()
{
	_registerProperty( "sample_per_light" , new SamplerPerLightProperty(this) );
}