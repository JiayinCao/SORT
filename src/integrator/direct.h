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

#include "integrator.h"

//! @brief  Only evaluate direct illumination.
/**
 * Comparing with whitted ray tracing , direct light requires more samples per pixel
 * and it supports soft shadow and area light.
 */
class   DirectLight : public Integrator{
public:
    DEFINE_RTTI( DirectLight , Integrator );

    //! @brief  Evaluate the radiance along a specific direction.
    //!
    //! @param  ray             The ray to be tested with.
    //! @param  ps              Pixel sample used to evaluate Monte Carlo method.
    //! @param  scene           The scene to be evaluated.
    //! @return                 The radiance along the opposite direction that the ray points to.
    Spectrum    Li( const Ray& ray , const PixelSample& ps , const Scene& scene, RenderContext& rc) const override;

    //! @brief      Serializing data from stream
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override {
        Integrator::Serialize( stream );
    }

private:
    SORT_STATS_ENABLE( "Direct Illumination" )
};
