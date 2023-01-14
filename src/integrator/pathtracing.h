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

#include "integrator.h"

//! @brief  The core of path tracing algorithm, the most commonly used algorithm in SORT.
/**
 * A path tracing algorithm works by tracing rays recursively to converge to the correct approximation of rendering equation.
 * It doesn't solve all corner cases well, but it is a pretty solid algorithm.
 */
class   PathTracing : public Integrator{
public:
    DEFINE_RTTI( PathTracing , Integrator );

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
        stream >> m_maxBouncesInBSSRDFPath;
    }

    SORT_STATS_ENABLE( "Path Tracing" )

private:
    // Maximum bounces supported in BSSRDF path.
    // BSSRDF solutions usually makes aggressive approximations resulting in less accuracy, multiple BSSRDF bounces will even make it worse.
    // Most importantly, it kills the performance and introduces quite some fireflies with bounces more than 2.
    int     m_maxBouncesInBSSRDFPath;

    //! @brief  Evaluate the radiance along a specific direction.
    //!
    //! @param  ray             The ray to be tested with.
    //! @param  ps              Pixel sample used to evaluate Monte Carlo method.
    //! @param  scene           The scene to be evaluated.
    //! @param  bounces         The current number of bounces considered.
    //! @param  indirectOnly    Whether just to evaluate the indirect light, default value is false.
    //! @param  bssrdfBounces   Bounces on BSSRDF surfaces in the path.
    //! @param  replaceSSS      Whether to replace SSS with lambert.
    //! @param  ms              Medium stack during radiance evaluation.
    //! @return                 The radiance along the opposite direction that the ray points to.
    Spectrum    li( const Ray& ray , const PixelSample& ps , const Scene& scene , int bounces , bool indirectOnly , 
                    int bssrdfBounces , bool replaceSSS , MediumStack& ms , RenderContext& rc) const;
};
