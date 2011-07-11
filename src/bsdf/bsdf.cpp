/*
 * filename :	bsdf.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "bsdf.h"
#include "bxdf.h"
#include "geometry/intersection.h"

// constructor
Bsdf::Bsdf( const Intersection* intersect )
{
	m_bsdfCount = 0;

	nn = intersect->normal;
	sn = intersect->tangent;
	tn = Cross( nn , sn );
}

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

	Vector swo = Normalize(_worldToLocal( wo ));
	Vector swi = Normalize(_worldToLocal( wi ));

	for( unsigned i = 0 ; i < m_bsdfCount ; i++ )
		r += m_bxdf[i]->f( swo , swi );

	return r;
}

// transform vector from worls coordinate to shading coordinate
Vector Bsdf::_worldToLocal( const Vector& v ) const
{
	return Vector( Dot(v,sn) , Dot(v,nn) , Dot(v,tn) );
}
