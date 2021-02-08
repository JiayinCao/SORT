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
#include <tsl_system.h>
#include "material.h"
#include "matmanager.h"
#include "core/log.h"
#include "core/globalconfig.h"
#include "core/strid.h"
#include "scatteringevent/scatteringevent.h"
#include "scatteringevent/bsdf/lambert.h"
#include "scatteringevent/bsdf/transparent.h"
#include "texture/imagetexture2d.h"

USE_TSL_NAMESPACE

#ifdef ENABLE_MULTI_THREAD_SHADER_COMPILATION
bool MaterialBase::IsMaterialBuilt() const{
    // std::memory_order_acquire is needed to make sure compiler doesn't do crazy out-of-order execution thing.
    return m_is_built.load(std::memory_order_acquire);
}
#endif

void Material::BuildMaterial() {
    const auto message = "Build Material '" + m_name + "'";
    SORT_PROFILE(message);

    static constexpr auto surface_shader_root = R"(
            shader SORT_Surface_Shader( in closure Surface, out closure result ){
                result = Surface;
            }
    )";
    static constexpr auto surface_volume_root = R"(
            shader SORT_Volume_Shader( in closure Volume, out closure result ){
                result = Volume;
            }
    )";

    const auto output_node_name = "ShaderOutput_" + m_name;

    auto tried_building_surface_shader = false;
    auto tried_building_volume_shader = false;

    auto build_shader_type = [&](const TSL_ShaderData& shader_data, const char* root_shader, const std::string prefix, bool& shader_valid, bool& trying_building_shader_type, ShaderUnitContainer& shader_units, std::shared_ptr<Tsl_Namespace::ShaderInstance>& shader_instance) {
        // Build surface shader
        if (shader_valid) {
            shader_valid = false;
            trying_building_shader_type = true;
    
            for (const auto& shader : shader_data.m_sources)
                shader_units[shader.name] = MatManager::GetSingleton().GetShaderUnitTemplate(shader.type);
    
            // build the root shader
            auto context = GetShadingContext();
            const auto root_shader_name = prefix + output_node_name;
            if(auto shader_unit_template = context->begin_shader_unit_template(root_shader_name)){
                // register tsl global
                TslGlobal::shader_unit_register(shader_unit_template.get());

                // compile the root shader
                const auto ret = shader_unit_template->compile_shader_source(root_shader);
                if (!ret)
                    return;

                // indicate the shader unit is done
                context->end_shader_unit_template(shader_unit_template.get());

                shader_units[root_shader_name] = shader_unit_template;
            } else {
                return;
            }
    
            // begin compiling shader group
            auto shader_group = context->begin_shader_group_template(prefix + m_name);
            if (!shader_group)
                return;
            
            // register tsl global
            TslGlobal::shader_unit_register(shader_group.get());

            for (auto su : shader_units) {
                const auto is_root = su.first == root_shader_name;
                const auto ret = shader_group->add_shader_unit(su.first, su.second, is_root);
                if (!ret)
                    return;
            }
    
            // connect the shader units
            for (auto connection : shader_data.m_connections) {
                const auto target_shader = connection.target_shader == output_node_name ? prefix + output_node_name : connection.target_shader;
                shader_group->connect_shader_units(connection.source_shader, connection.source_property, target_shader, connection.target_property);
            }
    
            // expose the shader interface
            shader_group->expose_shader_argument(root_shader_name, "result", true, "out_bxdf");
    
            // update default values
            for (const auto& dv : m_paramDefaultValues)
                shader_group->init_shader_input(dv.shader_unit_name, dv.shader_unit_param_name, dv.default_value);
            
            // end building the shader group
            auto ret = context->end_shader_group_template(shader_group.get());
            if (TSL_Resolving_Status::TSL_Resolving_Succeed != ret)
                return;
    
            shader_instance = shader_group->make_shader_instance();
            ret = shader_instance->resolve_shader_instance();
            if (TSL_Resolving_Status::TSL_Resolving_Succeed != ret)
                return;
    
            shader_valid = true;
        }
    };

    // build surface shader
    build_shader_type(m_surface_shader_data, surface_shader_root, "Surface", m_surface_shader_valid, tried_building_surface_shader, m_surface_shader_units, m_surface_shader);

    // build volume shader
    build_shader_type(m_volume_shader_data, surface_volume_root, "Volume", m_volume_shader_valid, tried_building_volume_shader, m_volume_shader_units, m_volume_shader);

    // if there is volume shader, but no surface shader, a special transparent material will be applied automatically
    // this will make the shader authoring a lot easier.
    if (!m_surface_shader_valid && m_volume_shader_valid && !tried_building_surface_shader)
        m_special_transparent = true;

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

    // fake transparent mode if necessary
    if (m_special_transparent)
        m_hasTransparentNode = true;

