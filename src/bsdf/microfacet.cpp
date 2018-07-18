/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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
#include <cmath>

// constructor
Blinn::Blinn( float roughnessU , float roughnessV )
{
    // UE4 style way to convert roughness to alpha used here because it still keeps sharp reflection with low value of roughness
    // http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
    // PBRT way of converting roughness will result slightly blurred reflection even with 0 as roughness.
    // This is tackled in PBRT because it has a separate way to do perfect reflection, which is not available in SORT.
    const static auto convert = [](float roughness) {
        roughness = max(0.01f, roughness);
        return 2.0f / pow(roughness, 4.0f) - 2.0f;
    };
    alphaU = convert(roughnessU);
    alphaV = convert(roughnessV);
}

// probability of facet with specific normal (h)
float Blinn::D(const Vector& h) const
{
    // An Anisotropic Phong BRDF Model (Micheal Ashikhmin, Peter Shirley)
    // http://www.irisa.fr/prive/kadi/Lopez/ashikhmin00anisotropic.pdf ( The original phong model was not energy conservative, see the following link for a fix. )
    // http://simonstechblog.blogspot.com/2011/12/microfacet-brdf.html ( Modified Phong model )
    // http://www.farbrausch.de/~fg/stuff/phong.pdf ( Derivation of phong model's scaling factor )
    // Anisotropic model:   D(w_h) = sqrt( ( alphaU + 2 ) * ( alphaV * 2 ) ) * cos(\theta) ^ ( cos(\phi)^2 * alphaU + sin(\phi)^2 * alphaV ) / ( 2 * PI )
    // Isotropic model:     D(w_h) = ( alpha + 2 ) * cos(\theta) ^ alpha / ( 2 * PI )

    float NoH = AbsCosTheta(h);
    if (NoH <= 0.0f) return 0.0f;
    const float sin_phi_h_sq = SinPhi2(h);
    const float cos_phi_h_sq = 1.0f - sin_phi_h_sq;
    return sqrt((alphaU + 2) * (alphaV + 2)) * pow(NoH, cos_phi_h_sq * alphaU + sin_phi_h_sq * alphaV) * INV_TWOPI;
}

#include "utility/sassert.h"
#include "utility/strhelper.h"

#pragma optimize( "" , off )

// sampling according to GGX
Vector Blinn::sample_f( const BsdfSample& bs , const Vector& wo ) const
{
    float phi = 0.0f;
    if (alphaU == alphaV) {
        // Refer the following link ( my blog ) for a full derivation of isotropic importance sampling for Blinn
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        phi = TWO_PI * bs.v;
    }else{
        const static int offset[5] = { 0 , 1 , 1 , 2 , 2 };
        const int i = bs.v == 0.25f ? 0 : (int)(bs.v / 0.25f);
        phi = std::atan(std::sqrt((alphaU + 2.0f) / (alphaV + 2.0f)) * std::tan(TWO_PI * bs.v)) + offset[i] * PI;
    }

    const float sin_phi_h = std::sin(phi);
    const float sin_phi_h_sq = sin_phi_h * sin_phi_h;
    const float alpha = alphaU * (1.0f - sin_phi_h_sq) + alphaV * sin_phi_h_sq;
    const float cos_theta = std::pow(bs.u, 1.0f / (alpha + 2.0f));
    const float sin_theta = sqrtf(max(0.0f, 1.0f - cos_theta * cos_theta));

    auto wh = SphericalVec(sin_theta, cos_theta, phi);
    if (!SameHemiSphere(wh, wo)) wh = -wh;
    return wh;
}

Beckmann::Beckmann( float roughnessU , float roughnessV )
{
    // UE4 style way to convert roughness to alpha used here because it still keeps sharp reflection with low value of roughness
    // http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
    // PBRT way of converting roughness will result slightly blurred reflection even with 0 as roughness.
    // This is tackled in PBRT because it has a separate way to do perfect reflection, which is not available in SORT. 
    const static auto convert = []( float roughness ) {
        roughness = std::max(roughness, (float)1e-3);
        return roughness * roughness;
    };
    alphaU = convert( roughnessU );
    alphaV = convert( roughnessV );
    alphaU2 = alphaU * alphaU;
    alphaV2 = alphaV * alphaV;
    alphaUV = alphaU * alphaV;
}

// probability of facet with specific normal (h)
float Beckmann::D(const Vector& h) const
{
    // Anisotropic Beckmann distribution formula, pbrt-v3 ( page 539 )
    // Anisotropic model:   D(w_h) = pow( e , -tan(\theta_h)^2 * ( (cos(\phi_h) / alphaU)^2 + (sin(\phi_h) / alphaV)^2 ) ) / ( PI * alphaU * alphaV * cos(\theta_h) ^ 4
    // Isotropic model:     D(w_h) = pow( e , -(tan(\theta_h)/alpha)^2 ) / ( PI * alpha^2 * cos(\theta_h)^4 )
    const float cos_theta_h_sq = CosTheta2(h);
    if( cos_theta_h_sq <= 0.0f ) return 0.f;
    const float tan_theta_h_sq = TanTheta2(h);
    
    const float sin_phi_h_sq = SinPhi2(h);
    const float cos_phi_h_sq = 1.0f - sin_phi_h_sq;
    return exp( -tan_theta_h_sq * ( cos_phi_h_sq / alphaU2 + sin_phi_h_sq / alphaV2 ) ) / ( PI * alphaUV * cos_theta_h_sq * cos_theta_h_sq );
}

