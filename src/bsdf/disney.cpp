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

    // The corner case is not possible with the current disney BRDF implementation.
    // Corner case where roughness equals to 1.0
    //if (alphaU == 1.0f)
    //        return INV_PI;  // Limit(alpha->1.0) D(h) = 1.0 / PI
    sAssert(alphaU != 1.0f, MATERIAL);

    const auto cos = CosTheta(h);
    return (alphaU2 - 1) / (TWO_PI * log(alphaU) * (1 + (alphaU2 - 1) * SQR(cos)));
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
    const static Spectrum white(1.0f);
    
    const auto aspect = sqrt(sqrt(1.0f - anisotropic * 0.9f));
    const auto diffuseWeight = (1.0f - metallic) * (1.0 - specTrans);

    const auto NoO = CosTheta(wo);
    const auto NoI = CosTheta(wi);

    const auto wh = Normalize(wo + wi);
    const auto HoO = Dot(wo, wh);
    const auto HoO2 = HoO * HoO;
    const auto HoO2ByRoughness = HoO2 * roughness;
    const auto FH = SchlickWeight(HoO);

    // Diffuse
    // Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering, eq (4)
    const auto FO = SchlickWeight(NoO);
    const auto FI = SchlickWeight(NoI);
    const auto fd = basecolor * (INV_PI * (1.0 - FO * 0.5f) * (1.0 - FI * 0.5f));

    // Special handling for thin surfaces
    if (thinSurface) {
        // Thin surface is guaranteed not to be metallic
        const auto diffuseWeight = 1.0 - specTrans;
        const auto disneyDiffuse = (1.0f - flatness) * (1.0f - diffTrans) * fd;

        // Fake sub-surface scattering
        // Reflection from Layered Surfaces due to Subsurface Scattering
        // https://cseweb.ucsd.edu/~ravir/6998/papers/p165-hanrahan.pdf
        // Based on Hanrahan-Krueger BRDF approximation of isotropic BSSRDF
        // 1.25 scale is used to (roughly) preserve albedo
        // Fss90 used to "flatten" retro-reflection based on roughness
        const auto Fss90 = HoO2ByRoughness;
        const auto Fss = slerp(1.0f, Fss90, FO) * slerp(1.0f, Fss90, FI);
        const auto ss = basecolor * (1.25f * (Fss * (1 / (NoO + NoI) - 0.5f) + 0.5f) * INV_PI);
        const auto disneyFakeSS = flatness * (1.0f - diffTrans) * ss;

        auto ret = diffuseWeight * (disneyDiffuse + disneyFakeSS) * NoI;

        // Count diffuse transmission if needed
        if (diffTrans > 0.0f && diffuseWeight > 0.0f) {
            LambertTransmission lambert_transmission(basecolor, diffTrans, DIR_UP);
            ret += diffuseWeight * lambert_transmission.f(wo, wi);
        }

        // Count specular reflection if needed
        if (specTrans > 0.0f) {
            const auto T = specTrans * basecolor.Sqrt();

            // Scale roughness based on IOR (Burley 2015, Figure 15).
            const auto rscaled = (0.65f * inv_eta - 0.35f) * roughness;
            const auto ru = SQR(rscaled) / aspect;
            const auto rv = SQR(rscaled) * aspect;
            const GGX scaledDist(ru, rv);

            MicroFacetRefraction mr(T, &scaledDist, ior_ex, ior_in, white, DIR_UP);
            mr.UpdateGNormal(gnormal);
            ret += mr.f(wo, wi);
        }
        return ret;
    }

    auto ret = RGBSpectrum(0.0f);
    
    const auto luminance = basecolor.GetIntensity();
    const auto Ctint = luminance > 0.0f ? basecolor * (1.0f / luminance) : Spectrum(1.0f);

    if (diffuseWeight > 0.0f) {
        if (scatterDistance > 0.0f) {
            // Handle sub-surface scattering branch, to be done.
            // There is a following up task to support SSS in SORT, after which this can be easily done.
            // Issue tracking ticket, https://github.com/JerryCao1985/SORT/issues/85
        }
        else {
            ret += diffuseWeight * fd * NoI;
        }

        // Retro-reflection
        // Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering, eq (4)
        const auto Rr = 2.0 * HoO2ByRoughness;
        const auto frr = basecolor * (INV_PI * Rr * (FO + FI + FO * FI * (Rr - 1.0f)));
        ret += diffuseWeight * frr * NoI;

        // Count sheen if needed
        if (sheen > 0.0f) {
            // Sheen
            const auto Csheen = slerp(Spectrum(1.0f), Ctint, sheenTint);
            const auto Fsheen = FH * sheen * Csheen;

            ret += diffuseWeight * Fsheen * NoI;
        }
    }

    // Specular term in Disney BRDF
    const GGX ggx(roughness / aspect, roughness * aspect);
    const auto Cspec0 = slerp(specular * 0.08f * slerp(Spectrum(1.0f), Ctint, specularTint), basecolor, metallic);
    const FresnelDisney fresnel(Cspec0, ior_in, ior_ex, metallic);
    const MicroFacetReflection mf(Cspec0, &fresnel, &ggx, white, DIR_UP);
    mf.UpdateGNormal(gnormal);
    ret += mf.f(wo, wi);

    // Clear coat
    if (clearcoat > 0.0f) {
        const ClearcoatGGX cggx(sqrt(slerp(0.1f, 0.001f, clearcoatGloss)));
        const FresnelSchlick<float> fresnel(0.04f);
        const MicroFacetReflection mf_clearcoat(Spectrum(clearcoat), &fresnel, &cggx, white, DIR_UP);
        mf_clearcoat.UpdateGNormal(gnormal);
        ret += mf_clearcoat.f(wo, wi);
    }
    
    // Specular transmittance
    if (specTrans > 0.0f) {
        const auto T = specTrans * basecolor.Sqrt();
        MicroFacetRefraction mr(T, &ggx, ior_ex, ior_in, white, DIR_UP);
        mr.UpdateGNormal(gnormal);
        ret += mr.f(wo, wi);
    }

    return ret;
}

