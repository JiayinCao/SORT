/*
 * filename :	sky.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_SKY
#define	SORT_SKY

// include the header
#include "spectrum/spectrum.h"
#include "utility/propertyset.h"

class Ray;

//////////////////////////////////////////////////////////////////
//	definition of sky
//	note: we could also use a very large box or sphere to replace
//		  the sky we need , while that would make some accelerator
//		  such as uniform grid unefficient. So we use a isolated
//		  sky to show the environment.
class	Sky : public PropertySet<Sky>
{
// public method
public:
	// default constructor
	Sky(){}
	// destructor
	virtual ~Sky(){}

	// evaluate value from sky
	// para 'r' : the ray which misses all of the triangle in the scene
	// result   : the spectrum in the sky
	virtual Spectrum Evaluate( const Ray& r ) const = 0;
};

#endif