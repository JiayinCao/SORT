/*
 * filename :	glass.cpp
 *
 * programmer :	Cao Jiayin
 */

// include headers
#include "glass.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"
#include "bsdf/reflection.h"
#include "bsdf/refraction.h"

// default constructor
Glass::Glass()
{
	eta_i = 1.0f;
	eta_t = 1.1f;

	_registerAllProperty();
}

// register all properties
void Glass::_registerAllProperty()
{
	_registerProperty( "eta_i" , new EtaIProperty( this ) );
	_registerProperty( "eta_t" , new EtaTProperty( this ) );
}

// get bsdf
Bsdf* Glass::GetBsdf( const Intersection* intersect ) const
{
	Bsdf* bsdf = SORT_MALLOC(Bsdf)(intersect);

	Fresnel* fresnel = SORT_MALLOC(FresnelDielectric)( eta_i , eta_t );

	bsdf->AddBxdf( SORT_MALLOC(Reflection)(fresnel) );
	bsdf->AddBxdf( SORT_MALLOC(Refraction)(eta_i,eta_t,fresnel) );

	return bsdf;	
}