Spectrum DisneyBRDF::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const {
    const static Spectrum white(1.0f);
    const auto aspect = sqrt(sqrt(1.0f - anisotropic * 0.9f));
    
    // Special handling for thin surface
    if (thinSurface) {
        const auto r = sort_canonical();
        if ( r < specTrans || specTrans == 1.0f ) {
            const auto T = specTrans * basecolor.Sqrt();

            // Scale roughness based on IOR (Burley 2015, Figure 15).
            const auto rscaled = (0.65f * inv_eta - 0.35f) * roughness;
            const auto ru = SQR(rscaled) / aspect;
            const auto rv = SQR(rscaled) * aspect;
            const GGX scaledDist(ru, rv);

            MicroFacetRefraction mr(T, &scaledDist, ior_ex, ior_in, white, DIR_UP);
            mr.UpdateGNormal(gnormal);

            mr.sample_f(wo, wi, bs, pPdf);
        } else {
            const auto r = sort_canonical();
            if (r < diffTrans || diffTrans == 1.0f) {
                LambertTransmission lambert_transmission(basecolor, diffTrans, DIR_UP);
                lambert_transmission.sample_f(wo, wi, bs, pPdf);
            }
            else {
                wi = CosSampleHemisphere(sort_canonical(), sort_canonical());
            }
        }

        if (pPdf) *pPdf = pdf(wo, wi);
        return f(wo, wi);
    }

    const GGX ggx(roughness / aspect, roughness * aspect);
    const auto t = ( 1.0f - metallic ) * ( 1.0f - specular * 0.08f ) * basecolor.GetIntensity();
    if( bs.u < t || t == 1.0f ){
        const auto r = sort_canonical();
        if (r < specTrans || specTrans == 1.0f) {
            // Sampling the transmission BTDF
            const auto T = specTrans * basecolor.Sqrt();
            MicroFacetRefraction mr(T, &ggx, ior_ex, ior_in, white, DIR_UP);
            mr.UpdateGNormal(gnormal);
            mr.sample_f(wo, wi, bs, pPdf);
        } else {
            // Sampling the reflection BRDF
            wi = CosSampleHemisphere(bs.u / t, bs.v);
        }
    }else{
        // Sampling the metallic BRDF, including clear-coat if needed
        const auto r = sort_canonical();
        BsdfSample sample(true);
        Vector wh;

        const auto luminance = basecolor.GetIntensity();
        const auto Ctint = luminance > 0.0f ? basecolor * (1.0f / luminance) : Spectrum(1.0f);
        const auto Cspec0 = slerp(specular * 0.08f * slerp(Spectrum(1.0f), Ctint, specularTint), basecolor, metallic);
        const auto clearcoat_intensity = 0.25f * clearcoat;
        const auto specular_intensity = Cspec0.GetIntensity();
        const auto total_intensity = clearcoat_intensity + specular_intensity;
        if (total_intensity == 0.0f) {
            wi = CosSampleHemisphere(bs.u / t, bs.v);
        } else {
            const auto clearcoat_ratio = clearcoat_intensity / total_intensity;
            if (r < clearcoat_ratio || clearcoat_ratio == 1.0f) {
                const ClearcoatGGX cggx(sqrt(slerp(0.1f, 0.001f, clearcoatGloss)));
                wh = cggx.sample_f(sample);
            } else {
                wh = ggx.sample_f(sample);
            }
            wi = 2 * Dot(wo, wh) * wh - wo;
        }
    }
    
    if( pPdf ) *pPdf = pdf( wo , wi );
    return f( wo , wi );
}

