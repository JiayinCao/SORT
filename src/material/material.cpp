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

#include "material.h"
#include "managers/matmanager.h"

void Material::Serialize(IStreamBase& stream){
    stream >> m_name;
    auto message = "Parsing Material '" + m_name + "'";
    SORT_PROFILE(message.c_str());

    auto shader_cnt = 0u, connection_cnt = 0u;
    stream >> shader_cnt;
    for (auto i = 0u; i < shader_cnt; ++i) {
        std::string node_name, node_type;
        stream >> node_name >> node_type;

        std::vector<std::string> paramDefaultValues;
        auto parameter_cnt = 0u;
        stream >> parameter_cnt;
        for (auto j = 0u; j < parameter_cnt; ++j) {
            std::string defaultValue;
            stream >> defaultValue;
            paramDefaultValues.push_back(defaultValue);
        }

        // construct the shader source code
        const auto shader_source = MatManager::GetSingleton().ConstructShader(node_type, paramDefaultValues);
    }

    stream >> connection_cnt;
    for (auto i = 0u; i < connection_cnt; ++i) {
        std::string source_shader, source_socket;
        std::string target_shader, target_socket;
        stream >> source_shader >> source_socket;
        stream >> target_shader >> target_socket;
    }
}

// temporary solution before OSL shader is constructed
#include "bsdf/lambert.h"
Bsdf* Material::GetBsdf(const class Intersection* intersect) const {
    Bsdf* bsdf = SORT_MALLOC(Bsdf)(intersect);
    Spectrum weight(1.0f);
    const Lambert* lambert = SORT_MALLOC(Lambert)(weight, weight, DIR_UP);
    bsdf->AddBxdf(lambert);
    return bsdf;
}