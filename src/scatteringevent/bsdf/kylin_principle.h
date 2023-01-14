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

#pragma once

#include "bxdf.h"
#include "math/vector3.h"

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeKylinPrinciple, "kylin_principle")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeKylinPrinciple, Tsl_float3, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeKylinPrinciple, Tsl_float, metallic)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeKylinPrinciple, Tsl_float, specular)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeKylinPrinciple, Tsl_float, roughness)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeKylinPrinciple, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeKylinPrinciple)

//! @brief  Kylin Principle Brdf.
/**
 * This brdf should be idencical with the principle shading node in Kylin Engine.
 * The purpose of this is purely to offer a reference implementation for Kylin Engine to iterate 
 * since it is in the context of a ray tracer with unbiased solution to the rendering equation.
 * 
 * Note, even if it is supposed to be exactly the same with the real time solution, due to the nature
 * of real time rendering, which is to aim at interactive rendering speed, there are still some 
 * differences compared to this implementation. And those differences may result in bias in rendering
 * result. Not everything in this brdf is physically based, there are things that are commonly available
 * in real time rendering engine that is not pure physically based though.
 */
class KylinPrinciple : public Bxdf{
public:
    //! @brief      Constructor taken attenuation.
    //!
    //! @param param  Parameter set.
    KylinPrinciple(RenderContext& rc, const ClosureTypeKylinPrinciple& param, const Spectrum& weight = 1.0f) : 
        Bxdf(rc, weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), param.normal, true), base_color(param.base_color),
        metallic(param.metallic), roughness(std::max(param.roughness, 0.003f)), specular(param.specular) {}

    //! Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;

    //! @brief Importance sampling for the microfacet btdf.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The Evaluated BRDF value.
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override;

    //! @brief Evaluate the pdf of an existance direction given the Incident direction.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float pdf( const Vector& wo , const Vector& wi ) const override;

private:
    const Spectrum  base_color;
    const float     metallic;
    const float     roughness;
    const float     specular;
};