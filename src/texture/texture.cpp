/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

// include the header file
#include "texture.h"
#include <math.h>
#include "managers/texmanager.h"
#include "compositetexture.h"
#include "math/intersection.h"

// default constructor
Texture::Texture()
{
	// inintialize default value
	_init();
}

// destructor
Texture::~Texture()
{
	m_iTexWidth = 0;
	m_iTexHeight = 0;
}

// initialize default value
void Texture::_init()
{
	m_iTexWidth = 0;
	m_iTexHeight = 0;
	m_TexCoordFilter = TCF_WARP;
}

// output the texture
bool Texture::Output( const std::string& str )
{	
	// output the texture
	return TexManager::GetSingleton().Write( str , this );
}

// get the color
Spectrum Texture::Evaluate( const Intersection* intersect ) const
{
	return GetColor( (int) (intersect->u *  m_iTexWidth) , (int) (intersect->v * m_iTexHeight) );
}

// set texture coordinate filter mode
void Texture::SetTexCoordFilter( TEXCOORDFILTER mode )
{
	m_TexCoordFilter = mode;
}

// do texture coordinate filter
void Texture::_texCoordFilter( int& x , int& y ) const
{
	switch( m_TexCoordFilter )
	{
	case TCF_WARP:
		if( x >= 0 ) 
			x = x % m_iTexWidth;
		else
			x = m_iTexWidth - ( -x ) % m_iTexWidth - 1;
		if( y >= 0 )
			y = y % m_iTexHeight;
		else
			y = m_iTexHeight - ( -y ) % m_iTexHeight - 1;
		break;
	case TCF_CLAMP:
		x = std::min( (int)m_iTexWidth - 1 , std::max( x , 0 ) );
		y = std::min( (int)m_iTexHeight - 1 , std::max( y , 0 ) );
		break;
	case TCF_MIRROR:
		x = ( x >= 0 )?x:(1-x);
		x = x % ( 2 * m_iTexWidth );
		x -= m_iTexWidth;
		x = ( x >= 0 )?x:(1-x);
		x = m_iTexWidth - 1 - x;
		y = ( y >= 0 )?y:(1-y);
		y = y % ( 2 * m_iTexHeight );
		y -= m_iTexHeight;
		y = ( y >= 0 )?y:(1-y);
		y = m_iTexHeight - 1 - y;
		break;
	}
}

// get color from uv coordinate
Spectrum Texture::GetColorFromUV( float u , float v ) const{
    int w = (int) (u * m_iTexWidth );
    int h = (int) (v * m_iTexHeight );
    return GetColor( w , h );
}
