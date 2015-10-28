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
Spectrum	SpecularReflection( const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator , const PixelSample& ps )
{
	Ray r;
	float pdf;
	Spectrum f = bsdf->sample_f( -ray.m_Dir , r.m_Dir , BsdfSample(true) , &pdf , BXDF_SPECULAR_REFLECTION );
	if( f.IsBlack() || r.m_Dir.IsZero() )
		return 0.0f;

	r.m_Depth = ray.m_Depth + 1;
	r.m_Ori = intersect->intersect + r.m_Dir ;

	float density = AbsDot( r.m_Dir , intersect->normal );

	return f * integrator->Li( r , ps ) * density / pdf ;
}

// radiance along specular refraction
Spectrum	SpecularRefraction( const Ray& ray , const Intersection* intersect , const Bsdf* bsdf , const Integrator* integrator , const PixelSample& ps )
{
	Ray r;
	float pdf;
	Spectrum f = bsdf->sample_f( -ray.m_Dir , r.m_Dir , BsdfSample(true) , &pdf , BXDF_SPECULAR_TRANSMISSION );
	if( f.IsBlack() || r.m_Dir.IsZero() )
		return 0.0f;

	r.m_Depth = ray.m_Depth + 1;
	r.m_Ori = intersect->intersect + r.m_Dir;

	float density = AbsDot( r.m_Dir , intersect->normal ) ;

	return f * integrator->Li( r , ps ) * density / pdf ;
}

// evaluate direct lighting
Spectrum	EvaluateDirect( const Ray& r , const Scene& scene , const Light* light , const Intersection& ip , 
							const LightSample& ls ,const BsdfSample& bs , BXDF_TYPE type )
{
	// get bsdf
	Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );

	Spectrum radiance;
	Visibility visibility(scene);
	float light_pdf;
	float bsdf_pdf;
	Vector wo = -r.m_Dir;
	Vector wi;
	Spectrum li = light->sample_l( ip , &ls , wi , 0.1f , &light_pdf , visibility );
	if( light_pdf > 0.0f && !li.IsBlack() )
	{
		Spectrum f = bsdf->f( wo , wi , type );
		float dot = SatDot( wi , ip.normal );
		if( f.IsBlack() == false && visibility.IsVisible() && dot > 0.0f )
		{
			if( light->IsDelta() )
				radiance = li * f * dot / light_pdf;
			else
			{
				bsdf_pdf = bsdf->Pdf( wo , wi , type );
				float power_hueristic = MisFactor( 1 , light_pdf , 1 , bsdf_pdf );
				radiance = li * f * dot * power_hueristic / light_pdf;
			}
		}
	}

	if( !light->IsDelta() )
	{
		BXDF_TYPE bxdf_type;
		Spectrum f = bsdf->sample_f( wo , wi , bs , &bsdf_pdf , type , &bxdf_type );
		if( !f.IsBlack() && bsdf_pdf != 0.0f )
		{
			float weight = 1.0f;
			if( !( bxdf_type & BXDF_SPECULAR ) )
			{
				float light_pdf;
				light_pdf = light->Pdf( ip.intersect , wi );
				if( light_pdf <= 0.0f )
					return radiance;
				weight = MisFactor( 1 , bsdf_pdf , 1 , light_pdf );
			}
			
			Spectrum li;
			Intersection _ip;
			if( false == light->Le( Ray( ip.intersect , wi ) , &_ip , li ) )
				return radiance;

			float dot = SatDot( wi , ip.normal );
			visibility.ray = Ray( ip.intersect , wi , 0 , 0.1f , _ip.t - 0.1f );
			if( dot > 0.0f && !li.IsBlack() && visibility.IsVisible() )
				radiance += li * f * dot * weight / bsdf_pdf;
		}
	}

	return radiance;
}

// mutilpe importance sampling factors , power heuristic is used 
float	MisFactor( int nf, float fPdf, int ng, float gPdf )
{
	float f = nf * fPdf, g = ng * gPdf;
    return (f*f) / (f*f + g*g);
}
