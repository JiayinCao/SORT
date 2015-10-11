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

Beckmann::Beckmann( float roughness )
{
	m = pow( roughness , 4.0f );
}

// probabilty of facet with specific normal (v)
float Beckmann::D(float NoH) const
{
	float NoH2 = NoH * NoH;
	return exp( (NoH2 - 1) / (m * NoH2) ) / ( PI * m * NoH2 * NoH2 );
}

GGX::GGX( float roughness )
{
	m = pow( roughness , 4.0f );
}

// probabilty of facet with specific normal (v)
float GGX::D(float NoH) const
{
	float d = ( NoH * m - NoH ) * NoH + 1;
	return m / ( PI*d*d );
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

	// return Torrance–Sparrow BRDF
	return R * distribution->D(NoH) * F * visterm->Vis_Term( NoL , NoV , VoH );
}