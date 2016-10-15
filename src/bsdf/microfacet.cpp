/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

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
	float theta = atan( alpha * sqrt(bs.v / ( 1.0f - bs.v )) );
	float phi = TWO_PI * bs.u;
	return SphericalVec( theta , phi );
}

float VisImplicit::Vis_Term( float NoL , float NoV , float VoH , float NoH)
{
	return 0.25f;
}

float VisNeumann::Vis_Term( float NoL , float NoV , float VoH , float NoH)
{
	return 1 / ( 4 * max( NoL, NoV ) );
}

float VisKelemen::Vis_Term( float NoL , float NoV , float VoH , float NoH)
{
	return 1.0f / ( 4.0f * VoH * VoH );
}

float VisSchlick::Vis_Term( float NoL , float NoV , float VoH , float NoH)
{
	float k = roughness * roughness * 0.5f;
	float Vis_SchlickV = NoV * (1 - k) + k;
	float Vis_SchlickL = NoL * (1 - k) + k;
	return 0.25f / ( Vis_SchlickV * Vis_SchlickL );
}

float VisSmith::Vis_Term( float NoL , float NoV , float VoH , float NoH)
{
	float a = roughness * roughness;
	float a2 = a*a;

	float Vis_SmithV = NoV + sqrt( NoV * (NoV - NoV * a2) + a2 );
	float Vis_SmithL = NoL + sqrt( NoL * (NoL - NoL * a2) + a2 );
	return 1.0f / ( Vis_SmithV * Vis_SmithL );
}

float VisSmithJointApprox::Vis_Term( float NoL , float NoV , float VoH , float NoH)
{
	float a = roughness * roughness;
	float Vis_SmithV = NoL * ( NoV * ( 1 - a ) + a );
	float Vis_SmithL = NoV * ( NoL * ( 1 - a ) + a );
	return 0.5f / ( Vis_SmithV + Vis_SmithL );
}

float VisCookTorrance::Vis_Term( float NoL , float NoV , float VoH , float NoH)
{
	return min( 1.0f , 2.0f * min( NoH * NoV / VoH , NoH * NoL / VoH ) ) / ( 4.0f * NoL * NoV );
}

// get reflected ray
Vector	Microfacet::getReflected( Vector v , Vector n ) const
{
	return 2.0f * n * Dot( v , n ) - v;
}

// get refracted ray
Vector Microfacet::getRefracted( Vector v , Vector n , float in_eta , float ext_eta , bool& inner_reflection ) const
{
	float coso = Dot( v , n );
	float eta = coso > 0 ? (ext_eta / in_eta) : (in_eta / ext_eta);
	float t = 1.0f - eta * eta * ( 1.0f - coso * coso );

	// total inner reflection
	if( t < 0.0f ){
		inner_reflection = true;
		return Vector(0.0f,0.0f,0.0f);
	}

	// not total reflection
	inner_reflection = false;

	// get the tranmistance/refracted ray
	float factor = (coso<0.0f)? 1.0f : -1.0f;
	return -1.0f * v * eta + ( eta * coso + factor * sqrt(t)) * n;
}

// constructor
MicroFacetReflection::MicroFacetReflection(const Spectrum &reflectance, Fresnel* f , MicroFacetDistribution* d , VisTerm* v )
{
	R = reflectance;
	distribution = d;
	fresnel = f;
	visterm = v;
	
	m_type = (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION);
}

// evaluate bxdf
Spectrum MicroFacetReflection::f( const Vector& wo , const Vector& wi ) const
{
	if( SameHemiSphere( wo , wi ) == false )
		return 0.0f;
    
    // ignore reflection at the back face
    if( wo.y <= 0.0f )
        return 0.0f;

	float NoL = AbsCosTheta( wi );
	float NoV = AbsCosTheta( wo );

	if (NoL == 0.f || NoV == 0.f)
		return Spectrum(0.f);
	
	// evaluate fresnel term
	Vector wh = Normalize(wi + wo);
	float VoH = Dot(wi, wh);
	float NoH = AbsCosTheta( wh );

	Spectrum F = fresnel->Evaluate(Dot(wi,wh), VoH);
	
	// return Torrance–Sparrow BRDF
	return R * distribution->D(NoH) * F * visterm->Vis_Term( NoL , NoV , VoH , NoH );
}

// sample a direction randomly
Spectrum MicroFacetReflection::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
	// sampling the normal
	Vector wh = distribution->sample_f( bs );

	// reflect the incident direction
	wi = getReflected( wo , wh );

	// Make sure the generate wi is in the same hemisphere with wo
	if( !SameHemiSphere( wo , wi ) )
		return 0.0f;

	if(pdf)
		*pdf = Pdf( wo , wi );

	return f( wo , wi );
}

// get the pdf of the sampled direction
float MicroFacetReflection::Pdf( const Vector& wo , const Vector& wi ) const
{
	if( !SameHemisphere( wo , wi ) )
		return 0.0f;

	Vector h = Normalize( wo + wi );
	float EoH = AbsDot( wo , h );
	float HoN = AbsCosTheta(h);
	return distribution->D(HoN) * HoN / (4.0f * EoH);
}