// sampling according to GGX
Vector Beckmann::sample_f( const BsdfSample& bs , const Vector& wo ) const
{
    const float logSample = std::log( bs.u );
    sAssert(!std::isinf(logSample), "Bad sample in Beckman sampling.");
    
    float theta, phi;
    if( alphaU == alphaV ){
        // Refer the following link ( my blog ) for a full derivation of isotropic importance sampling for Beckmann
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        theta = atan( sqrt( -1.0f * alphaUV * logSample ) );
        phi = TWO_PI * bs.v;
    }else{
        phi = std::atan(alphaV / alphaU * std::tan(TWO_PI * bs.v));
        if (bs.v > 0.5f) phi += PI;
        const float sin_phi = std::sin(phi);
        const float sin_phi_sq = sin_phi * sin_phi;
        theta = atan(sqrt(-logSample / ((1.0f - sin_phi_sq) / alphaU2 + sin_phi_sq / alphaV2)));
    }
    
    auto wh = SphericalVec(theta, phi);
    if (!SameHemiSphere(wh, wo)) wh = -wh;
    return wh;
}

GGX::GGX( float roughnessU , float roughnessV )
{
    // UE4 style way to convert roughness to alpha used here because it still keeps sharp reflection with low value of roughness
    // http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
    // PBRT way of converting roughness will result slightly blurred reflection even with 0 as roughness.
    // This is tackled in PBRT because it has a separate way to do perfect reflection, which is not available in SORT. 
    const static auto convert = []( float roughness ) {
        roughness = std::max(roughness, (float)1e-3);
        return roughness * roughness;
    };
    alphaU = convert( roughnessU );
    alphaV = convert( roughnessV );
    alphaU2 = alphaU * alphaU;
    alphaV2 = alphaV * alphaV;
    alphaUV = alphaU * alphaV;
}

// probability of facet with specific normal (h)
float GGX::D(const Vector& h) const
{
    // Anisotropic GGX (Trowbridge-Reitz) distribution formula, pbrt-v3 ( page 539 )
    // Anisotropic model:   D(w_h) = 1.0f / ( PI * alphaU * alphaV * cos(\theta)^4 * ( 1 + tan(\thete)^2 * ( ( cos(\theta) / alphaU ) ^ 2 + ( sin(\theta) / alphaV ) ^ 2 ) ) ^ 2 )
    // Isotrocpic model:    D(w_h) = alpha ^ 2 / ( PI * ( 1 + ( alpha ^ 2 - 1 ) * cos(\theta) ^ 2 ) ^ 2
    const float cos_theta_h_sq = CosTheta2(h);
    if( cos_theta_h_sq <= 0.0f ) return 0.f;
    
    const float sin_phi_h_sq = SinPhi2(h);
    const float cos_phi_h_sq = 1.0f - sin_phi_h_sq;
    const float beta = ( cos_theta_h_sq + ( 1.0f - cos_theta_h_sq ) * (cos_phi_h_sq / alphaU2 + sin_phi_h_sq / alphaV2));
    return 1.0f / ( PI * alphaUV * beta * beta );
}

Vector GGX::sample_f( const BsdfSample& bs , const Vector& wo ) const
{
    float theta, phi;
    if( alphaU == alphaV ){
        // Refer the following link ( my blog ) for a full derivation of isotropic importance sampling for GGX
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        theta = atan( alphaU * sqrt( bs.v / ( 1.0f - bs.v )) );
        phi = TWO_PI * bs.u;
    }else{
        phi = std::atan( alphaV / alphaU * std::tan( TWO_PI * bs.v ) );
        if( bs.u > 0.5f ) phi += PI;
        const float sin_phi = std::sin(phi);
        const float sin_phi_sq = sin_phi * sin_phi;
        const float cos_phi_sq = 1.0f - sin_phi_sq;
        float beta = 1.0f / ( cos_phi_sq / alphaU2 + sin_phi_sq / alphaV2 );
        theta = atan( sqrt( beta * bs.u / ( 1.0f - bs.u ) ) );
    }
    auto wh = SphericalVec( theta , phi );
    if( !SameHemiSphere(wh , wo) ) wh = -wh;
    return wh;
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
    const float k = roughness * roughness * 0.5f;
    const float Vis_SchlickV = NoV * (1 - k) + k;
    const float Vis_SchlickL = NoL * (1 - k) + k;
	return 0.25f / ( Vis_SchlickV * Vis_SchlickL );
}

float VisSmith::Vis_Term( float NoL , float NoV , float VoH , float NoH)
{
    const float a = roughness * roughness;
    const float a2 = a*a;

    const float Vis_SmithV = NoV + sqrt( NoV * (NoV - NoV * a2) + a2 );
    const float Vis_SmithL = NoL + sqrt( NoL * (NoL - NoL * a2) + a2 );
	return 1.0f / ( Vis_SmithV * Vis_SmithL );
}

