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

// include the header file
#include "disney.h"
#include "microfacet.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"

float ClearcoatGGX::D(const Vector& h) const {
    // D(h) = ( alpha^2 - 1 ) / ( 2 * PI * ln(alpha) * ( 1 + ( alpha^2 - 1 ) * cos(\theta) ^ 2 )

    // Corner case where roughness equals to 1.0
    if (alphaU == 1.0f)
        return INV_PI;  // Limit(alpha->1.0) D(h) = 1.0 / PI

    const float cos = CosTheta(h);
    return (alphaU2 - 1) / (TWO_PI * log(alphaU) * (1 + (alphaU2 - 1) * cos * cos));
}

Vector ClearcoatGGX::sample_f(const BsdfSample& bs) const {
    // phi = 2 * PI * u
    // theta = acos( sqrt( ( exp( 2 * ln(alpha) * v ) - 1 ) / ( alpha^2 - 1.0f ) ) )
    const float phi = TWO_PI * bs.u;
    const float theta = alphaU2 == 1.0f ? acos(sqrt(bs.v)) : acos(sqrt((exp(log(alphaU2) * bs.v) - 1.0f) / (alphaU2 - 1.0f)));
    return SphericalVec(theta, phi);
}

float ClearcoatGGX::G1(const Vector& v) const {
    const float tan_theta_sq = TanTheta2(v);
    if (isinf(tan_theta_sq)) return 0.0f;
    static const float roughness = 0.25f;
    const float alpha2 = roughness * roughness;
    return 2.0f / (1.0f + sqrt(1.0f + alpha2 * tan_theta_sq));
}

Spectrum DisneyBRDF::f( const Vector& wo , const Vector& wi ) const
{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const static Spectrum white(1.0f);
    
    const float NoO = CosTheta( wo );
    const float NoI = CosTheta( wi );
    
    const Vector h = Normalize( wo + wi );
    const float HoO = Dot( wo , h );
    const float HoO2 = HoO * HoO;
    
    // Fresnel term
    const float FH = SchlickWeight(HoO);
    
    // Sheen term in Disney BRDF model
    const float luminance = basecolor.GetIntensity();
    const Spectrum Ctint = luminance > 0.0f ? basecolor * ( 1.0f / luminance ) : Spectrum( 1.0f );
    const Spectrum Cspec0 = lerp( specular * 0.08f * lerp( Spectrum(1.0f) , Ctint , specularTint ) , basecolor , metallic);
    const Spectrum Csheen = lerp( Spectrum(1.0f) , Ctint , sheenTint );
    const Spectrum Fsheen = FH * sheen * Csheen ;
    
    // Diffuse term in Disney BRDF model
    const float FO = SchlickWeight( NoO );
    const float FI = SchlickWeight( NoI );
    const float Fd90 = 0.5f + 2.0f * HoO2 * roughness;
    const float Fd = lerp( 1.0f , Fd90 , FO )  * lerp( 1.0f , Fd90 , FI );
    
    // Reflection from Layered Surfaces due to Subsurface Scattering
    // https://cseweb.ucsd.edu/~ravir/6998/papers/p165-hanrahan.pdf
    // Based on Hanrahan-Krueger BRDF approximation of isotropic BSSRDF
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retro-reflection based on roughness
    const float Fss90 = HoO2*roughness;
    const float Fss = lerp(1.0f, Fss90, FO) * lerp(1.0f, Fss90, FI);
    const float ss = 1.25f * (Fss * (1 / (NoO + NoI) - 0.5f) + 0.5f);
    
    // Final diffuse term for Disney BRDF
    const Spectrum diff = ( INV_PI * lerp( Fd , ss , subsurface ) * basecolor + Fsheen ) * ( 1.0f - metallic );
    
    // Specular term in Disney BRDF
    const float aspect = sqrt(sqrt( 1.0f - anisotropic * 0.9f ));
    const GGX ggx( roughness / aspect , roughness * aspect );
    const FresnelDisney fresnel0(Cspec0, 1.0f, 1.5f, metallic);
    const MicroFacetReflection mf(Cspec0, &fresnel0, &ggx, white, DIR_UP);
    
    // Clear coat term (ior = 1.5 -> F0 = 0.04)
    const ClearcoatGGX cggx(sqrt(lerp(0.1f, 0.001f, clearcoatGloss)));
    const FresnelSchlick<float> fresnel1(0.04f);
    const MicroFacetReflection mf_clearcoat( Spectrum( 0.25f * clearcoat ) , &fresnel1, &cggx, white, DIR_UP);
    
    mf.UpdateGNormal(gnormal);
    mf_clearcoat.UpdateGNormal(gnormal);
    
    // Final specular term
    const Spectrum spec = mf.f(wo,wi) + mf_clearcoat.f(wo, wi);
    
    return diff * AbsCosTheta(wi) + spec;
}

