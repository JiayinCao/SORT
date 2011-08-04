/*
   FileName:      rendertarget.cpp

   Created Time:  2011-08-04 12:45:29

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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


