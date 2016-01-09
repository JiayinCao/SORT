/*
   FileName:      intersection.cpp

   Created Time:  2011-08-04 12:49:26

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "intersection.h"
#include <float.h>
#include "light/light.h"

// default constructor
Intersection::Intersection()
{
	u = 0.0f;
	v = 0.0f;
	t = FLT_MAX;
	primitive = 0;
}

// destructor
Intersection::~Intersection()
{
}

// get the emissive
Spectrum Intersection::Le( const Vector& wo , float* directPdfA , float* emissionPdf ) const
{
	if( primitive == 0 ) return 0.0f;
	Light* light = primitive->GetLight();
	if( light ) 
		return light->Le( *this , wo , directPdfA , emissionPdf );
	return 0.0f;
}