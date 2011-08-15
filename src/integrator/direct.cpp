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
Spectrum DirectLight::Li( const Scene& scene , const Ray& r ) const
{
	if( r.m_Depth > 6 )
		return 0.0f;

	// get the intersection between the ray and the scene
	Intersection ip;
	if( false == scene.GetIntersect( r , &ip ) )
		return scene.EvaluateSky( r );

	Spectrum t;

	// get bsdf
	Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );

	// lights
	Visibility visibility(scene);
	const vector<Light*>& lights = scene.GetLights();
	vector<Light*>::const_iterator it = lights.begin();
	while( it != lights.end() )
	{
		Vector	lightDir;
		float	pdf;
		Spectrum ld = (*it)->sample_f( ip , lightDir , &pdf , visibility );
		if( ld.IsBlack() )
		{
			it++;
			continue;
		}
		Spectrum f = bsdf->f( -r.m_Dir , lightDir );
		if( f.IsBlack() )
		{
			it++;
			continue;
		}
		bool visible = visibility.IsVisible();
		if( visible )
			t += (ld * f * SatDot( lightDir , ip.normal ) / pdf);
		it++;
	}

	// add reflection
	if( bsdf->NumComponents( BXDF_REFLECTION ) > 0 )
		t += SpecularReflection( scene , r , &ip , bsdf , this );
	if( bsdf->NumComponents( BXDF_TRANSMISSION ) > 0 )
		t += SpecularRefraction( scene , r , &ip , bsdf , this );

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
	float* ld_1d = SORT_MALLOC_ARRAY( float , total_ls );
	float* ld_2d = SORT_MALLOC_ARRAY( float , 2 * total_ls );
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
	// actual number of light sample per pixel sample
	unsigned lpp = (unsigned)ceil( (float)total_ls / (float)ps );
	offset = 0;
	for( unsigned i = 0 ; i < ps ; ++i )
	{
		for( unsigned k = 0 ; k < lpp ; k++ )
		{
			LightSample* ls = SORT_MALLOC( LightSample )();
			ls->light_id = light_id[offset];
			ls->t = p1d[offset];
			ls->u = p2d[2*offset];
			ls->v = p2d[2*offset+1];
			samples[i].light_sample.push_back( ls );
			++offset;
		}
	}

	unsigned bsdf_sample = sampler->RoundSize( ps );
	float* bd_1d = SORT_MALLOC_ARRAY( float , bsdf_sample );
	float* bd_2d = SORT_MALLOC_ARRAY( float , bsdf_sample * 2 );
	sampler->Generate1D( bd_1d , bsdf_sample );
	sampler->Generate2D( bd_2d , bsdf_sample );
	for( unsigned i = 0 ; i < bsdf_sample ; ++i )
	{
		BsdfSample* bs = SORT_MALLOC(BsdfSample)();
		bs->t = bd_1d[i];
		bs->u = bd_2d[2*i];
		bs->v = bd_2d[2*i+1];
		samples[i].bsdf_sample.push_back( bs );
	}
}
