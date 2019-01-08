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

#include "light_entity.h"
#include "shape/rectangle.h"
#include "core/primitive.h"

IMPLEMENT_RTTI(PointLightEntity);
IMPLEMENT_RTTI(DirLightEntity);
IMPLEMENT_RTTI(SpotLightEntity);
IMPLEMENT_RTTI(SkyLightEntity);
IMPLEMENT_RTTI(AreaLightEntity);

void PointLightEntity::Serialize( IStreamBase& stream ){
    stream >> m_light->light2world;
    stream >> m_light->intensity;
}

void PointLightEntity::FillScene(class Scene& scene) {
    m_light->SetupScene(&scene);
    scene.AddLight(m_light.get());
}

void DirLightEntity::Serialize(IStreamBase& stream) {
    stream >> m_light->light2world;
    stream >> m_light->intensity;
}

void DirLightEntity::FillScene(class Scene& scene) {
    m_light->SetupScene(&scene);
    scene.AddLight(m_light.get());
}

void SpotLightEntity::Serialize(IStreamBase& stream) {
    stream >> m_light->light2world;
    stream >> m_light->intensity;
    float cos_falloff_start, cos_total_range;
    stream >> cos_falloff_start;
    stream >> cos_total_range;
    m_light->cos_falloff_start = (float)cos(Radians(cos_falloff_start));
    m_light->cos_total_range = (float)cos(Radians(cos_total_range));
}

void SpotLightEntity::FillScene(class Scene& scene) {
    m_light->SetupScene(&scene);
    scene.AddLight(m_light.get());
}

void SkyLightEntity::Serialize(IStreamBase& stream) {
    stream >> m_light->light2world;
    stream >> m_light->intensity;

    // the following code needs to be changed later.
    std::string filename;
    stream >> filename;
    m_light->sky.Load(filename);
}

void SkyLightEntity::FillScene(class Scene& scene) {
    m_light->SetupScene(&scene);
    scene.AddLight(m_light.get());
    scene.SetSkyLight(m_light.get());
}

void AreaLightEntity::Serialize(IStreamBase& stream) {
    stream >> m_light->light2world;
    stream >> m_light->intensity;
    auto rect = std::make_unique<Rectangle>();
    float sizeX, sizeY;
    stream >> sizeX >> sizeY;
    rect->SetSizeX(sizeX);
    rect->SetSizeY(sizeY);
    rect->SetTransform(m_light->light2world);
    m_light->shape = std::move(rect);
}

void AreaLightEntity::FillScene(class Scene& scene) {
    m_light->SetupScene(&scene);
    scene.AddLight(m_light.get());

    auto primitive = std::make_unique<Primitive>(nullptr, m_light->shape.get() , m_light.get());
    scene.AddPrimitive(std::move(primitive));
}