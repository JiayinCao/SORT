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

#include "math/transform.h"

//! @brief Basic unit of objects in world.
/**
 * An entity is the very basic concept in a world. Everything, including camera, mesh, light or anything else is an
 * entity. An entity could parse itself and decouple itself into one or multiple Primitive depending how its complexity.
 * An entity itself doesn't touch rendering directly. It serves as a place where the logic operations should be performed.
 */
class Entity{
public:
    //! Empty virtual destructor
    virtual ~Entity() {}

protected:
    Transform   m_transform;
};
