/*
 * filename :	constanttexture.h
 *
 * programmer : Cao Jiayin
 */

#ifndef SORT_CONSTANTTEXTURE
#define	SORT_CONSTANTTEXTURE

// include the header
#include "texture.h"
#include "../spectrum/spectrum.h"

////////////////////////////////////////////////////////////
// defination of constant texture
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

// private field
private:
	// the color for the texture
	Spectrum	m_Color;
};

#endif
