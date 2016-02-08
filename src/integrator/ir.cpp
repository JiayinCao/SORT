/*
   FileName:      ir.cpp

   Created Time:  2016-2-6

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "ir.h"
#include "integratormethod.h"
#include "geometry/intersection.h"
#include "geometry/scene.h"
#include "light/light.h"
#include "bsdf/bsdf.h"

IMPLEMENT_CREATOR( InstantRadiosity );

// Preprocess
void InstantRadiosity::PreProcess()
{
	m_pVirtualLightSources = new vector<VirtualLightSource>[m_nLightPathSet];

	for( int k = 0 ; k < m_nLightPathSet ; ++k )
	{
		for( int i = 0 ; i < m_nLightPaths ; ++i )
		{
			// pick a light first
			float light_pick_pdf;
			const Light* light = scene.SampleLight( sort_canonical() , &light_pick_pdf );

			// sample a ray from the light source
			float	light_emission_pdf = 0.0f;
			float	light_pdfa = 0.0f;
			Ray		ray;
			float   cosAtLight = 1.0f;
			Spectrum le = light->sample_l( LightSample(true) , ray , &light_emission_pdf , &light_pdfa , &cosAtLight );

			Spectrum throughput = le * cosAtLight / ( light_pick_pdf * light_emission_pdf );

			int current_depth = 0;
			Intersection intersect;
			while( true )
			{
				if (false == scene.GetIntersect(ray, &intersect))
					break;

				VirtualLightSource ls;
				ls.power = throughput;
				ls.intersect = intersect;
				ls.wi = -ray.m_Dir;
				ls.depth = ++current_depth;
				m_pVirtualLightSources[k].push_back( ls );

				float bsdf_pdf;
				Vector wo;
				Bsdf* bsdf = intersect.primitive->GetMaterial()->GetBsdf(&intersect);
				Spectrum bsdf_value = bsdf->sample_f(ls.wi, wo, BsdfSample(true), &bsdf_pdf, BXDF_ALL);

				if( bsdf_pdf == 0.0f )
					break;

				// apply russian roulette
				float continueProperbility = min( 1.0f , throughput.GetIntensity() );
				if( sort_canonical() > continueProperbility )
					break;
				throughput /= continueProperbility;

				// update throughput
				throughput *= bsdf_value * ( AbsDot(wo, intersect.normal) / bsdf_pdf );

				// update next ray
				ray = Ray(intersect.intersect, wo, 0, 0.001f);
			}
		}
	}
}

// PostProcess
void InstantRadiosity::PostProcess()
{
	SAFE_DELETE_ARRAY(m_pVirtualLightSources);
}

// radiance along a specific ray direction
Spectrum InstantRadiosity::Li( const Ray& r , const PixelSample& ps ) const
{
	// return if it is larger than the maximum depth
    if( r.m_Depth > max_recursive_depth )
		return 0.0f;

	// get intersection from camera ray
	Intersection ip;
	if( false == scene.GetIntersect( r , &ip ) )
		return scene.Le( r );

	// eavluate light path less than two vertices
	Spectrum radiance = ip.Le( -r.m_Dir );
	unsigned light_num = scene.LightNum();
	for( unsigned i = 0 ; i < light_num ; ++i )
	{
		const Light* light = scene.GetLight(i);
		radiance += EvaluateDirect( r , scene , light , ip , LightSample(true) , BsdfSample(true) , BXDF_TYPE( BXDF_ALL ) );
	}
	
	// pick a virtual light source randomly
	const unsigned lps_id = min( m_nLightPathSet - 1 , (int)(sort_canonical() * m_nLightPathSet) );
	const vector<VirtualLightSource>& vps = m_pVirtualLightSources[lps_id];

	Bsdf*	bsdf = ip.primitive->GetMaterial()->GetBsdf(&ip);

	// evaluate indirect illumination
	Spectrum indirectIllum;
	vector<VirtualLightSource>::const_iterator it = vps.begin();
	while( it != vps.end() )
	{
		if( r.m_Depth + it->depth > max_recursive_depth )
			continue;

		Vector	delta = ip.intersect - it->intersect.intersect;
		float	sqrLen = delta.SquaredLength();
		float	len = sqrt( sqrLen );
		Vector	n_delta = delta / len;

		Bsdf* bsdf1 = it->intersect.primitive->GetMaterial()->GetBsdf(&(it->intersect));

		float		gterm = min( m_fGTermThrshold , AbsDot( n_delta , ip.normal ) * AbsDot( n_delta , it->intersect.normal ) / sqrLen );
		Spectrum	f0 = bsdf->f( -r.m_Dir , -n_delta );
		Spectrum	f1 = bsdf1->f( n_delta , it->wi );

		Spectrum	contr = gterm * f0 * f1 * it->power;
		if( !contr.IsBlack() )
		{
			Visibility vis(scene);
			vis.ray = Ray( it->intersect.intersect , n_delta , 0 , 0.001f , len - 0.001f );

			if( vis.IsVisible() )
				indirectIllum += contr;
		}

		++it;
	}
	radiance += indirectIllum / (float)m_nLightPaths;

	return radiance;
}

// register property
void InstantRadiosity::_registerAllProperty()
{
	_registerProperty( "light_path_num" , new LightPathNumProperty(this) );
	_registerProperty( "light_path_set_num" , new LightPathSetProperty(this) );
	_registerProperty( "gterm_threshold" , new GTermThresholdProperty(this) );
}