/*
   FileName:      rendertarget.h

   Created Time:  2011-08-04 12:45:32

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_RENDERTARGET
#define	SORT_RENDERTARGET

// include the header file
#include "compositetexture.h"

/////////////////////////////////////////////////////////////////////////
//	definition of render target
//	all of the generated image will be rendered to render target.
class	RenderTarget : public ComTexture
{
//public method
public:
	// constructor and destructor
	RenderTarget(){}
	~RenderTarget(){}

	// para 'x' : x coordinate
	// para 'y' : y coordinate
	// para 'r' : red component
	// para 'g' : green component
	// para 'b' : blue component
	void SetColor( int x , int y , float r , float g , float b );

	// para 'x' : x coordinate
	// para 'y' : y coordinate
	// para 'c' : color in spectrum form
	void SetColor( int x , int y , const Spectrum& c )
	{
		SetColor( x , y , c.GetR() , c.GetG() , c.GetB() );
	}
};

#endif