float VisSmithJointApprox::Vis_Term( float NoL , float NoV , float VoH , float NoH)
{
    const float a = roughness * roughness;
    const float Vis_SmithV = NoL * ( NoV * ( 1 - a ) + a );
    const float Vis_SmithL = NoV * ( NoL * ( 1 - a ) + a );
	return 0.5f / ( Vis_SmithV + Vis_SmithL );
}

float VisCookTorrance::Vis_Term( float NoL , float NoV , float VoH , float NoH)
{
	return min( 1.0f , 2.0f * min( NoH * NoV / VoH , NoH * NoL / VoH ) ) / ( 4.0f * NoL * NoV );
}

// get reflected ray
Vector	Microfacet::getReflected( Vector v , Vector n ) const
{
	return ( 2.0f * Dot( v , n ) ) * n - v;
}

// get refracted ray
Vector Microfacet::getRefracted( Vector v , Vector n , float in_eta , float ext_eta , bool& inner_reflection ) const
{
	const float coso = Dot( v , n );
	const float eta = CosTheta(v) > 0 ? (ext_eta / in_eta) : (in_eta / ext_eta);
	const float t = 1.0f - eta * eta * max( 0.0f , 1.0f - coso * coso );

	// total inner reflection
    inner_reflection = (t <= 0.0f);
	if( inner_reflection )
		return Vector(0.0f,0.0f,0.0f);

	// get the refracted ray
	const float factor = (coso<0.0f)? 1.0f : -1.0f;
	return -eta * v  + ( eta * coso - sqrt(t)) * n;
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

	const float NoL = AbsCosTheta( wi );
	const float NoV = AbsCosTheta( wo );

	if (NoL == 0.f || NoV == 0.f)
		return Spectrum(0.f);
	
	// evaluate fresnel term
	const Vector wh = Normalize(wi + wo);
	const float VoH = Dot(wi, wh);
	const float NoH = AbsCosTheta( wh );

	const Spectrum F = fresnel->Evaluate( AbsDot(wo,wh) , fabs(VoH) );
	
	// return Torrance–Sparrow BRDF
	return R * distribution->D(wh) * F * visterm->Vis_Term( NoL , NoV , VoH , NoH );
}

// sample a direction randomly
Spectrum MicroFacetReflection::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
	// sampling the normal
	const Vector wh = distribution->sample_f( bs , wo );

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

	const Vector h = Normalize( wo + wi );
	const float EoH = AbsDot( wo , h );
	const float HoN = AbsCosTheta(h);
	return distribution->D(h) * HoN / (4.0f * EoH);
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
    if( SameHemiSphere(wi, wo) )
        return Spectrum(0.f);
    
	const float NoL = AbsCosTheta( wi );
	const float NoV = AbsCosTheta( wo );
	if (NoL == 0.f || NoV == 0.f)
		return Spectrum(0.f);
	
	const float eta = CosTheta(wo) > 0 ? (eta_in / eta_ext) : (eta_ext / eta_in);

	const Vector3f wh = Normalize(wo + wi * eta);

	const float NoH = AbsCosTheta( wh );
	const float sVoH = Dot(wo , wh);
	const float VoH = fabs(sVoH);
    const float sIoH = Dot(wi, wh);
    const float IoH = fabs(sIoH);
	
	// Fresnel term
	const Spectrum F = fresnel->Evaluate( VoH , IoH );

	const float sqrtDenom = sVoH + eta * sIoH;
	const float t = eta / sqrtDenom;
	return (Spectrum(1.f) - F) * R * distribution->D(wh) * visterm->Vis_Term( NoL , NoV , VoH , NoH ) * t * t * IoH * VoH * 4.0f ;
}

// sample a direction using importance sampling
Spectrum MicroFacetRefraction::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
    if( CosTheta( wo ) == 0.0f )
        return 0.0f;
    
	// sampling the normal
	const Vector wh = distribution->sample_f( bs , wo );

	// try to get refracted ray
	bool total_reflection = false;
	wi = getRefracted( wo , wh , eta_in , eta_ext , total_reflection );
	if( total_reflection ) return 0.0f;

    if( pdf )
        *pdf = Pdf( wo , wi );
	return f( wo , wi );
}

// get the pdf of the sampled direction
float MicroFacetRefraction::Pdf( const Vector& wo , const Vector& wi ) const
{
	if( SameHemisphere( wo , wi ) )
        return 0.0f;

	const float eta = CosTheta(wo) > 0 ? (eta_in / eta_ext) : (eta_ext / eta_in);
    const Vector3f wh = Normalize(wo + wi * eta);

    // Compute change of variables _dwh\_dwi_ for microfacet transmission
    const float sqrtDenom = Dot(wo, wh) + eta * Dot(wi, wh);
    const float dwh_dwi = eta * eta * AbsDot(wi, wh) / (sqrtDenom * sqrtDenom);
	const float HoN = AbsCosTheta(wh);
    return distribution->D(wh) * HoN * dwh_dwi;
}
