/*
 * filename :	rendertarget.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_RENDERTARGET
#define	SORT_RENDERTARGET

// include the header file
#include "compositetexture.h"

/////////////////////////////////////////////////////////////////////////
//	defination of render target
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
