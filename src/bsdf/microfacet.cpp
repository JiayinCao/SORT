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
    const static auto convert_alpha = [](float roughness) {
        roughness = std::max(0.01f, roughness);
        return pow( roughness , 4.0f );
    };
    const static auto convert_exp = [](float roughness) {
        return 2.0f / convert_alpha(roughness) - 2.0f;
    };
    
    expU = convert_exp(roughnessU);
    expV = convert_exp(roughnessV);
    expUV = sqrt( ( expU + 2.0f ) * ( expV + 2.0f ) );
    exp = sqrt( ( expU + 2.0f ) / ( expV + 2.0f ) );
    alphaU2 = convert_alpha(roughnessU);
    alphaV2 = convert_alpha(roughnessV);
}

// probability of facet with specific normal (h)
float Blinn::D(const Vector& h) const
{
    // An Anisotropic Phong BRDF Model (Micheal Ashikhmin, Peter Shirley)
    // http://www.irisa.fr/prive/kadi/Lopez/ashikhmin00anisotropic.pdf ( The original phong model was not energy conservative, see the following link for a fix. )
    // http://simonstechblog.blogspot.com/2011/12/microfacet-brdf.html ( Modified Phong model )
    // http://www.farbrausch.de/~fg/stuff/phong.pdf ( Derivation of phong model's scaling factor )
    // Anisotropic model:   D(w_h) = sqrt( ( exponentU + 2 ) * ( exponentV + 2 ) ) * cos(\theta) ^ ( cos(\phi)^2 * alphaU + sin(\phi)^2 * alphaV ) / ( 2 * PI )
    // Isotropic model:     D(w_h) = ( exponent + 2 ) * cos(\theta) ^ alpha / ( 2 * PI )

    float NoH = AbsCosTheta(h);
    if (NoH <= 0.0f) return 0.0f;
    const float sin_phi_h_sq = SinPhi2(h);
    const float cos_phi_h_sq = 1.0f - sin_phi_h_sq;
    return expUV * pow(NoH, cos_phi_h_sq * expU + sin_phi_h_sq * expV) * INV_TWOPI;
}

// sampling according to Blinn
Vector Blinn::sample_f( const BsdfSample& bs ) const
{
    float phi = 0.0f;
    if (expU == expV) {
        // Refer the following link ( my blog ) for a full derivation of isotropic importance sampling for Blinn
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        phi = TWO_PI * bs.v;
    }else{
        // Refer the following link ( my blog ) for a full derivation of anisotropic importance sampling for Blinn
        // https://agraphicsguy.wordpress.com/2018/07/18/sampling-anisotropic-microfacet-brdf/
        const static int offset[5] = { 0 , 1 , 1 , 2 , 2 };
        const int i = bs.v == 0.25f ? 0 : (int)(bs.v * 4.0f);
        phi = std::atan( exp * std::tan(TWO_PI * bs.v) ) + offset[i] * PI;
    }

    const float sin_phi_h = std::sin(phi);
    const float sin_phi_h_sq = sin_phi_h * sin_phi_h;
    const float alpha = expU * (1.0f - sin_phi_h_sq) + expV * sin_phi_h_sq;
    const float cos_theta = std::pow(bs.u, 1.0f / (alpha + 2.0f));
    const float sin_theta = sqrtf(std::max(0.0f, 1.0f - cos_theta * cos_theta));

    return SphericalVec(sin_theta, cos_theta, phi);
}

