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

#include "bidirpath.h"

//! @brief  This integrator only traces rays from light sources instead of cameras.
/**
 * Light tracing is only part of bi-directional path tracing. There is no separate
 * algorithm implemented in this integration, but only one parameter to drive the
 * differences.
 * The actual implementation is actually hidden in bidirectional path tracing integrator.
 */
class LightTracing : public BidirPathTracing{
public:
    DEFINE_RTTI(LightTracing, Integrator );

    //! @brief  Constructor simply setup light_tracing_only parameter.
    LightTracing(){
        light_tracing_only = true;
    }

    //! @brief  Whether to refreshtile in Blender user interface.
    //!
    //! Since there is no specific order of rendering, there is no way to support live refresh.
    //!
    //! @return     Whether there is tile refresh supported in Blender.
    bool NeedRefreshTile() const override{
        return false; 
    }

    //! @brief  Need full target real time update.
    bool NeedFullTargetRealtimeUpdate() const override{
        return true;
    }
};
