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

// radiance along a specific ray direction
Spectrum DirectLight::Li( const Scene& scene , const Ray& r , const PixelSample& ps ) const
{
	if( r.m_Depth > 6 )
		return 0.0f;

	// get the intersection between the ray and the scene
	Intersection ip;
	if( false == scene.GetIntersect( r , &ip ) )
		return scene.EvaluateSky( r );

	// evaluate light directly
	Spectrum t = ip.primitive->GetEmissive( -r.m_Dir , ip );

	// eavluate direct light
	t += EvaluateDirect( r , scene , ip , ps );

	// evaluate specular reflection or refraction
	Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );
	if( bsdf->NumComponents( BXDF_REFLECTION ) > 0 )
		t += SpecularReflection( scene , r , &ip , bsdf , this , ps );
	if( bsdf->NumComponents( BXDF_TRANSMISSION ) > 0 )
		t += SpecularRefraction( scene , r , &ip , bsdf , this , ps );

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

// generate samples
void DirectLight::GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const
{
	Integrator::GenerateSample( sampler , samples , ps , scene );

	// total light sample
	unsigned total = ps * ls_per_ps;
	unsigned light_num = scene.LightNum();

	unsigned* ls_num = SORT_MALLOC_ARRAY( unsigned , light_num )();
	for( unsigned i = 0 ; i < light_num ; ++i )
		ls_num[i] = sampler->RoundSize((unsigned)(scene.LightProperbility(i) * total ));
	unsigned total_ls = 0 ;
	for( unsigned i = 0 ; i < light_num ; ++i )
		total_ls += ls_num[i];
	float* ld_1d = SORT_MALLOC_ARRAY( float , total_ls * 3 );
	float* ld_2d = ld_1d + total_ls;
	unsigned* light_id = SORT_MALLOC_ARRAY( unsigned , total_ls );
	float* p1d = ld_1d;
	float* p2d = ld_2d;
	unsigned offset = 0;
	for( unsigned i = 0 ; i < light_num ; ++i )
	{
		sampler->Generate1D( p1d , ls_num[i] );
		sampler->Generate2D( p2d , ls_num[i] );
		p1d += ls_num[i];
		p2d += ls_num[i] * 2;

		for( unsigned k = 0 ; k < ls_num[i] ; k++ )
		{
			light_id[offset] = i;
			++offset;
		}
	}
	// shuffle the index
	const unsigned* shuffled_id = ShuffleIndex( total_ls );
	// actual number of light sample per pixel sample
	unsigned lpp = (unsigned)ceil( (float)total_ls / (float)ps );
	offset = 0;
	LightSample* ls = SORT_MALLOC_ARRAY(LightSample,total_ls)();
	for( unsigned i = 0 ; i < ps ; ++i )
		samples[i].light_sample.clear();
	for( unsigned k = 0 ; k < lpp ; k++ )
	{
		for( unsigned i = 0 ; i < ps ; i++ )
		{
			unsigned shuffled = shuffled_id[offset];
			ls[offset].light_id = light_id[shuffled];
			ls[offset].t = ld_1d[shuffled];
			ls[offset].u = ld_2d[2*shuffled];
			ls[offset].v = ld_2d[2*shuffled+1];
			samples[i].light_sample.push_back( &ls[offset] );
			++offset;
			if( offset >= total_ls ) break;
		}
		if( offset >= total_ls ) break;
	}

	unsigned bsdf_sample = sampler->RoundSize( samples[0].light_sample.size() );
	float* bd_1d = SORT_MALLOC_ARRAY( float , bsdf_sample * 3 );
	float* bd_2d = bd_1d + bsdf_sample;
	BsdfSample* bsdf_samples = SORT_MALLOC_ARRAY( BsdfSample , bsdf_sample * ps )();
	offset = 0;
	for( unsigned i = 0 ; i < ps ; ++i )
	{
		samples[i].bsdf_sample.clear();
		if( bsdf_sample != samples[i].bsdf_sample.size() )
			samples[i].bsdf_sample.resize(bsdf_sample);
		sampler->Generate1D( bd_1d , bsdf_sample );
		sampler->Generate2D( bd_2d , bsdf_sample );
		for( unsigned k = 0 ; k < bsdf_sample ; ++k )
		{
			bsdf_samples[offset].t = bd_1d[k];
			bsdf_samples[offset].u = bd_2d[2*k];
			bsdf_samples[offset].v = bd_2d[2*k+1];
			samples[i].bsdf_sample[k] = &bsdf_samples[offset];
			++offset;
		}
	}
}

// pre-process before rendering
void DirectLight::PreProcess( Scene& scene )
{
	// generate triangle distributuion for sampling
	scene.PreProcessLight();
}