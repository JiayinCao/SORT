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

// pre-declera class
class Bxdf;
class Vector;

#define	MAX_BXDF_COUNT 8

////////////////////////////////////////////////////////////////////////
// definition of bsdf
class	Bsdf
{
// public method
public:
	// default constructor
	Bsdf(){m_bsdfCount=0;}
	// destructor
	~Bsdf();

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
};

#endif
