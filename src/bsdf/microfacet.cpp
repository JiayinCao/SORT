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
Blinn::Blinn( float roughness )
{
    exp = max( 0.01f , roughness );
	exp = 2.0f / pow( exp , 4.0f ) - 2.0f;
}

// probability of facet with specific normal (h)
float Blinn::D(const Vector& h) const
{
    float NoH = AbsCosTheta(h);
	return (exp+2.0f) * INV_TWOPI * powf( NoH , exp );
}

// sampling according to GGX
Vector Blinn::sample_f( const BsdfSample& bs , const Vector& wo ) const
{
	const float costheta = powf( bs.u , 1.0f / (exp+2.0f) );
    const float sintheta = sqrtf( max( 0.0f , 1.0f - costheta * costheta ) );
    const float phi = TWO_PI * bs.v;

	return SphericalVec( sintheta , costheta , phi );
}

Beckmann::Beckmann( float roughnessU , float roughnessV )
{
    const static auto convert = []( float roughness ) {
        roughness = std::max(roughness, (float)1e-3);
        const float x = std::log(roughness);
        return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
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
    // Anisotropic Beckmann distribution formular, pbrt-v3 ( page 539 )
    // D(w_h) = pow( e , -tan(\theta_h)^2 * ( (cos(\phi_h) / alphaU)^2 + (sin(\phi_h) / alphaV)^2 ) ) / ( PI * alphaU * alphaV * cos(\theta_h) ^ 4
    // When alphaU equals to alphaV , it is isotropic, the formular is simpler
    // D(w_h) = pow( e , -(tan(\theta_h)/alpha)^2 ) / ( PI * alpha^2 * cos(\theta_h)^4 )
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
        // Refer the following link ( my blog ) for a full derivation
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        theta = atan( sqrt( -1.0f * alphaUV * logSample ) );
        phi = TWO_PI * bs.v;
    }else {
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
    const static auto convert = []( float roughness ) {
        roughness = std::max(roughness, (float)1e-3);
        float x = std::log(roughness);
        return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
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
    // Anisotropic GGX (Trowbridge-Reitz) distribution formular, pbrt-v3 ( page 539 )
    // D(w_h) = 1.0f / ( PI * alphaU * alphaV * cos(\theta)^4 * ( 1 + tan(\thete)^2 * ( ( cos(\theta) / alphaU ) ^ 2 + ( sin(\theta) / alphaV ) ^ 2 ) ) ^ 2 )
    // When alphaU equals to alphaV, it is isotropic, the formula is simpler
    // D(w_h) = alpha ^ 2 / ( PI * ( 1 + ( alpha ^ 2 - 1 ) * cos(\theta) ^ 2 ) ^ 2
    const float cos_theta_h_sq = CosTheta2(h);
    if( cos_theta_h_sq <= 0.0f ) return 0.f;
    const float tan_theta_h_sq = TanTheta2(h);
    
    const float sin_phi_h_sq = SinPhi2(h);
    const float cos_phi_h_sq = 1.0f - sin_phi_h_sq;
    return 1.0f / ( PI * alphaUV * cos_theta_h_sq * cos_theta_h_sq * ( 1 + tan_theta_h_sq * ( cos_phi_h_sq / alphaU2 + sin_phi_h_sq / alphaV2 ) ) );
}

Vector GGX::sample_f( const BsdfSample& bs , const Vector& wo ) const
{
    float theta, phi;
    if( alphaU == alphaV ){
        // Refer the following link ( my blog ) for a full derivation
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        theta = atan( alphaU * sqrt( bs.v / ( 1.0f - bs.v )) );
        phi = TWO_PI * bs.u;
    }else{
        phi = std::atan( alphaV / alphaU * std::tan( TWO_PI * bs.v ) );
        if( bs.u > 0.5f ) phi += PI;
        const float sin_phi = std::sin(phi);
        const float sin_phi_sq = sin_phi * sin_phi;
        const float cos_phi_sq = 1 - sin_phi_sq;
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
	const float eta = coso > 0 ? (ext_eta / in_eta) : (in_eta / ext_eta);
	const float t = 1.0f - eta * eta * max( 0.0f , 1.0f - coso * coso );

	// total inner reflection
    inner_reflection = (t <= 0.0f);
	if( inner_reflection )
		return Vector(0.0f,0.0f,0.0f);

	// get the tranmistance/refracted ray
	const float factor = (coso<0.0f)? 1.0f : -1.0f;
	return -eta * v  + ( eta * coso + factor * sqrt(t)) * n;
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
