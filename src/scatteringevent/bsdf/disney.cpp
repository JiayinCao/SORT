/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "disney.h"
#include "sampler/sample.h"
#include "core/samplemethod.h"
#include "core/sassert.h"
#include "lambert.h"
#include "microfacet.h"
#include "core/memory.h"
#include "math/exp.h"
#include "material/tsl_utils.h"

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeDisney)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, metallic)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, specular)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, specularTint)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, roughness)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, anisotropic)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, sheen)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, sheenTint)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, clearcoat)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, clearcoatGloss)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, specTrans)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float3, scatterDistance)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, flatness)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float, diffTrans)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_bool,  thinSurface)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float3, baseColor)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDisney, Tsl_float3, normal)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeDisney)

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeSSS)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeSSS, Tsl_float3, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeSSS, Tsl_float3, scatter_distance)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeSSS, Tsl_float3, normal)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeSSS)

constexpr static float ior_in = 1.5f;          // hard coded index of refraction below the surface
constexpr static float ior_ex = 1.0f;          // hard coded index of refraction above the surface
constexpr static float eta = ior_ex / ior_in;  // hard coded index of refraction ratio
constexpr static float inv_eta = 1.0f / eta;   // hard coded reciprocal of IOR ratio

// 8.0f bears no physical law, it is purely just to increase the probability to avoid fireflies.
// Microfacet specular reflection is easily the source of fireflies, scaling the value up will efficiently avoid fireflies caused by it.
SORT_STATIC_FORCEINLINE float specularPdfScale( const float roughness ){
    constexpr static float specular_pdf_scale = 8.0f;
    return specular_pdf_scale * ( 1.0f - roughness );
}

//! Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering, section 3.1
SORT_STATIC_FORCEINLINE float SchlickR0FromEta( float rROI ){
    return SQR( ( rROI - 1.0f ) / ( rROI + 1.0f ) );
}

constexpr float burley_max_cdf_calc( float max_r_d ){
    return 0.25f * ( 4.0f - exp_compile( -max_r_d ) - 3.0f * exp_compile( -max_r_d / 3.0f ) );
}
constexpr static float burley_max_r_d = 16.0f;
constexpr static float burley_max_cdf = burley_max_cdf_calc( burley_max_r_d );
constexpr static float burley_inv_max_cdf = 1.0f / burley_max_cdf;

float ClearcoatGGX::D(const Vector& h) const {
    // D(h) = ( alpha^2 - 1 ) / ( 2 * PI * ln(alpha) * ( 1 + ( alpha^2 - 1 ) * cos(\theta) ^ 2 )

    // This function should return INV_PI when alphaU equals to 1.0, which is not possible given the implementation of disney BRDF in SORT.
    sAssert(alphaU != 1.0f, MATERIAL);

    const auto cos = cosTheta(h);
    return (alphaU2 - 1) / (PI * log(alphaU2) * (1 + (alphaU2 - 1) * SQR(cos)));
}

Vector ClearcoatGGX::sample_f(const BsdfSample& bs) const {
    // phi = 2 * PI * u
    // theta = acos( sqrt( ( exp( 2 * ln(alpha) * v ) - 1 ) / ( alpha^2 - 1.0f ) ) )
    const auto phi = TWO_PI * bs.u;
    const auto theta = alphaU2 == 1.0f ? acos(sqrt(bs.v)) : acos(sqrt((exp(log(alphaU2) * bs.v) - 1.0f) / (alphaU2 - 1.0f)));
    return sphericalVec(theta, phi);
}

float ClearcoatGGX::G1(const Vector& v) const {
    if (absCosTheta(v) == 1.0f)
        return 0.0f;
    const auto tan_theta_sq = tanTheta2(v);
    constexpr auto alpha = 0.25f;
    constexpr auto alpha2 = alpha * alpha;
    return 1.0f / (1.0f + sqrt(1.0f + alpha2 * tan_theta_sq));
}

DisneyBssrdf::DisneyBssrdf(const SurfaceInteraction* intersection, const ClosureTypeSSS& params, const Spectrum& weight)
    :DisneyBssrdf(intersection, params.base_color, params.scatter_distance, weight) {
}

