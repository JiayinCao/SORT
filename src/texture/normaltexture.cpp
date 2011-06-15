/*
 * filename :	normaltexture.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "normaltexture.h"
#include "../managers/logmanager.h"
#include "../geometry/intersection.h"

// get the texture value
Spectrum NormalTexture::GetColor( int x , int y ) const
{
	// normal texture doesn't support outputing
	LOG_ERROR<<"Normal texture doesn't support outputing."<<CRASH;

	return Spectrum();
}

// get the texture value
Spectrum NormalTexture::Evaluate( const Intersection* intersect ) const
{
	if( intersect )
		return Spectrum( intersect->normal.x , intersect->normal.y , intersect->normal.z );

	return Spectrum();
}