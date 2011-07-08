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
	Spectrum color = Spectrum( 1.0f , 1.0f , 1.0f );
	if( m_d )
		color = m_d->Evaluate( intersect );

	Bsdf* bsdf = SORT_MALLOC(Bsdf);
	Lambert* lambert = SORT_MALLOC(Lambert);
	lambert->SetColor( color );
	bsdf->AddBxdf( lambert );

	return bsdf;
}

// register property
void Matte::_registerAllProperty()
{
	_registerProperty( "color" , new ColorProperty( this ) );
}
