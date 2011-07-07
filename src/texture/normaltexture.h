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
// definition of normal texture
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

	// whether the texture support outputing
	// result : 'true' if the texture supports texture output
	virtual	bool CanOutput() const ;
	
	// get the texture value
	// para 'intersect' : the intersection
	// result :	the spectrum value
	virtual Spectrum Evaluate( const Intersection* intersect ) const;

	// create instance
	Texture* CreateInstance() { return new NormalTexture(); }
};

#endif
