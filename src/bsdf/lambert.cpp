/*
   FileName:      lambert.cpp

   Created Time:  2011-08-04 12:52:30

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "lambert.h"
#include "utility/define.h"
#include "math/vector3.h"
#include "bsdf.h"

// evaluate bxdf
Spectrum Lambert::f( const Vector& wo , const Vector& wi ) const
{
	if( SameHemiSphere( wo , wi ) == false )
		return 0.0f;

	return R * INV_PI;
}
