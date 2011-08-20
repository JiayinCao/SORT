/*
   FileName:      direct.h

   Created Time:  2011-08-12 12:52:40

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_DIRECT
#define	SORT_DIRECT

#include "integrator.h"

//////////////////////////////////////////////////////////////////////////////////////
//	definition of direct light
//	comparing with whitted ray tracing , direct light requires more samples per pixel
//	and it supports soft shadow and area light.
class	DirectLight : public Integrator
{
// public method
public:
	// default constructor
	DirectLight( unsigned lp ):ls_per_ps(lp){}
	// destructor
	~DirectLight(){}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Scene& scene , const Ray& ray , const PixelSample& ps ) const;

	// generate samples
	// para 'sampler' : the sampling method
	// para 'samples' : the samples to be generated
	// para 'ps'      : number of pixel sample to be generated
	// para 'scene'   : the scene to be rendered
	virtual void GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const;

	// pre-process before rendering
	// by default , nothing is done in pre-process
	// some integrator, such as Photon Mapping use pre-process step to
	// generate some neccessary infomation by latter stage.
	virtual void PreProcess( Scene& scene ){}

	// output log information
	virtual void OutputLog() const;

// private field
private:
	unsigned ls_per_ps; // light sample per pixel sample
};

#endif
