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
	Spectrum f = bsdf->sample_f( -ray.m_Dir , r.m_Dir , &pdf , BXDF_REFLECTION );
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
	Spectrum f = bsdf->sample_f( -ray.m_Dir , r.m_Dir , &pdf , BXDF_TRANSMISSION );
	if( f.IsBlack() || r.m_Dir.IsZero() )
		return 0.0f;

	r.m_Depth = ray.m_Depth + 1;
	r.m_Ori = intersect->intersect + r.m_Dir;

	float density = AbsDot( r.m_Dir , intersect->normal ) ;

	return f * integrator->Li( scene , r , ps ) * density / pdf ;
}

// evaluate direct lighting
Spectrum	EvaluateDirect( const Ray& r , const Scene& scene , const Intersection& ip , const PixelSample& ps )
{
	Spectrum t;

	// get the lights
	const vector<Light*> lights = scene.GetLights();

	// get bsdf
	Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );

	// evaluate light
	Visibility visibility(scene);
	vector<LightSample*>::const_iterator it = ps.light_sample.begin();
	while( it != ps.light_sample.end() )
	{
		const Light* light = lights[(*it)->light_id];
/*		if( light->IsDelta() )
		{
			Vector lightDir;
			Spectrum c = light->sample_l( ip , *it , lightDir , 0.1f , 0 , visibility );
			if( visibility.IsVisible() )
				t += c * bsdf->f( -r.m_Dir , lightDir ) * SatDot( lightDir , ip.normal );
		}else
		{
			// to be added in the next couple of days
		}
*/
		Vector lightDir;
		float pdf;
		Spectrum c = light->sample_l( ip , *it , lightDir , 0.1f , &pdf , visibility );
		if( pdf != 0.0f && visibility.IsVisible() )
			t += c * bsdf->f( -r.m_Dir , lightDir ) * SatDot( lightDir , ip.normal ) / pdf;
		it++;
	}
	t /= (float)ps.light_sample.size();

	return t;
}
