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
	Sort_Assert( m_pMemory != 0 && m_pMemory->m_ImgMem != 0 );

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