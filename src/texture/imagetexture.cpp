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
#include "imagetexture.h"
#include "managers/texmanager.h"

IMPLEMENT_CREATOR( ImageTexture );

// initialize default data
void ImageTexture::_init()
{
	// set default value
	m_pMemory = 0;

	// register properties
	_registerAllProperty();
}

// get color from image
Spectrum ImageTexture::GetColor( int x , int y ) const
{
	// if there is no image, just crash
    sAssertMsg( m_pMemory != 0 && m_pMemory->m_ImgMem != 0 , IMAGE , stringFormat( "Texture %s not loaded!" , m_Name.c_str() ) );

	// filter the texture coordinate
	_texCoordFilter( x , y );

	// get the offset
	int offset = ( m_iTexHeight - 1 - y ) * m_iTexWidth + x;

	// get the color
	return m_pMemory->m_ImgMem[ offset ];
}

// release the texture memory
void ImageTexture::Release()
{
	m_pMemory = 0;
	m_iTexWidth = 0;
	m_iTexHeight = 0;
}

// load image from file
bool ImageTexture::LoadImageFromFile( const std::string& str )
{
	// release an image first
	Release();

    m_Name = str;

	// get the texture manager
	if( TexManager::GetSingleton().Read( str , this ) )
	{
		_average();
		return true;
	}
	return false;
}

// register properties
void ImageTexture::_registerAllProperty()
{
	_registerProperty( "filename" , new FileNameProperty( this ) );
}

// get average color
Spectrum ImageTexture::GetAverage() const
{
	return m_Average;
}

// compute average radiance
void ImageTexture::_average()
{
	// if there is no image, just crash
	if( m_pMemory == 0 || m_pMemory->m_ImgMem == 0 )
		return;

	Spectrum average;
	for( unsigned i = 0 ; i < m_iTexHeight ; ++i )
		for( unsigned j = 0 ; j < m_iTexWidth ; ++j )
		{
			// get the offset
			int offset = i * m_iTexWidth + j;
			// get the color
			average += m_pMemory->m_ImgMem[ offset ];
		}

	m_Average = average / (float)( m_iTexWidth * m_iTexHeight );
}
