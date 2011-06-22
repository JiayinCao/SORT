/*
 * filename :	normaltexture.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "normaltexture.h"
#include "managers/logmanager.h"
#include "geometry/intersection.h"

// get the texture value
Spectrum NormalTexture::GetColor( int x , int y ) const
{
	return Spectrum();
}

// get the texture value
Spectrum NormalTexture::Evaluate( const Intersection* intersect ) const
{
	if( intersect )
		return Spectrum( intersect->normal.x , intersect->normal.y , intersect->normal.z );

	return Spectrum();
}

// whether the image supports outputing
bool NormalTexture::CanOutput() const
{
	LOG_WARNING<<"NormalTexture doesn't support outputing."<<ENDL;

	return false;
}
