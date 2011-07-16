/*
 * filename :	refraction.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header file
#include "refraction.h"
#include "geometry/vector.h"
#include "bsdf/bsdf.h"

// evaluate bxdf
Spectrum Refraction::f( const Vector& wo , const Vector& wi ) const
{
	return Spectrum();
}

// sample a direction randomly
Spectrum Refraction::sample_f( const Vector& wo , Vector& wi , float* pdf ) const
{
	float coso = CosTheta( wo );
	
	bool enter = coso > 0.0f;
	float ei = eta_i;
	float et = eta_t;
	if( !enter )
	{
		ei = eta_t;
		et = eta_i;
	}

	float eta = ei / et;
	float inv_eta = 1.0f / eta;
	float sini = eta * sqrtf( 1.0f - coso * coso );
	// note , fully reflection is handled in class Reflection
	if( sini >= 1.0f )
		return 0.0f;

	float cosi = sqrtf( 1.0f - sini );
	if( enter )
		cosi = -cosi;

	wi = Vector( eta * -wo.x , cosi , eta * -wo.z );

	Spectrum t = m_fresnel->Evaluate( CosTheta( wi ) , CosTheta( wo ) ) ;

	return inv_eta * inv_eta * ( 1.0f - t ) / AbsCosTheta( wi ) ;
}
