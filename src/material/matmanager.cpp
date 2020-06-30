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
#include "texture/imagetexture2d.h"

namespace {
    struct ShaderResourceBinding {
        std::string resource_handle_name;
        std::string shader_resource_name;
    };
}

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

        Resource* ptr_resource = nullptr;

        if (resource_type == SID("MerlBRDFMeasuredData")) {
            m_resources[resource_file] = std::make_unique<MerlData>();
            ptr_resource = m_resources[resource_file].get();
        }
        else if (resource_type == SID("FourierBRDFMeasuredData")) {
            m_resources[resource_file] = std::make_unique<FourierBxdfData>();
            ptr_resource = m_resources[resource_file].get();
        }
        else if (resource_type == SID("Texture2D")) {
            m_resources[resource_file] = std::make_unique<ImageTexture2D>();
            ptr_resource = m_resources[resource_file].get();
        }
        
        if (!ptr_resource) {
            sAssertMsg(false, MATERIAL, "Resource type not supported!");
        } else {
            ptr_resource->LoadResource(resource_file);
        }
    }

    const bool noMaterialSupport = g_noMaterial;

    StringID material_type;
    while (true) {
        stream >> material_type;

        if (material_type == SID("End of Material"))
            break;
        else if (material_type == SID("ShaderUnitTemplate")) {
            // shader type, maybe I should use string id here.
            std::string shader_node_type;
            stream >> shader_node_type;

            // stream the shader source code
            std::string source_code;
            stream >> source_code;

            /**< Shader reousrce binding. */
            std::vector<ShaderResourceBinding>  m_shader_resources_binding;

            unsigned int shader_resources = 0;
            stream >> shader_resources;
            for (auto i = 0u; i < shader_resources; ++i) {
                ShaderResourceBinding srb;
                stream >> srb.resource_handle_name >> srb.shader_resource_name;
                m_shader_resources_binding.push_back(srb);
            }

            // compile the shader unit template
            auto shading_context = GetShadingContext();

            // allocate the shader unit template
            const auto shader_unit_template = shading_context->begin_shader_unit_template(shader_node_type);

            // bind shader resources
            for (auto sr : m_shader_resources_binding) {
                auto resource = MatManager::GetSingleton().GetResource(sr.shader_resource_name);

                if (resource->IsTexture())
                    shader_unit_template->register_texture(sr.resource_handle_name, (const void*)resource);
                else
                    shader_unit_template->register_shader_resource(sr.resource_handle_name, (const ShaderResourceHandle*)resource);
            }

            // compile the shader unit
            const auto ret = shading_context->compile_shader_unit_template(shader_unit_template, source_code.c_str());

            // indicate the end of shader unit compilation
            shading_context->end_shader_unit_template(shader_unit_template);

            // push it if it compiles the shader successful
            if( ret )
                m_shader_units[shader_node_type] = shader_unit_template;
        }
        else if (material_type == SID("ShaderGroupTemplate")) {
            // this is to be handled later.
        }
        else if (material_type == SID("Material")) {
            // allocate a new material
            auto mat = std::make_unique<Material>();

            // serialize the material
            mat->Serialize(stream);

            if (LIKELY(!noMaterialSupport)) {
                // build the material
                mat->BuildMaterial();

                // push the compiled material in the pool
                m_matPool.push_back(std::move(mat));
            }
        }
        else {
            sAssertMsg(false, MATERIAL, "Serialization is broken.");
        }
    }

    return (unsigned int)m_matPool.size();
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

ShaderUnitTemplate* MatManager::GetShaderUnitTemplate(const std::string& name_id) const {
    auto it = m_shader_units.find(name_id);
    if (it == m_shader_units.end())
        return nullptr;
    return it->second;
}