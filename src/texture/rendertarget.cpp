/*
 * filename :	rendertarget.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "rendertarget.h"
#include "../managers/logmanager.h"
#include "../utility/error.h"

// get color from render targe
const Spectrum& RenderTarget::GetColor( int x , int y ) const
{
	if( m_pData == 0 )
		SCrash( "No memory in the render target, can't get color." );

	// filter the x y coordinate
	_texCoordFilter( x , y );
	
	// get the offset
	int offset = y * m_iTexWidth + x;
	
	return m_pData[offset];
}

// set the size for the render target
void RenderTarget::SetSize( unsigned w , unsigned h )
{
	if( w == 0 )
		LOG<<"Width of the render target is 0."<<ENDL;
	if( h == 0 )
		LOG<<"Height of the render target is 0."<<ENDL;

	// set the size
	m_iTexWidth = w;
	m_iTexHeight = h;

	// delete the previous data
	SAFE_DELETE_ARRAY( m_pData );

	// allocate the data
	m_pData = new Spectrum[ w * h ];
}

// release the render target memory
void RenderTarget::Release()
{
	m_iTexWidth = 0;
	m_iTexHeight = 0;

	SAFE_DELETE_ARRAY( m_pData );
}

// set the color
void RenderTarget::SetColor( int x , int y , float r , float g , float b )
{
	// check if there is memory
	if( m_pData == 0 )
		SCrash( "There is no data in the render target , can't set color." );

	// use filter first
	_texCoordFilter( x , y );

	// get the offset
	unsigned offset = y * m_iTexHeight + x;

	// set the color
	m_pData[offset].SetColor( r , g , b );
}

// initialize the data
void RenderTarget::_init()
{
	m_iTexWidth = 0;
	m_iTexHeight = 0;
	m_pData = 0;
}
