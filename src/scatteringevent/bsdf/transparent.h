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

#pragma once

#include "bxdf.h"
#include "math/vector3.h"

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeTransparent, "transparent")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeTransparent, Tsl_float3, attenuation)
DECLARE_CLOSURE_TYPE_END(ClosureTypeTransparent)

//! @brief  Transparent BXDF
/**
 * Transparent material is a very special material that pass through lights without any attenuation
 * if attenuation is (1,1,1). Essentially, this is a Dirac Delta function. Its existance is for masking 
 * geometries and serving as proxy invisible geometries for volumes.
 */
class Transparent : public Bxdf{
public:
    //! @brief      Default constructor
    Transparent(RenderContext& rc) : Bxdf(rc, FULL_WEIGHT, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), DIR_UP, true), A(WHITE_SPECTRUM) {}

    //! @brief      Constructor taken attenuation.
    //!
    //! @param      Parameter set.
    Transparent(RenderContext& rc, const ClosureTypeTransparent& param, const Spectrum& weight) : Bxdf(rc, weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), DIR_UP, true), A(param.attenuation) {}

    //! @brief      Constructor from parameter set.
    //!
    //! @param param        All parameters.
    //! @param weight       Weight of this BRDF.
    // Transparent( const Params& param , const Spectrum& weight ):Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE|BXDF_REFLECTION), DIR_UP, true),A(param.attenuation){}

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
    const Spectrum A;         /**< Attenuation of radiance. */
};