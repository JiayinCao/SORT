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

#include "light_entity.h"

void PointLightEntity::Serialize( IStreamBase& stream ){
    stream >> m_light.light2world;
    stream >> m_light.intensity;
}

void PointLightEntity::Serialize( OStreamBase& stream ){
    stream << m_transform;
    stream << m_light.intensity;
}

void PointLightEntity::FillScene( class Scene& scene ){
    m_light.SetupScene(&scene);
    scene.AddLight(&m_light);
}