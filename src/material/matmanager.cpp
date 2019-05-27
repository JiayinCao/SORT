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

#include "matmanager.h"
#include "material/material.h"
#include "stream/stream.h"
#include "core/profile.h"
#include "core/globalconfig.h"
#include "core/log.h"
#include "bsdf/merl.h"
#include "bsdf/fourierbxdf.h"

// parse material file and add the materials into the manager
unsigned MatManager::ParseMatFile( IStreamBase& stream ){
    SORT_PROFILE("Parsing Materials");

    auto shader_source_cnt = 0u;
    stream >> shader_source_cnt;
    for (auto i = 0u; i < shader_source_cnt; ++i) {
        std::string shader_name;
        std::string shader_source;
        stream >> shader_name;
        stream >> shader_source;
        m_shaderSources[shader_name] = shader_source;
    }

    auto resource_cnt = 0u;
    stream >> resource_cnt;
    for (auto i = 0u; i < resource_cnt; ++i) {
        std::string resource_file, resource_type;
        stream >> resource_file >> resource_type;

        if (resource_type == "MerlBRDFMeasuredData")
            m_resources.push_back(std::make_unique<MerlData>());
        else if (resource_type == "FourierBRDFMeasuredData")
            m_resources.push_back(std::make_unique<FourierBxdfData>());
        else
            sAssertMsg(false, MATERIAL, "Resource type %s not supported!" , resource_type.c_str());

        m_resources.back()->LoadResource(resource_file);
    }

    const bool noMaterialSupport = g_noMaterial;
    unsigned int material_cnt = 0;
    stream >> material_cnt;
    for( unsigned int i = 0 ; i < material_cnt ; ++i ){
        auto mat = std::make_unique<Material>();

        // serialize shader
        mat->Serialize( stream );

        // compile shader
        mat->BuildMaterial();

        if( !noMaterialSupport )
            m_matPool.push_back( std::move(mat) );
    }

    return material_cnt;
}

std::string MatManager::ConstructShader(const std::string& shaderName, const std::string& shaderType, const std::vector<std::string>& paramValue) {
    std::string shader;

    // If there is no such a shader type, just return an empty string.
    if (m_shaderSources.count(shaderType) == 0) {
        slog(WARNING, MATERIAL, "Can't find OSL shader type %s.", shaderType.c_str());
        slog(WARNING, MATERIAL, "Failed to build shader %s.", shaderName.c_str());
        return shader;
    }

    const auto& shader_template = m_shaderSources[shaderType];
    int i = 0, j = 0;
    while (j < shader_template.size()) {
        const char c = shader_template[j++];
        if (i < paramValue.size() && c == '@')
            shader += paramValue[i++];
        else
            shader += c;
    }

    if( j < paramValue.size() )
        slog(WARNING, MATERIAL, "Failed to build shader %s.", shaderName.c_str());

    return shader;
}

Resource* MatManager::GetResource(int index) {
    if (index < 0 || index >= m_resources.size())
        return nullptr;
    return m_resources[index].get();
}