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

// return the radiance of a specific direction
Spectrum BidirPathTracing::Li( const Ray& ray , const PixelSample& ps ) const
{
	// random pick a light
	float pdf;
	const Light* light = scene.SampleLight( ps.light_sample[0].t , &pdf );
	if( light == 0 || pdf == 0.0f )
		return 0.0f;

	float	light_pdf = 0.0f;
	Ray		light_ray;
	light->sample_l( ps.light_sample[0] , light_ray , &light_pdf );

	// the path from light and eye
	vector<BDPT_Vertex> light_path , eye_path;

	unsigned eps = _generatePath( ray , eye_path , path_per_pixel );
	if( eps == 0 )	return 0.0f;

	unsigned lps = _generatePath( light_ray , light_path , path_per_pixel );

	Spectrum directWt = 1.0f;
	Spectrum li;
	for( unsigned i = 1 ; i <= eps ; ++i )
	{
		const BDPT_Vertex& vert = eye_path[i-1];
		li += directWt * EvaluateDirect( Ray( Point( 0.0f ) , -vert.wi ) , scene , light , vert.inter , LightSample(true) , BsdfSample(true) ) / i ;
		directWt *= vert.bsdf->f( vert.wi , vert.wo ) * SatDot( vert.wo , vert.n ) / vert.pdf;

		for( unsigned j = 1 ; j <= lps ; ++j )
			li += 450.0f * _evaluatePath( eye_path , min( eps , i ) , light_path , min( lps , j ) ) / ( ( i + j ) * light_pdf );
	}

	return li / pdf;
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

	if( sampler->RoundSize( ps ) == ps )
	{
		float* data_1d = samples[0].data;
		float* data_2d = samples[0].data + ps;

		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		for( int k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*k;
			samples[k].bsdf_sample[0].t = data_1d[k];
			samples[k].bsdf_sample[0].u = data_2d[two_k];
			samples[k].bsdf_sample[0].v = data_2d[two_k+1];
		}

		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		for( int k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*k;
			samples[k].light_sample[0].t = data_1d[k];
			samples[k].light_sample[0].u = data_2d[two_k];
			samples[k].light_sample[0].v = data_2d[two_k+1];
		}
	}else
	{
		for (int k = 0; k < ps; ++k) 
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

// generate path
unsigned BidirPathTracing::_generatePath( const Ray& ray , vector<BDPT_Vertex>& path , unsigned max_vert ) const
{
	Ray wi = ray;
	Ray wo;
	while( path.size() < max_vert )
	{
		Intersection inter;
		Spectrum li = scene.EvaluateLight( wi , &inter );
		if( false == scene.GetIntersect( wi , &inter ) )
			break;

		BDPT_Vertex vert;

		vert.p = inter.intersect;
		vert.n = inter.normal;
		vert.pri = inter.primitive;
		vert.wi = -wi.m_Dir;
		vert.inter = inter;
		
		if (path.size() > 1)
			if (sort_canonical() > 0.5f)
				break;

		vert.bsdf = inter.primitive->GetMaterial()->GetBsdf(&inter);
		vert.bsdf->sample_f( vert.wi , vert.wo , BsdfSample(true) , &vert.pdf );
		vert.pdf *= 2.0f;

		if( vert.pdf == 0.0f )
			break;

		wi = Ray( inter.intersect , vert.wo , 0 , 0.1f );

		path.push_back( vert );
	}

	return path.size();
}

// evaluate path
Spectrum BidirPathTracing::_evaluatePath(const vector<BDPT_Vertex>& epath , int esize , 
										const vector<BDPT_Vertex>& lpath , int lsize ) const
{
	Spectrum li(1.0f);

	for( int i = 0 ; i < esize - 1 ; ++i )
	{
		const BDPT_Vertex& vert = epath[i];
		li *= vert.bsdf->f( vert.wi , vert.wo ) * SatDot( vert.wo , vert.n ) / vert.pdf;
	}

	for( int i = 0 ; i < lsize - 1 ; ++i )
	{
		const BDPT_Vertex& vert = lpath[i];
		li *= vert.bsdf->f( vert.wo , vert.wi ) * SatDot( vert.wi , vert.n ) / vert.pdf;
	}

	if( lpath.empty() )
		return li;

	const BDPT_Vertex& evert = epath[esize-1];
	const BDPT_Vertex& lvert = lpath[lsize-1];
	Vector delta = evert.p - lvert.p;
	Vector n_delta = Normalize(delta);
	Spectrum l0 = evert.bsdf->f( evert.wi , -n_delta );
	Spectrum l1 = _Gterm( evert , lvert );
	Spectrum l2 = lvert.bsdf->f( n_delta , lvert.wi ) / ( evert.pdf * lvert.pdf );

	li *= l0 * l1 * l2;

	return li;
}

// compute G term
Spectrum BidirPathTracing::_Gterm( const BDPT_Vertex& p0 , const BDPT_Vertex& p1 ) const
{
	Vector delta = p0.p - p1.p;
	Vector n_delta = Normalize(delta);

	Visibility visible( scene );
	visible.ray = Ray( p1.p , n_delta  , 0 , 0.1f , delta.Length() - 0.1f );
	if( visible.IsVisible() == false )
		return 0.0f;
	
	return SatDot( p0.n , -n_delta ) * SatDot( p1.n , n_delta ) / delta.SquaredLength();
}