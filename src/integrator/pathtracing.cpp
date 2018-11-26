/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "pathtracing.h"
#include "geometry/intersection.h"
#include "bsdf/bsdf.h"
#include "geometry/scene.h"
#include "integratormethod.h"
#include "camera/camera.h"
#include "utility/log.h"

SORT_STATS_DEFINE_COUNTER(sTotalPathLength)
SORT_STATS_DECLARE_COUNTER(sPrimaryRayCount)

SORT_STATS_COUNTER("Path Tracing", "Primary Ray Count" , sPrimaryRayCount);
SORT_STATS_AVG_COUNT("Path Tracing", "Average Length of Path", sTotalPathLength , sPrimaryRayCount);    // This also counts the case where ray hits sky

IMPLEMENT_CREATOR( PathTracing );

// return the radiance of a specific direction
// note : there are one factor makes the method biased.
//		there is a limitation on the number of vertexes in the path
Spectrum PathTracing::Li( const Ray& ray , const PixelSample& ps ) const
{
    SORT_PROFILE("Path tracing");
    SORT_STATS(++sPrimaryRayCount);

	Spectrum	L = 0.0f;
	Spectrum	throughput = 1.0f;
    
	int			bounces = 0;
	Ray	r = ray;
	while(true)
	{
        SORT_STATS(++sTotalPathLength);
        
		Intersection inter;

		// get the intersection between the ray and the scene
		// if it's a light , accumulate the radiance and break
		if( false == scene.GetIntersect( r , &inter ) )
		{
			if( bounces == 0 )
				return scene.Le( r );
			break;
		}

		if( bounces == 0 ) L+=inter.Le(-r.m_Dir);

		// make sure there is intersected primitive
		sAssert( inter.primitive != 0 , INTEGRATOR );

		// evaluate the light
		Bsdf*			bsdf = inter.primitive->GetMaterial()->GetBsdf(&inter);
		float			light_pdf = 0.0f;
		LightSample		light_sample = (bounces==0)?ps.light_sample[0]:LightSample(true);
		BsdfSample		bsdf_sample = (bounces==0)?ps.bsdf_sample[0]:BsdfSample(true);
		const Light*	light = scene.SampleLight( light_sample.t , &light_pdf );
		if( light_pdf > 0.0f )
			L += throughput * EvaluateDirect(	r  , scene , light , inter , light_sample ,
												bsdf_sample , BXDF_TYPE(BXDF_ALL) ) / light_pdf;

		// sample the next direction using bsdf
		float		path_pdf;
		Vector		wi;
		BXDF_TYPE	bxdf_type;
		Spectrum f;
		BsdfSample	_bsdf_sample = (bounces==0)?ps.bsdf_sample[1]:BsdfSample(true);
		f = bsdf->sample_f( -r.m_Dir , wi , _bsdf_sample , &path_pdf , BXDF_ALL , &bxdf_type );
		if( f.IsBlack() || path_pdf == 0.0f )
			break;

		// update path weight
		throughput *= f / path_pdf;

		if( throughput.GetIntensity() == 0.0f )
			break;
        
		if( bounces > 3 && throughput.GetMaxComponent() < 0.1f )
		{
			float continueProperbility = std::max( 0.05f , 1.0f - throughput.GetMaxComponent() );
			if( sort_canonical() < continueProperbility )
				break;
			throughput /= 1 - continueProperbility;
		}
        
		r.m_Ori = inter.intersect;
		r.m_Dir = wi;
		r.m_fMin = 0.0001f;

		++bounces;

		// note :	the following code makes the method biased
		//			'path_per_pixel' could be set very large to reduce the side-effect.
		if( bounces >= max_recursive_depth )
			break;
	}

	return L;
}

// request samples
void PathTracing::RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num )
{
	for( unsigned i = 0 ; i < ps_num ; i++ )
	{
		// the first half samples are used to sample bsdf for shading
		// the second half samples are used to sample bsdf for direction
		ps[i].bsdf_sample = new BsdfSample[ 2 ];
		ps[i].light_sample = new LightSample[ 1 ];
	}

	ps[0].data = new float[ ps_num * 3 ];
}

// generate samples
void PathTracing::GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const
{
	Integrator::GenerateSample( sampler , samples , ps , scene );

	if( sampler->RoundSize( ps ) == ps )
	{
		float* data_1d = samples[0].data;
		float* data_2d = samples[0].data + ps;

		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		for( unsigned k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*k;
			samples[k].bsdf_sample[0].t = data_1d[k];
			samples[k].bsdf_sample[0].u = data_2d[two_k];
			samples[k].bsdf_sample[0].v = data_2d[two_k+1];
		}

		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		for( unsigned k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*k;
			samples[k].bsdf_sample[1].t = data_1d[k];
			samples[k].bsdf_sample[1].u = data_2d[two_k];
			samples[k].bsdf_sample[1].v = data_2d[two_k+1];
		}

		sampler->Generate1D( data_1d , ps );
		sampler->Generate2D( data_2d , ps );
		for( unsigned k = 0 ; k < ps ; ++k )
		{
			int two_k = 2*k;
			samples[k].light_sample[0].t = data_1d[k];
			samples[k].light_sample[0].u = data_2d[two_k];
			samples[k].light_sample[0].v = data_2d[two_k+1];
		}
	}else
	{
		for (unsigned k = 0; k < ps; ++k) 
		{
			samples[k].bsdf_sample[0] = BsdfSample(true);
			samples[k].bsdf_sample[1] = BsdfSample(true);
			samples[k].light_sample[0] = LightSample(true);
		}
	}
}
