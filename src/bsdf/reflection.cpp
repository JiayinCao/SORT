/*
   FileName:      reflection.cpp

   Created Time:  2011-08-04 12:52:43

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "reflection.h"
#include "math/vector3.h"
#include "bsdf/bsdf.h"

// sample a direction randomly
Spectrum Reflection::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
	wi = Vector( -wo.x , wo.y , -wo.z );
	if( pdf ) *pdf = 1.0f;
	return color * m_fresnel->Evaluate( CosTheta(wi) , CosTheta(wo) ) / AbsCosTheta( wi );
}
