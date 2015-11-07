/*
   FileName:      refraction.cpp

   Created Time:  2011-08-04 12:52:49

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "refraction.h"
#include "math/vector3.h"
#include "bsdf/bsdf.h"

// sample a direction randomly
Spectrum Refraction::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
	float coso = CosTheta( wo );
	
	bool enter = coso > 0.0f;
	float eta = coso > 0 ? (eta_t / eta_i) : (eta_i / eta_t);
	float inv_eta = 1.0f / eta;
	float sini = eta * sqrtf( 1.0f - coso * coso );
	// note , fully reflection is handled in class Reflection
	if( sini >= 1.0f )
		return 0.0f;

	float cosi = sqrtf( 1.0f - sini * sini );
	if( enter )
		cosi = -cosi;

	wi = Vector( eta * -wo.x , cosi , eta * -wo.z );

	Spectrum t = m_fresnel->Evaluate( CosTheta( wi ) , CosTheta( wo ) ) ;

	if( pdf ) 
		*pdf = 1.0f;

	return color * inv_eta * inv_eta * ( 1.0f - t ) / AbsCosTheta( wi ) ;
}
