/*
 * filename :	uvtexture.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header file
#include "uvtexture.h"
#include "../geometry/intersection.h"

// get the texture value
Spectrum UVTexture::GetColor( int x , int y ) const
{
	// filter the x y coordinate
	_texCoordFilter( x , y );

	float u = ((float)x) / m_iTexWidth;
	float v = ((float)y) / m_iTexHeight;

	return Spectrum( u , v , 0.0f );
}

// get the texture value
// para 'intersect' : the intersection
// result :	the spectrum value
Spectrum UVTexture::Evaluate( const Intersection* intersect ) const
{
	if( intersect != 0 )
		return Spectrum( intersect->u , intersect->v , 0 );

	return Spectrum();
}

// initialize default data
void UVTexture::_init()
{
	m_iTexWidth = 16;
	m_iTexHeight = 16;
}
