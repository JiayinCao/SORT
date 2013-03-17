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
MicroFacet::MicroFacet(const Spectrum &reflectance, Fresnel* f , MicroFacetDistribution* d )
{
	R = reflectance;
	distribution = d;
	fresnel = f;
	
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
	
	// return Torrance–Sparrow BRDF
	return R * distribution->D(wh) * G(wo, wi, wh) * F / (4.f * cosThetaI * cosThetaO);
}

// geometric attenuation term
float MicroFacet::G(const Vector &wo, const Vector &wi, const Vector &wh) const
{
	float NdotWh = AbsCosTheta(wh);
	float NdotWo = AbsCosTheta(wo);
	float NdotWi = AbsCosTheta(wi);
	float WOdotWh = AbsDot(wo, wh);
	return min(1.f, min((2.f * NdotWh * NdotWo / WOdotWh),
						(2.f * NdotWh * NdotWi / WOdotWh)));
}