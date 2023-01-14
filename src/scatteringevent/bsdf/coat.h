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
#include "core/sassert.h"
#include "microfacet.h"
#include "scatteringevent/scatteringevent.h"

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeCoat, "coat")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeCoat, Tsl_closure, closure)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeCoat, Tsl_float, roughness)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeCoat, Tsl_float, ior)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeCoat, Tsl_float3, sigma)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeCoat, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeCoat)

//! @brief Coat BRDF.
/**
 * 'Arbitrarily Layered Micro-Facet Surfaces'
 * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.160.2363&rep=rep1&type=pdf
 *
 * 'Rendering Layered Materials'
 * https://ivokabel.github.io/2018/05/15/rendering-layered-materials.html
 *
 * This BRDF is a simplified version of the above mentioned work. Only one layer of coating is supported.
 * The coating layer is hard-coded with Microfacet BRDF model, which is not configurable. However, the underlying layer could be configurable by attaching
 * other BXDF. If nothing is attached, a 'null' material, which is totally based will be at the bottom.
 * This BRDF doesn't work with transmissive materials like glass.
 * Due to the assumption made in the algorithm, there is no support for coating BSSRDF materials since the exit position and enter position are different,
 * leading to failure of some assumptions in the original algorithm, for which reason, SORT will fall back to Lambert for all BSSRDF materials under the coating
 * layer.
 */
class Coat : public Bxdf{
public:
    //! Constructor.
    //!
    //! @param params           Parameter set.
    //! @param weight           Weight of the BXDF.
    //! @param bottom           Bottom layer scattering event.
    Coat(RenderContext& rc, const ClosureTypeCoat& params , const Spectrum& weight , const ScatteringEvent* bottom );

    //! Evaluate the BRDF.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum F( const Vector& wo , const Vector& wi ) const override;


    //! @brief Importance sampling for the fresnel BRDF.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The Evaluated BRDF value.
    Spectrum Sample_F( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override;

    //! @brief Evaluate the pdf of an exitant direction given the Incident direction.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float Pdf( const Vector& wo , const Vector& wi ) const override;

private:
    const float thickness ;     /**< Thickness of the layer. */
    const float ior ;           /**< Index of refraction out side the surface where the normal points. */
    const Spectrum sigma;       /**< Sigma of the BRDF model. */

    const GGX                   ggx;            /**< Using GGX as default NDF. */
    const FresnelDielectric     fresnel;        /**< Fresnel term. */
    const Spectrum              coat_weight;    /**< Default weight for coated layer. */
    const MicroFacetReflection  coat;           /**< Using Microfacet as coated layer. */
    const ScatteringEvent*      bottom;         /**< Bottom layer. */

    Spectrum f( const Vector& wo , const Vector& wi ) const override{
        sAssertMsg( false , MATERIAL , "This function shouldn't be called" );
        return 0.0f;
    }
    float pdf( const Vector& wo , const Vector& wi ) const override{
        sAssertMsg( false , MATERIAL , "This function shouldn't be called" );
        return 0.0f;
    }
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override{
        sAssertMsg( false , MATERIAL , "This function shouldn't be called" );
        return 0.0f;
    }
};
