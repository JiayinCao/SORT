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

#include <string.h>
#include "material.h"
#include "matmanager.h"
#include "core/log.h"
#include "core/globalconfig.h"
#include "core/strid.h"
#include "osl_system.h"
#include "scatteringevent/scatteringevent.h"
#include "scatteringevent/bsdf/lambert.h"

void Material::BuildMaterial(){
    const auto message = "Build Material '" + m_name + "'";
    SORT_PROFILE(message);

    static constexpr auto surface_shader_root = "shader SORT_Surface_Shader( closure color Surface = color(0) ){\nCi = Surface;\n}";
    static constexpr auto surface_volume_root = "shader SORT_Volume_Shader( closure color Volume = color(0) ){\nCi = Volume;\n}";

    const auto output_node_name = "ShaderOutput_" + m_name;

    auto tried_building_surface_shader = false;
    auto tried_building_volume_shader = false;

    auto build_shader_type = [&](const OSL_ShaderData& shader_data , bool& shader_valid , bool& trying_building_shader_type , OSL::ShaderGroupRef& shader_ref ) {
        // Build surface shader
        if (shader_valid) {
            shader_ref = BeginShaderGroup(m_name);

            // build all shader nodes
            for (const auto& shader : shader_data.m_sources)
                BuildShader(shader.source, shader.name, shader.name, m_name);

            // root surface shader
            BuildShader(surface_shader_root, output_node_name, output_node_name, m_name);

            // connecting surface shader nodes
            for (const auto& connection : shader_data.m_connections) {
                if (!ConnectShader(connection.source_shader, connection.source_property, connection.target_shader, connection.target_property))
                    m_surface_shader_valid = false;
            }

            shader_valid &= EndShaderGroup();

            if (shader_valid) {
                const auto message = "Optimizing surface shader in material '" + m_name + "'";
                SORT_PROFILE(message);
                OptimizeShader(shader_ref.get());
            }

            trying_building_shader_type = true;
        }
    };

    // build surface shader
    build_shader_type(m_surface_shader_data, m_surface_shader_valid, tried_building_surface_shader, m_surface_shader);

    // build volume shader
    build_shader_type(m_volume_shader_data, m_volume_shader_valid, tried_building_volume_shader, m_volume_shader);

    auto build_shader_succesfully = true;
    if (tried_building_surface_shader && !m_surface_shader_valid ) {
        slog(WARNING, MATERIAL, "Build surface shader of material %s unsuccessfully.", m_name.c_str());
        build_shader_succesfully = false;
    }
    if (tried_building_volume_shader && !m_volume_shader_valid) {
        slog(WARNING, MATERIAL, "Build volume shader of material %s unsuccessfully.", m_name.c_str());
        build_shader_succesfully = false;
    }

    if (build_shader_succesfully)
        slog(INFO, MATERIAL, "Build material %s successfully.", m_name.c_str());
    else
		slog(WARNING, MATERIAL, "Build material %s unsuccessfully.", m_name.c_str());
}

void Material::Serialize(IStreamBase& stream){
    stream >> m_name;
    m_matID = StringID(m_name);

    const auto message = "Parsing Material '" + m_name + "'";
    SORT_PROFILE(message.c_str());

    auto parse_shader_type = [&](OSL_ShaderData& shader_data, bool& is_shader_valid) {
        is_shader_valid = true;

        // parse surface shader
        do {
            ShaderSource shader_source;
            stream >> shader_source.name >> shader_source.type;

            if (shader_source.name.empty()) {
                if (shader_source.type == "invalid_shader")
                    is_shader_valid = false;
                else if (shader_source.type != "shader_done")
                    sAssertMsg(false, RESOURCE, "Serialization is broken.");
                break;
            }

            // it seems that shader name is a global unit, same shader name may conflict even if they are in different shader group
            shader_source.name = shader_source.name;

            std::vector<std::string> paramDefaultValues;
            auto parameter_cnt = 0u;
            stream >> parameter_cnt;
            for (auto j = 0u; j < parameter_cnt; ++j) {
                std::string defaultValue;
                stream >> defaultValue;
                paramDefaultValues.push_back(defaultValue);
            }

            // construct the shader source code
            shader_source.source = MatManager::GetSingleton().ConstructShader(shader_source.name, shader_source.type, paramDefaultValues);

            shader_data.m_sources.push_back(shader_source);
        } while (true);

        if (is_shader_valid) {
            auto connection_cnt = 0u;
            stream >> connection_cnt;
            for (auto i = 0u; i < connection_cnt; ++i) {
                ShaderConnection connection;
                stream >> connection.source_shader >> connection.source_property;
                stream >> connection.target_shader >> connection.target_property;
                shader_data.m_connections.push_back(connection);
            }
        }
    };

    parse_shader_type(m_surface_shader_data, m_surface_shader_valid);
    parse_shader_type(m_volume_shader_data, m_volume_shader_valid);

    stream >> m_hasTransparentNode;
}

void Material::UpdateScatteringEvent( ScatteringEvent& se ) const {
    if( !g_noMaterial && m_surface_shader_valid)
        ExecuteShader(m_surface_shader.get() , se );
    else
        se.AddBxdf(SORT_MALLOC(Lambert)(WHITE_SPECTRUM, FULL_WEIGHT, DIR_UP));
}

void Material::UpdateMediumStack(MediumStack& ms) const {
    if (m_volume_shader_valid)
        ExecuteShader(m_volume_shader.get(), ms);
}