//! @brief Smith shadow-masking function G1
float Blinn::G1( const Vector& v ) const
{
    const float absTan = fabs( TanTheta(v) );
    if( isinf( absTan ) ) return 0.0f;
    const float cos_phi_sq = CosPhi2(v);
    const float a = 1.0f / ( sqrt( cos_phi_sq * alphaU2 + ( 1.0f - cos_phi_sq ) * alphaV2 ) * absTan );
    if( a > 1.6f || isinf( a ) ) return 1.0f;
    return ( 3.535f * a + 2.181f * a * a ) / ( 1.0f + 2.276f * a + 2.577f * a * a );
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
    alpha = alphaV / alphaU;
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

// sampling according to Beckmann
Vector Beckmann::sample_f( const BsdfSample& bs ) const
{
    const float logSample = std::log( bs.u );
    
    float theta, phi;
    if( alphaU == alphaV ){
        // Refer the following link ( my blog ) for a full derivation of isotropic importance sampling for Beckmann
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        theta = atan( sqrt( -1.0f * alphaUV * logSample ) );
        phi = TWO_PI * bs.v;
    }else{
        // Refer the following link ( my blog ) for a full derivation of anisotropic importance sampling for Beckmann
        // https://agraphicsguy.wordpress.com/2018/07/18/sampling-anisotropic-microfacet-brdf/
        const static int offset[5] = { 0 , 1 , 1 , 2 , 2 };
        const int i = bs.v == 0.25f ? 0 : (int)(bs.v * 4.0f);
        phi = std::atan( alpha * std::tan(TWO_PI * bs.v)) + offset[i] * PI;
        const float sin_phi = std::sin(phi);
        const float sin_phi_sq = sin_phi * sin_phi;
        theta = atan(sqrt(-logSample / ((1.0f - sin_phi_sq) / alphaU2 + sin_phi_sq / alphaV2)));
    }
    
    return SphericalVec(theta, phi);
}

//! @brief Smith shadow-masking function G1
float Beckmann::G1( const Vector& v ) const
{
    const float absTan = fabs( TanTheta(v) );
    if( isinf( absTan ) ) return 0.0f;
    const float cos_phi_sq = CosPhi2(v);
    const float a = 1.0f / ( sqrt( cos_phi_sq * alphaU2 + ( 1.0f - cos_phi_sq ) * alphaV2 ) * absTan );
    if( a > 1.6f || isinf( a ) ) return 1.0f;
    return ( 3.535f * a + 2.181f * a * a ) / ( 1.0f + 2.276f * a + 2.577f * a * a );
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
    alpha = alphaV / alphaU;
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

Vector GGX::sample_f( const BsdfSample& bs ) const
{
    float theta, phi;
    if( alphaU == alphaV ){
        // Refer the following link ( my blog ) for a full derivation of isotropic importance sampling for GGX
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        theta = atan( alphaU * sqrt( bs.v / ( 1.0f - bs.v )) );
        phi = TWO_PI * bs.u;
    }else{
        // Refer the following link ( my blog ) for a full derivation of anisotropic importance sampling for GGX
        // https://agraphicsguy.wordpress.com/2018/07/18/sampling-anisotropic-microfacet-brdf/
        const static int offset[5] = { 0 , 1 , 1 , 2 , 2 };
        const int i = bs.v == 0.25f ? 0 : (int)(bs.v * 4.0f);
        phi = std::atan( alpha * std::tan(TWO_PI * bs.v)) + offset[i] * PI;
        const float sin_phi = std::sin(phi);
        const float sin_phi_sq = sin_phi * sin_phi;
        const float cos_phi_sq = 1.0f - sin_phi_sq;
        float beta = 1.0f / ( cos_phi_sq / alphaU2 + sin_phi_sq / alphaV2 );
        theta = atan( sqrt( beta * bs.u / ( 1.0f - bs.u ) ) );
    }
    return SphericalVec(theta, phi);
}

//! @brief Smith shadow-masking function G1
float GGX::G1( const Vector& v ) const
{
    const float tan_theta_sq = TanTheta2(v);
    if( isinf( tan_theta_sq ) ) return 0.0f;
    const float cos_phi_sq = CosPhi2(v);
    const float alpha2 = cos_phi_sq * alphaU2 + ( 1.0f - cos_phi_sq ) * alphaV2;
    return 2.0f / ( 1.0f + sqrt( 1.0f + alpha2 * tan_theta_sq ) );
}

// evaluate bxdf
Spectrum MicroFacetReflection::f( const Vector& wo , const Vector& wi ) const
{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

	const float NoV = AbsCosTheta( wo );
	if (NoV == 0.f)
		return Spectrum(0.f);
	
	// evaluate fresnel term
	const Vector wh = Normalize(wi + wo);
	const Spectrum F = fresnel->Evaluate( Dot(wo,wh) );
    return R * distribution->D(wh) * F * distribution->G(wo,wi) / ( 4.0f * NoV );
}

// sample a direction randomly
Spectrum MicroFacetReflection::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const
{
	// sampling the normal
	const Vector wh = distribution->sample_f( bs );

	// reflect the incident direction
	wi = reflect( wo , wh );

    if (pPdf) *pPdf = pdf(wo, wi);

    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

	return f( wo , wi );
}

// get the pdf of the sampled direction
float MicroFacetReflection::pdf( const Vector& wo , const Vector& wi ) const
{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

	const Vector h = Normalize( wo + wi );
	const float EoH = AbsDot( wo , h );
	return distribution->Pdf(h) / (4.0f * EoH);
}

// evaluate bxdf
Spectrum MicroFacetRefraction::f( const Vector& wo , const Vector& wi ) const
{
    if( SameHemiSphere(wi, wo) )
        return Spectrum(0.f);
    
	const float NoV = CosTheta( wo );
	if (NoV == 0.f)
		return Spectrum(0.f);
	
	const float eta = CosTheta(wo) > 0 ? (etaT / etaI) : (etaI / etaT);

    Vector3f wh = Normalize(wo + wi * eta);
    if( wh.y < 0.0f ) wh = -wh;

	const float sVoH = Dot(wo, wh);
    const float sIoH = Dot(wi, wh);
	
	// Fresnel term
	const Spectrum F = fresnel.Evaluate( Dot( wh , wo ) );
	const float sqrtDenom = sVoH + eta * sIoH;
	const float t = eta / sqrtDenom;
    return (Spectrum(1.f) - F) * T * fabs(distribution->D(wh) * distribution->G(wo,wi) * t * t * sIoH * sVoH / NoV );
}

// sample a direction using importance sampling
Spectrum MicroFacetRefraction::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const
{
    if( CosTheta( wo ) == 0.0f )
        return 0.0f;
    
	// sampling the normal
	const Vector wh = distribution->sample_f( bs );

	// try to get refracted ray
	bool total_reflection = false;
	wi = refract( wo , wh , etaT , etaI , total_reflection );
	if( total_reflection ) return 0.0f;

    if( pPdf ) *pPdf = pdf( wo , wi );
	return f( wo , wi );
}

// get the pdf of the sampled direction
float MicroFacetRefraction::pdf( const Vector& wo , const Vector& wi ) const
{
	if( SameHemisphere( wo , wi ) )
        return 0.0f;

	const float eta = CosTheta(wo) > 0 ? (etaT / etaI) : (etaI / etaT);
    const Vector3f wh = Normalize(wo + wi * eta);

    // Compute change of variables _dwh\_dwi_ for microfacet transmission
    const float sqrtDenom = Dot(wo, wh) + eta * Dot(wi, wh);
    const float dwh_dwi = eta * eta * AbsDot(wi, wh) / (sqrtDenom * sqrtDenom);
    return distribution->Pdf(wh) * dwh_dwi;
}
