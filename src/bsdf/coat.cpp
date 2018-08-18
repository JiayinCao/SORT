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
#include "coat.h"
#include "bsdf.h"
#include "sampler/sample.h"

Spectrum Coat::F( const Vector& wo , const Vector& wi ) const
{
    Vector swo = bsdfToBxdf( wo );
    Vector swi = bsdfToBxdf( wi );
    
    Spectrum ret = coat.f(swo, swi);
    bool tir_o = false, tir_i = false;
    Vector r_wo = refract(swo, Vector(0.0f, 1.0f, 0.0f), ior, 1.0f, tir_o);
    Vector r_wi = refract(swi, Vector(0.0f, 1.0f, 0.0f), ior, 1.0f, tir_i);
    if (!tir_o && !tir_i) {
        // Bouguer-Lambert-Beer law
        const Spectrum attenuation = ( -thickness * sigma * (1.0f / AbsCosTheta(r_wo) + 1.0f / AbsCosTheta(r_wi))).Exp();
        // Fresnel attenuation between the boundary across layer0 and layer1
        const Spectrum T12 = (1.0f - fresnel.Evaluate(CosTheta(swo))) * (1.0f - fresnel.Evaluate(CosTheta(swi)));
        // Attenuation of rays coming upward is exactly the same with the downward attenuation, but there is a constant factor for
        // compensating the loss caused by TIR. The constant number is not mentioned in the original paper, 0.2 is used here by default.
        static const float G = 0.2f;
        ret += bottom->f( wo , wi ) * attenuation * T12 * lerp( T12 , 1.0f , G );
    }

    return ret;
}

Spectrum Coat::Sample_F( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const{
    Vector swo = bsdfToBxdf( wo );
    Vector lwi;
    
    // Because attenuation from the incident direction is not considered at all, an scaled offset by roughness is added here to compensate it
    const float specProp = saturate( 0.5f + 0.5f * ( 1.0f - roughness ) - ( -thickness * sigma * (1.0f / AbsCosTheta(swo)) ).Exp().GetIntensity() * (1.0f - fresnel.Evaluate(CosTheta(swo))).GetIntensity() );

    Spectrum ret;
    if( bs.u < specProp ){
        // Importance sampling based on the top layer, Microfacet model
        ret = coat.sample_f( swo , lwi , BsdfSample(true) , pPdf );
        wi = bxdfToBsdf(lwi);
    }else{
        // Importance sampling using the underlying layer
        ret = bottom->sample_f( wo , wi , BsdfSample(true) , pPdf );
    }
    
    if( pPdf ){
        if( bs.u < specProp )
            *pPdf = lerp( bottom->Pdf( wo , wi ) , *pPdf , specProp );
        else
            *pPdf = lerp( *pPdf , coat.pdf( swo , bsdfToBxdf(wi) ) , specProp );
    }
    
    return ret;
}

float Coat::Pdf( const Vector& wo , const Vector& wi ) const{
    Vector swo = bsdfToBxdf( wo );
    Vector swi = bsdfToBxdf( wi );
    
    // Because attenuation from the incident direction is not considered at all, an scaled offset by roughness is added here to compensate it
    const float specProp = saturate( 0.5f + 0.5f * ( 1.0f - roughness ) - ( -thickness * sigma * (1.0f / AbsCosTheta(swo)) ).Exp().GetIntensity() * (1.0f - fresnel.Evaluate(CosTheta(swo))).GetIntensity() );
    
    const float layer0_pdf = coat.pdf( swo , swi );
    const float layer1_pdf = bottom->Pdf( wo , wi );
    return lerp( layer1_pdf , layer0_pdf , specProp );
}
