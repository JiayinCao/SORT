/*
 * filename :	constanttexture.h
 *
 * programmer : Cao Jiayin
 */

#ifndef SORT_CONSTANTTEXTURE
#define	SORT_CONSTANTTEXTURE

// include the header
#include "texture.h"
#include "spectrum/spectrum.h"

////////////////////////////////////////////////////////////
// definition of constant texture
class ConstantTexture : public Texture
{
// public method
public:
	// default constructor
	ConstantTexture();
	// constructor from three float
	// para 'r' : r component
	// para 'g' : g component
	// para 'b' : b component
	ConstantTexture( float r , float g , float b );
	// destructor
	~ConstantTexture();

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual Spectrum GetColor( int x , int y ) const
	{
		return m_Color;
	}

	// get the texture value
	// para 'intersect' : the intersection
	// para 'result'    : the result
	virtual Spectrum Evaluate( const Intersection* intersect ) const
	{
		return m_Color;
	}

// private field
private:
	// the color for the texture
	Spectrum	m_Color;

// private method
private:
	// initialize the data
	void _init();
};

#endif
