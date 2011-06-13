/*
 * filename :	uvtexture.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_UVTEXTURE
#define	SORT_UVTEXTURE

// include the header
#include "texture.h"
#include "../spectrum/spectrum.h"

/////////////////////////////////////////////////////////////////////////////
//	defination of uvtexture
class	UVTexture : public Texture
{
// public function
public:
	// constructor
	UVTexture(){_init();}
	// destructor
	~UVTexture(){}

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual Spectrum GetColor( int x , int y ) const;

//private function
private:
	// initialize default data
	void _init();
};

#endif
