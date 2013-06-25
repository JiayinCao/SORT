/*
   FileName:      uvtexture.h

   Created Time:  2011-08-04 12:45:50

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_UVTEXTURE
#define	SORT_UVTEXTURE

// include the header
#include "texture.h"
#include "spectrum/spectrum.h"

/////////////////////////////////////////////////////////////////////////////
//	definition of uvtexture
class	UVTexture : public Texture
{
// public function
public:
	DEFINE_CREATOR( UVTexture , "uv" );

	// constructor
	UVTexture(){_init();}
	// destructor
	~UVTexture(){}

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual Spectrum GetColor( int x , int y ) const;

	// get the texture value
	// para 'intersect' : the intersection
	// result :	the spectrum value
	virtual Spectrum Evaluate( const Intersection* intersect ) const;

//private function
private:
	// initialize default data
	void _init();
};

#endif
