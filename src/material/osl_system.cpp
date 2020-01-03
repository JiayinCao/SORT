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

#include <OSL/oslcomp.h>
#include <algorithm>
#include <vector>
#include "osl_system.h"
#include "texture_system.h"
#include "core/profile.h"
#include "core/thread.h"
#include "core/globalconfig.h"
#include "math/intersection.h"
#include "scatteringevent/scatteringevent.h"

using namespace OSL;

class SORTRenderServices : public OSL::RendererServices{
public:
    SORTRenderServices( OIIO::TextureSystem* ts ) : OSL::RendererServices(ts){}
    int supports (OSL::string_view feature) const override { return 0; }
    bool get_matrix (OSL::ShaderGlobals *sg, OSL::Matrix44 &result, OSL::TransformationPtr xform, float time)override{ return true; }
    bool get_matrix (OSL::ShaderGlobals *sg, OSL::Matrix44 &result, OSL::ustring from, float time)override{ return true; }
    bool get_matrix (OSL::ShaderGlobals *sg, OSL::Matrix44 &result, OSL::TransformationPtr xform)override{ return true; }
    bool get_matrix (OSL::ShaderGlobals *sg, OSL::Matrix44 &result, OSL::ustring from)override{ return true; }
    bool get_inverse_matrix (OSL::ShaderGlobals *sg, OSL::Matrix44 &result, OSL::ustring to, float time)override{ return true; }
    bool get_array_attribute (OSL::ShaderGlobals *sg, bool derivatives, OSL::ustring object, OSL::TypeDesc type, OSL::ustring name, int index, void *val )override{ return true; }
    bool get_attribute (OSL::ShaderGlobals *sg, bool derivatives, OSL::ustring object, OSL::TypeDesc type, OSL::ustring name, void *val)override{ return true; }
    bool get_userdata (bool derivatives, OSL::ustring name, OSL::TypeDesc type, OSL::ShaderGlobals *sg, void *val)override{ return true;}
};

static std::unique_ptr<OSL::ShadingSystem>  g_shadingsys = nullptr;
static SORTTextureSystem                    g_textureSystem;
static ErrorHandler                         g_errhandler;
static SORTRenderServices                   g_rendererSystem(&g_textureSystem);
static std::vector<ShadingContext*>         g_contexts;
static std::vector<ShadingContextWrapper>   g_shadingContexts;

std::unique_ptr<ShadingSystem>  MakeShadingSystem() {
    return std::move(std::make_unique<ShadingSystem>(&g_rendererSystem, &g_textureSystem, &g_errhandler));
}

bool BuildShader(const std::string& shader_source, const std::string& shader_name, const std::string& shader_layer, const std::string& shader_group_name) {
    // Compile a OSL shader.
    std::string osobuffer;
    {
        OSL::OSLCompiler compiler;
        std::vector<std::string> options;
        const auto message = "Compile shader layer '" + shader_layer + "'";
        SORT_PROFILE(message);
        if (!compiler.compile_buffer(shader_source, osobuffer, options, STDOSL_PATH))
            return false;
    }

    // Load shader from compiled object file.
    {
        const auto message = "Loading shader obj file (" + shader_layer + ")";
        SORT_PROFILE(message);
        if (!g_shadingsys->LoadMemoryCompiledShader(shader_name, osobuffer))
            return false;
    }

    return g_shadingsys->Shader("surface", shader_name, shader_layer);
};

bool ConnectShader(const std::string& source_shader, const std::string& source_param, const std::string& target_shader, const std::string& target_param) {
    return g_shadingsys->ConnectShaders(source_shader, source_param, target_shader, target_param);
}

ShaderGroupRef BeginShaderGroup(const std::string& group_name) {
    return g_shadingsys->ShaderGroupBegin(group_name);
}

bool EndShaderGroup() {
    return g_shadingsys->ShaderGroupEnd();
}

void OptimizeShader(OSL::ShaderGroup* group) {
    g_shadingsys->optimize_group(group);
}

void ExecuteShader( OSL::ShaderGroup* shader , ScatteringEvent& se ){
    const Intersection& intersection = se.GetIntersection();

    ShaderGlobals shaderglobals;
    shaderglobals.P = Vec3( intersection.intersect.x , intersection.intersect.y , intersection.intersect.z );
    shaderglobals.u = intersection.u;
    shaderglobals.v = intersection.v;
    shaderglobals.N = Vec3( intersection.normal.x , intersection.normal.y , intersection.normal.z );
    shaderglobals.Ng = Vec3( intersection.gnormal.x , intersection.gnormal.y , intersection.gnormal.z );
    shaderglobals.I = Vec3( intersection.view.x , intersection.view.y , intersection.view.z );
    shaderglobals.dPdu = Vec3( 0.0f );
    shaderglobals.dPdu = Vec3( 0.0f );
    g_shadingsys->execute(g_contexts[ ThreadId() ], *shader, shaderglobals);

    ProcessClosure( shaderglobals.Ci , Color3( 1.0f ) , se );
}

Spectrum EvaluateTransparency( OSL::ShaderGroup* shader , const Intersection& intersection ){
    ShaderGlobals shaderglobals;
    shaderglobals.P = Vec3( intersection.intersect.x , intersection.intersect.y , intersection.intersect.z );
    shaderglobals.u = intersection.u;
    shaderglobals.v = intersection.v;
    shaderglobals.N = Vec3( intersection.normal.x , intersection.normal.y , intersection.normal.z );
    shaderglobals.Ng = Vec3( intersection.gnormal.x , intersection.gnormal.y , intersection.gnormal.z );
    shaderglobals.I = Vec3( intersection.view.x , intersection.view.y , intersection.view.z );
    shaderglobals.dPdu = Vec3( 0.0f );
    shaderglobals.dPdu = Vec3( 0.0f );
    g_shadingsys->execute(g_contexts[ ThreadId() ], *shader, shaderglobals);

    return 1.0f - ProcessOcclusion( shaderglobals.Ci , Color3( 1.0f ) );
}

void ShadingContextWrapper::DestroyContext(OSL::ShadingSystem* shadingsys) {
    shadingsys->release_context(ctx);
    shadingsys->destroy_thread_info(thread_info);
}

OSL::ShadingContext* ShadingContextWrapper::GetShadingContext(OSL::ShadingSystem* shadingsys){
    thread_info = shadingsys->create_thread_info();
    return ctx = shadingsys->get_context(thread_info);
}

void CreateOSLThreadContexts(){
    g_shadingsys = MakeShadingSystem();
    RegisterClosures(g_shadingsys.get());

    g_contexts.resize( g_threadCnt );
    g_shadingContexts.resize( g_threadCnt );

    for( auto i = 0u ; i < g_threadCnt ; ++i )
        g_contexts[i] = g_shadingContexts[i].GetShadingContext( g_shadingsys.get() );
}

void DestroyOSLThreadContexts(){
    for( auto i = 0u ; i < g_threadCnt ; ++i ){
        g_contexts[i] = nullptr;
        g_shadingContexts[i].DestroyContext( g_shadingsys.get() );
    }
}