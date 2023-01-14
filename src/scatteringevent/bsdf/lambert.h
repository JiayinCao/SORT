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

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeLambert, "lambert")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, Tsl_float3, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeLambert)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeLambertTransmission, "lambert_transmission")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambertTransmission, Tsl_float3, transmittance)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambertTransmission, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeLambertTransmission)

//! @brief Lambert brdf.
/**
 * Lambert is the simplest BRDF that exists in any renderers.
 * It reflects equal radiance along all Exitant directions.
 * One can use Lambert to simulate Matte-like material.
 */
class Lambert : public Bxdf{
public:
    Lambert(RenderContext& rc, const ClosureTypeLambert& params, const Spectrum& weight, bool doubleSided = false) :Bxdf(rc, weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), params.normal, doubleSided), R(params.base_color) {}

    //! Constructor taking spectrum information.
    //!
    //! @param s            Direction-Hemisphere reflection.
    //! @param weight       Weight of this BRDF
    //! @param t            Type of this BRDF
    Lambert(RenderContext& rc, const Spectrum& s, const Spectrum& weight, const Vector& n, bool doubleSided = false) :Bxdf(rc, weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided), R(s) {}

    //! Constructor taking spectrum information.
    //!
    //! @param s            Direction-Hemisphere reflection.
    //! @param ew           Evaluation weight of this BRDF.
    //! @param sw           Sampling weight of this BRDF.
    //! @param t            Type of this BRDF
    Lambert(RenderContext& rc, const Spectrum& s, const Spectrum& ew, const float sw, const Vector& n, bool doubleSided = false) :Bxdf(rc, ew, sw, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided), R(s) {}

    //! Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;

private:
    const Spectrum R;         /**< Direction-Hemisphere reflection or total reflection. */
};

//! @brief Lambert transmittance brdf.
/**
 * LambertTransmittance is the transmittance version of lambert model
 */
class LambertTransmission : public Bxdf{
public:
    //! Constructor from parameter set.
    //!
    //! @param param        All parameters.
    //! @param weight       Weight of this BRDF.
    //! @param doubleSided  Whether the material is double-sided.
    LambertTransmission( RenderContext& rc, const ClosureTypeLambertTransmission& param , const Spectrum& weight):Bxdf(rc, weight, (BXDF_TYPE)(BXDF_DIFFUSE|BXDF_REFLECTION), param.normal, true),T(param.transmittance){}

    //! Constructor taking spectrum information.
    //! @param s            Direction-Hemisphere refraction.
    //! @param weight       Weight of this BRDF
    //! @param t            Type of this BRDF
    LambertTransmission( RenderContext& rc, const Spectrum& t , const Spectrum& weight , const Vector& n ):Bxdf(rc, weight, (BXDF_TYPE)(BXDF_DIFFUSE|BXDF_TRANSMISSION), n, true),T(t){}

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
    const Spectrum T;         /**< Direction-Hemisphere reflection or total reflection. */
};
