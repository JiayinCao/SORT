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
#include "matmanager.h"
#include "core/log.h"
#include "core/globalconfig.h"
#include "osl_system.h"
#include "bsdf/lambert.h"

bool Material::BuildMaterial(){
    const auto message = "Build Material '" + m_name + "'";
    SORT_PROFILE(message);

    m_shader = BeginShaderGroup( m_name );
    for( const auto& shader : m_sources )
        BuildShader( shader.source , shader.name , shader.name , m_name );
    for( const auto& connection : m_connections )
        ConnectShader( connection.source_shader , connection.source_property , connection.target_shader , connection.target_property );
    m_valid = EndShaderGroup();

    if (m_valid) {
        slog(INFO, MATERIAL, "Build shader %s successfully.", m_name.c_str());
        const auto message = "Optimizing Material '" + m_name + "'";
        SORT_PROFILE(message);

        // Optimize the shader
        OptimizeShader(m_shader.get());
    }
    return m_valid;
}

void Material::Serialize(IStreamBase& stream){
    stream >> m_name;
    const auto message = "Parsing Material '" + m_name + "'";
    SORT_PROFILE(message.c_str());

    auto shader_cnt = 0u, connection_cnt = 0u;
    stream >> shader_cnt;
    for (auto i = 0u; i < shader_cnt; ++i) {
        ShaderSource shader_source;
        stream >> shader_source.name >> shader_source.type;

        // it seems that shader name is a global unit, same shader name may conflict even if they are in different shader group
        shader_source.name = m_name + "_" + shader_source.name;

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

        m_sources.push_back( shader_source );
    }

    stream >> connection_cnt;
    for (auto i = 0u; i < connection_cnt; ++i) {
        ShaderConnection connection;
        stream >> connection.source_shader >> connection.source_property;
        stream >> connection.target_shader >> connection.target_property;
        m_connections.push_back( connection );
    }
}

void Material::UpdateScattering(const class Intersection& intersect, Bsdf*& bsdf , Bssrdf*& bssrdf) const {
    bsdf = SORT_MALLOC(Bsdf)(&intersect);
    if ( !g_noMaterial && m_valid) {
        ExecuteShader(bsdf, bssrdf, intersect, m_shader.get());
    } else {
        Spectrum weight(1.0f);
        const Lambert* lambert = SORT_MALLOC(Lambert)(weight, weight, DIR_UP);
        bsdf->AddBxdf(lambert);
    }
}