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

// instance property set
DEFINE_PROPERTY(Matte);

// constructor
Matte::Matte()
{
	m_d = 0;
	_registerAllProperty();
}

// destructor
Matte::~Matte()
{
	// delete the texture
	SAFE_DELETE( m_d );
}

// get bsdf
Bsdf* Matte::GetBsdf( const Intersection* intersect ) const
{
	return 0;
}

// register property
void Matte::_registerAllProperty()
{
	if( m_propertySet.empty() )
	{
		_registerProperty( "Test" , new DiffuseProperty( this ) );
	}
}
