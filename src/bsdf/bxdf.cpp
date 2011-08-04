/*
   FileName:      bxdf.cpp

   Created Time:  2011-08-04 12:52:21

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "bxdf.h"

// sample a direction randomly
Spectrum Bxdf::sample_f( const Vector& wo , Vector& wi , float* pdf ) const
{
	// not implemented
	return Spectrum();
}