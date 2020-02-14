/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include <algorithm>
#include "light_entity.h"
#include "shape/quad.h"
#include "shape/disk.h"
#include "core/primitive.h"

IMPLEMENT_RTTI(PointLightEntity);
IMPLEMENT_RTTI(DirLightEntity);
IMPLEMENT_RTTI(SpotLightEntity);
IMPLEMENT_RTTI(SkyLightEntity);
IMPLEMENT_RTTI(AreaLightEntity);

void PointLightEntity::Serialize( IStreamBase& stream ){
    stream >> m_light->m_light2world;
    auto energy = 0.0f;
    stream >> energy;
    stream >> m_light->intensity;
    m_light->intensity *= energy / FOUR_PI;
}

void PointLightEntity::FillScene(class Scene& scene) {
    scene.AddLight(m_light.get());
}

void DirLightEntity::Serialize(IStreamBase& stream) {
    stream >> m_light->m_light2world;

    auto energy = 0.0f;
    stream >> energy;
    stream >> m_light->intensity;
    m_light->intensity *= energy;
}

void DirLightEntity::FillScene(class Scene& scene) {
    scene.AddLight(m_light.get());
}

void SpotLightEntity::Serialize(IStreamBase& stream) {
    stream >> m_light->m_light2world;

    auto energy = 0.0f;
    stream >> energy;
    stream >> m_light->intensity;
    m_light->intensity *= energy / FOUR_PI;
    
    float cos_falloff_start, cos_total_range;
    stream >> cos_falloff_start;
    stream >> cos_total_range;
    m_light->cos_falloff_start = (float)cos(Radians(cos_falloff_start));
    m_light->cos_total_range = (float)cos(Radians(cos_total_range));
}

void SpotLightEntity::FillScene(class Scene& scene) {
    scene.AddLight(m_light.get());
}

void SkyLightEntity::Serialize(IStreamBase& stream) {
    stream >> m_light->m_light2world;
    auto energy = 1.0f;
    stream >> energy >> m_light->intensity;
    m_light->intensity *= energy;

    // the following code needs to be changed later.
    std::string filename;
    stream >> filename;
    m_light->sky.Load(filename);
}

void SkyLightEntity::FillScene(class Scene& scene) {
    scene.AddLight(m_light.get());
    scene.SetSkyLight(m_light.get());
}

void AreaLightEntity::Serialize(IStreamBase& stream) {
    stream >> m_light->m_light2world;
    const auto sx = Vector( m_light->m_light2world.matrix.m[0] , m_light->m_light2world.matrix.m[4] , m_light->m_light2world.matrix.m[8] ).Length();
    const auto sy = Vector( m_light->m_light2world.matrix.m[1] , m_light->m_light2world.matrix.m[5] , m_light->m_light2world.matrix.m[9] ).Length();
    const auto sz = Vector( m_light->m_light2world.matrix.m[2] , m_light->m_light2world.matrix.m[6] , m_light->m_light2world.matrix.m[10] ).Length();
    const auto mat = m_light->m_light2world.matrix * Scale( 1 / sx , 1 / sy , 1 / sz ).matrix;
    const auto inv_mat = Scale( sx , sy , sz ).matrix * m_light->m_light2world.invMatrix;
    m_light->m_light2world = Transform( mat , inv_mat );
    
    auto energy = 0.0f;
    stream >> energy;
    stream >> m_light->intensity;

    StringID area_type;
    stream >> area_type;
    if( area_type == SID("SQUARE") ){
        auto rect = std::make_unique<Quad>();
        float size;
        stream >> size;
        rect->SetSizeX(size);
        rect->SetSizeY(size);
        rect->SetTransform(m_light->m_light2world);
        m_light->m_shape = std::move(rect);
        
        // The 0.8 factor is purely just to stick the same power with cycles in Blender so that it is easier to compare results with Cycles.
        m_light->intensity *= 0.8f * energy / ( SQR(size) * sx * sy * PI );

    }else if( area_type == SID("RECTANGLE") ){
        auto rect = std::make_unique<Quad>();
        float sizeX, sizeY;
        stream >> sizeX >> sizeY;
        rect->SetSizeX(sizeX);
        rect->SetSizeY(sizeY);
        rect->SetTransform(m_light->m_light2world);
        m_light->m_shape = std::move(rect);

        // The 0.8 factor is purely just to stick the same power with cycles in Blender so that it is easier to compare results with Cycles.
        m_light->intensity *= 0.8f * energy / ( sizeX * sx * sizeY * sy * PI );
    }else if( area_type == SID("DISK") ){
        // scaling is not supported for now
        auto rect = std::make_unique<Disk>();
        float radius;
        stream >> radius;
        rect->SetRadius(radius);
        rect->SetTransform(m_light->m_light2world);
        m_light->m_shape = std::move(rect);

        // The 0.8 factor is purely just to stick the same power with cycles in Blender so that it is easier to compare results with Cycles.
        m_light->intensity *= 0.8f * energy / ( SQR(radius * PI) * sx * sy );
    }else{
        slog( WARNING , LIGHT , "Unrecognized area light type (%u)." , area_type.m_sid );
    }

    m_primitive = std::make_unique<Primitive>(nullptr, m_light->m_shape.get(), m_light.get());
}

void AreaLightEntity::FillScene(class Scene& scene) {
    scene.AddLight(m_light.get());
    scene.AddPrimitive(m_primitive.get());
}