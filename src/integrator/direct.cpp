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

// include header file
#include "direct.h"
#include "integratormethod.h"
#include "geometry/intersection.h"
#include "geometry/scene.h"
#include "bsdf/bsdf.h"
#include "light/light.h"
#include "managers/memmanager.h"
#include "sampler/sampler.h"
#include "utility/log.h"

SORT_STATS_DECLARE_COUNTER(sPrimaryRayCount)

SORT_STATS_COUNTER("Direct Illumination", "Primary Ray Count" , sPrimaryRayCount);

IMPLEMENT_CREATOR( DirectLight );

// radiance along a specific ray direction
Spectrum DirectLight::Li( const Ray& r , const PixelSample& ps ) const
{
    SORT_STATS(++sPrimaryRayCount);
    
	if( r.m_Depth > max_recursive_depth )
		return 0.0f;

	// get the intersection between the ray and the scene
	Intersection ip;
	// evaluate light directly
	if( false == scene.GetIntersect( r , &ip ) )
		return scene.Le( r );

	Spectrum li = ip.Le( -r.m_Dir );

	// eavluate direct light
	unsigned light_num = scene.LightNum();
	for( unsigned i = 0 ; i < light_num ; ++i )
	{
		const Light* light = scene.GetLight(i);
		li += EvaluateDirect( r , scene , light , ip , LightSample(true) , BsdfSample(true), BXDF_TYPE( BXDF_ALL ) );
	}

	return li;
}

// request samples
void DirectLight::RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num )
{
	/*SAFE_DELETE_ARRAY(light_sample_offsets);
	SAFE_DELETE_ARRAY(bsdf_sample_offsets);

	unsigned light_num = scene.LightNum();
	unsigned total_light_num = ls_per_light * light_num;
	light_sample_offsets = new SampleOffset[light_num];
	bsdf_sample_offsets = new SampleOffset[light_num];

	total_samples = 0;
	for( unsigned i = 0 ; i < ps_num ; i++ )
	{
		for( unsigned k = 0 ; k < light_num ; k++ )
		{
			float properbility = scene.LightProperbility(k);
			unsigned lsn = (unsigned)( properbility * total_light_num + 0.5f );

			const Light* light = scene.GetLight(k);
			if( light->IsDelta() ) lsn = 1;

			if( i == 0 )
			{
				light_sample_offsets[k].num = sampler->RoundSize( lsn );
				light_sample_offsets[k].offset = ps[i].RequestMoreLightSample( light_sample_offsets[i].num );
				bsdf_sample_offsets[k].num = light_sample_offsets[k].num;
				bsdf_sample_offsets[k].offset = ps[i].RequestMoreBsdfSample( bsdf_sample_offsets[i].num );
				total_samples += bsdf_sample_offsets[k].num;
			}else
			{
				ps[i].RequestMoreLightSample( light_sample_offsets[i].num );
				ps[i].RequestMoreLightSample( light_sample_offsets[i].num );
			}
		}
		ps[i].bsdf_sample = new BsdfSample[ total_samples ];
		ps[i].light_sample = new LightSample[ total_samples ];
	}

	// allocate the data to be used
	ps[0].data = new float[ total_samples * 5 ];*/
}

// generate samples
void DirectLight::GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const
{
	Integrator::GenerateSample( sampler , samples , ps , scene );
	/*
	unsigned light_num = scene.LightNum();
	for( unsigned i = 0 ; i < ps ; ++i )
	{
		unsigned offset = 0;

		float* light_2d = samples[0].data;
		float* bsdf_1d = samples[0].data + total_samples * 2;
		float* bsdf_2d = samples[0].data + total_samples ;
		for( unsigned k = 0 ; k < light_num ; ++k )
		{
			sampler->Generate2D( light_2d + 2 * offset , light_sample_offsets[k].num );
			sampler->Generate1D( bsdf_1d + offset , light_sample_offsets[k].num );
			sampler->Generate2D( bsdf_2d + 2 * offset , bsdf_sample_offsets[k].num );
			offset += bsdf_sample_offsets[k].num;
		}

		for( unsigned k = 0 ; k < total_samples ; k++ )
		{
			samples[i].light_sample[k].u = light_2d[2*k];
			samples[i].light_sample[k].v = light_2d[2*k+1];
			samples[i].bsdf_sample[k].t = bsdf_1d[k];
			samples[i].bsdf_sample[k].u = bsdf_2d[2*k];
			samples[i].bsdf_sample[k].v = bsdf_2d[2*k+1];
		}
	}*/
}

void DirectLight::_registerAllProperty()
{
	_registerProperty( "sample_per_light" , new SamplerPerLightProperty(this) );
}
