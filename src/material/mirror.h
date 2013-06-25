/*
   FileName:      mirror.h

   Created Time:  2011-08-04 12:46:56

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MIRROR
#define	SORT_MIRROR

#include "material.h"

///////////////////////////////////////////////////////////////////////
// definition of mirror
class	Mirror : public Material
{
// public method
public:
	DEFINE_CREATOR( Mirror , "Mirror" );

	// default constructor
	Mirror();
	// destructor
	~Mirror();

	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;
};

#endif