DisneyBssrdf::DisneyBssrdf( const SurfaceInteraction* intersection , const Spectrum& R , const Spectrum& mfp , const Spectrum& ew )
:DisneyBssrdf( intersection , R , mfp , ew , ew.GetIntensity() ){
}

DisneyBssrdf::DisneyBssrdf( const SurfaceInteraction* intersection , const Spectrum& R , const Spectrum& mfp , const Spectrum& ew , const float sw )
:SeparableBssrdf( R , intersection , ew , sw ){
    // Approximate Reflectance Profiles for Efficient Subsurface Scattering, Eq 6
    const auto s = Spectrum(1.9f) - R + 3.5f * ( R - Spectrum( 0.8f ) ) * ( R - Spectrum( 0.8f ) );
    
    channels = SPECTRUM_SAMPLE;

#ifdef SSS_REPLACE_WITH_LAMBERT
    constexpr float delta = 0.0001f;
    for( int i = 0 ; i < SPECTRUM_SAMPLE ; ++i ){
        if( mfp[i] == 0.0f )
            --channels;
    }
#endif

    // prevent the scatter distance to be zero, not a perfect solution, but it works.
    // the divide four pi thing is just to get similar result with Cycles SSS implementation with same inputs.
    const auto l = mfp * INV_FOUR_PI;
    d = l.Clamp( 0.0001f , FLT_MAX ) / s;
}

int DisneyBssrdf::Sample_Ch(RenderContext& rc) const{
    auto ch = clamp( (int)(sort_rand<float>(rc) * channels) , 0 , channels - 1 );
#ifdef SSS_REPLACE_WITH_LAMBERT
    // Here it is assumed that the spectrum is RGB only. 
    // This code needs change if SORT needs to support full spectrum rendering, which is not my plan for now.
    if( ch == 0 )
        ch = ( d[0] > 0.0f ) ? 0 : ( d[1] > 0.0f ? 1 : 2 );
    else if( ch == 1 )
        ch = ( d[1] > 0.0f ) ? 1 : 2;
#endif
    return ch;
}

Spectrum DisneyBssrdf::S( const Vector& wo , const Point& po , const Vector& wi , const Point& pi ) const{
    // This is clearly not disney BSSRDF, but it converges to lambert when mean free path is zero.
    // Converging to lambert when mfp approaches to zero is a critically important feature in SORT renderer,
    // this gives the flexibility of driving the parameter with a texture with black pixels and no noticeable
    // gap will be since around the transition from SSS to lambert.
    return Sr( distance( po , pi ) );
}

Spectrum DisneyBssrdf::Sr( float r ) const{
    r = ( r < 0.000001f ) ? 0.000001f : r;
    constexpr auto EIGHT_PI = 4.0 * TWO_PI;
    return R * ( ( Spectrum( -r ) / d ).Exp() + ( Spectrum( -r ) / ( 3.0f * d ) ).Exp() ) / ( EIGHT_PI * d * r );
}

float DisneyBssrdf::Sample_Sr(int ch, float r) const{
    constexpr auto quater_cutoff = 0.25f;

    // importance sampling burley profile
    const auto ret = ( r < quater_cutoff ) ? -d[ch] * log( 4.0f * r ) : -3.0f * d[ch] * log( ( r - quater_cutoff ) * 1.3333f );

    // ignore all samples outside the sampling range
    return ( ret > burley_max_r_d * d[ch] ) ? -1.0f : ret;
}

float DisneyBssrdf::Pdf_Sr(int ch, float r) const{
    // Sr(ch,r) = ( 0.25f * exp( -r / d[ch] ) / ( TWO_PI * d[ch] * r ) + 0.75f * exp( -r / ( 3.0f * d[ch] ) ) / ( SIX_PI * d[ch] * r )
    constexpr auto EIGHT_PI = 4.0f * TWO_PI;
    r = ( r < 0.000001f ) ? 0.000001f : r;
    return ( exp( -r / d[ch] ) + exp( -r / ( 3.0f * d[ch] ) ) ) / ( EIGHT_PI * d[ch] * r ) * burley_inv_max_cdf;
}

