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

#include "core/define.h"
#include "spectrum/spectrum.h"

//! @brief  Scattering unit is an abstraction of bxdf/bssrdf.
/**
 * Unlike other abstraction, this ScatteringUnit structure only captures the bare minimal
 * common shared information between Bxdf and Bssrdf, which is evaluation weight and
 * sampling weight.
 * SORT chooses to put more specific interfaces in Bxdf/Bssrdf class since they don't share
 * anything in common. Technically speaking, Bxdf is a delta funtcion version of Bssrdf.
 */
class ScatteringUnit{
public:
    //! @brief  Default constructor
    ScatteringUnit() = default;

    //! @brief  Empty default destructor
    virtual ~ScatteringUnit() = default;

    //! @brief  Construction by evaluation weight.
    //!
    //! Most of the time evaluation weight should be used well enough as sample weight.
    //!
    //! @param  ew      Evaluation weight of the scattering unit.
    ScatteringUnit( const Spectrum& ew ): m_evalWeight(ew) , m_sampleWeight( ew.GetIntensity() ){
    }

    //! @brief  Construbtion by evaluation weight and sample weight.
    //!
    //! @param  ew      Evaluation weight of the scattering unit.
    //! @param  sw      Sample weight of the scattering unit.
    ScatteringUnit( const Spectrum& ew , const float sw ): m_evalWeight(ew) , m_sampleWeight(sw) {
    }

    //! @brief  Evaluation weight.
    //!
    //! @return     Evaluation weight defines how much contribution it makes to the final result.
    SORT_FORCEINLINE Spectrum     GetEvalWeight() const {
        return m_evalWeight;
    }

    //! @brief  Sampling weight of the scattering unit.
    //!
    //! @return     The sampling weight that defines how much properbility it has to be picked.
    SORT_FORCEINLINE float        GetSampleWeight() const {
        return m_sampleWeight;
    }
protected:
    Spectrum    m_evalWeight    = Spectrum( 1.0f );     /**< Evaluation weight defines how much contribution it makes to the final results. */
    float       m_sampleWeight  = 1.0f;                 /**< Sample weight defines the properbility of picking the ScatteringUnit. */
};