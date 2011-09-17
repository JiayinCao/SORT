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
#include "bsdf.h"
#include "utility/samplemethod.h"
#include "sampler/sample.h"

// sample a direction randomly
Spectrum Bxdf::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
	wi = CosSampleHemisphere( bs.u , bs.v );
	if( wo.y < 0.0f )
		wi.y = -wi.y;
	if( pdf ) *pdf = Pdf( wo , wi );
	return f( wo , wi );
}

// the pdf for the sampled direction
float Bxdf::Pdf( const Vector& wo , const Vector& wi ) const
{
	if( !SameHemisphere( wo , wi ) )
		return 0.0f;
	return fabs(CosHemispherePdf( wi )) * INV_PI;
}
