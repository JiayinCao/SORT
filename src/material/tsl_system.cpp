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
#include <vector>
#include <shading_context.h>
#include "tsl_system.h"
#include "texture_system.h"
#include "core/profile.h"
#include "core/thread.h"
#include "core/globalconfig.h"
#include "math/interaction.h"
#include "scatteringevent/scatteringevent.h"
#include "medium/medium.h"
#include "core/log.h"
#include "material.h"

using namespace Tsl_Namespace;

IMPLEMENT_TSLGLOBAL_BEGIN()
IMPLEMENT_TSLGLOBAL_VAR(float3, normal)
IMPLEMENT_TSLGLOBAL_END()

class TSL_ShadingSystemInterface : public ShadingSystemInterface {
public:
    void* allocate(unsigned int size) const override {
        return SORT_MALLOC_ARRAY(char, size);
    }

    void catch_debug(const DEBUG_LEVEL level, const char* error) const override {
        slog(WARNING, GENERAL, error);
    }
};

// Tsl shading system
static std::vector<ShadingContext*>    g_contexts;

Tsl_Namespace::ShadingContext* GetShadingContext() {
    return g_contexts[ThreadId()];
}

ShaderGroupTemplate*    BeginShaderGroup(const std::string& group_name) {
    return g_contexts[ThreadId()]->begin_shader_group_template(group_name);
}

bool EndShaderGroup(ShaderGroupTemplate* sg) {
    return g_contexts[ThreadId()]->end_shader_group_template(sg);
}

bool ResolveShaderInstance(ShaderInstance* si) {
    return g_contexts[ThreadId()]->resolve_shader_instance(si);
}

void ExecuteSurfaceShader( Tsl_Namespace::ShaderInstance* shader , ScatteringEvent& se ){
    const SurfaceInteraction& intersection = se.GetInteraction();
    TslGlobal global;
    global.normal = make_float3(intersection.normal.x, intersection.normal.y, intersection.normal.z);

    // shader execution
    ClosureTreeNodeBase* closure = nullptr;
    auto raw_function = (void(*)(ClosureTreeNodeBase**, TslGlobal*))shader->get_function();
    raw_function(&closure, &global);

    // parse the surface shader
    ProcessSurfaceClosure(closure, Tsl_Namespace::make_float3(1.0f, 1.0f, 1.0f) , se );
}

void ExecuteVolumeShader(Tsl_Namespace::ShaderInstance* shader, const MediumInteraction& mi, MediumStack& ms, const SE_Interaction flag, const MaterialBase* material ) {
    //const SurfaceInteraction& intersection = se.GetInteraction();
    TslGlobal global;
    //global.normal = make_float3(intersection.normal.x, intersection.normal.y, intersection.normal.z);

     // objdata points to world to local volume transform
    // auto thread_info = reinterpret_cast<SORTTextureThreadInfo*>(g_textureSystem.get_perthread_info());
    // if (mi.mesh)
    //     thread_info->mesh = mi.mesh;

    // shader execution
    ClosureTreeNodeBase* closure = nullptr;
    auto raw_function = (void(*)(ClosureTreeNodeBase**, TslGlobal*))shader->get_function();
    raw_function(&closure, &global);

    ProcessVolumeClosure(closure, Tsl_Namespace::make_float3(1.0f, 1.0f, 1.0f), ms, flag, material, mi.mesh);
}

void EvaluateVolumeSample(Tsl_Namespace::ShaderInstance* shader, const MediumInteraction& mi, MediumSample& ms) {
    TslGlobal global;

    ClosureTreeNodeBase* closure = nullptr;
    auto raw_function = (void(*)(ClosureTreeNodeBase**, TslGlobal*))shader->get_function();
    raw_function(&closure, &global);

    EvaluateVolumeSample(closure, Tsl_Namespace::make_float3(1.0f, 1.0f, 1.0f), ms);
}

Spectrum EvaluateTransparency(Tsl_Namespace::ShaderInstance* shader , const SurfaceInteraction& intersection ){
    TslGlobal global;
    global.normal = make_float3(intersection.normal.x, intersection.normal.y, intersection.normal.z);

    // shader execution
    ClosureTreeNodeBase* closure = nullptr;
    auto raw_function = (void(*)(ClosureTreeNodeBase**, TslGlobal*))shader->get_function();
    raw_function(&closure, &global);

    // parse the surface shader
    const auto opacity = ProcessOpacity(closure, Tsl_Namespace::make_float3(1.0f, 1.0f, 1.0f));
    return Spectrum(1.0f - opacity).Clamp(0.0f, 1.0f);
}

void CreateTSLThreadContexts(){
    auto& shading_system = ShadingSystem::get_instance();
    shading_system.register_shadingsystem_interface(std::make_unique<TSL_ShadingSystemInterface>());

    // register all closures
    RegisterClosures();

    // register tsl global, this may need to be per-shader data later for more flexibility
    TslGlobal::RegisterGlobal(shading_system);

    // allocate shading context for each thread
    g_contexts.resize( g_threadCnt );
    for (auto i = 0u; i < g_threadCnt; ++i)
        g_contexts[i] = shading_system.make_shading_context();
}

 void DestroyTSLThreadContexts(){
     // to be implemented
 }