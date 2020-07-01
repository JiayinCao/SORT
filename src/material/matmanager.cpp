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

ShaderArgumentTypeEnum shader_arg_type_from_sid(const StringID id) {
    if (id == "closure"_sid)
        return TSL_TYPE_CLOSURE;
    else if (id == "vector"_sid)
        return TSL_TYPE_FLOAT3;
    else if (id == "float"_sid)
        return TSL_TYPE_FLOAT;
    return TSL_TYPE_INVALID;
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
            // The following logic is very similar with 
            std::string shader_template_type;
            stream >> shader_template_type;

            unsigned shader_unit_cnt = 0;
            stream >> shader_unit_cnt;

            TSL_ShaderData shader_data;

            for (auto i = 0u; i < shader_unit_cnt; ++i) {
                // parse surface shader
                ShaderSource shader_source;
                stream >> shader_source.name >> shader_source.type;

                auto parameter_cnt = 0u;
                stream >> parameter_cnt;
                for (auto j = 0u; j < parameter_cnt; ++j) {
                    ShaderParamDefaultValue default_value;
                    default_value.shader_unit_name = shader_source.name;
                    stream >> default_value.shader_unit_param_name;
                    int channel_num = 0;
                    stream >> channel_num;
                    // currently only float and float3 are supported for now
                    if (channel_num == 1) {
                        default_value.default_value.m_type = ShaderArgumentTypeEnum::TSL_TYPE_FLOAT;
                        float x;
                        stream >> x;
                        default_value.default_value.m_val.m_float = x;
                    }
                    else if (channel_num == 3) {
                        default_value.default_value.m_type = ShaderArgumentTypeEnum::TSL_TYPE_FLOAT3;
                        float x, y, z;
                        stream >> x >> y >> z;
                        default_value.default_value.m_val.m_float3 = Tsl_Namespace::make_float3(x, y, z);
                    }
                    else if (channel_num == 4) { // this is fairly ugly, but it works, I will find time to refactor it later.
                        default_value.default_value.m_type = ShaderArgumentTypeEnum::TSL_TYPE_GLOBAL;
                        m_string_container.push_back(std::string());
                        stream >> m_string_container.back();

                        default_value.default_value.m_val.m_global_var_name = m_string_container.back().c_str();
                    }

                    m_paramDefaultValues.push_back(default_value);
                }

                shader_data.m_sources.push_back(shader_source);
            }

            auto connection_cnt = 0u;
            stream >> connection_cnt;
            for (auto i = 0u; i < connection_cnt; ++i) {
                ShaderConnection connection;
                stream >> connection.source_shader >> connection.source_property;
                stream >> connection.target_shader >> connection.target_property;
                shader_data.m_connections.push_back(connection);
            }

            // compiling the shader group template
            std::unordered_map<std::string, ShaderUnitTemplate*> shader_units;
            for (const auto& shader : shader_data.m_sources)
                shader_units[shader.name] = MatManager::GetSingleton().GetShaderUnitTemplate(shader.type);

            // begin compiling shader group
            auto shader_group = BeginShaderGroup(shader_template_type);
            if (!shader_group)
                continue;

            // expose arguments in output node
            std::string root_shader_name;
            stream >> root_shader_name;
            unsigned int exposed_out_arg_cnt = 0;
            stream >> exposed_out_arg_cnt;
            for (auto i = 0u; i < exposed_out_arg_cnt; ++i) {
                std::string arg_name;
                stream >> arg_name;

                StringID data_type;
                stream >> data_type;

                // expose the shader interface
                ArgDescriptor arg;
                arg.m_name = arg_name;
                arg.m_type = shader_arg_type_from_sid(data_type);
                arg.m_is_output = true;
                shader_group->expose_shader_argument(root_shader_name, arg_name, arg);
            }

            std::string shader_group_input_name;
            stream >> shader_group_input_name;

            if (!shader_group_input_name.empty()) {
                unsigned int exposed_in_arg_cnt = 0;
                stream >> exposed_in_arg_cnt;
                for (auto i = 0u; i < exposed_in_arg_cnt; ++i) {
                    std::string arg_name;
                    stream >> arg_name;

                    StringID data_type;
                    stream >> data_type;

                    // expose the shader interface
                    ArgDescriptor arg;
                    arg.m_name = arg_name;
                    arg.m_type = shader_arg_type_from_sid(data_type);
                    arg.m_is_output = false;
                    shader_group->expose_shader_argument(shader_group_input_name, arg_name, arg);
                }
            }

            for (auto su : shader_units) {
                const auto is_root = (su.first == root_shader_name);
                const auto ret = shader_group->add_shader_unit(su.first, su.second, is_root);
                if (!ret)
                    continue;
            }

            // connect the shader units
            for (auto connection : shader_data.m_connections)
                shader_group->connect_shader_units(connection.source_shader, connection.source_property, connection.target_shader, connection.target_property);

            // update default values
            for (const auto& dv : m_paramDefaultValues)
                shader_group->init_shader_input(dv.shader_unit_name, dv.shader_unit_param_name, dv.default_value);

            // end building the shader group
            auto ret = EndShaderGroup(shader_group);

            // push it if it compiles the shader successful
            if (TSL_Resolving_Succeed == ret)
                m_shader_units[shader_template_type] = shader_group;
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