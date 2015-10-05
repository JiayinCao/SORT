/*
   FileName:      orennayar.cpp

   Created Time:  2013-03-16 23:27:29

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "orennayar.h"
#include "bsdf.h"

// constructor
OrenNayar::OrenNayar( const Spectrum& reflectence , float roughness)
{
	R = reflectence;
	
	// rough ness ranges from 0 to infinity
	roughness = max( 0.0f , roughness );

	float roughness2 = roughness * roughness;
	A = 1.0f - (roughness2 / ( 2.0f * (roughness2+0.33f)));
	B = 0.45f * roughness2 / ( roughness2 + 0.09f );
	
	m_type = (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION);
}

// evaluate bxdf
// para 'wo' : out going direction
// para 'wi' : in direction
// result    : the portion that comes along 'wo' from 'wi'
Spectrum OrenNayar::f( const Vector& wo , const Vector& wi ) const
{
	float sintheta_i = SinTheta(wi);
	float sintheta_o = SinTheta(wo);
	
	float sinphii = SinPhi(wi);
	float cosphii = CosPhi(wi);
	float sinphio = SinPhi(wo);
	float cosphio = CosPhi(wo);
	float dcos = cosphii * cosphio + sinphii * sinphio;
	if( dcos < 0.0f ) dcos = 0.0f;
	
	float abs_cos_theta_o = (float)AbsCosTheta(wo);
	float abs_cos_theta_i = (float)AbsCosTheta(wi);
	
	if( abs_cos_theta_i < 0.00001f && abs_cos_theta_o < 0.00001f )
		return 0.0f;
	
	float sinalpha , tanbeta;
	if( abs_cos_theta_o > abs_cos_theta_i )
	{
		sinalpha = sintheta_i;
		tanbeta = sintheta_o / abs_cos_theta_o;
	}else
	{
		sinalpha = sintheta_o;
		tanbeta = sintheta_i / abs_cos_theta_i;
	}
	
	return R * INV_PI * ( A + B * dcos * sinalpha * tanbeta );
}