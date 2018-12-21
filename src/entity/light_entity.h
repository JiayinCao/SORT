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

#include "entity.h"
#include "light/pointlight.h"
#include "light/distant.h"
#include "light/spot.h"
#include "light/skylight.h"
#include "light/area.h"

//! @brief Light entity definition.
/**
 * This could be any type of light supported in SORT. Common light types are point light,
 * area light, directional light, spot light and sky light.
 */
class LightEntity : public Entity{
};

//! @brief  Point light entity.
class PointLightEntity : public LightEntity {
public:
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the entity. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void    Serialize( IStreamBase& stream ) override;

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the entity. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void    Serialize( OStreamBase& stream ) override;

    //! @brief  Fill the scene with primitives.
    //!
    //! Fill the scene with the light.
    //!
    //! @param  scene       The scene to be filled.
    void   FillScene(class Scene& scene) override;

protected:
    std::shared_ptr<PointLight>  m_light = std::make_shared<PointLight>();    /**< Light in the entity. */
};

//! @brief  Spot light entity.
class SpotLightEntity : public LightEntity {
public:
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the entity. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void    Serialize(IStreamBase& stream) override;

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the entity. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void    Serialize(OStreamBase& stream) override;

    //! @brief  Fill the scene with primitives.
    //!
    //! Fill the scene with the light.
    //!
    //! @param  scene       The scene to be filled.
    void   FillScene(class Scene& scene) override;

protected:
    std::shared_ptr<SpotLight>  m_light = std::make_shared<SpotLight>();    /**< Light in the entity. */
};

//! @brief  Directional light entity.
class DirLightEntity : public LightEntity {
public:
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the entity. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void    Serialize(IStreamBase& stream) override;

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the entity. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void    Serialize(OStreamBase& stream) override;

    //! @brief  Fill the scene with primitives.
    //!
    //! Fill the scene with a point light.
    //!
    //! @param  scene       The scene to be filled.
    void   FillScene(class Scene& scene) override;

protected:
    std::shared_ptr<DistantLight>  m_light = std::make_shared<DistantLight>();    /**< Light in the entity. */
};

//! @brief  Area light entity.
class AreaLightEntity : public LightEntity {
public:
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the entity. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void    Serialize(IStreamBase& stream) override;

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the entity. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void    Serialize(OStreamBase& stream) override;

    //! @brief  Fill the scene with primitives.
    //!
    //! Base entity has nothing in it, which pops nothing in the world.
    //!
    //! @param  scene       The scene to be filled.
    void   FillScene(class Scene& scene) override;

protected:
    std::shared_ptr<AreaLight>  m_light = std::make_shared<AreaLight>();    /**< Light in the entity. */
};

//! @brief  Sky light entity.
class SkyLightEntity : public LightEntity {
public:
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the entity. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void    Serialize(IStreamBase& stream) override;

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the entity. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void    Serialize(OStreamBase& stream) override;

    //! @brief  Fill the scene with primitives.
    //!
    //! Fill the scene with a point light.
    //!
    //! @param  scene       The scene to be filled.
    void   FillScene(class Scene& scene) override;

protected:
    std::shared_ptr<SkyLight>  m_light = std::make_shared<SkyLight>();    /**< Light in the entity. */
};