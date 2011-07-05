/*
 * filename :	integrator.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_INTEGRATOR
#define	SORT_INTEGRATOR

// include the header
#include "spectrum/spectrum.h"

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
	Integrator(){}
	// destructor
	virtual ~Integrator(){}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Scene& scene , const Ray& ray ) const = 0;

	// pre-process before rendering
	// by default , nothing is done in pre-process
	// some integrator, such as Photon Mapping use pre-process step to
	// generate some neccessary infomation by latter stage.
	virtual void PreProcess() {}

	// output log information
	virtual void OutputLog() const {}
};

#endif
