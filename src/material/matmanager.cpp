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

#include "matmanager.h"
#include "material/material.h"
#include "stream/stream.h"
#include "core/profile.h"
#include "core/globalconfig.h"
#include "core/log.h"
#include "core/timer.h"
#include "scatteringevent/bsdf/merl.h"
#include "scatteringevent/bsdf/fourierbxdf.h"

void ShaderCompiling_Task::Execute(){
    TIMING_EVENT( "Compiling Shader" );
    m_material->BuildMaterial();
}

// parse material file and add the materials into the manager
unsigned MatManager::ParseMatFile( IStreamBase& stream ){
    SORT_PROFILE("Parsing Materials");

    const auto current_task = GetCurrentTask();
    const auto dependencies = current_task->GetDependencies();

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
        std::string resource_file;
        StringID resource_type;
        stream >> resource_file >> resource_type;

        if (resource_type == SID("MerlBRDFMeasuredData"))
            m_resources[resource_file] = std::make_unique<MerlData>();
        else if (resource_type == SID("FourierBRDFMeasuredData"))
            m_resources[resource_file] = std::make_unique<FourierBxdfData>();
        else
            sAssertMsg(false, MATERIAL, "Resource type not supported!");

        m_resources[resource_file]->LoadResource(resource_file);
    }

    const bool noMaterialSupport = g_noMaterial;
    unsigned int material_cnt = 0;
    stream >> material_cnt;
    for( unsigned int i = 0 ; i < material_cnt ; ++i ){
        auto mat = std::make_unique<Material>();

        // serialize shader
        mat->Serialize( stream );

        if ( LIKELY(!noMaterialSupport) ) {
            // schedule a separate task to async compile shaders
            //SCHEDULE_TASK<ShaderCompiling_Task>( "Compiling Shader" , DEFAULT_TASK_PRIORITY, dependencies ,  mat.get() );
            mat->BuildMaterial();

            // push the compiled material in the pool
            m_matPool.push_back(std::move(mat));
        }
    }

    return material_cnt;
}

std::string MatManager::LoadShaderSourceCode(const std::string& shaderName, const std::string& shaderType){
    std::string shader;

    // If there is no such a shader type, just return an empty string.
    if (m_shaderSources.count(shaderType) == 0) {
        slog(WARNING, MATERIAL, "Can't find TSL shader type %s.", shaderType.c_str());
        slog(WARNING, MATERIAL, "Failed to build shader %s.", shaderName.c_str());
        return shader;
    }

    const auto& shader_template = m_shaderSources[shaderType];
    return shader_template;
}

const Resource* MatManager::GetResource(const std::string& name) const {
    auto it = m_resources.find(name);
    if (it == m_resources.end())
        return nullptr;
    return it->second.get();
}

const MaterialBase* MatManager::CreateMaterialProxy(const MaterialBase& material) {
    m_matPool.push_back(std::move(std::make_unique<MaterialProxy>(material)));
    return m_matPool.back().get();
}