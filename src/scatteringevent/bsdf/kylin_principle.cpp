/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "kylin_principle.h"
#include "microfacet.h"
#include "lambert.h"
#include "core/rand.h"
#include "multi_scattering.h"

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeKylinPrinciple)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeKylinPrinciple, Tsl_float3, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeKylinPrinciple, Tsl_float, metallic)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeKylinPrinciple, Tsl_float, specular)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeKylinPrinciple, Tsl_float, roughness)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeKylinPrinciple, Tsl_float3, normal)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeKylinPrinciple)

static float DielectricSpecularToF0(const float specular){
    return 0.08f * specular;
}

static Spectrum ComputeF0(const float specular, const Spectrum& base_color, float metallic){
    Spectrum ret;
    const float f0 = DielectricSpecularToF0(specular);
    ret[0] = f0 + metallic * ( base_color[0] - f0 );
    ret[1] = f0 + metallic * ( base_color[1] - f0 );
    ret[2] = f0 + metallic * ( base_color[2] - f0 );
    return ret;
}

Spectrum KylinPrinciple::f( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    auto ret = RGBSpectrum(0.0f);

    const auto f0 = ComputeF0(specular, base_color, metallic);
    // There are clearly more accurate fresnel approximation in the world of offline rendering.
    // However, in order to be similiar with Kylin Engine's implementation, Schlick's approximation
    // is used here for consistency.
    const FresnelSchlick<Spectrum> fresnel(f0);

    // Specular
    const float aspect = 1.f;   // to be implemented based on anisotropic level
    const GGX ggx(roughness / aspect, roughness * aspect);
    const MicroFacetReflectionMS mf(rc, WHITE_SPECTRUM, &fresnel, &ggx, FULL_WEIGHT, nn);
    ret += mf.f(wo, wi);

    // Diffuse
    const auto diffuse_attenuation = DiffuseAttenuation(f0, roughness, wo.y, wi.y);
    const auto diffuse_base_color = base_color * ( 1.f - metallic ) * diffuse_attenuation;
    if(!diffuse_base_color.IsBlack()){
        const Lambert lambert( rc, diffuse_base_color , diffuse_base_color , DIR_UP );
        ret += lambert.f(wo, wi);
    }

    return ret;
}

Spectrum KylinPrinciple::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* ppdf ) const{
    const auto diffuse_base_color = base_color * ( 1.f - metallic );
    const auto f0 = ComputeF0(specular, base_color, metallic) + 0.1f;
    const auto sample_diffuse_ratio = diffuse_base_color.GetIntensity() / ( diffuse_base_color.GetIntensity() + f0.GetIntensity());
    const auto sample_diffuse = sort_rand<float>(rc) < sample_diffuse_ratio;

    if(sample_diffuse){
        Lambert lambert( rc, diffuse_base_color , diffuse_base_color , DIR_UP );
        lambert.sample_f(wo, wi, bs, ppdf);
    }else{
        const FresnelSchlick<Spectrum> fresnel(f0);

        const float aspect = 1.f;   // to be implemented based on anisotropic level
        const GGX ggx(roughness / aspect, roughness * aspect);

        const MicroFacetReflection mf(rc, WHITE_SPECTRUM, &fresnel, &ggx, FULL_WEIGHT, nn);

        mf.sample_f(wo, wi, bs, ppdf);
    }

    if(ppdf)
        *ppdf = pdf(wo, wi);

    return f(wo, wi);
}

float KylinPrinciple::pdf( const Vector& wo , const Vector& wi ) const {
    const auto diffuse_base_color = base_color * ( 1.f - metallic );
    const auto f0 = ComputeF0(specular, base_color, metallic) + 0.1f;
    const auto sample_diffuse_ratio = diffuse_base_color.GetIntensity() / ( diffuse_base_color.GetIntensity() + f0.GetIntensity());

    float pdf = 0.f;

    if( sample_diffuse_ratio > 0.f ){
        Lambert lambert( rc, diffuse_base_color , diffuse_base_color , DIR_UP );
        pdf += lambert.pdf(wo, wi) * sample_diffuse_ratio;
    }
    
    if( sample_diffuse_ratio < 1.f ){
        // There are clearly more accurate fresnel approximation in the world of offline rendering.
        // However, in order to be similiar with Kylin Engine's implementation, Schlick's approximation
        // is used here for consistency.
        const auto f0 = ComputeF0(specular, base_color, metallic);
        const FresnelSchlick<Spectrum> fresnel(f0);

        const float aspect = 1.f;   // to be implemented based on anisotropic level
        const GGX ggx(roughness / aspect, roughness * aspect);

        const MicroFacetReflection mf(rc, WHITE_SPECTRUM, &fresnel, &ggx, FULL_WEIGHT, nn);

        pdf += mf.pdf(wo, wi) * ( 1.f - sample_diffuse_ratio );
    }

    return pdf;
}