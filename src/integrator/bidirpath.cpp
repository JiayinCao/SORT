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
#include "camera/camera.h"
#include "imagesensor/imagesensor.h"

IMPLEMENT_CREATOR( BidirPathTracing );

// return the radiance of a specific direction
Spectrum BidirPathTracing::Li( const Ray& ray , const PixelSample& ps ) const
{
	// pick a light randomly
	float pdf;
	const Light* light = scene.SampleLight( sort_canonical() , &pdf );
	if( light == 0 || pdf == 0.0f )
		return 0.0f;

	Spectrum li;

	float	light_emission_pdf = 0.0f;
	float	light_pdfa = 0.0f;
	Ray		light_ray;
    float   cosAtLight = 1.0f;
	LightSample light_sample(true);
	Spectrum le = light->sample_l( light_sample , light_ray , &light_emission_pdf , &light_pdfa , &cosAtLight );
	
	//-----------------------------------------------------------------------------------------------------
	// Trace light path from light source
	vector<BDPT_Vertex> light_path;
	Ray wi = light_ray;
	float vc = (light->IsDelta())?0.0f:cosAtLight / light_emission_pdf;
	float vcm = light_pdfa / light_emission_pdf;
	Spectrum accu_radiance = le * cosAtLight / (light_emission_pdf * pdf);
	while ((int)light_path.size() < path_per_pixel)
	{
		BDPT_Vertex vert;
		if (false == scene.GetIntersect(wi, &vert.inter))
			break;

		float distSqr = ( wi.m_Ori - vert.inter.intersect ).SquaredLength();
		vcm *= distSqr;
		vcm /= AbsDot( -wi.m_Dir , vert.inter.normal );
		vc /= AbsDot( -wi.m_Dir , vert.inter.normal );

		vert.p = vert.inter.intersect;
		vert.n = vert.inter.normal;
		vert.wi = -wi.m_Dir;
		vert.bsdf = vert.inter.primitive->GetMaterial()->GetBsdf(&vert.inter);
		vert.accu_radiance = accu_radiance;
		vert.vcm = vcm;
		vert.vc = vc;

		light_path.push_back(vert);

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: light tracing
		_ConnectCamera( vert , light_path.size() , light );

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

		float rev_bsdf_pdfw = vert.bsdf->Pdf( vert.wo , vert.wi );
		vc = AbsDot( vert.wo , vert.n ) * ( rev_bsdf_pdfw * vc + vcm ) / bsdf_pdf;
		vcm = 1.0f / bsdf_pdf;

		wi = Ray(vert.inter.intersect, vert.wo, 0, 0.001f);
	}

	//-----------------------------------------------------------------------------------------------------
	// Trace light path from eye point
	unsigned lps = light_path.size();
	wi = ray;
	accu_radiance = 1.0f;;
	int light_path_len = 0;
	vc = 0.0f;
	vcm = ray.m_fPDF;
	while (light_path_len < path_per_pixel)
	{
		BDPT_Vertex vert;
		if (false == scene.GetIntersect(wi, &vert.inter))
		{
			if (scene.GetSkyLight() == light)
			{
				if( light_path_len )
					li += scene.Le(wi) * accu_radiance * _Weight(light_path_len, -2, light) / pdf;
				else
					li += scene.Le(wi);
			}

			break;
		}

		float distSqr = ( wi.m_Ori - vert.inter.intersect ).SquaredLength();
		vcm *= distSqr;
		vcm /= AbsDot( -wi.m_Dir , vert.inter.normal );
		vc /= AbsDot( -wi.m_Dir , vert.inter.normal );

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: it hits a light source
		if (vert.inter.primitive->GetLight() == light)
		{
			if( light_path_len )
			{
				if( !mis_enabled )
					li += vert.inter.Le(-wi.m_Dir) * accu_radiance * _Weight(light_path_len, -2, light) / pdf;
				else
				{
					float emissionPdf;
					float directPdfA;
					Spectrum _li = vert.inter.Le(-wi.m_Dir , &directPdfA , &emissionPdf ) * accu_radiance / pdf;
					li += _li / ( 1.0f + directPdfA * vcm + emissionPdf * vc );
				}
			}
			else
				li += vert.inter.Le(-wi.m_Dir) / pdf;
		}
		if( light_tracing_only )
			return li;

		vert.p = vert.inter.intersect;
		vert.n = vert.inter.normal;
		vert.wi = -wi.m_Dir;
		vert.bsdf = vert.inter.primitive->GetMaterial()->GetBsdf(&vert.inter);
		vert.accu_radiance = accu_radiance;
		vert.vc = vc;
		vert.vcm = vcm;

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: connect light sample first
		if( !mis_enabled )
			li += _ConnectLight(vert, light) * _Weight(light_path_len, -1, light) / pdf;
		else
			li += _ConnectLight(vert, light) / pdf;

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: connect vertices
		for (unsigned j = 0; j < lps; ++j)
		{
			if( !mis_enabled )
				li += vert.accu_radiance * light_path[j].accu_radiance * _Gterm( vert , light_path[j] ) * _Weight(light_path_len, j, light);
			else
				li += _ConnectVertices( light_path[j] , vert );
		}

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

		float rev_bsdf_pdfw = vert.bsdf->Pdf( vert.wo , vert.wi );
		vc = AbsDot( vert.wo , vert.n ) * ( rev_bsdf_pdfw * vc + vcm ) / bsdf_pdf;
		vcm = 1.0f / bsdf_pdf;

		wi = Ray(vert.inter.intersect, vert.wo, 0, 0.001f);
	}

	return li;
}

