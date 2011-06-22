/*
 * filename :	rendertarget.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "rendertarget.h"
#include "managers/logmanager.h"

// set the color
void RenderTarget::SetColor( int x , int y , float r , float g , float b )
{
	// check if there is memory
	if( m_pData == 0 )
		LOG_ERROR<<"There is no data in the render target , can't set color."<<CRASH;

	// use filter first
	_texCoordFilter( x , y );

	// get the offset
	unsigned offset = y * m_iTexWidth + x;

	// set the color
	m_pData[offset].SetColor( r , g , b );
}


