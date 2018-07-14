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

// include header file
#include "compositetexture.h"
#include "spectrum/spectrum.h"
#include "utility/log.h"
#include "utility/define.h"

// constructor from a texture
void ComTexture::_copy( const Texture& tex )
{
	// the texture to copy
	SetSize( tex.GetWidth() , tex.GetHeight() );

	// copy the data
	for( unsigned i = 0 ; i < m_iTexHeight ; i++ )
		for( unsigned j = 0 ; j < m_iTexWidth ; j++ )
		{
			unsigned offset = i * m_iTexWidth + j;
			m_pData[offset] = tex.GetColor( (int)j , (int)i );
		}
}
// copy from a comtexture
void ComTexture::_copy( const ComTexture& tex )
{
	// the texture to copy
	SetSize( tex.GetWidth() , tex.GetHeight() );

	// copy the data
	for( unsigned i = 0 ; i < m_iTexHeight ; i++ )
		for( unsigned j = 0 ; j < m_iTexWidth ; j++ )
		{
			unsigned offset = i * m_iTexWidth + j;
			m_pData[offset] = tex.GetColor( (int)j , (int)i );
		}
}

// initialize the data
void ComTexture::_init()
{
	m_pData = 0;
	m_iTexWidth = 0;
	m_iTexHeight = 0;
}

// release the memory
void ComTexture::Release()
{
	SAFE_DELETE_ARRAY( m_pData );
	m_iTexWidth = 0;
	m_iTexHeight = 0;
}

// get color from the texture
Spectrum ComTexture::GetColor( int x , int y ) const
{
    sAssertMsg( nullptr != m_pData , IMAGE , "No memory in the render target, can't get color." );

	// filter the x y coordinate
	_texCoordFilter( x , y );
	
	// get the offset
	int offset = y * m_iTexWidth + x;
	
	return m_pData[offset];
}

// set the size for the render target
void ComTexture::SetSize( unsigned w , unsigned h )
{
	if( w == 0 )
        slog( WARNING , IMAGE , "Width of the texture is 0." );
	if( h == 0 )
		slog( WARNING , IMAGE , "Height of the texture is 0." );

	// set the size
	m_iTexWidth = w;
	m_iTexHeight = h;

	// delete the previous data
	SAFE_DELETE_ARRAY( m_pData );

	// allocate the data
	m_pData = new Spectrum[ w * h ];
}
