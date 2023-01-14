/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

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
#include "scatteringevent/scatteringevent.h"

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeDoubleSided, "double_sided")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeDoubleSided, Tsl_closure, closure0)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeDoubleSided, Tsl_closure, closure1)
DECLARE_CLOSURE_TYPE_END(ClosureTypeDoubleSided)

//! @brief DoubleSided BRDF.
/**
 * This is a BXDF wrapper shows double sided BRDF.
 * Note there may be unknown behavior if this bxdf is fed with a BTDF, which is not suggested!
 */
class DoubleSided : public Bxdf{
public:
    //! Constructor
    //! @param bxdf0        Bxdf0.
    //! @param bxdf1        Bxdf1.
    //! @param weight       Weight of the BXDF.
    DoubleSided(RenderContext& rc, const ScatteringEvent* se0 , const ScatteringEvent* se1 , const Spectrum& weight ) : Bxdf(rc, weight, (BXDF_TYPE)(BXDF_ALL_TYPES), DIR_UP, false), m_se0(se0), m_se1(se1) {}

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
    const ScatteringEvent* m_se0;      /**< Scattering event on the front side of the surface. */
    const ScatteringEvent* m_se1;      /**< Scattering event on the back side of the surface. */
};
