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

//! @brief Clearcoat GGX NDF.
class ClearcoatGGX : public GGX
{
public:
    //! @brief Constructor
    //! @param roughness    Roughness of the surface formed by the micro facets.
    ClearcoatGGX(float roughness) : GGX(roughness, roughness) {}

    //! @brief probability of facet with specific normal (h)
    float D(const Vector& h) const override {
        // D(h) = ( alpha^2 - 1 ) / ( 2 * PI * ln(alpha) * ( 1 + ( alpha^2 - 1 ) * cos(\theta) )
        return (alphaU2 - 1) / (TWO_PI * log(alphaU) * (1 + (alphaU2 - 1) * CosTheta(h)));
    }

    //! @brief Sampling a normal respect to the NDF.
    //!
    //! @param bs   Sample holding all necessary random variables.
    //! @param wo   Outgoing direction
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f(const BsdfSample& bs, const Vector& wo) const override {
        // phi = 2 * PI * u
        // theta = acos( sqrt( ( exp( 2 * ln(alpha) * v ) - 1 ) / ( alpha^2 - 1.0f ) ) )
        const float phi = TWO_PI * bs.u;
        const float theta = acos(sqrt((exp(log(alphaU2) * bs.v) - 1.0f) / (alphaU2 - 1.0f)));
        auto wh = SphericalVec(theta, phi);
        if (!SameHemiSphere(wh, wo)) wh = -wh;
        return wh;
    }

protected:
    //! @brief Smith shadow-masking function G1
    float G1(const Vector& v) const override{
        const float tan_theta_sq = TanTheta2(v);
        if (isinf(tan_theta_sq)) return 0.0f;
        static const float roughness = 0.25f;
        const float alpha2 = roughness * roughness;
        return 2.0f / (1.0f + sqrt(1.0f + alpha2 * tan_theta_sq));
    }
};

Spectrum DisneyBRDF::f( const Vector& wo , const Vector& wi ) const
{
    if( !SameHemiSphere(wo, wi) ) return 0.0f;

    const static Spectrum white(1.0f);
    
    const auto lerp = []( const float a , const float b , const float t ){ return a * ( 1.0f - t ) + b * t; };
    const auto lerp_spectrum = []( const Spectrum& a , const Spectrum& b , const float t ){ return a * ( 1.0f - t ) + b * t; };
    const float NoO = CosTheta( wo );
    const float NoI = CosTheta( wi );
    
    const Vector h = Normalize( wo + wi );
    const float HoO = Dot( wo , h );
    const float HoO2 = HoO * HoO;
    
    const float luminance = 0.3f * basecolor.GetR() + 0.6f * basecolor.GetG() + 0.1f * basecolor.GetB();
    
    // Fresnel term
    const float FH = SchlickWeight(HoO);
    
    // Sheen term in Disney BRDF model
    const Spectrum Ctint = luminance > 0.0f ? basecolor * ( 1.0f / luminance ) : Spectrum( 1.0f );
    const Spectrum Cspec0 = lerp_spectrum( specular * 0.08f * lerp_spectrum( Spectrum(1.0f) , Ctint , specularTint ) , basecolor , metallic);
    const Spectrum Csheen = lerp_spectrum( Spectrum(1.0f) , Ctint , sheenTint );
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
    const float Fss = lerp(1.0, Fss90, FO) * lerp(1.0, Fss90, FI);
    const float ss = 1.25f * (Fss * (1 / (NoO + NoI) - 0.5f) + 0.5f);
    
    // Final diffuse term for Disney BRDF
    const Spectrum diff = ( INV_PI * lerp( Fd , ss , subsurface ) * basecolor + Fsheen ) * ( 1.0f - metallic );
    
    // Specular term in Disney BRDF
    const float aspect = sqrt(sqrt( 1.0f - anisotropic * 0.9f ));
    const GGX ggx( roughness / aspect , roughness * aspect );
    const FresnelSchlick<Spectrum> fresnel0(Cspec0);
    const MicroFacetReflection mf(white, &fresnel0, &ggx, white);
    
    // Clear coat term (ior = 1.5 -> F0 = 0.04)
    const ClearcoatGGX cggx(lerp(0.1f, 0.001f, clearcoatGloss));
    const FresnelSchlick<float> fresnel1(0.04f);
    const MicroFacetReflection mf_clearcoat( Spectrum( 0.25f ) , &fresnel1, &cggx, white);
    
    // Final specular term
    const Spectrum spec = mf.f(wo, wi) + mf_clearcoat.f(wo, wi);
    
    return diff + spec;
}

Spectrum DisneyBRDF::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const{
    // to be implemented
    return Bxdf::sample_f( wo , wi , bs , pdf );
}

float DisneyBRDF::Pdf( const Vector& wo , const Vector& wi ) const{
    // to be implemented
    return Bxdf::Pdf( wo , wi );
}
