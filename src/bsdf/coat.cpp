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
#include "utility/samplemethod.h"
#include "fresnel.h"

Spectrum Coat::f( const Vector& wo , const Vector& wi ) const
{
    Spectrum ret = coat.f(wo, wi);
    
    bool tir_o = false, tir_i = false;
    Vector r_wo = refract(wo, Vector(0.0f, 1.0f, 0.0f), ior, 1.0f, tir_o);
    Vector r_wi = refract(wi, Vector(0.0f, 1.0f, 0.0f), ior, 1.0f, tir_i);
    if (!tir_o && !tir_i) {
        // Hard coded Lambert model for now, will be exposed
        const Spectrum bottom = basecolor * INV_PI * CosTheta(wi);
        // Bouguer-Lambert-Beer law
        const Spectrum attenuation = ( -thickness * sigma * (1.0f / AbsCosTheta(r_wo) + 1.0f / AbsCosTheta(r_wi))).Exp();
        // Fresnel attenuation between the boundary across layer0 and layer1
        const Spectrum T12 = (1.0f - fresnel.Evaluate(CosTheta(wo))) * (1.0f - fresnel.Evaluate(CosTheta(wi)));
        // Attenuation during rays coming backward from bottom to top is not taken into consideration yet
        ret += bottom * attenuation * T12;
    }

    return ret;
}

Spectrum Coat::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const{
    // A better importance sampling algorithm to be implemented
    return coat.sample_f(wo, wi, bs, pPdf);
}

float Coat::pdf( const Vector& wo , const Vector& wi ) const{
    // A better importance sampling algorithm to be implemented
    return coat.pdf(wo, wi);
}
