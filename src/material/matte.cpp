/*
 * filename	:	matte.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "matte.h"
#include "utility/define.h"
#include "bsdf/bsdf.h"
#include "bsdf/lambert.h"
#include "managers/memmanager.h"

// constructor
Matte::Matte()
{
	_init();
}

// destructor
Matte::~Matte()
{
	// delete the texture
	SAFE_DELETE( m_d );
}

// initiailize default value and register properties
void Matte::_init()
{
	m_d = 0;

	_registerAllProperty();
}

// get bsdf
Bsdf* Matte::GetBsdf( const Intersection* intersect ) const
{
	Spectrum color = m_d->Evaluate( intersect );

	Bsdf* bsdf = SORT_MALLOC(Bsdf);
	bsdf->AddBxdf( SORT_MALLOC( Lambert ) );
	return bsdf;
}

// register property
void Matte::_registerAllProperty()
{
	if( m_propertySet.empty() )
	{
		_registerProperty( "Test2" , new DiffuseProperty( this ) );
	}
}
