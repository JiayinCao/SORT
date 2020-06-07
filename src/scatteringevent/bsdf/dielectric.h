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

//#include <OSL/genclosure.h>
//#include <OSL/oslexec.h>
//#include <OSL/oslclosure.h>
#include "microfacet.h"
#include "spectrum/rgbspectrum.h"

 //! @brief Coat BRDF.
 /**
  * 'Microfacet Models for Refraction through Rough Surfaces'
  * https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
  *
  * This BRDF implementation is the combination of MicrofacetReflection and MicrofacetRefraction defined in 'microfacet.h'.
  * The reason for implementing such a BXDF instead using node to combine them is for better importance sampling, which
  * is fairly important for this kind of BXDF rendering. Blending the two BXDF in BSDF will result too much noise, the convergence
  * rate is too slow to be practical.
  */
class Dielectric : public Bxdf {
public:
    // Input parameters to construct the BRDF.
    // struct Params {
    //     OSL::Vec3       reflectance;
    //     OSL::Vec3       transmittance;
    //     float           roughnessU;
    //     float           roughnessV;
    //     OSL::Vec3       n;
    // };

    //! Constructor from parameter set.
    //!
    //! @param param        All parameters.
    //! @param weight       Weight of this BRDF.
    // Dielectric(const Params& params, const Spectrum& weight);

    //! Constructor
    //! @param reflectance      Direction hemisphere reflection.
    //! @param transmittance    Direction hemisphere transmittance.
    //! @param ior              Index of refraction outside the surface where the normal points to.
    //! @param ior_in           Index of refraction inside the surface opposite to the direction normal points.
    //! @param weight           Weight of the BXDF.
    //! @param n                Normal from normal map.
    //! @param doubleSided      Whether the surface is double sided.
    Dielectric(const Spectrum& reflectance, const Spectrum& tranmisttance, const MicroFacetDistribution* d, float ior, float ior_in, const Spectrum& weight, const Vector& n)
        : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, true), R(reflectance), T(tranmisttance), fresnel(ior, ior_in),
        mf_reflect(reflectance, &fresnel, d, FULL_WEIGHT, n, true),
        mf_refract(tranmisttance, d, ior, ior_in, FULL_WEIGHT, n)
        {}

    //! Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f(const Vector& wo, const Vector& wi) const override;

    //! @brief Importance sampling for the fresnel BRDF.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The Evaluated BRDF value.
    Spectrum sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pdf) const override;

    //! @brief Evaluate the pdf of an exitant direction given the Incident direction.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float pdf(const Vector& wo, const Vector& wi) const override;

private:
    const Spectrum              R, T;       /**< Reflectance and transmittance. */
    const FresnelDielectric     fresnel;    /**< Fresnel term. */
    const MicroFacetReflection  mf_reflect; /**< Microfacet reflection model. */
    const MicroFacetRefraction  mf_refract; /**< Microfacet refraction model. */
};