Spectrum DisneyBRDF::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const{
    const float t = ( 1.0f - metallic ) * ( 1.0f - specular * 0.08f ) * basecolor.GetIntensity();
    if( bs.u < t || t == 1.0f ){
        // Cosine-weighted sample
        wi = CosSampleHemisphere( bs.u / t , bs.v );
    }else{
        const float r = sort_canonical();
        BsdfSample sample(true);
        Vector wh;
        
        const float luminance = basecolor.GetIntensity();
        const Spectrum Ctint = luminance > 0.0f ? basecolor * ( 1.0f / luminance ) : Spectrum( 1.0f );
        const Spectrum Cspec0 = lerp( specular * 0.08f * lerp( Spectrum(1.0f) , Ctint , specularTint ) , basecolor , metallic);
        const float clearcoat_intensity = 0.25f * clearcoat;
        const float specular_intensity = Cspec0.GetIntensity();
        const float total_intensity = clearcoat_intensity + specular_intensity;
        if( total_intensity == 0.0f ){
            wi = CosSampleHemisphere( bs.u / t , bs.v );
        }else{
            const float clearcoat_ratio = clearcoat_intensity / total_intensity;
            if( r < clearcoat_ratio || clearcoat_ratio == 1.0f ){
                const ClearcoatGGX cggx(sqrt(lerp(0.1f, 0.001f, clearcoatGloss)));
                wh = cggx.sample_f(sample);
            }else{
                const float aspect = sqrt(sqrt( 1.0f - anisotropic * 0.9f ));
                const GGX ggx( roughness / aspect , roughness * aspect );
                wh = ggx.sample_f(sample);
            }
            wi = 2 * Dot( wo , wh ) * wh - wo;
        }
    }
    
    if( pPdf ) *pPdf = pdf( wo , wi );
    return f( wo , wi );
}

float DisneyBRDF::pdf( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const float aspect = sqrt(sqrt( 1.0f - anisotropic * 0.9f ));
    const GGX ggx( roughness / aspect , roughness * aspect );
    
    const ClearcoatGGX cggx(sqrt(lerp(0.1f, 0.001f, clearcoatGloss)));
    
    const float luminance = basecolor.GetIntensity();
    const Spectrum Ctint = luminance > 0.0f ? basecolor * ( 1.0f / luminance ) : Spectrum( 1.0f );
    const Spectrum Cspec0 = lerp( specular * 0.08f * lerp( Spectrum(1.0f) , Ctint , specularTint ) , basecolor , metallic);
    const float clearcoat_intensity = 0.25f * clearcoat;
    const float specular_intensity = Cspec0.GetIntensity();
    const float total_intensity = clearcoat_intensity + specular_intensity;
    const float clearcoat_ratio = clearcoat_intensity / total_intensity;
    
    if( total_intensity == 0.0f )
        return CosHemispherePdf(wi);
    const Vector wh = Normalize( wi + wo );
    const float pdf_wh = lerp( ggx.Pdf(wh) , cggx.Pdf(wh) , clearcoat_ratio );
    return lerp( pdf_wh / ( 4.0f * AbsDot( wo , wh ) ) , CosHemispherePdf(wi) , ( 1.0f - metallic ) * ( 1.0f - specular * 0.08f) * basecolor.GetIntensity() );
}
