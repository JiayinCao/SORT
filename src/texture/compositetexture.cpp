/*
   FileName:      compositetexture.cpp

   Created Time:  2011-08-04 12:44:30

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "compositetexture.h"
#include "spectrum/spectrum.h"
#include "managers/logmanager.h"
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
	if( m_pData == 0 )
		LOG_ERROR<<"No memory in the render target, can't get color."<<CRASH;

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
		LOG_WARNING<<"Width of the texture is 0."<<ENDL;
	if( h == 0 )
		LOG_WARNING<<"Height of the texture is 0."<<ENDL;

	// set the size
	m_iTexWidth = w;
	m_iTexHeight = h;

	// delete the previous data
	SAFE_DELETE_ARRAY( m_pData );

	// allocate the data
	m_pData = new Spectrum[ w * h ];
}
