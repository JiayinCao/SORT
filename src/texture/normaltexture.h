/*
 * filename :	normaltexture.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_NORMALTEXTURE
#define	SORT_NORMALTEXTURE

// include the header
#include "texture.h"

////////////////////////////////////////////////////////////////////////////
// defination of normal texture
class	NormalTexture : public Texture
{
// public method
public:
	// default constructor
	NormalTexture() {}
	// destructor
	~NormalTexture() {}

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual Spectrum GetColor( int x , int y ) const;

	// get the texture value
	// para 'intersect' : the intersection
	// result :	the spectrum value
	virtual Spectrum Evaluate( const Intersection* intersect ) const;
};

#endif