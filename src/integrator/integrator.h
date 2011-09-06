/*
   FileName:      integrator.h

   Created Time:  2011-08-04 12:48:53

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_INTEGRATOR
#define	SORT_INTEGRATOR

// include the header
#include "spectrum/spectrum.h"
#include "managers/memmanager.h"
#include "sampler/sampler.h"

// pre-declera classes
class	Ray;
class	Scene;

////////////////////////////////////////////////////////////////////////////
//	definition of integrator
class	Integrator
{
// public method
public:
	// default constructor
	Integrator( const Scene& s ):scene(s){max_recursive_depth=6;}
	// destructor
	virtual ~Integrator(){}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const = 0;

	// generate samples
	// para 'sampler' : the sampling method
	// para 'samples' : the samples to be generated
	// para 'ps'      : number of pixel sample to be generated
	// para 'scene'   : the scene to be rendered
	virtual void GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const
	{
		float* data = SORT_MALLOC_ARRAY( float , 2 * ps )();
		sampler->Generate2D( data , ps );
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
			samples[i].dof_u = 2 * data[sid] - 1.0f ;
			samples[i].dof_v = 2 * data[sid+1] - 1.0f;
		}
	}

	// request samples
	virtual void RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num ) {}

	// pre-process before rendering
	// by default , nothing is done in pre-process
	// some integrator, such as Photon Mapping use pre-process step to
	// generate some neccessary infomation by latter stage.
	virtual void PreProcess() {}

	// output log information
	virtual void OutputLog() const {}

// protected method
protected:
	// maxium recursive depth
	unsigned		max_recursive_depth;

	// the scene description
	const Scene&	scene;

	// the pixel sample
	PixelSample		pixel_sample;
};

#endif
