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

#include "entity.h"
#include "camera/perspective.h"

//! @brief Camera entity definition.
/**
 * Camera definition in the scene. SORT supports several type of cameras, like perspective camera,
 * orthogonal camera and environment camera.
 */
class CameraEntity : public Entity{
};

//! @brief Perspective camera.
class PerspectiveCameraEntity : public CameraEntity {
public:
    DEFINE_CREATOR( PerspectiveCameraEntity , Entity , "PerspectiveCameraEntity" );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the entity. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void    Serialize(IStreamBase& stream) override;

    //! @brief  Setup the scene's camera.
    //!
    //! Setup camera for the scene.
    //!
    //! @param  scene       The scene to be filled.
    void   FillScene(class Scene& scene) override;

private:
    std::unique_ptr<PerspectiveCamera>  m_camera = std::make_unique<PerspectiveCamera>();   /**< Perspective camera. */
};