float DisneyBssrdf::Max_Sr(int ch) const{
    return burley_max_r_d * d[ch];
}

Spectrum DisneyBRDF::f( const Vector& wo , const Vector& wi ) const {
    const auto aspect = sqrt(sqrt(1.0f - anisotropic * 0.9f));
    const auto diffuseWeight = (1.0f - metallic) * (1.0 - specTrans);

    const auto wh = normalize(wo + wi);
    const auto HoO = dot(wo, wh);
    const auto HoO2ByRoughness = SQR(HoO) * roughness;

    const auto luminance = basecolor.GetIntensity();
    const auto Ctint = luminance > 0.0f ? basecolor * (1.0f / luminance) : Spectrum(1.0f);

    auto ret = RGBSpectrum(0.0f);

    const auto evaluate_reflection = PointingUp( wo ) && PointingUp( wi );

    if (diffuseWeight > 0.0f) {
        const auto NoO = cosTheta(wo);
        const auto NoI = cosTheta(wi);
        const auto Clampped_NoI = saturate(NoI);
        const auto FO = SchlickWeight(NoO);
        const auto FI = SchlickWeight(NoI);

        if (thinSurface) {
            if( evaluate_reflection ){
                if (flatness < 1.0f) {
                    // Diffuse
                    // Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering, eq (4)
                    const auto disneyDiffuse = basecolor * (INV_PI * (1.0 - FO * 0.5f) * (1.0 - FI * 0.5f));
                    ret += diffuseWeight * (1.0 - flatness) * (1.0f - diffTrans) * disneyDiffuse * Clampped_NoI;
                }
                if (flatness > 0.0f) {
                    // Fake sub-surface scattering
                    // Reflection from Layered Surfaces due to Subsurface Scattering
                    // https://cseweb.ucsd.edu/~ravir/6998/papers/p165-hanrahan.pdf
                    // Based on Hanrahan-Krueger BRDF approximation of isotropic BSSRDF
                    // 1.25 scale is used to (roughly) preserve albedo
                    // Fss90 used to "flatten" retro-reflection based on roughness
                    const auto Fss90 = HoO2ByRoughness;
                    const auto Fss = slerp(1.0f, Fss90, FO) * slerp(1.0f, Fss90, FI);
                    const auto disneyFakeSS = basecolor * (1.25f * (Fss * (1 / (NoO + NoI) - 0.5f) + 0.5f) * INV_PI);
                    ret += diffuseWeight * flatness * (1.0f - diffTrans) * disneyFakeSS * Clampped_NoI;
                }
            }
        } else {
            if (!scatterDistance.IsBlack()) {
                // Nothing needs to be done in this branch, it is intentional.
            } else if( evaluate_reflection ){
                // Fall back to the Disney diffuse due to the lack of sub-surface scattering
                const auto disneyDiffuse = basecolor * (INV_PI * (1.0 - FO * 0.5f) * (1.0 - FI * 0.5f));
                ret += diffuseWeight * disneyDiffuse * Clampped_NoI;
            }
        }

        if( evaluate_reflection ){
            // Retro-reflection
            // Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering, eq (4)
            const auto Rr = 2.0 * HoO2ByRoughness;
            const auto frr = basecolor * (INV_PI * Rr * (FO + FI + FO * FI * (Rr - 1.0f)));
            ret += diffuseWeight * frr * Clampped_NoI;

            // This is not totally physically correct. However, dielectric model presented by Walter et al. loses energy due to lack
            // of MicroFacet inter-reflection/refraction and the sheen component can approximately compensate for it.
            if (sheen > 0.0f) {
                const auto Csheen = slerp(Spectrum(1.0f), Ctint, sheenTint);
                const auto FH = SchlickWeight(HoO);
                const auto Fsheen = FH * sheen * Csheen;
                ret += diffuseWeight * Fsheen * Clampped_NoI;
            }
        }
    }

    // Specular reflection term in Disney BRDF
    const GGX ggx(roughness / aspect, roughness * aspect);
    const auto Cspec0 = slerp(specular * SchlickR0FromEta( ior_ex / ior_in ) * slerp(Spectrum(1.0f), Ctint, specularTint), basecolor, metallic);
    if (!Cspec0.IsBlack() && evaluate_reflection) {
        const FresnelSchlick<Spectrum> fresnel(Cspec0);
        const MicroFacetReflection mf(rc, WHITE_SPECTRUM, &fresnel, &ggx, FULL_WEIGHT, nn);
        ret += mf.f(wo, wi);
    }

    // Another layer of clear coat on top of everything below.
    if (clearcoat > 0.0f && evaluate_reflection) {
        const ClearcoatGGX cggx(sqrt(slerp(0.1f, 0.001f, clearcoatGloss)));
        const FresnelSchlick<float> fresnel(0.04f);
        const MicroFacetReflection mf_clearcoat(rc, WHITE_SPECTRUM, &fresnel, &cggx, FULL_WEIGHT, nn);
        ret += clearcoat * mf_clearcoat.f(wo, wi);
    }

    // Specular transmission
    if (specTrans > 0.0f) {
        if (thinSurface) {
            // Scale roughness based on IOR (Burley 2015, Figure 15).
            const auto rscaled = (0.65f * inv_eta - 0.35f) * roughness;
            const auto ru = SQR(rscaled) / aspect;
            const auto rv = SQR(rscaled) * aspect;
            const GGX scaledDist(ru, rv);

            MicroFacetRefraction mr(rc, basecolor.Sqrt(), &scaledDist, ior_ex, ior_in, FULL_WEIGHT, nn);
            ret += specTrans * (1.0f - metallic) * mr.f(wo, wi);
        } else {
            // Microfacet Models for Refraction through Rough Surfaces
            // https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
            MicroFacetRefraction mr(rc, basecolor, &ggx, ior_ex, ior_in, FULL_WEIGHT, nn);
            ret += specTrans * (1.0f - metallic) * mr.f(wo, wi);
        }
    }

    // Diffuse transmission
    if ( thinSurface && diffTrans > 0.0f && diffuseWeight > 0.0f ) {
        LambertTransmission lambert_transmission(rc, basecolor, 1.0f, nn);
        ret += diffTrans * diffuseWeight * lambert_transmission.f(wo, wi) ;
    }

    return ret;
}

