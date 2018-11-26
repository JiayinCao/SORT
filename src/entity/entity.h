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
#include "stream/stream.h"

class Scene;

//! @brief Basic unit of objects in world.
/**
 * An entity is the very basic concept in a world. Everything, including camera, mesh, light or anything else is an
 * entity. An entity could parse itself and decouple itself into one or multiple Primitive depending how its complexity.
 * An entity itself doesn't touch rendering directly. It serves as a place where the logic operations should be performed.
 */
class Entity : public SerializableObject{
public:
    //! Empty virtual destructor
    virtual ~Entity() {}

    //! @brief  Set transformation of the entity
    //!
    //! @param  transform   New transform to be set up.
    void  SetTransform( const Transform& transform ){
        m_transform = transform;
    }

    //! @brief  Get current transformation of the entity
    //!
    //! @return             Current transformation of the entity
    const Transform&    GetTransform() const{
        return m_transform;
    }

    //! @brief  Fill the scene with primitives.
    //!
    //! Some entity, like Visual entity, will commonly fill primitives in the scene. Others like Light Entity, will fill
    //! lights in the scene.
    //!
    //! @param  scene       The scene to be filled.
    virtual void        FillScene( Scene& scene ) {};

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the entity. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void        Serialize( IStreamBase& stream ) override {}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the entity. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void        Serialize( OStreamBase& stream ) override {}

protected:
    Transform   m_transform;    /**< Transform of the entity from local space to world space. */
};
