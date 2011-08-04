/*
   FileName:      normaltexture.h

   Created Time:  2011-08-04 12:45:26

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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
	DEFINE_CREATOR( NormalTexture );

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
};

#endif
