/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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
#include "microfacet.h"
#include "sampler/sample.h"
#include "core/samplemethod.h"
#include "core/sassert.h"
#include "bsdf/lambert.h"

constexpr static float ior_in = 1.5f;          // hard coded index of refraction below the surface
constexpr static float ior_ex = 1.0f;          // hard coded index of refraction above the surface
constexpr static float eta = ior_ex / ior_in;  // hard coded index of refraction ratio
constexpr static float inv_eta = 1.0f / eta;   // hard coded reciprocal of IOR ratio

float ClearcoatGGX::D(const Vector& h) const {
    // D(h) = ( alpha^2 - 1 ) / ( 2 * PI * ln(alpha) * ( 1 + ( alpha^2 - 1 ) * cos(\theta) ^ 2 )

    // This function should return INV_PI when alphaU equals to 1.0, which is not possible given the implementation of disney BRDF in SORT.
    sAssert(alphaU != 1.0f, MATERIAL);

    const auto cos = CosTheta(h);
    return (alphaU2 - 1) / (PI * log(alphaU2) * (1 + (alphaU2 - 1) * SQR(cos)));
}

Vector ClearcoatGGX::sample_f(const BsdfSample& bs) const {
    // phi = 2 * PI * u
    // theta = acos( sqrt( ( exp( 2 * ln(alpha) * v ) - 1 ) / ( alpha^2 - 1.0f ) ) )
    const auto phi = TWO_PI * bs.u;
    const auto theta = alphaU2 == 1.0f ? acos(sqrt(bs.v)) : acos(sqrt((exp(log(alphaU2) * bs.v) - 1.0f) / (alphaU2 - 1.0f)));
    return SphericalVec(theta, phi);
}

float ClearcoatGGX::G1(const Vector& v) const {
    if (AbsCosTheta(v) == 1.0f)
        return 0.0f;
    const auto tan_theta_sq = TanTheta2(v);
    constexpr auto alpha = 0.25f;
    constexpr auto alpha2 = alpha * alpha;
    return 1.0f / (1.0f + sqrt(1.0f + alpha2 * tan_theta_sq));
}

