/*
 * filename :	bsdf.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "bsdf.h"
#include "bxdf.h"
#include "geometry/vector.h"

// destructor
Bsdf::~Bsdf()
{
}

// get the number of components in current bsdf
unsigned Bsdf::NumComponents() const
{
	return m_bsdfCount;
}

// add a new bxdf
void Bsdf::AddBxdf( Bxdf* bxdf )
{
	if( m_bsdfCount == MAX_BXDF_COUNT )
		return;
	m_bxdf[m_bsdfCount] = bxdf ;
	m_bsdfCount++;
}

// evaluate bxdf
Spectrum Bsdf::f( const Vector& wo , const Vector& wi ) const
{
	// the result
	Spectrum r;

	for( unsigned i = 0 ; i < m_bsdfCount ; i++ )
		r += m_bxdf[i]->f( wo , wi );

	return r;
}
