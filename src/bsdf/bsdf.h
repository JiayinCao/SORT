/*
 * filename :	bsdf.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_BSDF
#define	SORT_BSDF

// include the header
#include "sort.h"
#include <vector>
#include "spectrum/spectrum.h"

// pre-declera class
class Bxdf;
class Vector;

////////////////////////////////////////////////////////////////////////
// definition of bsdf
class	Bsdf
{
// public method
public:
	// default constructor
	Bsdf(){}
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
	vector<Bxdf*>	m_bxdf;
};

#endif
