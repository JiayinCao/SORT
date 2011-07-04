/*
 * filename :	bsdf.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "bsdf.h"
#include "bxdf.h"
#include "geometry/vector.h"

// get the number of components in current bsdf
unsigned Bsdf::NumComponents() const
{
	return m_bxdf.size();
}

// add a new bxdf
void Bsdf::AddBxdf( Bxdf* bxdf )
{
	m_bxdf.push_back( bxdf );
}

// evaluate bxdf
Spectrum Bsdf::f( const Vector& wo , const Vector& wi ) const
{
	// the result
	Spectrum r;
	vector<Bxdf*>::const_iterator it = m_bxdf.begin();
	while( it != m_bxdf.end() )
	{
		r += (*it)->f( wo , wi );
		it++;
	}

	return r;
}
