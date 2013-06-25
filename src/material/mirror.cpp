/*
   FileName:      mirror.cpp

   Created Time:  2011-08-04 12:46:51

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "mirror.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"
#include "bsdf/reflection.h"

IMPLEMENT_CREATOR( Mirror );

// default constructor
Mirror::Mirror()
{
}

// destructor
Mirror::~Mirror()
{
}

// get bsdf
Bsdf* Mirror::GetBsdf( const Intersection* intersect ) const
{
	Bsdf* bsdf = SORT_MALLOC(Bsdf)(intersect);
	bsdf->AddBxdf( SORT_MALLOC(Reflection)(SORT_MALLOC(FresnelNo)()) );

	return bsdf;
}