Spectrum DisneyBRDF::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    const auto aspect = sqrt(sqrt(1.0f - anisotropic * 0.9f));
    const auto luminance = basecolor.GetIntensity();
    const auto Ctint = luminance > 0.0f ? basecolor * (1.0f / luminance) : Spectrum(1.0f);
    const auto min_specular_amount = SchlickR0FromEta(ior_ex / ior_in);
    const auto Cspec0 = slerp(specular * min_specular_amount * slerp(Spectrum(1.0f), Ctint, specularTint), basecolor, metallic);
    const auto hasSSS = !scatterDistance.IsBlack();

    const auto base_color_intensity = basecolor.GetIntensity();
    const auto clearcoat_weight = clearcoat * 0.04f;
    const auto specular_reflection_weight = Cspec0.GetIntensity() * specularPdfScale( roughness );
    const auto specular_transmission_weight = base_color_intensity * (1.0f - metallic) * specTrans;
    const auto diffuse_reflection_weight = hasSSS ? 0.0f : base_color_intensity * (1.0f - metallic) * (1.0f - specTrans) * (thinSurface ? (1.0f - diffTrans) : 1.0f);
    const auto diffuse_transmission_weight = thinSurface ? base_color_intensity * (1.0f - metallic) * (1.0f - specTrans) * diffTrans : 0.0f;

    const auto total_weight = clearcoat_weight + specular_reflection_weight + specular_transmission_weight + diffuse_reflection_weight + diffuse_transmission_weight;
    if (total_weight <= 0.0f) {
        if (pPdf)
            *pPdf = 0.0f;
        return 0.0f;
    }

    const auto inv_total_weight = 1.0f / total_weight;
    const auto cc_w = clearcoat_weight * inv_total_weight;
    const auto sr_w = specular_reflection_weight * inv_total_weight + cc_w;
    const auto st_w = specular_transmission_weight * inv_total_weight + sr_w;
    const auto dr_w = diffuse_reflection_weight * inv_total_weight + st_w;
    //const auto dt_w = diffuse_transmission_weight * inv_total_weight + dr_w;

    const GGX ggx(roughness / aspect, roughness * aspect);
    const auto r = sort_rand<float>(rc);
    if (r <= cc_w) {
        BsdfSample sample(rc);
        Vector wh;
        const ClearcoatGGX cggx(sqrt(slerp(0.1f, 0.001f, clearcoatGloss)));
        wh = cggx.sample_f(sample);
        wi = 2 * dot(wo, wh) * wh - wo;
    }else if (r <= sr_w) {
        BsdfSample sample(rc);
        Vector wh;
        wh = ggx.sample_f(sample);
        wi = 2 * dot(wo, wh) * wh - wo;
    }else if (r <= st_w) {
        if (thinSurface) {
            // Scale roughness based on IOR (Burley 2015, Figure 15).
            const auto rscaled = (0.65f * inv_eta - 0.35f) * roughness;
            const auto ru = SQR(rscaled) / aspect;
            const auto rv = SQR(rscaled) * aspect;
            const GGX scaledDist(ru, rv);
            MicroFacetRefraction mr(rc, WHITE_SPECTRUM, &scaledDist, ior_ex, ior_in, FULL_WEIGHT, nn);
            mr.sample_f(wo, wi, bs, pPdf);
        }
        else {
            // Sampling the transmission BTDF
            MicroFacetRefraction mr(rc, WHITE_SPECTRUM, &ggx, ior_ex, ior_in, FULL_WEIGHT, nn);
            mr.sample_f(wo, wi, bs, pPdf);
        }
    }else if (r <= dr_w) {
        // albedo doesn't matter here, we are only interested in light direction.
        if( UNLIKELY(hasSSS) ){
            // it is intentional to leave it empty
            sAssertMsg( false , MATERIAL , "Incorrect sampling in Disney BRDF, something is wrong." );
        }else{
            wi = CosSampleHemisphere(sort_rand<float>(rc), sort_rand<float>(rc));
        }
    }else // if( r <= dt_w )
    {
        LambertTransmission lambert_transmission(rc, basecolor, diffTrans, nn);
        lambert_transmission.sample_f(wo, wi, bs, pPdf);
    }

    if (pPdf) *pPdf = pdf(wo, wi);

    auto ret = f( wo , wi );
    return ret;
}

