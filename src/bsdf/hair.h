/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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
#include "core/sassert.h"

#define PMAX        3

//! @brief Hair BRDF.
/**
 * 'The Implementation of a Hair Scattering Model' by Matt Pharr.
 * https://www.pbrt.org/hair.pdf
 */
class Hair : public Bxdf{
public:
	//! Constructor
    //!
    //! @param absorption       Absorption coefficient.
    //! @param lRoughness       Longtitudinal Roughness.
    //! @param aRoughness       Azimuthal Roughness.
    //! @param ior              Index of Refraction inside hair.
    //! @param weight           Weight of the BXDF
    Hair(const Spectrum& absorption, const float lRoughness, const float aRoughness, const float ior, const Spectrum& weight, bool doubleSided = false);
	
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
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override;
    
    //! @brief Evaluate the pdf of an exitant direction given the Incident direction.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float pdf( const Vector& wo , const Vector& wi ) const override;
    
private:
    const Spectrum  m_sigma;            /**< Absorption coefficient. */
    const float     m_lRoughness;       /**< Longtitudinal roughness. */
    const float     m_aRoughness;       /**< Azimuthal roughness. */
    const float     m_eta;              /**< Index of refraction inside the hair volume. */

    float           m_v[PMAX+1];          /**< Some pre-calculated cached data. */
    float           m_cos2kAlpha[PMAX];   /**< Some pre-calculated cached data, cos( 2 ^ k ). */
    float           m_sin2kAlpha[PMAX];   /**< Some pre-calculated cached data, sin( 2 ^ k ). */
    float           m_scale;              /**< Azimuhthal logisitic scale factor. */
    float           m_etaSqr;             /**< Squared eta. */
};
