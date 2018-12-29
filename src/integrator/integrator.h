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

// include the header
#include "spectrum/spectrum.h"
#include "managers/memmanager.h"
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
	// destructor
	virtual ~Integrator(){}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const = 0;

	// set sample per pixel
	// para 'spp' : sample per pixel
	void SetSamplePerPixel( unsigned spp ){ sample_per_pixel = spp; }
	
	// setup scene
	inline void SetupScene( const Scene* scene ) { 
		m_scene = scene;
		camera = m_scene->GetCamera(); 
	}

	// generate samples
	// para 'sampler' : the sampling method
	// para 'samples' : the samples to be generated
	// para 'ps'      : number of pixel sample to be generated
	// para 'scene'   : the scene to be rendered
	virtual void GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const
	{
		float* data = SORT_MALLOC_ARRAY( float , 2 * ps )();
		sampler->Generate2D( data , ps , true );
		for( unsigned i = 0 ; i < ps ; ++i )
		{
			samples[i].img_u = data[2*i];
			samples[i].img_v = data[2*i+1];
		}

		// shuffle the index
		const unsigned* shuffled_id = ShuffleIndex( ps );
		sampler->Generate2D( data , ps );
		for( unsigned i = 0 ; i < ps ; ++i )
		{
			unsigned sid = 2*shuffled_id[i];
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
	virtual void PreProcess() {}

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
	// Camera
    std::shared_ptr<class Camera>	camera = nullptr;

	// maxium recursive depth
	int				max_recursive_depth = 6;

	// the scene description
	const Scene*	m_scene;

	// the pixel sample
	PixelSample		pixel_sample;

	// light sample per pixel sample per light
	unsigned sample_per_pixel;
};
