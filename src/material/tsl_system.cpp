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
#include "shading_callback.h"
#include "core/profile.h"
#include "core/thread.h"
#include "core/globalconfig.h"
#include "math/interaction.h"
#include "scatteringevent/scatteringevent.h"
#include "medium/medium.h"
#include "core/log.h"
#include "material.h"
#include "texture/imagetexture2d.h"

using namespace Tsl_Namespace;

IMPLEMENT_TSLGLOBAL_BEGIN()
IMPLEMENT_TSLGLOBAL_VAR(float3, uvw)          // UV coordinate, W is preserved for now.
IMPLEMENT_TSLGLOBAL_VAR(float3, position)     // this is world space position
IMPLEMENT_TSLGLOBAL_VAR(float3, normal)       // this is world space normal
IMPLEMENT_TSLGLOBAL_VAR(float3, gnormal)      // this is world space geometric normal
IMPLEMENT_TSLGLOBAL_VAR(float3, I)            // this is world space input direction
IMPLEMENT_TSLGLOBAL_VAR(float, density)       // volume density
IMPLEMENT_TSLGLOBAL_END()

class TSL_ShadingSystemInterface : public ShadingSystemInterface {
public:
    void*   allocate(unsigned int size) const override {
        return SORT_MALLOC_ARRAY(char, size);
    }

    void    catch_debug(const TSL_DEBUG_LEVEL level, const char* error) const override {
        slog(WARNING, GENERAL, error);
    }

    void    sample_2d(const void* texture, float u, float v, float3& color) const override {
        auto resource = (const Resource*)texture;
        auto sort_texture = dynamic_cast<const ImageTexture2D*>(resource);
        auto ret = sort_texture->GetColorFromUV(u, v);
        color = make_float3(ret.x, ret.y, ret.z);
    }

    void    sample_alpha_2d(const void* texture, float u, float v, float& alpha) const override {
        auto resource = (const Resource*)texture;
        auto sort_texture = dynamic_cast<const ImageTexture2D*>(resource);
        alpha = sort_texture->GetAlphaFromtUV(u, v);
    }
};

// Tsl shading system
static std::vector<std::shared_ptr<ShadingContext>>    g_contexts;

Tsl_Namespace::ShadingContext* GetShadingContext() {
    return g_contexts[ThreadId()].get();
}

std::shared_ptr<ShaderGroupTemplate> BeginShaderGroup(const std::string& group_name) {
    return g_contexts[ThreadId()]->begin_shader_group_template(group_name);
}

TSL_Resolving_Status EndShaderGroup(ShaderGroupTemplate* sg) {
    return g_contexts[ThreadId()]->end_shader_group_template(sg);
}

TSL_Resolving_Status ResolveShaderInstance(ShaderInstance* si) {
    return g_contexts[ThreadId()]->resolve_shader_instance(si);
}

void ExecuteSurfaceShader( Tsl_Namespace::ShaderInstance* shader , ScatteringEvent& se ){
    const SurfaceInteraction& intersection = se.GetInteraction();
    TslGlobal global;
    global.uvw = make_float3(intersection.u, intersection.v, 0.0f);
    global.normal = make_float3(intersection.normal.x, intersection.normal.y, intersection.normal.z);
    global.I = make_float3(intersection.view.x, intersection.view.y, intersection.view.z);

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

    // shader execution
    ClosureTreeNodeBase* closure = nullptr;
    auto raw_function = (void(*)(ClosureTreeNodeBase**, TslGlobal*))shader->get_function();
    raw_function(&closure, &global);

    ProcessVolumeClosure(closure, Tsl_Namespace::make_float3(1.0f, 1.0f, 1.0f), ms, flag, material, mi.mesh);
}

void EvaluateVolumeSample(Tsl_Namespace::ShaderInstance* shader, const MediumInteraction& mi, MediumSample& ms) {
    TslGlobal global;
    global.density = mi.mesh->SampleVolumeDensity(mi.intersect);

    ClosureTreeNodeBase* closure = nullptr;
    auto raw_function = (void(*)(ClosureTreeNodeBase**, TslGlobal*))shader->get_function();
    raw_function(&closure, &global);

    EvaluateVolumeSample(closure, Tsl_Namespace::make_float3(1.0f, 1.0f, 1.0f), ms);
}

Spectrum EvaluateTransparency(Tsl_Namespace::ShaderInstance* shader , const SurfaceInteraction& intersection ){
    TslGlobal global;
    global.uvw = make_float3(intersection.u, intersection.v, 0.0f);
    global.normal = make_float3(intersection.normal.x, intersection.normal.y, intersection.normal.z);
    global.I = make_float3(intersection.view.x, intersection.view.y, intersection.view.z);
    global.gnormal = make_float3(intersection.gnormal.x, intersection.gnormal.y, intersection.gnormal.z);
    global.position = make_float3(intersection.intersect.x, intersection.intersect.y, intersection.intersect.z);

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