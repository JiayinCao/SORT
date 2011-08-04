/*
   FileName:      uvtexture.cpp

   Created Time:  2011-08-04 12:45:43

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "uvtexture.h"
#include "geometry/intersection.h"

// get the texture value
Spectrum UVTexture::GetColor( int x , int y ) const
{
	// filter the x y coordinate
	_texCoordFilter( x , y );

	float u = ((float)x) / m_iTexWidth;
	float v = ((float)y) / m_iTexHeight;

	return Spectrum( u , v , 0.0f );
}

// get the texture value
// para 'intersect' : the intersection
// result :	the spectrum value
Spectrum UVTexture::Evaluate( const Intersection* intersect ) const
{
	if( intersect != 0 )
		return Spectrum( intersect->u , intersect->v , 0 );

	return Spectrum();
}

// initialize default data
void UVTexture::_init()
{
	m_iTexWidth = 16;
	m_iTexHeight = 16;
}
