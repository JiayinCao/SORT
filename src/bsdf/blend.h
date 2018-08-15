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
#include "utility/sassert.h"
#include "microfacet.h"

//! @brief Blend BRDF.
/**
 * This BXDF blends two other bxdf based on a weight ranging from 0 to 1.
 * If no bxdf is attached, a 'null' shader will be returned.
 * If only one of the bxdf is attached, no blending will happen, this shader will just pass through the attached shader.
 */
class Blend : public Bxdf
{
public:
	//! Constructor
    //! @param bxdf0        Bxdf0.
    //! @param bxdf1        Bxdf1.
    //! @param bf           Blend factor.
    //! @param weight       Weight of the BXDF.
    Blend(const Bsdf* bxdf0 , const Bsdf* bxdf1 , float bf , const Spectrum& weight ) : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), DIR_UP, false), bxdf0(bxdf0), bxdf1(bxdf1) , weight( bf ){}
	
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
    const Bsdf* bxdf0;      /**< Bxdf 0 */
    const Bsdf* bxdf1;      /**< Bxdf 1 */
    const float weight;     /**< A linearize blend factor, 0 means bxdf0 , 1 means bxdf1. */
};