void BidirPathTracing::RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num )
{
    sample_per_pixel = ps_num;
}

// connnect vertices
Spectrum BidirPathTracing::_ConnectVertices( const BDPT_Vertex& p0 , const BDPT_Vertex& p1 ) const
{
	Vector delta = p0.p - p1.p;
	float distSqr = delta.SquaredLength();
	Vector n_delta = Normalize(delta);

	Spectrum g = AbsDot( p0.n , -n_delta ) * p1.bsdf->f( p1.wi , n_delta ) * p0.bsdf->f( -n_delta, p0.wi ) * AbsDot( p1.n , n_delta ) / distSqr;
	if( g.IsBlack() )
		return 0.0f;

	Visibility visible( scene );
	visible.ray = Ray( p1.p , n_delta  , 0 , 0.01f , delta.Length() - 0.01f );
	if( visible.IsVisible() == false )
		return 0.0f;
	
	float p0_bsdf_pdfw = p0.bsdf->Pdf( p0.wi , -n_delta );
	float p0_bsdf_rev_pdfw = p0.bsdf->Pdf( -n_delta , p0.wi );
	float p1_bsdf_pdfw = p1.bsdf->Pdf( p1.wi , n_delta );
	float p1_bsdf_rev_pdfw = p1.bsdf->Pdf( n_delta , p1.wi );

	float p0_a = p1_bsdf_pdfw * AbsDot( n_delta , p0.n ) / distSqr;
	float p1_a = p0_bsdf_pdfw * AbsDot( n_delta , p1.n ) / distSqr;

	float mis_0 = p0_a * ( p0.vcm + p0.vc * p0_bsdf_rev_pdfw );
	float mis_1 = p1_a * ( p1.vcm + p1.vc * p1_bsdf_rev_pdfw );

	Spectrum li = p0.accu_radiance * p1.accu_radiance * g / ( mis_0 + 1.0f + mis_1 );

	return li;
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
		return 1.0f / (esize+lsize+3);

	// MIS to be implemented
	return 1.0f / (esize + lsize + 4);
}

// connect light sample
Spectrum BidirPathTracing::_ConnectLight(const BDPT_Vertex& eye_vertex , const Light* light ) const
{
	// drop the light vertex, take a new sample here
	LightSample sample(true);
	Vector wi;
	Visibility visibility(scene);
	float directPdfW;
	float emissionPdfW;
	float cosAtLight;
	Spectrum li = light->sample_l(eye_vertex.inter, &sample, wi, 0 , &directPdfW, &emissionPdfW , &cosAtLight , visibility);
	li *= eye_vertex.accu_radiance * eye_vertex.bsdf->f(eye_vertex.wi, wi) * AbsDot(eye_vertex.n, wi) / directPdfW;

	if (li.IsBlack())
		return 0.0f;

	if (visibility.IsVisible() == false)
		return 0.0f;

	if( mis_enabled )
	{
		float eye_bsdf_pdfw = eye_vertex.bsdf->Pdf( eye_vertex.wi , wi );
		float eye_bsdf_rev_pdfw = eye_vertex.bsdf->Pdf( wi , eye_vertex.wi );

		float mis0 = light->IsDelta()?0.0f:(eye_bsdf_pdfw / directPdfW);
		float mis1 = AbsDot( eye_vertex.n , wi ) * emissionPdfW * ( eye_vertex.vcm + eye_vertex.vc * eye_bsdf_rev_pdfw ) / ( cosAtLight * directPdfW );

		li /= mis0 + mis1 + 1.0f;
	}

	return li;
}

// connect camera point
void BidirPathTracing::_ConnectCamera(const BDPT_Vertex& light_vertex, int len , const Light* light ) const
{
	float weight = 1.0f;
	if( light_tracing_only == false )
		weight = _Weight( len , -2 , light );

	Vector delta = light_vertex.p - camera->GetEye();
	Vector n_delta = Normalize(delta);

	float camera_pdf;
	Vector2i coord = camera->GetScreenCoord(n_delta, &camera_pdf);

	camera_pdf /= AbsDot( light_vertex.n, n_delta ) / delta.SquaredLength();

	if (coord.x < 0.0f || coord.y < 0.0f ||
		coord.x >= (int)camera->GetImageSensor()->GetWidth() ||
		coord.y >= (int)camera->GetImageSensor()->GetHeight() ||
		camera_pdf == 0.0f )
		return;

	Spectrum g = 1.0f;
	if( light_vertex.bsdf )
		g *= light_vertex.bsdf->f( light_vertex.wi , -n_delta );
	if( g.IsBlack() )
		return;

	Visibility visible( scene );
	visible.ray = Ray( light_vertex.p , -n_delta , 0 , 0.01f , delta.Length() );
	if( visible.IsVisible() == false )
		return;

	Spectrum radiance = light_vertex.accu_radiance * g / camera_pdf / (float)sample_per_pixel;

	if( mis_enabled && !light_tracing_only )
	{
		float bsdf_rev_pdfw = light_vertex.bsdf->Pdf( n_delta , light_vertex.wi );
		float mis0 = ( light_vertex.vcm + light_vertex.vc * bsdf_rev_pdfw ) / camera_pdf;
		radiance /= ( 1.0f + mis0 );
	}else
		radiance *= weight;

	// update image sensor
	ImageSensor* is = camera->GetImageSensor();
	if (!is)
		return;
	is->UpdatePixel(coord.x , coord.y , radiance);
}