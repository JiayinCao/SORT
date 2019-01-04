/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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

#include "spectrum/spectrum.h"
#include "core/memory.h"
#include "sampler/sampler.h"
#include "core/creator.h"
#include "core/stats.h"
#include "core/profile.h"
#include "core/primitive.h"
#include "stream/stream.h"
#include "core/scene.h"

class	Ray;

////////////////////////////////////////////////////////////////////////////
//	definition of integrator
class	Integrator : public SerializableObject
{
public:
	//! @brief	Empty virtual destructor.
	virtual ~Integrator(){}

	//! @brief	Evaluate the radiance along the opposite direction of the ray in the scene.
	//!
	//! The exact way of importance sampling could depend on the rendering algorithm itself.
	//! Each way of rendering has their unique features.
	//!
	//! @param	ray		The extent ray in rendering equation.
	//! @param	ps		The pixel samples. Currently not used.
	//! @param	scene	The rendering scene.
	//! @return			The spectrum of the radiance along the opposite direction of the ray.
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps , const Scene& scene ) const = 0;

	// set sample per pixel
	// para 'spp' : sample per pixel
	void SetSamplePerPixel( unsigned spp ){ sample_per_pixel = spp; }

	// generate samples
	// para 'sampler' : the sampling method
	// para 'samples' : the samples to be generated
	// para 'ps'      : number of pixel sample to be generated
	// para 'scene'   : the scene to be rendered
	virtual void GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const{
        auto data = std::make_unique<float[]>(2 * ps);
		sampler->Generate2D( data.get() , ps , true );
		for( unsigned i = 0 ; i < ps ; ++i )
		{
			samples[i].img_u = data[2*i];
			samples[i].img_v = data[2*i+1];
		}

        auto shuffle = std::make_unique<unsigned[]>(ps);
        for (unsigned i = 0; i < ps; i++)
            shuffle[i] = i;
        std::random_shuffle(shuffle.get(), shuffle.get() + ps);

		sampler->Generate2D(data.get(), ps );
		for( unsigned i = 0 ; i < ps ; ++i )
		{
			unsigned sid = 2* shuffle[i];
			samples[i].dof_u = data[sid];
			samples[i].dof_v = data[sid+1];
		}
	}

	// request samples
	virtual void RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num ) {}

	// pre-process before rendering
	// by default , nothing is done in pre-process
	// some integrator, such as Photon Mapping use pre-process step to
	// generate some neccessary infomation by latter stage.
	virtual void PreProcess(const Scene& scene) {}

	// post process
	virtual void PostProcess() {}

	// support pending write
	virtual bool SupportPendingWrite() { return false; }

	// refresh tile in blender
	virtual bool NeedRefreshTile() const { return true; }

	//! @brief      Serializing data from stream
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override {
		stream >> max_recursive_depth;
	}

protected:
	// maxium recursive depth
	int				max_recursive_depth = 6;

	// the pixel sample
	PixelSample		pixel_sample;

	// light sample per pixel sample per light
	unsigned sample_per_pixel;
};
