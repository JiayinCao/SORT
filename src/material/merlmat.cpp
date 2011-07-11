/*
 * filename :	merlmat.h
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "merlmat.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"

// default constructor
MerlMat::MerlMat()
{
	_init();
}
// destructor
MerlMat::~MerlMat()
{
	_release();
}

// register property
void MerlMat::_registerAllProperty()
{
	_registerProperty( "merl" , new MerlProperty( this ) );
}

// initialize default value and register property
void MerlMat::_init()
{
	_registerAllProperty();
}

// get bsdf
Bsdf* MerlMat::GetBsdf( const Intersection* intersect ) const
{
	Bsdf* bsdf = SORT_MALLOC( Bsdf )( intersect );
	vector<Merl*>::const_iterator it = m_bxdf.begin();
	while( it != m_bxdf.end() )
	{
		bsdf->AddBxdf( (*it)->Clone() );
		it++;
	}
	
	return bsdf;
}

// release
void MerlMat::_release()
{
	vector<Merl*>::const_iterator it = m_bxdf.begin();
	while( it != m_bxdf.end() )
	{
		delete *it;
		it++;
	}
}
