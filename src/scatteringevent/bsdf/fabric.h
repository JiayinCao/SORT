/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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

//! @brief Fabric BRDF from DreamWorks
/**
 * Physically Based Shading at DreamWorks Animation
 * https://blog.selfshadow.com/publications/s2017-shading-course/dreamworks/s2017_pbs_dreamworks_notes.pdf
 */
class Fabric : public Bxdf{
public:
    // Input parameters to construct the BRDF.
    struct Params {
        OSL::Vec3   baseColor;
        float       roughness;
        OSL::Vec3   n;
    };

    //! Constructor taking spectrum information.
    //!
    //! @param bc           Base color.
    //! @param r            Roughness.
    //! @param weight       Weight of this BRDF
    //! @param n            Normal, usually from normal map, in local coordinate.
    //! @param doubleSided  Whether the BRDF is double sided.
    Fabric(const Spectrum& bc, const float r , const Spectrum& weight, const Vector& n, bool doubleSided = false)
        : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided), baseColor(bc), roughness(r) {
    }

    //! Constructor from parameter set.
    //!
    //! @param param        All parameters.
    //! @param weight       Weight of this BRDF.
    //! @param doubleSided  Whether the material is double-sided.
    Fabric( const Params& param , const Spectrum& weight , bool doubleSided = false)
        : Fabric( param.baseColor , param.roughness , weight , param.n ) {
    }

    //! Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;

    //! @brief Importance sampling for the fresnel BRDF.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The Evaluated BRDF value.
    Spectrum sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pdf) const override;

    //! @brief Evaluate the pdf of an existance direction given the Incident direction.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float pdf( const Vector& wo , const Vector& wi ) const override;

private:
    const Spectrum  baseColor;  /**< Direction-Hemisphere reflection or total reflection. */
    const float     roughness;  /**< Roughness of the fabric. */
};