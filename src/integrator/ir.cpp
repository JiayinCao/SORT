/*
   FileName:      ir.cpp

   Created Time:  2016-2-6

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "ir.h"

IMPLEMENT_CREATOR( InstantRadiosity );

// Preprocess
void InstantRadiosity::Preprocess()
{
}

// radiance along a specific ray direction
Spectrum InstantRadiosity::Li( const Ray& r , const PixelSample& ps ) const
{
    return Spectrum( 1.0f , 0.0f , 1.0f );
}