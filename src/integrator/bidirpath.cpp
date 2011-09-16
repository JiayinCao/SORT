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
	Vector	n;
	Spectrum le = light->sample_l( ps.light_sample[0] , light_ray , n , &light_pdf );
	le *= SatDot( light_ray.m_Dir , n ) / pdf ;

	// the path from light and eye
	vector<BDPT_Vertex> light_path , eye_path;

	unsigned eps = _generatePath( ray , 1.0f , eye_path , path_per_pixel );
	if( eps == 0 )	return scene.Le( ray );

	unsigned lps = _generatePath( light_ray , light_pdf , light_path , path_per_pixel );
	BDPT_Vertex light_inter;
	light_inter.p = light_ray.m_Ori;
	light_inter.n = n;
	light_inter.pdf = light_pdf;

	Spectrum directWt = 1.0f;
	Spectrum li;
	for( unsigned i = 1 ; i <= eps ; ++i )
	{
		const BDPT_Vertex& vert = eye_path[i-1];
		directWt /= vert.pdf * vert.rr;
		if( directWt.IsBlack() == false )
		{
			li += directWt * EvaluateDirect( Ray( Point( 0.0f ) , -vert.wi ) , scene , light , vert.inter , 
				LightSample(true) , BsdfSample(true) ) *
				_Weight( eye_path , i , light_path , 0 , light_inter ) / pdf;
		}
		if( vert.pdf != 0 )
			directWt *= vert.bsdf->f( vert.wi , vert.wo ) * SatDot( vert.wo , vert.n );
		else
			directWt = 0.0f;

		for( unsigned j = 1 ; j <= lps ; ++j )
			li += le * _evaluatePath( eye_path , i , light_path , j ) * _Weight( eye_path , i , light_path , j , light_inter );
	}

	return li + eye_path[0].inter.Le( -ray.m_Dir );
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
		const unsigned* shuffled_id0 = ShuffleIndex( ps );

		for( int k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*shuffled_id0[k];
			samples[k].bsdf_sample[0].t = data_1d[two_k];
			samples[k].bsdf_sample[0].u = data_2d[two_k];
			samples[k].bsdf_sample[0].v = data_2d[two_k+1];
		}

		const unsigned* shuffled_id1 = ShuffleIndex( ps );
		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		for( int k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*shuffled_id1[k];
			samples[k].light_sample[0].t = data_1d[two_k];
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
unsigned BidirPathTracing::_generatePath( const Ray& ray , float base_pdf , vector<BDPT_Vertex>& path , unsigned max_vert ) const
{
	Ray wi = ray;
	Ray wo;
	float pdf = base_pdf;
	while( path.size() < max_vert )
	{
		BDPT_Vertex vert;
		if( false == scene.GetIntersect( wi , &vert.inter ) )
			break;

		vert.p = vert.inter.intersect;
		vert.n = vert.inter.normal;
		vert.pri = vert.inter.primitive;
		vert.wi = -wi.m_Dir;
		vert.pdf = pdf;
		vert.bsdf = vert.inter.primitive->GetMaterial()->GetBsdf(&vert.inter);

		if (path.size() > 4 )
		{
			if (sort_canonical() > 0.5f)
				break;
			else
				vert.rr = 0.5f;
		}

		vert.bsdf->sample_f( vert.wi , vert.wo , BsdfSample(true) , &pdf );

		path.push_back( vert );

		wi = Ray( vert.inter.intersect , vert.wo , 0 , 0.1f );
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
		li *= vert.bsdf->f( vert.wi , vert.wo ) * SatDot( vert.wo , vert.n ) / ( vert.pdf * vert.rr );
	}

	for( int i = 0 ; i < lsize - 1 ; ++i )
	{
		const BDPT_Vertex& vert = lpath[i];
		li *= vert.bsdf->f( vert.wo , vert.wi ) * SatDot( vert.wo , vert.n ) / ( vert.pdf * vert.rr );
	}

	if( lpath.empty() )
		return 0.0f;

	const BDPT_Vertex& evert = epath[esize-1];
	const BDPT_Vertex& lvert = lpath[lsize-1];
	Vector delta = evert.p - lvert.p;
	Vector n_delta = Normalize(delta);
	Spectrum l0 = evert.bsdf->f( evert.wi , -n_delta );
	float l1 = _Gterm( evert , lvert );
	Spectrum l2 = lvert.bsdf->f( n_delta , lvert.wi );

	li *= l0 * l1 * l2 / ( evert.pdf * evert.rr * lvert.rr * lvert.pdf );

	return li;
}

// compute G term
float BidirPathTracing::_Gterm( const BDPT_Vertex& p0 , const BDPT_Vertex& p1 ) const
{
	Vector delta = p0.p - p1.p;
	Vector n_delta = Normalize(delta);

	float g = SatDot( p0.n , -n_delta ) * SatDot( p1.n , n_delta ) / delta.SquaredLength();
	if( g == 0.0f )
		return 0.0f;

	Visibility visible( scene );
	visible.ray = Ray( p1.p , n_delta  , 0 , 0.1f , delta.Length() - 0.1f );
	if( visible.IsVisible() == false )
		return 0.0f;
	
	return g;
}

// weight the path
float BidirPathTracing::_Weight(const vector<BDPT_Vertex>& epath , int esize , 
								const vector<BDPT_Vertex>& lpath , int lsize ,
								const BDPT_Vertex& light_pos ) const
{
	float total_pdf = 0.0f;
	int total_size = esize + lsize;
	for( int i = 1 ; i <= total_size ; ++i )
		total_pdf += _PathPDF( epath , i , lpath , total_size - i , light_pos );

	float pdf = _PathPDF( epath , esize , lpath , lsize , light_pos );
	if( pdf == 0.0f )
		return 0.0f;

	return pdf / total_pdf;
}

// pdf of a specific path
float BidirPathTracing::_PathPDF(const vector<BDPT_Vertex>& epath , int esize ,
								const vector<BDPT_Vertex>& lpath , int lsize ,
								const BDPT_Vertex& light_pos ) const
{
	if( esize == 0 )
		return 0.0f;

	int epath_size = (int)epath.size();
	int lpath_size = (int)lpath.size();

	int offset = 1;
	const BDPT_Vertex* pre_vert = &epath[0];
	float pdf = 1.0f;
	const BDPT_Vertex* last = pre_vert;
	for( int i = 1 ; i < esize ; i++ )
	{
		if( i < epath_size )
		{
			pre_vert = &epath[i];
			pdf *= SatDot( pre_vert->wi , pre_vert->n ) * pre_vert->pdf / ( pre_vert->p - epath[i-1].p ).SquaredLength();
			last = pre_vert;
		}
		else
		{
			const BDPT_Vertex& vert = lpath[ lpath_size - offset ];
			Vector delta = pre_vert->p - vert.p ;
			Vector wi = Normalize( delta );
			pdf *= SatDot( wi , vert.n ) * pre_vert->bsdf->Pdf( pre_vert->wi , pre_vert->wo ) / delta.SquaredLength();
			pre_vert = &lpath[ lpath_size - offset ];
			++offset;

			last = pre_vert;
		}
	}

	for( int i = 1 ; i < lsize ; i++ )
	{
		if( i < lpath_size )
		{
			pre_vert = &lpath[i];
			pdf *= SatDot( pre_vert->wi , pre_vert->n ) * pre_vert->pdf / ( pre_vert->p - lpath[i-1].p ).SquaredLength();
		}else
		{
			const BDPT_Vertex& vert = epath[ epath_size - offset ];
			Vector delta = pre_vert->p - vert.p;
			Vector wi = Normalize( delta );
			pdf *= SatDot( wi , vert.n ) * pre_vert->bsdf->Pdf( pre_vert->wi , pre_vert->wo ) / delta.SquaredLength();
			pre_vert = &epath[ epath_size - offset ];
			++offset;
		}
	}
	
	if( lpath_size != 0 )
		last = &lpath[0];

	Vector wi = Normalize( last->p - light_pos.p );
	pdf *= AbsDot( wi , last->n ) * light_pos.pdf / ( last->p - light_pos.p ).SquaredLength();

	return pdf;
}