#ifdef ENABLE_MULTI_THREAD_SHADER_COMPILATION
    // indicate the material has been built
    m_is_built.store(true, std::memory_order_release);
#endif
}

void Material::Serialize(IStreamBase& stream){
    stream >> m_name;
    m_matID = StringID(m_name);

    const auto message = "Parsing Material '" + m_name + "'";
    SORT_PROFILE(message.c_str());

    auto parse_shader_type = [&](TSL_ShaderData& shader_data, bool& is_shader_valid) {
        is_shader_valid = true;

        unsigned shader_unit_cnt = 0;
        stream >> shader_unit_cnt;

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
                    float x;
                    stream >> x;
                    default_value.default_value = x;
                }
                else if (channel_num == 3) {
                    float x, y, z;
                    stream >> x >> y >> z;
                    default_value.default_value = Tsl_Namespace::make_float3(x, y, z);
                }
                else if (channel_num == 4) { // this is fairly ugly, but it works, I will find time to refactor it later.
                    std::string str;
                    stream >> str;
                    default_value.default_value = make_tsl_global_ref(str);
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
    };

    StringID surface_shader_tag;
    stream >> surface_shader_tag;
    if(surface_shader_tag == "Surface Shader"_sid)
        parse_shader_type(m_surface_shader_data, m_surface_shader_valid);

    // temporary for now
    StringID volume_shader_tag;
    stream >> volume_shader_tag;
    if(volume_shader_tag == "Volume Shader"_sid)
        parse_shader_type(m_volume_shader_data, m_volume_shader_valid);

    stream >> m_hasTransparentNode;
    stream >> m_hasSSSNode;

    stream >> m_volumeStep;
    stream >> m_volumeStepCnt;
}

void Material::UpdateScatteringEvent( ScatteringEvent& se, RenderContext& rc ) const {
    // all lambert surfaces if the render is in no material mode.
    if (UNLIKELY(g_noMaterial || ( !m_surface_shader_valid && !m_special_transparent ))) {
        se.AddBxdf(SORT_MALLOC(rc.m_memory_arena, Lambert)(rc, WHITE_SPECTRUM, FULL_WEIGHT, DIR_UP));
        return;
    }

    if( m_surface_shader_valid )
        ExecuteSurfaceShader(m_surface_shader.get() , se , rc);
    else if( m_special_transparent )
        se.AddBxdf(SORT_MALLOC(rc.m_memory_arena, Transparent)(rc));
}

void Material::UpdateMediumStack( const MediumInteraction& mi , const SE_Interaction flag , MediumStack& ms, RenderContext& rc ) const {
    if (m_volume_shader_valid)
        ExecuteVolumeShader(m_volume_shader.get(), mi, ms, flag, this, rc);
}

void Material::EvaluateMediumSample(const MediumInteraction& mi, MediumSample& ms) const {
    if (m_volume_shader_valid)
        EvaluateVolumeSample(m_volume_shader.get(), mi, ms);
}

void MaterialProxy::UpdateScatteringEvent(ScatteringEvent& se, RenderContext& rc) const {
    return m_material.UpdateScatteringEvent(se, rc);
}

void MaterialProxy::UpdateMediumStack(const MediumInteraction& mi, const SE_Interaction flag, MediumStack& ms, RenderContext& rc) const {
    return m_material.UpdateMediumStack(mi, flag, ms, rc);
}

void MaterialProxy::EvaluateMediumSample(const MediumInteraction& mi, MediumSample& ms) const {
    return m_material.EvaluateMediumSample(mi, ms);
}

StringID  MaterialProxy::GetUniqueID() const {
    // Hopefully there is no conflict with the hash key constructed by the name of the material.
    const std::uintptr_t ret = (const std::uintptr_t)this;
    return (StringID)(ret);
}

Spectrum MaterialProxy::EvaluateTransparency(const SurfaceInteraction& intersection) const {
    return m_material.EvaluateTransparency(intersection);
}

bool MaterialProxy::HasTransparency() const {
    return m_material.HasTransparency();
}

bool MaterialProxy::HasSSS() const {
    return m_material.HasSSS();
}

bool MaterialProxy::HasVolumeAttached() const {
    return m_material.HasVolumeAttached();
}

float MaterialProxy::GetVolumeStep() const {
    return m_material.GetVolumeStep();
}

unsigned int MaterialProxy::GetVolumeStepCnt() const {
    return m_material.GetVolumeStepCnt();
}