float DisneyBRDF::pdf( const Vector& wo , const Vector& wi ) const {
    const static Spectrum white(1.0f);

    const auto aspect = sqrt(sqrt( 1.0f - anisotropic * 0.9f ));
    const GGX ggx( roughness / aspect , roughness * aspect );
    const ClearcoatGGX cggx(sqrt(slerp(0.1f, 0.001f, clearcoatGloss)));
    const auto T = specTrans * basecolor.Sqrt();

    // Special handling for thin surface
    if (thinSurface) {
        LambertTransmission lambert_transmission(basecolor, diffTrans, DIR_UP);
        const auto pdf_non_trans = slerp(CosHemispherePdf(wi), lambert_transmission.pdf(wo, wi), diffTrans);

        // Scale roughness based on IOR (Burley 2015, Figure 15).
        const auto rscaled = (0.65f * inv_eta - 0.35f) * roughness;
        const auto ru = SQR(rscaled) / aspect;
        const auto rv = SQR(rscaled) * aspect;
        const GGX scaledDist(ru, rv);

        MicroFacetRefraction mr(T, &scaledDist, ior_ex, ior_in, white, DIR_UP);
        mr.UpdateGNormal(gnormal);

        const auto pdf_trans = mr.pdf(wo, wi);
        return slerp(pdf_non_trans, pdf_trans, specTrans);
    }

    const auto luminance = basecolor.GetIntensity();
    const auto Ctint = luminance > 0.0f ? basecolor * ( 1.0f / luminance ) : Spectrum( 1.0f );
    const auto Cspec0 = slerp( specular * 0.08f * slerp( Spectrum(1.0f) , Ctint , specularTint ) , basecolor , metallic);
    const auto clearcoat_intensity = 0.25f * clearcoat;
    const auto specular_intensity = Cspec0.GetIntensity();
    const auto total_intensity = clearcoat_intensity + specular_intensity;
    const auto clearcoat_ratio = total_intensity == 0.0f ? 0.0f : clearcoat_intensity / total_intensity;
    
    MicroFacetRefraction mr(T, &ggx, ior_ex, ior_in, white, DIR_UP);
    mr.UpdateGNormal(gnormal);
    const auto non_metallic_pdf = slerp(CosHemispherePdf(wi), mr.pdf(wo, wi), specTrans);

    if ( total_intensity == 0.0f && specTrans == 0.0f )
        return CosHemispherePdf(wi);

    const auto wh = Normalize( wi + wo );
    const auto pdf_wh = slerp( ggx.Pdf(wh) , cggx.Pdf(wh) , clearcoat_ratio );
    return slerp( pdf_wh / ( 4.0f * AbsDot( wo , wh ) ) , non_metallic_pdf, ( 1.0f - metallic ) * ( 1.0f - specular * 0.08f) * basecolor.GetIntensity() );
}
