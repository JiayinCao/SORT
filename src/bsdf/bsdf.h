/*
 * filename :	bsdf.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_BSDF
#define	SORT_BSDF

// include the header
#include "sort.h"
#include "spectrum/spectrum.h"
#include "geometry/vector.h"

// pre-declera class
class Bxdf;
class Intersection;

#define	MAX_BXDF_COUNT 8

////////////////////////////////////////////////////////////////////////
// definition of bsdf
class	Bsdf
{
// public method
public:
	// default constructor
	Bsdf();
	// destructor
	~Bsdf();

	// set intersection
	void SetIntersection( const Intersection* );

	// get the number of bxdf
	unsigned NumComponents() const;

	// add a new bxdf
	// para 'bxdf' : a bxdf to add
	void AddBxdf( Bxdf* bxdf );

	// evaluate bxdf
	// para 'wi' : input vector
	// para 'wo' : output vector
	Spectrum f( const Vector& wo , const Vector& wi ) const;

// private field
private:
	// the list for the bxdf
	Bxdf*	m_bxdf[MAX_BXDF_COUNT];

	// current bsdf
	unsigned m_bsdfCount;

	// the vectors
	Vector nn , sn , tn;

	// transform the vector from world to shading coordinate
	// para 'v' : a vector in world space
	// result   : a transformed vector in shading coordinate
	Vector _worldToLocal( const Vector& v ) const;
};

#endif
