/*
 * filename :	gridtexture.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_GRIDTEXTURE
#define	SORT_GRIDTEXTURE

// include the file
#include "texture.h"

///////////////////////////////////////////////////////////////
// defination of grid texture
class	GridTexture : public Texture
{
// public method
public:
	// default constructor
	GridTexture();
	// constructor from two colors
	// para 'c0' :	color0
	// para 'c1' :	color1
	GridTexture( const Spectrum& c0 , const Spectrum& c1 );
	// constructor from six float
	// para 'r0' :	r component of the first color
	// para 'g0' :	g component of the first color
	// para 'b0' :	b component of the first color
	// para 'r1' :	r component of the second color
	// para 'g1' :	g component of the second color
	// para 'b1' :	b component of the second color
	GridTexture( 	float r0 , float g0 , float b0 ,
					float r1 , float g1 , float b1 );
	// destructor
	~GridTexture();
	
	// get the texture value
	// para 'x'	:	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	virtual const Spectrum& GetColor( int x , int y ) const;

// protected field
private:
	// two colors
	Spectrum	m_Color0;
	Spectrum	m_Color1;

// protected method
private:
	// init default values
	void	_init();
};

#endif
