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

#include <OSL/oslcomp.h>
#include <OSL/oslquery.h>
#include <OSL/genclosure.h>
#include <OSL/oslclosure.h>
#include <algorithm>
#include "osl_system.h"
#include "spectrum/spectrum.h"
#include "core/memory.h"
#include "spectrum/spectrum.h"
#include "math/intersection.h"
#include "bsdf/bsdf.h"
#include "bsdf/lambert.h"
#include "bsdf/orennayar.h"
#include "bsdf/disney.h"
#include "bsdf/microfacet.h"

using namespace OSL;

class SORTRenderServices : public OSL::RendererServices{
public:
    SORTRenderServices(){}
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

std::unique_ptr<OSL::ShadingSystem>     g_shadingsys = nullptr;

// Register closures
void register_closures(ShadingSystem* shadingsys){
    // Register a closure
    constexpr int MaxParams = 32;
    struct BuiltinClosures {
        const char* name;
        int id;
        ClosureParam params[MaxParams];
    };
    
    constexpr int CC = 4; // Closure count
    BuiltinClosures closures[CC] = {
        { "lambert" , LAMBERT_ID, { 
            CLOSURE_COLOR_PARAM(Lambert::Params, baseColor),
            CLOSURE_VECTOR_PARAM(Lambert::Params, n),
            CLOSURE_FINISH_PARAM(Lambert::Params) }},
        { "orenNayar" , OREN_NAYAR_ID, { 
            CLOSURE_COLOR_PARAM(OrenNayar::Params, baseColor),
            CLOSURE_FLOAT_PARAM (OrenNayar::Params, sigma),
            CLOSURE_VECTOR_PARAM(OrenNayar::Params, n),
            CLOSURE_FINISH_PARAM(OrenNayar::Params) }},
        { "disney" , DISNEY_ID, {
            CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, subsurface),
            CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, metallic),
            CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, specular),
            CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, specularTint),
            CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, roughness),
            CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, anisotropic),
            CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, sheen),
            CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, sheenTint),
            CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, clearcoat),
            CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, clearcoatGloss),
            CLOSURE_COLOR_PARAM(DisneyBRDF::Params, baseColor),
            CLOSURE_VECTOR_PARAM(DisneyBRDF::Params, n),
            CLOSURE_FINISH_PARAM(DisneyBRDF::Params) } },
        { "microfacetReflection" , MICROFACET_REFLECTION_ID, {
            CLOSURE_STRING_PARAM(MicroFacetReflection::Params, dist),
            CLOSURE_COLOR_PARAM(MicroFacetReflection::Params, eta),
            CLOSURE_COLOR_PARAM(MicroFacetReflection::Params, absorption),
            CLOSURE_FLOAT_PARAM(MicroFacetReflection::Params, roughnessU),
            CLOSURE_FLOAT_PARAM(MicroFacetReflection::Params, roughnessV),
            CLOSURE_COLOR_PARAM(MicroFacetReflection::Params, baseColor),
            CLOSURE_VECTOR_PARAM(MicroFacetReflection::Params, n),
            CLOSURE_FINISH_PARAM(MicroFacetReflection::Params) } }
    };
    for( int i = 0 ; i < CC ; ++i )
        shadingsys->register_closure( closures[i].name , closures[i].id , closures[i].params , nullptr , nullptr );
}

// Create shading system
std::unique_ptr<ShadingSystem>  MakeShadingSystem(){
    static ErrorHandler errhandler;
    static SORTRenderServices rs;
    return std::move( std::make_unique<ShadingSystem>(&rs, nullptr, &errhandler) );
}

// Compile OSL source code
bool compile_buffer ( const std::string &sourcecode, const std::string &shadername ){    
    // Compile a OSL shader.
    OSL::OSLCompiler compiler;
    std::string osobuffer;
    std::vector<std::string> options;
    if (!compiler.compile_buffer(sourcecode, osobuffer, options, STDOSL_PATH))
        return false;

    // Load shader from compiled object file.
    if (! g_shadingsys->LoadMemoryCompiledShader (shadername, osobuffer))
        return false;
    
    return true;
}

// Build a shader from source code
bool build_shader( const std::string& shader_source, const std::string& shader_name, const std::string& shader_layer , const std::string& shader_group_name ) {
    auto ret = compile_buffer(shader_source, shader_name);
    g_shadingsys->Shader("surface", shader_name, shader_layer);
    return ret;
};

// begin building shader
ShaderGroupRef beginShaderGroup( const std::string& group_name ){
    if( !g_shadingsys ){
        g_shadingsys = MakeShadingSystem();
        register_closures(g_shadingsys.get());
    }
    return g_shadingsys->ShaderGroupBegin( group_name );
}
bool endShaderGroup(){
    return g_shadingsys->ShaderGroupEnd();
}

bool connect_shader( const std::string& source_shader , const std::string& source_param , const std::string& target_shader , const std::string& target_param ){
    return g_shadingsys->ConnectShaders( source_shader , source_param , target_shader , target_param );
}

void process_closure (Bsdf* bsdf, const ClosureColor* closure, const Color3& w) {
   if (!closure)
       return;
   switch (closure->id) {
       case ClosureColor::MUL: {
           Color3 cw = w * closure->as_mul()->weight;
           process_closure(bsdf, closure->as_mul()->closure, cw);
           break;
       }
       case ClosureColor::ADD: {
           process_closure(bsdf, closure->as_add()->closureA, w);
           process_closure(bsdf, closure->as_add()->closureB, w);
           break;
       }
       default: {
            const ClosureComponent* comp = closure->as_comp();
            Color3 cw = w * comp->w;
            Spectrum weight( 1.0f );
            Vector n = Vector( 0.0f , 1.0f , 0.0f );
            switch (comp->id) {
                case LAMBERT_ID:
                    {
                        const auto& params = *comp->as<Lambert::Params>();
                        bsdf->AddBxdf(SORT_MALLOC(Lambert)(params, weight));
                    }
                    break;
                case OREN_NAYAR_ID:
                    {   
                        const auto& params = *comp->as<OrenNayar::Params>();
                        bsdf->AddBxdf( SORT_MALLOC(OrenNayar)(params, weight) );
                    }
                    break;
                case DISNEY_ID:
                    {
                        const auto& params = *comp->as<DisneyBRDF::Params>();
                        bsdf->AddBxdf(SORT_MALLOC(DisneyBRDF)(params, weight));
                    }
                    break;
                case MICROFACET_REFLECTION_ID:
                    {
                        const auto& params = *comp->as<MicroFacetReflection::Params>();
                        const auto frenel = SORT_MALLOC( FresnelConductor )(params.eta, params.absorption);
                        bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, frenel, weight));
                    }
                    break;
            }
       }
   }
}

void execute_shader( Bsdf* bsdf , const Intersection* intersection , OSL::ShaderGroup* shader ){
    auto thread_info = g_shadingsys->create_thread_info();
    auto ctx = g_shadingsys->get_context(thread_info);

    ShaderGlobals shaderglobals;
    shaderglobals.u = intersection->u;
    shaderglobals.v = intersection->v;
    shaderglobals.N = Vec3(0.0f, 1.0f, 0.0f);
    g_shadingsys->execute(ctx, *shader, shaderglobals);

    process_closure( bsdf , shaderglobals.Ci , Color3( 1.0f ) );

    g_shadingsys->release_context (ctx);
    g_shadingsys->destroy_thread_info(thread_info);
}