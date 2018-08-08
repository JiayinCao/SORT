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

//! @brief Phong brdf.
/**
 * A modified version of Phong BRDF
 * It is not suggested to use this BRDF in practice. Switching to more advanced BRDF, like Microfacet or AshikhmanShirley, would be much better.
 */
class Phong : public Bxdf
{
public:
	//! Constructor
    //! @param diffuse          Direction-hemisphere reflection for diffuse.
    //! @param specular         Direction-hemisphere reflection for specular.
    //! @param roughnessU       Roughness along one axis.
    //! @param roughnessV       Roughness along the other axis
    //! @param weight           Weight of the BXDF
    Phong(const Spectrum& diffuse, const Spectrum& specular, const float specularPower, const Spectrum& weight, const Vector& n , bool doubleSided = false)
        : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided) , power(specularPower) {
        // This is to make sure this modified phong model is still enerty conservative so that it is PBR, otherwise it could break some rendering algorithms.
        const float diffRatio = diffuse.GetIntensity() / (diffuse.GetIntensity() + specular.GetIntensity());
        D = diffRatio * diffuse;
        S = (1.0f - diffRatio) * specular;
    }
	
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
	Spectrum  D , S;            /**< Direction-Hemisphere reflectance and transmittance. */
    const float     power;      /**< Specular power, controlling the specular lobe. >**/
};