float DisneyBRDF::pdf( const Vector& wo , const Vector& wi ) const {
    const auto aspect = sqrt(sqrt(1.0f - anisotropic * 0.9f));
    const auto luminance = basecolor.GetIntensity();
    const auto Ctint = luminance > 0.0f ? basecolor * (1.0f / luminance) : Spectrum(1.0f);
    const auto min_specular_amount = SchlickR0FromEta(ior_ex / ior_in);
    const auto Cspec0 = slerp(specular * min_specular_amount * slerp(Spectrum(1.0f), Ctint, specularTint), basecolor, metallic);
    const auto hasSSS = !scatterDistance.IsBlack();

    const auto base_color_intensity = basecolor.GetIntensity();
    const auto clearcoat_weight = clearcoat * 0.04f;
    const auto specular_reflection_weight = Cspec0.GetIntensity() * specularPdfScale( roughness );
    const auto specular_transmission_weight = base_color_intensity * (1.0f - metallic) * specTrans;
    const auto diffuse_reflection_weight = hasSSS ? 0.0f : base_color_intensity * (1.0f - metallic) * (1.0f - specTrans) * (thinSurface ? (1.0f - diffTrans) : 1.0f);
    const auto diffuse_transmission_weight = thinSurface ? base_color_intensity * (1.0f - metallic) * (1.0f - specTrans) * diffTrans : 0.0f;
    
    const auto total_weight = clearcoat_weight + specular_reflection_weight + specular_transmission_weight + diffuse_reflection_weight + diffuse_transmission_weight;
    if (total_weight <= 0.0f)
        return 0.0f;

    auto total_pdf = 0.0f;
    const auto wh = normalize(wi + wo);
    const GGX ggx(roughness / aspect, roughness * aspect);
    if (clearcoat_weight > 0.0f) {
        const ClearcoatGGX cggx(sqrt(slerp(0.1f, 0.001f, clearcoatGloss)));
        total_pdf += clearcoat_weight * cggx.Pdf(wh) / (4.0f * absDot(wo, wh));
    }
    if (specular_reflection_weight > 0.0f) {
        total_pdf += specular_reflection_weight * ggx.Pdf(wh) / (4.0f * absDot(wo, wh));
    }
    if (specular_transmission_weight > 0.0f) {
        if (thinSurface) {
            // Scale roughness based on IOR (Burley 2015, Figure 15).
            const auto rscaled = (0.65f * inv_eta - 0.35f) * roughness;
            const auto ru = SQR(rscaled) / aspect;
            const auto rv = SQR(rscaled) * aspect;
            const GGX scaledDist(ru, rv);

            MicroFacetRefraction mr(rc, WHITE_SPECTRUM, &scaledDist, ior_ex, ior_in, FULL_WEIGHT, nn);
            total_pdf += specular_transmission_weight * mr.pdf(wo, wi);
        }
        else {
            // Sampling the transmission BTDF
            MicroFacetRefraction mr(rc, WHITE_SPECTRUM, &ggx, ior_ex, ior_in, FULL_WEIGHT, nn);
            total_pdf += specular_transmission_weight * mr.pdf(wo, wi);
        }
    }
    if (diffuse_reflection_weight > 0.0f) {
        // albedo doesn't matter here, we are only interested in light direction.
        if( hasSSS ){
            // No PDF should be returned, otherwise it will introduce bugs!!
        }else{
            total_pdf += diffuse_reflection_weight * CosHemispherePdf(wi);
        }
    }
    if (diffuse_transmission_weight > 0.0f) {
        LambertTransmission lambert_transmission(rc, basecolor, diffTrans, nn);
        total_pdf += diffuse_transmission_weight * lambert_transmission.pdf(wo, wi);
    }

    return total_pdf / total_weight;
}

 float DisneyBRDF::Evaluate_Sampling_Weight( const ClosureTypeDisney& params ){
     const auto hasSSS = !(params.scatterDistance.x == 0.0f && params.scatterDistance.y == 0.0f && params.scatterDistance.z == 0.0f);

     // If there is no SSS, there will be 100% chance that a BXDF will be chosen.
     if( !hasSSS )
         return 1.0f;

     static const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
     const auto luminance = YWeight[0] * params.baseColor.x + YWeight[1] * params.baseColor.y + YWeight[2] * params.baseColor.z;
     const auto Ctint = luminance > 0.0f ? params.baseColor * (1.0f / luminance) : Spectrum(1.0f);
     const auto min_specular_amount = SchlickR0FromEta(ior_ex / ior_in);
     const auto Cspec0 = slerp(params.specular * min_specular_amount * slerp(Spectrum(1.0f), Ctint, params.specularTint), params.baseColor, params.metallic);

     const auto clearcoat_weight = params.clearcoat * 0.04f;
     const auto specular_reflection_weight = Cspec0.GetIntensity() * specularPdfScale( params.roughness );
     const auto specular_transmission_weight = luminance * (1.0f - params.metallic) * params.specTrans;
     const auto diffuse_reflection_weight = luminance * (1.0f - params.metallic) * (1.0f - params.specTrans) * (params.thinSurface ? (1.0f - params.diffTrans) : 1.0f);
     const auto diffuse_transmission_weight = params.thinSurface ? luminance * (1.0f - params.metallic) * (1.0f - params.specTrans) * params.diffTrans : 0.0f;
    
     const auto total_weight = clearcoat_weight + specular_reflection_weight + specular_transmission_weight + diffuse_reflection_weight + diffuse_transmission_weight;
     if (total_weight == 0.0f)
         return 0.0f;

     return 1.0f - diffuse_reflection_weight / total_weight;
 }
