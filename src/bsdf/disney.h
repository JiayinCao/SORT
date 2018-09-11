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

#include "bxdf.h"
#include "microfacet.h"

//! @brief Disney Principle BRDF.
/**
 * Disney Principle BRDF
 * https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
 * This implementation is based on the open-source github project from Walt Disney Animation Studios
 * https://github.com/wdas/brdf
 */
class DisneyBRDF : public Bxdf
{
public:
	//! Constructor
    //! @param basecolor        Direction-hemisphere reflection for diffuse.
    //! @param specular         Direction-hemisphere reflection for specular.
    //! @param roughnessU       Roughness along one axis.
    //! @param roughnessV       Roughness along the other axis
    //! @param weight           Weight of the BXDF
    DisneyBRDF( const Spectrum& basecolor , float subsurface , float metallic , float specular , float specularTint , float roughness ,
               float anisotropic , float sheen , float sheenTint , float clearcoat , float clearcoatGloss , const Spectrum& weight, const Vector& n , bool doubleSided = false)
        : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided) , basecolor(basecolor), subsurface(subsurface), metallic(metallic),
          specular(specular), specularTint(specularTint), roughness(roughness), anisotropic(anisotropic), sheen(sheen), sheenTint(sheenTint),
          clearcoat(clearcoat), clearcoatGloss(clearcoatGloss) {}
	
    //! Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;
	
    //! @brief Importance sampling for the fresnel brdf.
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
	const Spectrum  basecolor;      /**< The surface color, usually supplied by texture maps. */
    const float     subsurface;     /**< Controls diffuse shape using a subsurface approximation. */
    const float     metallic;       /**< The metallic-ness (0 = dielectric, 1 = metallic). This is a linear blend between two different models. The metallic model has no diffuse component and also has a tinted incident specular, equal to the base color. */
    const float     specular;       /**< Incident specular amount. This is in lieu of an explicit index-of-refraction. */
    const float     specularTint;   /**< A concession for artistic control that tints incident specular towards the base color. Grazing specular is still achromatic. */
    const float     roughness;      /**< Surface roughness, controls both diffuse and specular response. */
    const float     anisotropic;    /**< degree of anisotropy. This controls the aspect ratio of the specular highlight. (0 = isotropic, 1 = maximally anisotropic). */
    const float     sheen;          /**< An additional grazing component, primarily intended for cloth. */
    const float     sheenTint;      /**< Amount to tint sheen towards base color. */
    const float     clearcoat;      /**< A second, special-purpose specular lobe. */
    const float     clearcoatGloss; /**< controls clearcoat glossiness (0 = a “satin” appearance, 1 = a “gloss” appearance). */
};

//! @brief Clearcoat GGX NDF.
class ClearcoatGGX : public GGX
{
public:
    //! @brief Constructor
    //! @param roughness    Roughness of the surface formed by the micro facets.
    ClearcoatGGX(float roughness) : GGX(roughness, roughness) {}

    //! @brief probability of facet with specific normal (h)
    float D(const Vector& h) const override;

    //! @brief Sampling a normal respect to the NDF.
    //!
    //! @param bs   Sample holding all necessary random variables.
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f(const BsdfSample& bs) const override;

protected:
    //! @brief Smith shadow-masking function G1
    float G1(const Vector& v) const override;
};