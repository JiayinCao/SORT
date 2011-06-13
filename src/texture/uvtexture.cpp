/*
 * filename :	uvtexture.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header file
#include "uvtexture.h"

// get the texture value
Spectrum UVTexture::GetColor( int x , int y ) const
{
	// filter the x y coordinate
	_texCoordFilter( x , y );

	float u = ((float)x) / m_iTexWidth;
	float v = ((float)y) / m_iTexHeight;

	return Spectrum( u , v , 0.0f );
}

// initialize default data
void UVTexture::_init()
{
	m_iTexWidth = 16;
	m_iTexHeight = 16;
}
