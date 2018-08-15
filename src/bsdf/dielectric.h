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

#pragma once

#include "microfacet.h"

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
class Dielectric : public Bxdf 
{
public:
    //! Constructor
    //! @param reflectance      Direction hemisphere reflection.
    //! @param transmittance    Direction hemisphere transmittance.
    //! @param ior              Index of refraction outside the surface where the normal points to.
    //! @param ior_in           Index of refraction inside the surface opposite to the direction normal points.
    //! @param weight           Weight of the BXDF.
    //! @param n                Normal from normal map.
    //! @param doubleSided      Whether the surface is double sided.
    Dielectric(const Spectrum& reflectance, const Spectrum& tranmisttance, const MicroFacetDistribution* d, float ior, float ior_in, const Spectrum& weight, const Vector& n, bool doubleSided = false)
        : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided), R(reflectance), T(tranmisttance), fullweight(1.0f), fresnel(ior, ior_in), 
        mf_reflect(reflectance, &fresnel, d, fullweight, DIR_UP, true),
        mf_refract(tranmisttance, d, ior, ior_in, fullweight, DIR_UP)
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
    const Spectrum              fullweight; /**< Default weight for microfacet layer. */
    const FresnelDielectric     fresnel;    /**< Fresnel term. */
    const MicroFacetReflection  mf_reflect; /**< Microfacet reflection model. */
    const MicroFacetRefraction  mf_refract; /**< Microfacet refraction model. */
};
