/*
   FileName:      integratormethod.cpp

   Created Time:  2011-08-04 12:48:57

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header files
#include "integratormethod.h"
#include "geometry/ray.h"
#include "geometry/intersection.h"
#include "bsdf/bsdf.h"
#include "geometry/primitive.h"
#include "material/material.h"
#include "light/light.h"

// radiance along specular reflection
Spectrum	SpecularReflection( const Scene& scene , const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator , const PixelSample& ps )
{
	Ray r;
	float pdf;
	Spectrum f = bsdf->sample_f( -ray.m_Dir , r.m_Dir , BsdfSample(true) , &pdf , BXDF_SPECULAR_REFLECTION );
	if( f.IsBlack() || r.m_Dir.IsZero() )
		return 0.0f;

	r.m_Depth = ray.m_Depth + 1;
	r.m_Ori = intersect->intersect + r.m_Dir ;

	float density = AbsDot( r.m_Dir , intersect->normal );

	return f * integrator->Li( scene , r , ps ) * density / pdf ;
}

// radiance along specular refraction
Spectrum	SpecularRefraction( const Scene& scene , const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator , const PixelSample& ps )
{
	Ray r;
	float pdf;
	Spectrum f = bsdf->sample_f( -ray.m_Dir , r.m_Dir , BsdfSample(true) , &pdf , BXDF_SPECULAR_TRANSMISSION );
	if( f.IsBlack() || r.m_Dir.IsZero() )
		return 0.0f;

	r.m_Depth = ray.m_Depth + 1;
	r.m_Ori = intersect->intersect + r.m_Dir;

	float density = AbsDot( r.m_Dir , intersect->normal ) ;

	return f * integrator->Li( scene , r , ps ) * density / pdf ;
}

// evaluate direct lighting
Spectrum	EvaluateDirect( const Ray& r , const Scene& scene , const Intersection& ip , const PixelSample& ps , BXDF_TYPE type )
{
	Spectrum t;

	// get the lights
	const vector<Light*> lights = scene.GetLights();

	// get bsdf
	Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );

	// evaluate light
	Visibility visibility(scene);
	vector<LightSample*>::const_iterator lit = ps.light_sample.begin();
	while( lit != ps.light_sample.end() )
	{
		const Light* light = lights[(*lit)->light_id];
		if( light->IsDelta() )
		{
			Vector lightDir;
			float light_pdf;
			Spectrum c = light->sample_l( ip , *lit , lightDir , 0.1f , &light_pdf , visibility );
			light_pdf *= scene.LightProperbility((*lit)->light_id);
			if( light_pdf != 0.0f && !c.IsBlack() && visibility.IsVisible() )
				t += c * bsdf->f( -r.m_Dir , lightDir ) * AbsDot( lightDir , ip.normal ) / light_pdf;
		}else
		{
			Vector lightDir;
			float light_pdf;
			Spectrum lr = light->sample_l( ip , *lit , lightDir , 0.1f , &light_pdf , visibility );
			if( light_pdf != 0.0f && !lr.IsBlack() && visibility.IsVisible() )
			{
				light_pdf *= scene.LightProperbility((*lit)->light_id);
				Spectrum br = bsdf->f( -r.m_Dir , lightDir );
				float bsdf_pdf = bsdf->Pdf( -r.m_Dir , lightDir );
				float mis = MisFactor( 1 , light_pdf , 1 , bsdf_pdf );
				t += lr * br * AbsDot( lightDir , ip.normal ) * mis / light_pdf;
			}
		}
		lit++;
	}
	unsigned total_samples = ps.light_sample.size();

	vector<BsdfSample*>::const_iterator bit = ps.bsdf_sample.begin();
	while( bit != ps.bsdf_sample.end() )
	{
		Vector wi;
		float bsdf_pdf;
		BXDF_TYPE bxdf_type;
		Spectrum br = bsdf->sample_f( -r.m_Dir , wi , *(*bit) , &bsdf_pdf , type , &bxdf_type );
		Intersection _ip;
		if( bsdf_pdf != 0.0f )
		{
			if( ~( bxdf_type & BXDF_SPECULAR ) )
			{
				Spectrum lr = scene.EvaluateLight( Ray( ip.intersect , wi , 0 , 0.1f ) , &_ip );
				visibility.ray = Ray( ip.intersect , wi , 0 , 1.0f , _ip.t );
				if( !lr.IsBlack() && visibility.IsVisible() )
					t += lr * br * AbsDot( wi , ip.normal ) / bsdf_pdf;
			}else
			{
				Spectrum lr = scene.EvaluateLight( Ray( ip.intersect , wi , 0 , 1.0f ) , &_ip );
				if( !lr.IsBlack() && _ip.light_id > 0 )
				{
					const Light* light = scene.GetLight(_ip.light_id);
					float light_pdf = light->Pdf( ip.intersect , _ip.intersect , wi ) * scene.LightProperbility(_ip.light_id);
					float mis = MisFactor( 1 , bsdf_pdf , 1 , light_pdf );
					visibility.ray = Ray( ip.intersect , wi , 0 , 1.0f , _ip.t );
					if( visibility.IsVisible() )
						t += br * lr * AbsDot( wi , ip.normal ) * mis / bsdf_pdf;
				}
			}
		}
		bit++;
	}
	total_samples += ps.bsdf_sample.size();
	t /= (float) total_samples;
	return t;
}

// mutilpe importance sampling factors , power heuristic is adapted
float	MisFactor( int nf, float fPdf, int ng, float gPdf )
{
	float f = nf * fPdf, g = ng * gPdf;
    return (f*f) / (f*f + g*g);
}