Spectrum DisneyBRDF::f( const Vector& wo , const Vector& wi ) const {
    const auto aspect = sqrt(sqrt(1.0f - anisotropic * 0.9f));
    const auto diffuseWeight = (1.0f - metallic) * (1.0 - specTrans);

    const auto wh = Normalize(wo + wi);
    const auto HoO = Dot(wo, wh);
    const auto HoO2ByRoughness = SQR(HoO) * roughness;
    
    const auto luminance = basecolor.GetIntensity();
    const auto Ctint = luminance > 0.0f ? basecolor * (1.0f / luminance) : Spectrum(1.0f);

    auto ret = RGBSpectrum(0.0f);

    if (diffuseWeight > 0.0f) {
        const auto NoO = CosTheta(wo);
        const auto NoI = CosTheta(wi);
        const auto AbsNoI = fabs(NoI);
        const auto FO = SchlickWeight(NoO);
        const auto FI = SchlickWeight(NoI);

        if (thinSurface) {
            if (flatness < 1.0f) {
                // Diffuse
                // Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering, eq (4)
                const auto disneyDiffuse = basecolor * (INV_PI * (1.0 - FO * 0.5f) * (1.0 - FI * 0.5f));
                ret += diffuseWeight * (1.0 - flatness) * (1.0f - diffTrans) * disneyDiffuse * AbsNoI;
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
                ret += diffuseWeight * flatness * (1.0f - diffTrans) * disneyFakeSS * AbsNoI;
            }
        } else {
            if (scatterDistance > 0.0f) {
                // Handle sub-surface scattering branch, to be done.
                // There is a following up task to support SSS in SORT, after which this can be easily done.
                // Issue tracking ticket, https://github.com/JerryCao1985/SORT/issues/85
            } else {
                // Fall back to the Disney diffuse due to the lack of sub-surface scattering
                const auto disneyDiffuse = basecolor * (INV_PI * (1.0 - FO * 0.5f) * (1.0 - FI * 0.5f));
                ret += diffuseWeight * disneyDiffuse * AbsNoI;
            }
        }

        // Retro-reflection
        // Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering, eq (4)
        const auto Rr = 2.0 * HoO2ByRoughness;
        const auto frr = basecolor * (INV_PI * Rr * (FO + FI + FO * FI * (Rr - 1.0f)));
        ret += diffuseWeight * frr * AbsNoI;

        // This is not totally physically correct. However, dielectric model presented by Walter et al. loses energy due to lack
        // of microfacet inter-reflection/refraction and the sheen component can approximately compensate for it.
        if (sheen > 0.0f) {
            const auto Csheen = slerp(Spectrum(1.0f), Ctint, sheenTint);
            const auto FH = SchlickWeight(HoO);
            const auto Fsheen = FH * sheen * Csheen;
            ret += diffuseWeight * Fsheen * AbsNoI;
        }
    }

    // Specular reflection term in Disney BRDF
    const GGX ggx(roughness / aspect, roughness * aspect);
    const auto Cspec0 = slerp(specular * 0.08f * slerp(Spectrum(1.0f), Ctint, specularTint), basecolor, metallic);
    const FresnelDisney fresnel(Cspec0, ior_in, ior_ex, metallic);
    const MicroFacetReflection mf(Cspec0, &fresnel, &ggx, FULL_WEIGHT, nn);
    ret += mf.f(wo, wi);

    // Another layer of clear coat on top of everything below.
    if (clearcoat > 0.0f) {
        const ClearcoatGGX cggx(sqrt(slerp(0.1f, 0.001f, clearcoatGloss)));
        const FresnelSchlick<float> fresnel(0.04f);
        const MicroFacetReflection mf_clearcoat(Spectrum(clearcoat), &fresnel, &cggx, FULL_WEIGHT, nn);
        ret += mf_clearcoat.f(wo, wi);
    }

    // Specular transmission
    if (specTrans > 0.0f) {
        const auto T = specTrans * basecolor.Sqrt();
        if (thinSurface) {
            // Scale roughness based on IOR (Burley 2015, Figure 15).
            const auto rscaled = (0.65f * inv_eta - 0.35f) * roughness;
            const auto ru = SQR(rscaled) / aspect;
            const auto rv = SQR(rscaled) * aspect;
            const GGX scaledDist(ru, rv);

            MicroFacetRefraction mr(T, &scaledDist, ior_ex, ior_in, FULL_WEIGHT, nn);
            ret += mr.f(wo, wi);
        } else {
            // Microfacet Models for Refraction through Rough Surfaces
            // https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
            MicroFacetRefraction mr(T, &ggx, ior_ex, ior_in, FULL_WEIGHT, nn);
            ret += mr.f(wo, wi);
        }
    }

    // Diffuse transmission
    if ( thinSurface && diffTrans > 0.0f && diffuseWeight > 0.0f ) {
        LambertTransmission lambert_transmission(basecolor , 1.0f, nn);
        ret += diffuseWeight * diffTrans * lambert_transmission.f(wo, wi);
    }

    return ret;
}

Spectrum DisneyBRDF::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const {
    const auto aspect = sqrt(sqrt(1.0f - anisotropic * 0.9f));
    const auto luminance = basecolor.GetIntensity();
    const auto Ctint = luminance > 0.0f ? basecolor * (1.0f / luminance) : Spectrum(1.0f);
    const auto Cspec0 = slerp(specular * 0.08f * slerp(Spectrum(1.0f), Ctint, specularTint), basecolor, metallic);
    const auto clearcoat_intensity = clearcoat;
    const auto specular_intensity = Cspec0.GetIntensity();
    const auto total_specular_reflection = clearcoat_intensity + specular_intensity;

    const GGX ggx(roughness / aspect, roughness * aspect);
    const auto sample_nonspecular_reflection_ratio = total_specular_reflection == 0.0f ? 1.0f : ( 1.0f - metallic ) * ( 1.0f - specular * 0.08f ) * basecolor.GetIntensity();
    if( bs.u < sample_nonspecular_reflection_ratio || sample_nonspecular_reflection_ratio == 1.0f ){
        const auto r = sort_canonical();
        if (r < specTrans || specTrans == 1.0f) {
            if (thinSurface) {
                // Scale roughness based on IOR (Burley 2015, Figure 15).
                const auto rscaled = (0.65f * inv_eta - 0.35f) * roughness;
                const auto ru = SQR(rscaled) / aspect;
                const auto rv = SQR(rscaled) * aspect;
                const GGX scaledDist(ru, rv);

                const auto T = specTrans * basecolor.Sqrt();
                MicroFacetRefraction mr(T, &scaledDist, ior_ex, ior_in, FULL_WEIGHT, nn);
                mr.sample_f(wo, wi, bs, pPdf);
            } else {
                // Sampling the transmission BTDF
                const auto T = specTrans * basecolor.Sqrt();
                MicroFacetRefraction mr(T, &ggx, ior_ex, ior_in, FULL_WEIGHT, nn);
                mr.sample_f(wo, wi, bs, pPdf);
            }
        } else {
            const auto r = sort_canonical();

            if ( thinSurface && ( r < diffTrans || diffTrans == 1.0f ) ) {
                LambertTransmission lambert_transmission(basecolor, diffTrans, nn);
                lambert_transmission.sample_f(wo, wi, bs, pPdf);
            } else {
                // Sampling the reflection BRDF
                wi = CosSampleHemisphere( sort_canonical() , sort_canonical() );
            }
        }
    }else{
        // Sampling the metallic BRDF, including clear-coat if needed
        const auto r = sort_canonical();
        BsdfSample sample(true);
        Vector wh;

        const auto clearcoat_ratio = clearcoat_intensity / total_specular_reflection;
        if (r < clearcoat_ratio || clearcoat_ratio == 1.0f) {
            const ClearcoatGGX cggx(sqrt(slerp(0.1f, 0.001f, clearcoatGloss)));
            wh = cggx.sample_f(sample);
        } else {
            wh = ggx.sample_f(sample);
        }
        wi = 2 * Dot(wo, wh) * wh - wo;
    }
    
    if( pPdf ) *pPdf = pdf( wo , wi );
    return f( wo , wi );
}

