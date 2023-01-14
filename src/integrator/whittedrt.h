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

#include "integrator.h"

//! @brief  Whitted ray tracing is the simlest integration method in SORT.
/**
 * This is just a very barebone ray tracing that supports only delta light. Since there is no delta BRDF/BTDF 
 * in SORT, there is no reflection/refraction supported in this integrator. Non-delta light will be ignored.
 * Path with more than one bounces will be simply ignored. The result of whitted ray tracing is pretty much 
 * the same with a barebone rasterizer renderer.
 */
class   WhittedRT : public Integrator{
public:
    DEFINE_RTTI( WhittedRT , Integrator );

    //! @brief  Evaluate the radiance along a specific direction.
    //!
    //! @param  ray             The ray to be tested with.
    //! @param  ps              There is no Monte-Carlo evaluation in this integrator, this will be ignored.
    //! @param  scene           The scene to be evaluated.
    //! @return                 The radiance along the opposite direction that the ray points to.
    virtual Spectrum    Li( const Ray& ray , const PixelSample& ps , const Scene& scene, RenderContext& rc) const;

private:
    SORT_STATS_ENABLE( "Whitted Ray Tracing" )
};
