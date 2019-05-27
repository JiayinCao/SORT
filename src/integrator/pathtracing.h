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

#include "integrator.h"

//////////////////////////////////////////////////////////////////////////////////////
//  definition of direct light
class   PathTracing : public Integrator
{
public:
    DEFINE_RTTI( PathTracing , Integrator );

    // return the radiance of a specific direction
    // para 'scene' : scene containing geometry data
    // para 'ray'   : ray with specific direction
    // result       : radiance along the ray from the scene<F3>
    Spectrum    Li( const Ray& ray , const PixelSample& ps , const Scene& scene ) const override;

    // request samples
    void RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num ) override;

    // generate samples
    // para 'sampler' : the sampling method
    // para 'samples' : the samples to be generated
    // para 'ps'      : number of pixel sample to be generated
    // para 'scene'   : the scene to be rendered
    void GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const override;

    //! @brief      Serializing data from stream
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override {
        Integrator::Serialize( stream );
    }

    SORT_STATS_ENABLE( "Path Tracing" )
};
