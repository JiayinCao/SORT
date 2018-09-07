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

//! @brief Phong BRDF.
/**
 * A modified version of Phong BRDF, following is the exact algorithm that is used in this BRDF with slightly modification in term of exposed parameters.
 * 'Using the modified Phong reflection model for physically based rendering'
 * http://mathinfo.univ-reims.fr/IMG/pdf/Using_the_modified_Phong_reflectance_model_for_Physically_based_rendering_-_Lafortune.pdf
 * Following is the original paper proposed Phong shading model, which is not exactly PBS due to its lack of energy conservation
 * 'Illumination for Computer Generated Pictures'
 * http://www.cs.northwestern.edu/~ago820/cs395/Papers/Phong_1975.pdf
 *
 * Although, this modified Phong model is a PBR based BRDF, its non-PBR parameter interface really limits its application.
 * As a matter of fact, this is no guarantee that the original modified BRDF is energy conservative if DiffuseColor combined with SpecularColor is larger 1.
 * It was explicitly mentioned in the paper that the sum of diffuse and specular has to be smaller to be physically based. However, such an implicit constraint
 * is not very user-friendly to artist because sometimes, especially when it is driven by textures, it is not quite easy to obey the rule.
 * For this very reason, SORT provides a lerp factor forcing this BRDF to be smaller than 1 to keep it PBR based.
 * It is strongly not suggested to use this BRDF in practice. Switching to more advanced BRDF, like Microfacet or AshikhmanShirley, would be a much better idea.
 * Modern game engines have already deprecated this model for several years ever since they switched to physically based shading model.
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
        : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided) , D(diffuse), S(specular), power(specularPower), 
          diffRatio(diffuse.GetIntensity()/(diffuse.GetIntensity()+specular.GetIntensity())) {
        const auto combined = D + S;
        sAssert(combined.GetR() <= 1.0f, MATERIAL);
        sAssert(combined.GetG() <= 1.0f, MATERIAL);
        sAssert(combined.GetB() <= 1.0f, MATERIAL);
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
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override;
    
    //! @brief Evaluate the pdf of an exitant direction given the Incident direction.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float pdf( const Vector& wo , const Vector& wi ) const override;
    
private:
	const Spectrum  D , S;      /**< Direction-Hemisphere reflectance and transmittance. */
    const float     power;      /**< Specular power, controlling the specular lobe. */
    const float     diffRatio;  /**< Real ratio of diffuse term. */
};
