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

#include <list>
#include "integrator.h"
#include "math/interaction.h"

struct VirtualLightSource{
    SurfaceInteraction  intersect;
    Vector              wi;
    Spectrum            power;
    int                 depth;
};

//! @brief  Instant radiosity integrator.
/**
 * Instant Radiosity is a subset of directional path tracing. It has two seperate passes.
 * First pass generates virtual light sources along the path tracing from light sources.
 * Second pass will use those virtual light source to evaluate indirect illuimination.
 * Direct illumination is handled the same way in directlight integrator.
 */
class   InstantRadiosity : public Integrator{
public:
    DEFINE_RTTI( InstantRadiosity , Integrator );

    //! @brief  Evaluate the radiance along a specific direction.
    //!
    //! @param  ray             The ray to be tested with.
    //! @param  ps              Pixel sample used to evaluate Monte Carlo method.
    //! @param  scene           The scene to be evaluated.
    //! @return                 The radiance along the opposite direction that the ray points to.
    Spectrum    Li( const Ray& ray , const PixelSample& ps , const Scene& scene) const override;

    //! @brief  Preprocess before second phase happens.
    //!
    //! In preprocessing stage, numbers of virtual light sources are generated along the path tracing from light sources.
    //!
    //! @param  scene           The scene to be evaluated.
    void PreProcess( const Scene& scene ) override;

    //! @brief      Serializing data from stream
    //!
    //! @param stream    Where the serialization data comes from. Depending on different situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override {
        Integrator::Serialize( stream );
        stream >> m_nLightPathSet;
        stream >> m_nLightPaths;
        stream >> m_fMinDist;
    }

private:
    /**< light path set */
    int     m_nLightPathSet = 1;

    /**< number of paths to follow when creating virtual light sources. */
    int     m_nLightPaths   = 64;

    /**< distant threshold. */
    float   m_fMinDist      = 1.0f;
    float   m_fMinSqrDist   = 1.0f;

    /**< container for light sources. */
    std::unique_ptr<std::list<VirtualLightSource>[]>    m_pVirtualLightSources;

    Spectrum _li( const Ray& ray , const Scene& scene , bool ignoreLe = false , float* first_intersect_dist = 0 ) const;

    SORT_STATS_ENABLE( "Instant Radiosity" )
};
