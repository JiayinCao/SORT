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


Spectrum DisneyBRDF::f( const Vector& wo , const Vector& wi ) const
{
    if( !SameHemiSphere(wo, wi) ) return 0.0f;
    
    const auto lerp = []( const float a , const float b , const float t ){ return a * ( 1.0f - t ) + b * t; };
    const auto lerp_spectrum = []( const Spectrum& a , const Spectrum& b , const float t ){ return a * ( 1.0f - t ) + b * t; };
    const float NoO = CosTheta( wo );
    const float NoI = CosTheta( wi );
    
    const Vector h = Normalize( wo + wi );
    const float HoO = Dot( wo , h );
    const float HoO2 = HoO * HoO;
    
    const float luminance = 0.3 * basecolor.GetR() + 0.6 * basecolor.GetG() + 0.1 * basecolor.GetB();
    
    // Fresnel term
    const float FH = SchlickWeight(HoO);
    
    // Sheen term in Disney BRDF model
    const Spectrum Ctint = luminance > 0.0f ? basecolor * ( 1.0f / luminance ) : Spectrum( 1.0f );
    const Spectrum Cspec0 = lerp_spectrum( specular * 0.08 * lerp_spectrum( Spectrum(1.0f) , Ctint , specularTint ) , basecolor , metallic);
    const Spectrum Csheen = lerp_spectrum( Spectrum(1.0f) , Ctint , sheenTint );
    const Spectrum Fsheen = FH * sheen * Csheen;
    
    // Diffuse term in Disney BRDF model
    const float FO = SchlickWeight( NoO );
    const float FI = SchlickWeight( NoI );
    const float Fd90 = 0.5f + 2.0f * HoO2 * roughness;
    const float Fd = lerp( 1.0f , Fd90 , FO )  * lerp( 1.0f , Fd90 , FI );
    
    // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on roughness
    const float Fss90 = HoO2*roughness;
    const float Fss = lerp(1.0, Fss90, FO) * lerp(1.0, Fss90, FI);
    const float ss = 1.25 * (Fss * (1 / (NoO + NoI) - .5) + .5);
    
    // Final diffuse term for disney BRDF
    const Spectrum diff = INV_PI * ( lerp( Fd , ss , subsurface ) * basecolor + Fsheen ) * ( 1.0f - metallic );
    
    // Specular term in Disney BRDF
    const float aspect = sqrt(sqrt( 1.0f - anisotropic * 0.9f ));
    const GGX ggx( roughness / aspect , roughness * aspect );
    const Spectrum Fs = lerp_spectrum( Cspec0 , Spectrum( 1.0f ) , FH );
    
    // Clear coat term (ior = 1.5 -> F0 = 0.04)
    const float Fr = lerp( 0.04f , 1.0f , FH );
    const float r = lerp( 0.1f , 0.001f , clearcoatGloss );
    const GGX ggx2( r , r );
    
    // Final specular term
    const Spectrum spec = ggx.D(h) * Fs * ggx.G(wo,wi) / ( 4.0f * NoO * NoI ) + 0.25f * clearcoat * Fr * ggx2.G(wo,wi) * ggx2.G(wo,wi);
    
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
