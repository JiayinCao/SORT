/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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

//////////////////////////////////////////////////////////////////////////////////////
//  definition of direct light
class   PathTracing : public Integrator
{
public:
    DEFINE_RTTI( PathTracing , Integrator );

    //! @brief  Evalute the radiance along a specific direction.
    //!
    //! @param  ray             The ray to be tested with.
    //! @param  ps              Pixel sample used to evaluate Monte Calor method.
    //! @param  scene           The scene to be evaluted.
    //! @return                 The radiance along the opposite direction that the ray points to.
    Spectrum    Li( const Ray& ray , const PixelSample& ps , const Scene& scene ) const override;

    //! @brief      Serializing data from stream
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override {
        Integrator::Serialize( stream );
    }

    SORT_STATS_ENABLE( "Path Tracing" )

private:
    //! @brief  Evalute the radiance along a specific direction.
    //!
    //! @param  ray             The ray to be tested with.
    //! @param  ps              Pixel sample used to evaluate Monte Calor method.
    //! @param  scene           The scene to be evaluted.
    //! @param  bounces         The current number of bounces considered.
    //! @param  indirectOnly    Whether just to evalute the indirect light, default value is false.
    //! @return                 The radiance along the opposite direction that the ray points to.
    Spectrum    li( const Ray& ray , const PixelSample& ps , const Scene& scene , int bounces = 0 , bool indirectOnly = false ) const;
};