float DisneyBRDF::pdf( const Vector& wo , const Vector& wi ) const {
    const auto aspect = sqrt(sqrt(1.0f - anisotropic * 0.9f));
    const auto luminance = basecolor.GetIntensity();
    const auto Ctint = luminance > 0.0f ? basecolor * (1.0f / luminance) : Spectrum(1.0f);
    const auto Cspec0 = slerp(specular * 0.08f * slerp(Spectrum(1.0f), Ctint, specularTint), basecolor, metallic);
    const auto clearcoat_intensity = clearcoat;
    const auto specular_intensity = Cspec0.GetIntensity();
    const auto total_specular_reflection = clearcoat_intensity + specular_intensity;

    const GGX ggx(roughness / aspect, roughness * aspect);
    const auto sample_nonspecular_reflection_ratio = (1.0f - metallic) * (1.0f - specular * 0.08f) * basecolor.GetIntensity();

    // Sampling specular transmission
    auto pdf_sample_specular_tranmission = 0.0f;
    const auto T = specTrans * basecolor.Sqrt();
    if (thinSurface) {
        // Scale roughness based on IOR (Burley 2015, Figure 15).
        const auto rscaled = (0.65f * inv_eta - 0.35f) * roughness;
        const auto ru = SQR(rscaled) / aspect;
        const auto rv = SQR(rscaled) * aspect;
        const GGX scaledDist(ru, rv);

        MicroFacetRefraction mr(T, &scaledDist, ior_ex, ior_in, FULL_WEIGHT, nn);
        pdf_sample_specular_tranmission = mr.pdf(wo, wi);
    } else {
        // Sampling the transmission BTDF
        MicroFacetRefraction mr(T, &ggx, ior_ex, ior_in, FULL_WEIGHT, nn);
        pdf_sample_specular_tranmission = mr.pdf(wo, wi);
    }

    // Sampling diffuse component
    const auto pdf_sample_diffuse_reflection = CosHemispherePdf(wi);
    LambertTransmission lambert_transmission(basecolor, diffTrans, nn);
    const auto pdf_sample_diffuse_tranmission = lambert_transmission.pdf(wo, wi);
    const auto pdf_sample_diffuse = (thinSurface) ? slerp(pdf_sample_diffuse_reflection, pdf_sample_diffuse_tranmission, diffTrans) : pdf_sample_diffuse_reflection;

    if (total_specular_reflection == 0.0f)
        return slerp(pdf_sample_diffuse, pdf_sample_specular_tranmission, specTrans);

    // Sampling the metallic BRDF, including clear-coat if needed
    const auto wh = Normalize(wi + wo);
    const auto clearcoat_ratio = clearcoat_intensity / total_specular_reflection;
    const ClearcoatGGX cggx(sqrt(slerp(0.1f, 0.001f, clearcoatGloss)));
    const auto pdf_wh_specular_reflection = slerp(ggx.Pdf(wh), cggx.Pdf(wh), clearcoat_ratio);
    const auto same_hemisphere = SameHemiSphere( wi , wo );
    const auto pdf_specular_reflection = same_hemisphere ? pdf_wh_specular_reflection / (4.0f * AbsDot(wo, wh)) : 0.0f;

    return slerp(pdf_specular_reflection, slerp(pdf_sample_diffuse, pdf_sample_specular_tranmission, specTrans), sample_nonspecular_reflection_ratio);
}
