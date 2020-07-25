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

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeDistributionBRDF, "distribution_brdf")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeDistributionBRDF, float3, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeDistributionBRDF, float, roughness)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeDistributionBRDF, float, specular)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeDistributionBRDF, float, specular_tint)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeDistributionBRDF, float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeDistributionBRDF)

//! @brief Disbribution based brdf.
/**
 * Distribution-based BRDFs
 * http://www.cs.utah.edu/~premoze/dbrdf/dBRDF.pdf
 *
 * Crafting a Next-Gen Material Pipeline for The Order: 1886
 * https://blog.selfshadow.com/publications/s2013-shading-course/rad/s2013_pbs_rad_notes.pdf
 */
class DistributionBRDF : public Bxdf{
public:
    //! Constructor taking spectrum information.
    //!
    //! @param s            Direction-Hemisphere reflection.
    //! @oaram r            Roughnes.
    //! @param specular     Reflectance of non-lambert part at normal incident.
    //! @param st           Specular Tint towards @param s.
    //! @param weight       Weight of this BRDF
    //! @param n            Normal, usually from normal map, in local coordinate.
    //! @param doubleSided  Whether the BRDF is double sided.
    DistributionBRDF(const Spectrum& s, const float r , const float specular , const float st , const Spectrum& weight, const Vector& n, bool doubleSided = false)
        : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided), R(s), alpha(convert(r)), alphaSqr(alpha*alpha), specular(specular), specularTint(st) {
    }

    //! Constructor from parameter set.
    //!
    //! @param param        All parameters.
    //! @param weight       Weight of this BRDF.
    //! @param doubleSided  Whether the material is double-sided.
    DistributionBRDF( const ClosureTypeDistributionBRDF& param , const Spectrum& weight , bool doubleSided = false)
        : DistributionBRDF( param.base_color, param.roughness , param.specular , param.specular_tint , weight , param.normal ) {
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
    const Spectrum  R;              /**< Direction-Hemisphere reflection or total reflection. */
    const float     alpha;          /**< Alpha of the Brdf. */
    const float     alphaSqr;       /**< Squred of alpha. */
    const float     specular;       /**< Reflectance of non-lambert part at normal incident. */
    const float     specularTint;   /**< Specular Tint towards R. */

    // UE4's way of converting roughness to alpha
    // http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
    float convert( float roughness ) const {
        roughness = std::max(roughness, (float)1e-3);
        return 1.0f - SQR(roughness);
    };
};