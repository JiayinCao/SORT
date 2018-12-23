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

#include "camera_entity.h"
#include "core/scene.h"
#include "system.h"

IMPLEMENT_CREATOR(PerspectiveCameraEntity);

extern System g_System;

void PerspectiveCameraEntity::Serialize(IStreamBase& stream) {
    stream >> m_camera->m_eye;
    stream >> m_camera->m_up;
    stream >> m_camera->m_target;
    stream >> m_camera->m_lensRadius;
    stream >> m_camera->m_sensorW >> m_camera->m_sensorH >> m_camera->m_aspectFit;
    stream >> m_camera->m_aspectRatioW >> m_camera->m_aspectRatioH;
    stream >> m_camera->m_fov;

    m_camera->SetImageSensor(g_System.GetImageSensor());
    m_camera->PreProcess();
}

void PerspectiveCameraEntity::FillScene(class Scene& scene) {
    scene.SetupCamera(m_camera);
}