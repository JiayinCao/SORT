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

	float	light_pdf = 0.0f;
	Ray		light_ray;
	Vector	light_normal;   // it is never initialized!!!!
	float	light_area_pdf = 0.0f;
    float   cosAtLight = 1.0f;
	Spectrum le = light->sample_l( sort_canonical() , light_ray , &light_pdf , &light_area_pdf , &cosAtLight );
	Spectrum li;
	
	//-----------------------------------------------------------------------------------------------------
	// Path evaluation: light tracing
	BDPT_Vertex light_vert;
	light_vert.p = light_ray.m_Ori;
	light_vert.accu_radiance = le / pdf / light_area_pdf;
	light_vert.n = light_normal;    // there is a bug here!!!!!!!
	light_vert.bsdf = 0;
	_ConnectCamera( light_vert , 0 , light );

	vector<BDPT_Vertex> light_path;
	Ray wi = light_ray;
	Spectrum accu_radiance = le * cosAtLight / (light_pdf * pdf);
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

		wi = Ray(vert.inter.intersect, vert.wo, 0, 0.001f);
	}

	if( light_tracing_only )
		return 0.0f;

	unsigned lps = light_path.size();

	wi = ray;
	accu_radiance = 1.0f;;
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

void BidirPathTracing::RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num )
{
    sample_per_pixel = ps_num;
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
	Spectrum li = light->sample_l(eye_vertex.inter, &sample, wi, 0 , &directPdfW, 0 , 0 , visibility);
	li *= eye_vertex.accu_radiance * eye_vertex.bsdf->f(eye_vertex.wi, wi) * AbsDot(eye_vertex.n, wi) / directPdfW;

	if (li.IsBlack())
		return 0.0f;

	if (visibility.IsVisible() == false)
		return 0.0f;

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
		coord.x >= camera->GetImageSensor()->GetWidth() ||
		coord.y >= camera->GetImageSensor()->GetHeight() ||
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

	Spectrum radiance = light_vertex.accu_radiance * g / camera_pdf / sample_per_pixel * weight;

	// update image sensor
	ImageSensor* is = camera->GetImageSensor();
	if (!is)
		return;
	is->UpdatePixel(coord.x , coord.y , radiance);
}