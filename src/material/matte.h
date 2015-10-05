/*
   FileName:      matte.h

   Created Time:  2011-08-04 12:46:39

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MATTE
#define	SORT_MATTE

// include header file
#include "material.h"
#include "texture/texture.h"
#include "utility/strhelper.h"

//////////////////////////////////////////////////////////////////////
// definition of matte material
class Matte : public Material
{
// public method
public:
	DEFINE_CREATOR( Matte , "Matte" );

	// default constructor
	Matte();

	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;
	
// private field
private:
	// a scaled color , default is ( 0.1f , 0.1f , 0.1f )
	Spectrum m_color;
};

#endif


