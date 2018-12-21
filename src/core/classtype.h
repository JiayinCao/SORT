/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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

#include "sort.h"
#include "entity/entity.h"
#include "entity/light_entity.h"
#include "entity/visual_entity.h"
#include "entity/camera_entity.h"

// The id definition in this file has to match with the ones defined in python plugin.
// Otherwise, things will go wrong in serialization process.

constexpr   unsigned int    VISUAL_ENTITY             = 1;
constexpr   unsigned int    POINT_LIGHT_ENTITY        = 2;
constexpr   unsigned int    SPOT_LIGHT_ENTITY         = 3;
constexpr   unsigned int    DIR_LIGHT_ENTITY          = 4;
constexpr   unsigned int    AREA_LIGHT_ENTITY         = 5;
constexpr   unsigned int    SKY_LIGHT_ENTITY          = 6;
constexpr   unsigned int    PERSPECTIVE_CAMERA_ENTITY = 7;

//! @brief  Instance an entity based on class id
std::shared_ptr<Entity> MakeEntity( unsigned int class_id ){
    switch( class_id ){
        case VISUAL_ENTITY:
            return std::make_shared<VisualEntity>();
        case POINT_LIGHT_ENTITY:
            return std::make_shared<PointLightEntity>();
        case SPOT_LIGHT_ENTITY:
            return std::make_shared<SpotLightEntity>();
        case DIR_LIGHT_ENTITY:
            return std::make_shared<DirLightEntity>();
        case AREA_LIGHT_ENTITY:
            return std::make_shared<AreaLightEntity>();
        case SKY_LIGHT_ENTITY:
            return std::make_shared<SkyLightEntity>();
        case PERSPECTIVE_CAMERA_ENTITY:
            return std::make_shared<PerspectiveCameraEntity>();
    }
    return nullptr;
}