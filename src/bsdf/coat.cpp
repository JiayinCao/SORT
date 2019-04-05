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

#include "coat.h"
#include "bsdf.h"
#include "sampler/sample.h"

// Because this BRDF doesn't take TIR ( Total inner reflection ) into account, there is a small biased compensation introduced
// when evaluating the attenuation upward. The exact number is not mentioned in the original paper, 0.2 is used as default here.
#define TIR_COMPENSATION    0.2f

Coat::Coat( const Params& params , const Spectrum& weight, const Bsdf* bottom )
: Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), params.n, false), thickness(1.0f), ior(params.ior), sigma(params.sigma), ggx(params.roughness, params.roughness),
fresnel(1.0f,params.ior), coat_weight( 1.0f ), coat(coat_weight, &fresnel , &ggx , coat_weight , params.n ), bottom( bottom ){
}

Spectrum Coat::F( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!PointingUp(wo)) return 0.0f;

    const auto swo = bsdfToBxdf( wo );
    const auto swi = bsdfToBxdf( wi );
    
    auto ret = coat.f(swo, swi);
    auto tir_o = false, tir_i = false;
    const auto r_wo = refract(swo, DIR_UP, ior, 1.0f, tir_o);
    const auto r_wi = refract(swi, DIR_UP, ior, 1.0f, tir_i);
    if (!tir_o && !tir_i) {
        // Bouguer-Lambert-Beer law
        const auto attenuation = ( -thickness * sigma * (1.0f / AbsCosTheta(r_wo) + 1.0f / AbsCosTheta(r_wi))).Exp();
        // Fresnel attenuation between the boundary across layer0 and layer1
        const auto T12 = (1.0f - fresnel.Evaluate(CosTheta(swo)));
        const auto T21 = slerp( 1.0f - fresnel.Evaluate(CosTheta(swi)), 1.0f, TIR_COMPENSATION);

        ret += bottom->f( -r_wo , -r_wi ) * attenuation * T12 * T21 / ( ior * ior );
    }

    return ret;
}

Spectrum Coat::Sample_F( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const{
    const auto swo = bsdfToBxdf( wo );
    Vector swi;
    
    auto tir_o = false , tir_i = false;
    const auto r_wo = refract(swo, DIR_UP, ior, 1.0f, tir_o);
    Vector r_wi;
    const auto attenuation = ( -thickness * sigma * 2.0f / AbsCosTheta(r_wo) ).Exp();
    const auto I1 = fresnel.Evaluate(CosTheta(swo)).GetIntensity();
    const auto I2 = ( 1.0f - I1 ) * ( 1.0f - I1 ) * attenuation.GetIntensity() / ( ior * ior );
    const auto specProp = I1 / ( I1 + I2 );

    Spectrum ret;
    auto nbs = BsdfSample(true);
    if( bs.u < specProp || specProp == 1.0f ){
        // Importance sampling based on the top layer, Microfacet model
        ret = coat.sample_f( swo , swi, nbs, pPdf );
        wi = bxdfToBsdf(swi);

        auto r_wi = refract(swi, DIR_UP, ior, 1.0f, tir_i);
        if (!tir_o && !tir_i) {
            // Bouguer-Lambert-Beer law
            const auto attenuation = (-thickness * sigma * (1.0f / AbsCosTheta(r_wo) + 1.0f / AbsCosTheta(r_wi))).Exp();
            // Fresnel attenuation between the boundary across layer0 and layer1
            const auto T12 = (1.0f - fresnel.Evaluate(CosTheta(swo)));
            const auto T21 = slerp(1.0f - fresnel.Evaluate(CosTheta(swi)), 1.0f, TIR_COMPENSATION);

            ret += bottom->f(-r_wo, -r_wi) * attenuation * T12 * T21 / (ior * ior);
        }

        if(pPdf)
            *pPdf = slerp(bottom->Pdf(-r_wo,-r_wi) , *pPdf, specProp);
    }else{
        // Importance sampling using the underlying layer
        Vector r_wi;
        ret = bottom->sample_f( -r_wo , r_wi, nbs, pPdf );

        swi = refract(-r_wi, DIR_UP, ior, 1.0f, tir_i);
        wi = bxdfToBsdf(swi);

        // Handle corner case where TIR happens
        if (tir_i || tir_o) {
            if (pPdf) *pPdf = 0.0f;
            return 0.0f;
        }

        // Bouguer-Lambert-Beer law
        const auto attenuation = (-thickness * sigma * (1.0f / AbsCosTheta(r_wo) + 1.0f / AbsCosTheta(r_wi))).Exp();
        // Fresnel attenuation between the boundary across layer0 and layer1
        const auto T12 = (1.0f - fresnel.Evaluate(CosTheta(swo)));
        const auto T21 = slerp(1.0f - fresnel.Evaluate(CosTheta(swi)), 1.0f, TIR_COMPENSATION);

        ret *= attenuation * T12 * T21 / (ior * ior);

        if( pPdf )
            *pPdf = slerp(*pPdf, coat.pdf( swo , swi ) , specProp);
    }
    
    if (!SameHemiSphere(wo, wi) || !PointingUp(wo)) {
        if (pPdf) *pPdf = 0.0f;
        return 0.0f;
    }

    return ret;
}

float Coat::Pdf( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!PointingUp(wo)) return 0.0f;

    const auto swo = bsdfToBxdf( wo );
    const auto swi = bsdfToBxdf( wi );
    
    auto tir_o = false , tir_i = false;
    const auto r_wo = refract(swo, DIR_UP, ior, 1.0f, tir_o);
    const auto r_wi = refract(swi, DIR_UP, ior, 1.0f, tir_i);
    const auto attenuation = ( -thickness * sigma * 2.0f / AbsCosTheta(r_wo) ).Exp();
    const auto I1 = fresnel.Evaluate(CosTheta(swo)).GetIntensity();
    const auto I2 = ( 1.0f - I1 ) * ( 1.0f - I1 ) * attenuation.GetIntensity() / ( ior * ior );
    const auto specProp = I1 / ( I1 + I2 );

    const auto layer0_pdf = coat.pdf( swo , swi );
    const auto layer1_pdf = (tir_o || tir_i) ? 0.0f : bottom->Pdf(-r_wo, -r_wi);
    return slerp( layer1_pdf , layer0_pdf , specProp );
}
