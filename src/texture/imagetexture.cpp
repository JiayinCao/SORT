/*
 * filename :	imagetexture.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "imagetexture.h"
#include "managers/texmanager.h"
#include "managers/logmanager.h"

// initialize default data
void ImageTexture::_init()
{
	// set default value
	m_pMemory = 0;
}

// get color from image
Spectrum ImageTexture::GetColor( int x , int y ) const
{
	// if there is no image, just crash
	if( 0 == m_pMemory || m_pMemory->m_ImgMem == 0 )
		LOG_ERROR<<"There is no memory for the texture!!"<<CRASH;

	// filter the texture coordinate
	_texCoordFilter( x , y );

	// get the offset
	int offset = y * m_iTexWidth + x;

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
bool ImageTexture::LoadImageFromFile( const std::string& str , TEX_TYPE type )
{
	// release an image first
	Release();

	// get the texture manager
	return TexManager::GetSingleton().Read( str , this , type );
}
