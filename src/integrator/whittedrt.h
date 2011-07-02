/*
 * filename :	whittedrt.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_WHITTEDRT
#define	SORT_WHITTEDRT

// include the header file
#include "integrator.h"

/////////////////////////////////////////////////////////////////////////////
// definition of whittedrt
// note : Whitted ray tracer only takes direct light into consideration,
//		  there are also specular reflection and refraction. While indirect 
//		  light, like color bleeding , is not supported.
class	WhittedRT : public Integrator
{
// public method
public:
	// default constructor
	WhittedRT() {}
	// destructor
	~WhittedRT() {}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Scene& scene , const Ray& ray ) const;
};

#endif
