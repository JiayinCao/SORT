/*
   FileName:      bidirpath.cpp

   Created Time:  2011-09-03 10:18:18

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "sort.h"
#include "bidirpath.h"
#include "geometry/scene.h"
#include "light/light.h"
#include "bsdf/bsdf.h"
#include "integratormethod.h"

IMPLEMENT_CREATOR( BidirPathTracing );

// return the radiance of a specific direction
Spectrum BidirPathTracing::Li( const Ray& ray , const PixelSample& ps ) const
{
	// pick a light randomly
	float pdf;
	const Light* light = scene.SampleLight( ps.light_sample[0].t , &pdf );
	if( light == 0 || pdf == 0.0f )
		return 0.0f;

	float	light_pdf = 0.0f;
	Ray		light_ray;
	Vector	light_normal;
	Spectrum le = light->sample_l( ps.light_sample[0] , light_ray , light_normal, &light_pdf , 0 );
	Spectrum li;

	vector<BDPT_Vertex> light_path;
	Ray wi = light_ray;
	Spectrum accu_radiance = le * SatDot(light_ray.m_Dir, light_normal) / (light_pdf * pdf);
	while ((int)light_path.size() < path_per_pixel)
	{
		BDPT_Vertex vert;
		if (false == scene.GetIntersect(wi, &vert.inter))
			break;

		vert.p = vert.inter.intersect;
		vert.n = vert.inter.normal;
		vert.wi = -wi.m_Dir;
		vert.bsdf = vert.inter.primitive->GetMaterial()->GetBsdf(&vert.inter);
		vert.accu_radiance = accu_radiance;

		light_path.push_back(vert);

		float bsdf_pdf;
		Spectrum bsdf_value = vert.bsdf->sample_f(vert.wi, vert.wo, BsdfSample(true), &bsdf_pdf, BXDF_ALL);
		accu_radiance *= bsdf_value * AbsDot(vert.wo, vert.n) / bsdf_pdf;

		if (bsdf_pdf == 0 || bsdf_value.IsBlack())
			break;

		// russian roulette
		if (light_path.size() > 4 || accu_radiance.GetIntensity() < 0.01f)
		{
			if (sort_canonical() > 0.5f)
				break;
			else
				accu_radiance *= 2.0f;
		}

		wi = Ray(vert.inter.intersect, vert.wo, 0, 0.001f);
	}
	unsigned lps = light_path.size();

	wi = ray;
	accu_radiance = 1.0f;
	int light_path_len = 0;
	while (light_path_len < path_per_pixel)
	{
		BDPT_Vertex vert;
		if (false == scene.GetIntersect(wi, &vert.inter))
		{
			if (scene.GetSkyLight() == light)
				li += scene.Le(wi) * accu_radiance * _Weight(light_path_len, -2, light) / pdf;
			break;
		}

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: it hits a light source
		if (vert.inter.primitive->GetLight() == light)
			li += vert.inter.Le(-wi.m_Dir) * accu_radiance * _Weight(light_path_len, -2, light) / pdf;

		vert.p = vert.inter.intersect;
		vert.n = vert.inter.normal;
		vert.wi = -wi.m_Dir;
		vert.bsdf = vert.inter.primitive->GetMaterial()->GetBsdf(&vert.inter);
		vert.accu_radiance = accu_radiance;

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: connect light sample first
		li += _ConnectLight(vert, light) * _Weight(light_path_len, -1, light) / pdf;

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: connect vertices
		for (unsigned j = 0; j < lps; ++j)
			li += vert.accu_radiance * light_path[j].accu_radiance * _Gterm( vert , light_path[j] ) * _Weight(light_path_len, j, light);

		++light_path_len;

		float bsdf_pdf;
		Spectrum bsdf_value = vert.bsdf->sample_f(vert.wi, vert.wo, BsdfSample(true), &bsdf_pdf, BXDF_ALL);
		accu_radiance *= bsdf_value * AbsDot(vert.wo, vert.n) / bsdf_pdf;

		if (bsdf_pdf == 0 || bsdf_value.IsBlack())
			break;

		// russian roulette
		if (light_path_len > 4 || accu_radiance.GetIntensity() < 0.01f )
		{
			if (sort_canonical() > 0.5f)
				break;
			else
				accu_radiance *= 2.0f;
		}

		wi = Ray(vert.inter.intersect, vert.wo, 0, 0.001f);
	}

	return li;
}

// request samples
void BidirPathTracing::RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num )
{
	for( unsigned i = 0 ; i < ps_num ; i++ )
	{
		ps[i].bsdf_sample = new BsdfSample[ 1 ];
		ps[i].light_sample = new LightSample[ 1 ];
	}
	ps[0].data = new float[ ps_num * 3 ];
}

// generate samples
void BidirPathTracing::GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const
{
	Integrator::GenerateSample( sampler , samples , ps , scene );

	if( sampler->RoundSize( ps ) == ps && false )
	{
		float* data_1d = samples[0].data;
		float* data_2d = samples[0].data + ps;

		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		const unsigned* shuffled_id0 = ShuffleIndex( ps );

		for( unsigned k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*shuffled_id0[k];
			samples[k].bsdf_sample[0].t = data_1d[two_k];
			samples[k].bsdf_sample[0].u = data_2d[two_k];
			samples[k].bsdf_sample[0].v = data_2d[two_k+1];
		}

		const unsigned* shuffled_id1 = ShuffleIndex( ps );
		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		for( unsigned k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*shuffled_id1[k];
			samples[k].light_sample[0].t = data_1d[two_k];
			samples[k].light_sample[0].u = data_2d[two_k];
			samples[k].light_sample[0].v = data_2d[two_k+1];
		}
	}else
	{
		for (unsigned k = 0; k < ps; ++k) 
		{
			samples[k].bsdf_sample[0] = BsdfSample(true);
			samples[k].light_sample[0] = LightSample(true);
		}
	}
}

// output log information
void BidirPathTracing::OutputLog() const
{
	LOG_HEADER( "Integrator" );
	LOG<<"Integrator algorithm : Bidirectional Path Tracing."<<ENDL;
	LOG<<"It supports all of the features in direct lighting algorithm."<<ENDL;
	LOG<<"Some global illumination effect is also supported in path tracing."<<ENDL;
	LOG<<"While it requires much more samples to reduce the noise to an acceptable level."<<ENDL;
	LOG<<"Bidirectional Path Tracing trace rays from eye and light source at the same time."<<ENDL;
}

// compute G term
Spectrum BidirPathTracing::_Gterm( const BDPT_Vertex& p0 , const BDPT_Vertex& p1 ) const
{
	Vector delta = p0.p - p1.p;
	Vector n_delta = Normalize(delta);

	Spectrum g = AbsDot( p0.n , -n_delta ) * p1.bsdf->f( p1.wi , n_delta ) * p0.bsdf->f( -n_delta, p0.wi ) * AbsDot( p1.n , n_delta ) / delta.SquaredLength();
	if( g.IsBlack() )
		return 0.0f;

	Visibility visible( scene );
	visible.ray = Ray( p1.p , n_delta  , 0 , 0.01f , delta.Length() - 0.01f );
	if( visible.IsVisible() == false )
		return 0.0f;
	
	return g;
}

// weight the path
float BidirPathTracing::_Weight( int esize , int lsize , const Light* light ) const
{
	if( light->IsDelta() )
		return 1.0f / (esize+lsize+2);

	// MIS to be implemented
	return 1.0f / (esize + lsize+3);
}

// connect light sample
Spectrum BidirPathTracing::_ConnectLight(const BDPT_Vertex& eye_vertex , const Light* light ) const
{
	// drop the light vertex, take a new sample here
	LightSample sample(true);
	Vector wi;
	Visibility visibility(scene);
	float light_pdf;
	Spectrum li = light->sample_l(eye_vertex.inter, &sample, wi, 0.1f, &light_pdf, visibility);
	li *= eye_vertex.accu_radiance * eye_vertex.bsdf->f(eye_vertex.wi, wi) * AbsDot(eye_vertex.n, wi) / light_pdf;

	if (li.IsBlack())
		return 0.0f;

	if (visibility.IsVisible() == false)
		return 0.0f;

	return li;
}