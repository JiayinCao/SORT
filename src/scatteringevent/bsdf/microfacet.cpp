/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include <cmath>
#include "microfacet.h"
#include "sampler/sample.h"
#include "math/utils.h"
#include "core/memory.h"
#include "scatteringevent/bsdf/fresnel.h"

Blinn::Blinn( float roughnessU , float roughnessV ) {
    // UE4 style way to convert roughness to alpha used here because it still keeps sharp reflection with low value of roughness
    // http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
    // PBRT way of converting roughness will result slightly blurred reflection even with 0 as roughness.
    // This is tackled in PBRT because it has a separate way to do perfect reflection, which is not available in SORT.
    const static auto convert_alpha = [](float roughness) {
        roughness = std::max(0.01f, roughness);
        return Pow<4>( roughness );
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

float Blinn::D(const Vector& h) const {
    // An Anisotropic Phong BRDF Model (Micheal Ashikhmin, Peter Shirley)
    // http://www.irisa.fr/prive/kadi/Lopez/ashikhmin00anisotropic.pdf ( The original phong model was not energy conservative, see the following link for a fix. )
    // http://simonstechblog.blogspot.com/2011/12/microfacet-brdf.html ( Modified Phong model )
    // http://www.farbrausch.de/~fg/stuff/phong.pdf ( Derivation of phong model's scaling factor )
    // Anisotropic model:   D(w_h) = sqrt( ( exponentU + 2 ) * ( exponentV + 2 ) ) * cos(\theta) ^ ( cos(\phi)^2 * alphaU + sin(\phi)^2 * alphaV ) / ( 2 * PI )
    // Isotropic model:     D(w_h) = ( exponent + 2 ) * cos(\theta) ^ alpha / ( 2 * PI )

    const auto NoH = absCosTheta(h);
    if (NoH <= 0.0f) return 0.0f;
    const auto sin_phi_h_sq = sinPhi2(h);
    const auto cos_phi_h_sq = 1.0f - sin_phi_h_sq;
    return expUV * pow(NoH, cos_phi_h_sq * expU + sin_phi_h_sq * expV) * INV_TWOPI;
}

Vector Blinn::sample_f( const BsdfSample& bs ) const {
    auto phi = 0.0f;
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

    const auto sin_phi_h = std::sin(phi);
    const auto sin_phi_h_sq = sin_phi_h * sin_phi_h;
    const auto alpha = expU * (1.0f - sin_phi_h_sq) + expV * sin_phi_h_sq;
    const auto cos_theta = std::pow(bs.u, 1.0f / (alpha + 2.0f));
    const auto sin_theta = sqrt( 1.0f - SQR( cos_theta ) ) ;

    return sphericalVec(sin_theta, cos_theta, phi);
}

float Blinn::G1( const Vector& v ) const {
    const auto absTan = fabs( tanTheta(v) );
    if( IsInf( absTan ) ) return 0.0f;
    const auto cos_phi_sq = cosPhi2(v);
    const auto a = 1.0f / ( sqrt( cos_phi_sq * alphaU2 + ( 1.0f - cos_phi_sq ) * alphaV2 ) * absTan );
    if( a > 1.6f || IsInf( a ) ) return 1.0f;
    return ( 3.535f * a + 2.181f * a * a ) / ( 1.0f + 2.276f * a + 2.577f * a * a );
}

Beckmann::Beckmann( float roughnessU , float roughnessV ) {
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

float Beckmann::D(const Vector& h) const {
    // Anisotropic Beckmann distribution formula, pbrt-v3 ( page 539 )
    // Anisotropic model:   D(w_h) = pow( e , -tan(\theta_h)^2 * ( (cos(\phi_h) / alphaU)^2 + (sin(\phi_h) / alphaV)^2 ) ) / ( PI * alphaU * alphaV * cos(\theta_h) ^ 4
    // Isotropic model:     D(w_h) = pow( e , -(tan(\theta_h)/alpha)^2 ) / ( PI * alpha^2 * cos(\theta_h)^4 )
    const auto cos_theta_h_sq = cosTheta2(h);
    if( cos_theta_h_sq <= 0.0f ) return 0.f;
    return exp( ( SQR( h.x ) / alphaU2 + SQR( h.z ) / alphaV2 ) / (-cos_theta_h_sq) ) / ( PI * alphaUV * SQR( cos_theta_h_sq ) );
}

Vector Beckmann::sample_f( const BsdfSample& bs ) const {
    const auto logSample = std::log( bs.u );

    float theta, phi;
    if( alphaU == alphaV ){
        // Refer the following link ( my blog ) for a full derivation of isotropic importance sampling for Beckmann
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        theta = atan( sqrt( -1.0f * alphaUV * logSample ) );
        phi = TWO_PI * bs.v;
    }else{
        // Refer the following link ( my blog ) for a full derivation of anisotropic importance sampling for Beckmann
        // https://agraphicsguy.wordpress.com/2018/07/18/sampling-anisotropic-microfacet-brdf/
        static const int offset[5] = { 0 , 1 , 1 , 2 , 2 };
        const auto i = bs.v == 0.25f ? 0 : (int)(bs.v * 4.0f);
        phi = std::atan( alpha * std::tan(TWO_PI * bs.v)) + offset[i] * PI;
        const auto sin_phi = std::sin(phi);
        const auto sin_phi_sq = sin_phi * sin_phi;
        theta = atan(sqrt(-logSample / ((1.0f - sin_phi_sq) / alphaU2 + sin_phi_sq / alphaV2)));
    }

    return sphericalVec(theta, phi);
}

float Beckmann::G1( const Vector& v ) const {
    const auto absTan = fabs( tanTheta(v) );
    if( IsInf( absTan ) ) return 0.0f;
    const auto cos_phi_sq = cosPhi2(v);
    const auto a = 1.0f / ( sqrt( cos_phi_sq * alphaU2 + ( 1.0f - cos_phi_sq ) * alphaV2 ) * absTan );
    if( a > 1.6f || IsInf( a ) ) return 1.0f;
    return ( 3.535f * a + 2.181f * a * a ) / ( 1.0f + 2.276f * a + 2.577f * a * a );
}

GGX::GGX( float roughnessU , float roughnessV ) {
    // UE4 style way to convert roughness to alpha used here because it still keeps sharp reflection with low value of roughness
    // http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
    // PBRT way of converting roughness will result slightly blurred reflection even with 0 as roughness.
    // This is tackled in PBRT because it has a separate way to do perfect reflection, which is not available in SORT.
    const static auto convert = []( float roughness ) {
        roughness = std::max(roughness, (float)1e-3);
        return SQR(roughness);
    };
    alphaU = convert( roughnessU );
    alphaV = convert( roughnessV );
    alphaU2 = alphaU * alphaU;
    alphaV2 = alphaV * alphaV;
    alphaUV = alphaU * alphaV;
    alpha = alphaV / alphaU;
}

float GGX::D(const Vector& h) const {
    // Anisotropic GGX (Trowbridge-Reitz) distribution formula, pbrt-v3 ( page 539 )
    // Anisotropic model:   D(w_h) = 1.0f / ( PI * alphaU * alphaV * cos(\theta)^4 * ( 1 + tan(\thete)^2 * ( ( cos(\theta) / alphaU ) ^ 2 + ( sin(\theta) / alphaV ) ^ 2 ) ) ^ 2 )
    // Isotrocpic model:    D(w_h) = alpha ^ 2 / ( PI * ( 1 + ( alpha ^ 2 - 1 ) * cos(\theta) ^ 2 ) ^ 2
    const auto cos_theta_h_sq = cosTheta2(h);
    if( cos_theta_h_sq <= 0.0f ) return 0.f;
    const auto beta = ( cos_theta_h_sq + ( SQR( h.x ) / alphaU2 + SQR( h.z ) / alphaV2));
    return 1.0f / ( PI * alphaUV * beta * beta );
}

Vector GGX::sample_f( const BsdfSample& bs ) const {
    float theta, phi;
    if( alphaU == alphaV ){
        // Refer the following link ( my blog ) for a full derivation of isotropic importance sampling for GGX
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        theta = atan( alphaU * sqrt( bs.v / ( 1.0f - bs.v )) );
        phi = TWO_PI * bs.u;
    }else{
        // Refer the following link ( my blog ) for a full derivation of anisotropic importance sampling for GGX
        // https://agraphicsguy.wordpress.com/2018/07/18/sampling-anisotropic-microfacet-brdf/
        static const int offset[5] = { 0 , 1 , 1 , 2 , 2 };
        const auto i = bs.v == 0.25f ? 0 : (int)(bs.v * 4.0f);
        phi = std::atan( alpha * std::tan(TWO_PI * bs.v)) + offset[i] * PI;
        const auto sin_phi = std::sin(phi);
        const auto sin_phi_sq = sin_phi * sin_phi;
        const auto cos_phi_sq = 1.0f - sin_phi_sq;
        float beta = 1.0f / ( cos_phi_sq / alphaU2 + sin_phi_sq / alphaV2 );
        theta = atan( sqrt( beta * bs.u / ( 1.0f - bs.u ) ) );
    }
    return sphericalVec(theta, phi);
}

float GGX::G1( const Vector& v ) const {
    const auto tan_theta_sq = tanTheta2(v);
    if( IsInf( tan_theta_sq ) ) return 0.0f;
    const auto cos_phi_sq = cosPhi2(v);
    const auto alpha2 = cos_phi_sq * alphaU2 + ( 1.0f - cos_phi_sq ) * alphaV2;
    return 2.0f / ( 1.0f + sqrt( 1.0f + alpha2 * tan_theta_sq ) );
}

Microfacet::Microfacet(const std::string& distType, float ru , float rv , const Spectrum& w, const BXDF_TYPE t , const Vector& n , bool doubleSided ) :
    Bxdf(w, t, n, doubleSided ) {
    if( distType == "GGX" )
        distribution = SORT_MALLOC(GGX)( ru , rv );
    else if( distType == "Beckmann" )
        distribution = SORT_MALLOC(Beckmann)( ru , rv );
    else if( distType == "Blinn" )
        distribution = SORT_MALLOC(Blinn)( ru , rv );
}

// MicroFacetReflection::MicroFacetReflection(const Params &params, const Spectrum& weight , bool doubleSided):
// Microfacet( params.dist.c_str() , params.roughnessU , params.roughnessV , weight , (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), params.n, doubleSided) ,
// R(params.baseColor), fresnel(SORT_MALLOC( FresnelConductor )(params.eta, params.absorption)){
// }

// MicroFacetReflection::MicroFacetReflection(const ParamsDieletric &params, const Spectrum& weight , bool doubleSided):
// Microfacet( params.dist.c_str() , params.roughnessU , params.roughnessV , weight , (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), params.n, doubleSided) ,
// R(params.baseColor), fresnel(SORT_MALLOC( FresnelDielectric )(params.iorI, params.iorT)){
// }

// MicroFacetReflection::MicroFacetReflection(const MirrorParams &params, const Spectrum& weight ):
// Microfacet( "GGX" , 0.0f , 0.0f , weight , (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), params.n, false) , R(params.baseColor), fresnel(SORT_MALLOC(FresnelNo)()){
// }

Spectrum MicroFacetReflection::f( const Vector& wo , const Vector& wi ) const {
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto NoV = absCosTheta( wo );
    if (NoV == 0.f)
        return Spectrum(0.f);

    // evaluate fresnel term
    const auto wh = normalize(wi + wo);
    const auto F = fresnel->Evaluate( dot(wo,wh) );
    return R * distribution->D(wh) * F * distribution->G(wo,wi) / ( 4.0f * NoV );
}

Spectrum MicroFacetReflection::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const {
    // sampling the normal
    const auto wh = distribution->sample_f( bs );

    // reflect the incident direction
    wi = reflect( wo , wh );

    if (pPdf) *pPdf = pdf(wo, wi);

    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    return f( wo , wi );
}

float MicroFacetReflection::pdf( const Vector& wo , const Vector& wi ) const {
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto h = normalize( wo + wi );
    const auto EoH = absDot( wo , h );
    return distribution->Pdf(h) / (4.0f * EoH);
}

// MicroFacetRefraction::MicroFacetRefraction(const Params &params, const Spectrum& weight):
// Microfacet( params.dist.c_str() , params.roughnessU , params.roughnessV , weight , (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), params.n, true),
// T(params.transmittance), etaI(params.etaI) , etaT(params.etaT) , fresnel( params.etaI , params.etaT ) {
// }

Spectrum MicroFacetRefraction::f( const Vector& wo , const Vector& wi ) const {
    if( SameHemiSphere(wi, wo) )
        return Spectrum(0.f);

    const auto NoV = cosTheta( wo );
    if (NoV == 0.f)
        return Spectrum(0.f);

    const auto eta = cosTheta(wo) > 0 ? (etaT / etaI) : (etaI / etaT);

    Vector3f wh = normalize(wo + wi * eta);
    if( wh.y < 0.0f ) wh = -wh;

    const auto sVoH = dot(wo, wh);
    const auto sIoH = dot(wi, wh);

    // Fresnel term
    const auto F = fresnel.Evaluate( dot( wh , wo ) );
    const auto sqrtDenom = sVoH + eta * sIoH;
    const auto t = eta / sqrtDenom;
    return (Spectrum(1.f) - F) * T * fabs(distribution->D(wh) * distribution->G(wo,wi) * t * t * sIoH * sVoH / NoV );
}

Spectrum MicroFacetRefraction::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const {
    if( cosTheta( wo ) == 0.0f )
        return 0.0f;

    // sampling the normal
    const auto wh = distribution->sample_f( bs );

    // try to get refracted ray
    auto total_reflection = false;
    wi = refract( wo , wh , etaT , etaI , total_reflection );
    if( total_reflection ) return 0.0f;

    if( pPdf ) *pPdf = pdf( wo , wi );
    return f( wo , wi );
}

float MicroFacetRefraction::pdf( const Vector& wo , const Vector& wi ) const {
    if( sameHemisphere( wo , wi ) )
        return 0.0f;

    const auto eta = cosTheta(wo) > 0 ? (etaT / etaI) : (etaI / etaT);
    const auto wh = normalize(wo + wi * eta);

    // Compute change of variables _dwh\_dwi_ for microfacet transmission
    const auto sqrtDenom = dot(wo, wh) + eta * dot(wi, wh);
    const auto dwh_dwi = eta * eta * absDot(wi, wh) / (sqrtDenom * sqrtDenom);
    return distribution->Pdf(wh) * dwh_dwi;
}
