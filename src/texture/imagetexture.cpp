/*
   FileName:      imagetexture.cpp

   Created Time:  2011-08-04 12:45:06

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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

	// register properties
	_registerAllProperty();
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
bool ImageTexture::LoadImageFromFile( const std::string& str )
{
	// release an image first
	Release();

	// get the texture manager
	return TexManager::GetSingleton().Read( str , this );
}

// register properties
void ImageTexture::_registerAllProperty()
{
	_registerProperty( "filename" , new FileNameProperty( this ) );
}