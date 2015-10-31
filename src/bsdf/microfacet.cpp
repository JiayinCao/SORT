/*
   FileName:      microfacet.cpp

   Created Time:  2013-03-17 19:09:29

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "microfacet.h"
#include "bsdf.h"
#include "sampler/sample.h"

// constructor
Blinn::Blinn( float roughness )
{
	exp = roughness;
	exp = 2.0f / pow( exp , 4.0f ) - 2.0f;
}

// probabilty of facet with specific normal (v)
float Blinn::D(float NoH) const
{
	return (exp+2.0f) * INV_TWOPI * powf( NoH , exp );
}

// sampling according to GGX
Vector Blinn::sample_f( const BsdfSample& bs ) const
{
	float costheta = powf( bs.u , 1.0f / (exp+2.0f) );
	float sintheta = sqrtf( max( 0.0f , 1.0f - costheta * costheta ) );
	float phi = TWO_PI * bs.v;

	return SphericalVec( sintheta , costheta , phi );
}

Beckmann::Beckmann( float roughness )
{
	alpha = roughness * roughness;
	m = alpha * alpha;
}

// probabilty of facet with specific normal (v)
float Beckmann::D(float NoH) const
{
	float NoH2 = NoH * NoH;
	return exp( (NoH2 - 1) / (m * NoH2) ) / ( PI * m * NoH2 * NoH2 );
}

// sampling according to GGX
Vector Beckmann::sample_f( const BsdfSample& bs ) const
{
	float theta = atan( sqrt( -1.0f * alpha * alpha * log( 1.0f - bs.u ) ) );
	float phi = TWO_PI * bs.v;

	return SphericalVec( theta , phi );
}

GGX::GGX( float roughness )
{
	alpha = roughness * roughness;
	m = alpha * alpha;
}

// probabilty of facet with specific normal (v)
float GGX::D(float NoH) const
{
	float d = ( m - 1.0f ) * NoH * NoH + 1.0f;
	return m / ( PI*d*d );
}

Vector GGX::sample_f( const BsdfSample& bs ) const
{
	float phi = TWO_PI * bs.u;
	float theta = acos( sqrt( ( 1.0f - bs.v ) / ( ( m - 1.0f ) * bs.v + 1.0f ) ) );

	return SphericalVec( theta , phi );
}

float VisImplicit::Vis_Term( float NoL , float NoV , float VoH )
{
	return 0.25f;
}

float VisNeumann::Vis_Term( float NoL , float NoV , float VoH )
{
	return 1 / ( 4 * max( NoL, NoV ) );
}

float VisKelemen::Vis_Term( float NoL , float NoV , float VoH )
{
	return 1.0f / ( 4.0f * VoH * VoH );
}

float VisSchlick::Vis_Term( float NoL , float NoV , float VoH )
{
	float k = roughness * roughness * 0.5f;
	float Vis_SchlickV = NoV * (1 - k) + k;
	float Vis_SchlickL = NoL * (1 - k) + k;
	return 0.25f / ( Vis_SchlickV * Vis_SchlickL );
}

float VisSmith::Vis_Term( float NoL , float NoV , float VoH )
{
	float a = roughness * roughness;
	float a2 = a*a;

	float Vis_SmithV = NoV + sqrt( NoV * (NoV - NoV * a2) + a2 );
	float Vis_SmithL = NoL + sqrt( NoL * (NoL - NoL * a2) + a2 );
	return 1.0f / ( Vis_SmithV * Vis_SmithL );
}

float VisSmithJointApprox::Vis_Term( float Roughness, float NoV, float NoL )
{
	float a = roughness * roughness;
	float Vis_SmithV = NoL * ( NoV * ( 1 - a ) + a );
	float Vis_SmithL = NoV * ( NoL * ( 1 - a ) + a );
	return 0.5f / ( Vis_SmithV + Vis_SmithL );
}

// constructor
MicroFacet::MicroFacet(const Spectrum &reflectance, Fresnel* f , MicroFacetDistribution* d , VisTerm* v )
{
	R = reflectance;
	distribution = d;
	fresnel = f;
	visterm = v;
	
	m_type = (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION);
}

// evaluate bxdf
// para 'wo' : out going direction
// para 'wi' : in direction
// result    : the portion that comes along 'wo' from 'wi'
Spectrum MicroFacet::f( const Vector& wo , const Vector& wi ) const
{
	float cosThetaO = AbsCosTheta(wo);
	float cosThetaI = AbsCosTheta(wi);
	
	if (cosThetaI == 0.f || cosThetaO == 0.f)
		return Spectrum(0.f);
	
	// evaluate fresnel term
	Vector wh = Normalize(wi + wo);
	float cosThetaH = Dot(wi, wh);
	Spectrum F = fresnel->Evaluate(cosThetaH);
	
	float NoH = AbsCosTheta( wh );
	float NoL = AbsCosTheta( wi );
	float NoV = AbsCosTheta( wo );
	float VoH = Dot( wh , wo );

	// g-term
	float gterm = min( 1.0f , 2.0f * min( NoH * NoV / VoH , NoH * NoL / VoH ) );

	// return Torrance–Sparrow BRDF
	//return R * distribution->D(NoH) * F * visterm->Vis_Term( NoL , NoV , VoH );
	return R * distribution->D(NoH) * F * gterm / ( 4.0f * NoL * NoV );
}

// sample a direction randomly
// para 'wo'  : out going direction
// para 'wi'  : in direction generated randomly
// para 'bs'  : bsdf sample variable
// para 'pdf' : property density function value of the specific 'wi'
// result     : brdf value for the 'wo' and 'wi'
Spectrum MicroFacet::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
	// sampling the normal
	Vector wh = distribution->sample_f( bs );

	// reflect the incident direction
	wi = 2.0f * wh * Dot( wo , wh ) - wo;

	// Make sure the generate wi is in the same hemisphere with wo
	if( !SameHemiSphere( wo , wi ) )
	{
		if( pdf )
			*pdf = 0.0f;
		return 0.0f;
	}

	if(pdf)
		*pdf = Pdf( wo , wi );

	return f( wo , wi );
}

// get the pdf of the sampled direction
// para 'wo' : out going direction
// para 'wi' : coming in direction from light
// result    : the pdf for the sample
float MicroFacet::Pdf( const Vector& wo , const Vector& wi ) const
{
	if( !SameHemisphere( wo , wi ) )
		return 0.0f;

	Vector h = Normalize( wo + wi );
	float EoH = AbsDot( wo , h );
	if( EoH <= 0.0f )
		return 0.0f;
	return distribution->D(AbsCosTheta(h)) / ( 4.0f * EoH );
}