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

#pragma once

#include "integrator.h"

//////////////////////////////////////////////////////////////////////////////////////
//	definition of direct light
//	comparing with whitted ray tracing , direct light requires more samples per pixel
//	and it supports soft shadow and area light.
class	DirectLight : public Integrator
{
public:
	DEFINE_CREATOR( DirectLight , Integrator , "direct" );

	// default constructor
	DirectLight(){_registerAllProperty();}
	// destructor
	~DirectLight(){
        SAFE_DELETE_ARRAY(light_sample_offsets);
        SAFE_DELETE_ARRAY(bsdf_sample_offsets);
    }

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const;

	// request samples
	virtual void RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num );

	// generate samples
	// para 'sampler' : the sampling method
	// para 'samples' : the samples to be generated
	// para 'ps'      : number of pixel sample to be generated
	// para 'scene'   : the scene to be rendered
	virtual void GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const;

private:
	unsigned		ls_per_light = 16; // light sample per pixel sample per light

	SampleOffset*	light_sample_offsets = nullptr;	// light sample offset
	SampleOffset*	bsdf_sample_offsets = nullptr;	// bsdf sample offset

	// register property
	void _registerAllProperty();

	class SamplerPerLightProperty : public PropertyHandler<Integrator>
	{
	public:
		PH_CONSTRUCTOR(SamplerPerLightProperty,Integrator);
		void SetValue( const std::string& str )
		{
			DirectLight* direct = CAST_TARGET(DirectLight);

			if( direct )
				direct->ls_per_light = atoi( str.c_str() );
		}
	};
    
    SORT_STATS_ENABLE( "Direct Illumination" )
};