// constructor
MicroFacetRefraction::MicroFacetRefraction(const Spectrum &reflectance, Fresnel* f , MicroFacetDistribution* d , VisTerm* v , float ieta , float eeta )
{
	R = reflectance;
	distribution = d;
	fresnel = f;
	visterm = v;
	eta_in = ieta;
	eta_ext = eeta;

	// make sure IORs are not the same inside and outside
	if(eta_in == eta_ext)
		eta_in = eta_ext + 0.01f;
	
	m_type = (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION);
}

// evaluate bxdf
Spectrum MicroFacetRefraction::f( const Vector& wo , const Vector& wi ) const
{
	float NoL = AbsCosTheta( wi );
	float NoV = AbsCosTheta( wo );
	if (NoL == 0.f || NoV == 0.f)
		return Spectrum(0.f);
	
	float eta = CosTheta(wo) > 0 ? (eta_in / eta_ext) : (eta_ext / eta_in);

	Vector3f wh;
	bool eval_reflection = SameHemisphere(wo, wi);
	if (eval_reflection)
		wh = Normalize(wi + wo);
	else
		wh = Normalize(wo + wi * eta);

	float NoH = AbsCosTheta( wh );
	float sVoH = Dot(wo , wh);
	float VoH = fabs(sVoH);
	
	// Fresnel term
	Spectrum F = fresnel->Evaluate(Dot(wi, wh),sVoH);

	// eveluate reflection
	if( eval_reflection )
	{
		bool total_reflection = false;
		Vector _wi = getRefracted( wo , wh , eta_in , eta_ext , total_reflection );
		
		// get fresnel term
		float fresnel_term = (total_reflection)?1.0f:fresnel->Evaluate( Dot( _wi , wh ) , Dot( wo , wh ) ).GetR();

		return fresnel_term * R * distribution->D(NoH) * visterm->Vis_Term( NoL , NoV , VoH , NoH );
	}

	float sqrtDenom = Dot(wo, wh) + eta * Dot(wi, wh);
	float t = eta / sqrtDenom;
	return (Spectrum(1.f) - F) * R * distribution->D(NoH) * visterm->Vis_Term( NoL , NoV , VoH , NoH ) * 
				t * t * AbsDot(wi, wh) * AbsDot(wo, wh) * 4.0f ;
}

// sample a direction using importance sampling
Spectrum MicroFacetRefraction::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
	// whether sample reflection
	bool sampleReflection = false;

	// sampling the normal
	Vector wh = distribution->sample_f( bs );

	// try to get refracted ray
	bool total_reflection = false;
	wi = getRefracted( wo , wh , eta_in , eta_ext , total_reflection );

	// handle total inner relection seperately
	if( total_reflection )
	{
		// get reflected ray
		wi = getReflected( wo , wh );

		// Make sure the generate wi is in the same hemisphere with wo
		if( !SameHemiSphere( wo , wi ) )
			return 0.0f;

		if(pdf)
			*pdf = Pdf( wo , wi );

		return f( wo , wi );
	 }

	// get fresnel term
	float fresnel_term = (total_reflection)?1.0f:fresnel->Evaluate( Dot( wi , wh ) , Dot( wo , wh ) ).GetR();

	// sample reflection or refraction by fresnel term
	if( sort_canonical() <= fresnel_term )
		sampleReflection = true;

	// sample reflection or refraction
	if( sampleReflection )
	{
		// get reflected ray
		wi = getReflected( wo , wh );

		// Make sure the generate wi is in the same hemisphere with wo
		if( !SameHemiSphere( wo , wi ) )
			return 0.0f;

		if(pdf)
			*pdf = Pdf( wo , wi ) * fresnel_term;
	}else
	{
		// Make sure the generate wi is not in the same hemisphere with wo
		if( SameHemiSphere( wo , wi ) )
			return 0.0f;

		if(pdf)
			*pdf = Pdf( wo , wi ) * ( 1.0f - fresnel_term );
	}

	return f( wo , wi );
}

// get the pdf of the sampled direction
float MicroFacetRefraction::Pdf( const Vector& wo , const Vector& wi ) const
{
	if( SameHemisphere( wo , wi ) )
	{
		Vector h = Normalize( wo + wi );
		float EoH = AbsDot( wo , h );
		float HoN = AbsCosTheta(h);
		return distribution->D(HoN) * HoN / (4.0f * EoH);
	}

	float eta = CosTheta(wo) > 0 ? (eta_in / eta_ext) : (eta_ext / eta_in);
    Vector3f wh = Normalize(wo + wi * eta);

    // Compute change of variables _dwh\_dwi_ for microfacet transmission
    float sqrtDenom = Dot(wo, wh) + eta * Dot(wi, wh);
    float dwh_dwi = eta * eta * AbsDot(wi, wh) / (sqrtDenom * sqrtDenom);
	float HoN = AbsCosTheta(wh);
    return distribution->D(HoN) * HoN * dwh_dwi;
}
