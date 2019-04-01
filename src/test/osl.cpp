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

#include <OSL/oslexec.h>
#include <OSL/oslcomp.h>
#include <OSL/oslquery.h>
#include <OSL/genclosure.h>
#include <OSL/oslclosure.h>
#include "thirdparty/gtest/gtest.h"

#include "core/define.h"

using namespace OSL;

#define OSL_TEST_VERBOSE    0

#if defined(SORT_IN_WINDOWS)
    #define STDOSL_PATH     "..\\dependencies\\osl\\shader\\stdosl.h"
#else
    #define STDOSL_PATH     "../dependencies/osl/shader/stdosl.h"
#endif

class SORTRenderServices : public RendererServices{
public:
    SORTRenderServices(){}
    virtual int supports (string_view feature) const{ return 0; }
    virtual bool get_matrix (ShaderGlobals *sg, Matrix44 &result,
                             TransformationPtr xform,
                             float time){ return true; }
    virtual bool get_matrix (ShaderGlobals *sg, Matrix44 &result,
                             ustring from, float time){ return true; }
    virtual bool get_matrix (ShaderGlobals *sg, Matrix44 &result,
                             TransformationPtr xform){ return true; }
    virtual bool get_matrix (ShaderGlobals *sg, Matrix44 &result,
                             ustring from){ return true; }
    virtual bool get_inverse_matrix (ShaderGlobals *sg, Matrix44 &result,
                                     ustring to, float time) { return true; }

    virtual bool get_array_attribute (ShaderGlobals *sg, bool derivatives, 
                                      ustring object, TypeDesc type, ustring name,
                                      int index, void *val ){ return true; }
    virtual bool get_attribute (ShaderGlobals *sg, bool derivatives, ustring object,
                                TypeDesc type, ustring name, void *val){ return true; }
    virtual bool get_userdata (bool derivatives, ustring name, TypeDesc type, 
                               ShaderGlobals *sg, void *val){ return true;}
};

// Compile OSL source code
bool compile_buffer ( ShadingSystem* shadingsys , const std::string &sourcecode, const std::string &shadername){
    // Compile a OSL shader.
    OSLCompiler compiler;
    std::string osobuffer;
    std::vector<std::string> options;
    if (!compiler.compile_buffer(sourcecode, osobuffer, options, STDOSL_PATH))
        return false;

    // Load shader from compiled object file.
    if (! shadingsys->LoadMemoryCompiledShader (shadername, osobuffer))
        return false;
    
    return true;
}

// This unit test will cover the basics of Open Shading Language, like read input and output from a shader ,
// exectute a shader and verify the result
TEST(OSL, CheckingSymbol) {
    static std::string shader_source =  "shader TestBasic("
                                        "   color Cin = 0.5 ,"
                                        "   float exponent = 2.0 ,"
                                        "   output color Cout = 1.0 )"
                                        "{"
                                        "       Cout = pow( Cin , exponent );"
                                        "}";

    ErrorHandler errhandler;
    SORTRenderServices rs;

    auto shadingsys = std::make_unique<ShadingSystem>(&rs, nullptr, &errhandler);

    // Making sure shading system is created already.
    EXPECT_TRUE( shadingsys != nullptr );
    
    // Making sure shading system is created already.
    EXPECT_TRUE( shadingsys != nullptr );

    shadingsys->attribute("lockgeom", 1);
#if OSL_TEST_VERBOSE
    errhandler.verbosity (ErrorHandler::VERBOSE);
#endif

    const auto group_name = "Basic_Test_ShaderGroup";
    const auto shader_name = "test_basic_osl";
    const auto shader_layer = "test_basic_layer";
    const auto shadergroup = shadingsys->ShaderGroupBegin ("Basic_Test_ShaderGroup");
    const auto ret = compile_buffer( shadingsys.get() , shader_source , shader_name );
    shadingsys->Shader ("surface", shader_name, shader_layer);
    shadingsys->ShaderGroupEnd ();

    // Making sure the shader is successfully compiled.
    EXPECT_TRUE( ret );

    ustring groupname;
    shadingsys->getattribute (shadergroup.get(), "groupname", groupname);
#if OSL_TEST_VERBOSE
    std::cout << "Shader group name:\t" << groupname << std::endl;
#endif
    const auto group_name_match = std::strcmp( groupname.c_str() , group_name ) == 0;
    EXPECT_TRUE( group_name_match );

    // There should be exactly one layer
    int num_layers = 0;
    shadingsys->getattribute (shadergroup.get(), "num_layers", num_layers);
    EXPECT_EQ( num_layers , 1 );

    // Checking the layer name
    const char* layers[1] = { nullptr };
    shadingsys->getattribute (shadergroup.get(), "layer_names", TypeDesc(TypeDesc::STRING, num_layers), &layers[0]);
#if OSL_TEST_VERBOSE
    std::cout<<"Layer name:\t\t"<<layers[0]<<std::endl;
#endif
    const auto layer_name_match = std::strcmp( layers[0] , shader_layer ) == 0;
    EXPECT_TRUE( layer_name_match );

#if OSL_TEST_VERBOSE
    std::cout << layers[0] << std::endl;
#endif
    OSLQuery q;
    q.init (shadergroup.get(), 0);
    for (size_t p = 0;  p < q.nparams(); ++p) {
        const OSLQuery::Parameter *param = q.getparam(p);
#if OSL_TEST_VERBOSE
        std::cout << "\t" << (param->isoutput ? "output "  : "input ") << param->type << ' ' << param->name << "\n";
#endif
    }

    static const auto verify_param = [=]( const int index , const std::string& name , const bool input ){
        const auto param = q.getparam(index);
#if OSL_TEST_VERBOSE
        std::cout << "\t" << (param->isoutput ? "output "  : "input ") << param->type << ' ' << param->name << "\n";
#endif
        const auto param_match = std::strcmp( param->name.c_str() , name.c_str() ) == 0;
        EXPECT_TRUE( param_match );
        EXPECT_EQ( param->isoutput , !input );
    };

    verify_param( 0 , "Cin" , true );
    verify_param( 1 , "exponent" , true );
    verify_param( 2 , "Cout" , false );

    // Set attribute
    const std::vector<const char *> aovnames( 1 , { "Cout" } );
    shadingsys->attribute ( nullptr, "renderer_outputs", TypeDesc(TypeDesc::STRING,1), &aovnames[0] );

    // create a new thread and the context
    OSL::PerThreadInfo *thread_info = shadingsys->create_thread_info();
    ShadingContext *ctx = shadingsys->get_context (thread_info);

    // Execute the shader
    ShaderGlobals shaderglobals;
    shadingsys->execute (ctx, *shadergroup, shaderglobals);

    // Getting back the result
    const auto sym = shadingsys->find_symbol (*shadergroup, ustring("Cout") );
    EXPECT_NE( sym , nullptr );

    TypeDesc t;
    const void *data = shadingsys->get_symbol (*ctx, ustring("Cout"), t);
    EXPECT_NE( data , nullptr );

    // It should be exactly 0.25f, there should be no error.
    const float* fdata = ( const float* ) data;
    EXPECT_EQ( fdata[0] , 0.25f );
    EXPECT_EQ( fdata[1] , 0.25f );
    EXPECT_EQ( fdata[2] , 0.25f );

    shadingsys->release_context (ctx);
    shadingsys->destroy_thread_info(thread_info);
}

// Check basic closure operations
TEST(OSL, CheckingClosure) {
    static std::string shader_source =  "shader TestBasic()"
                                        "{"
                                        "       color tint = color( 0.25 , 1.25 , 0.125 );"
                                        "       float gamma = 0.1;"
                                        "       Ci = diffuse(N) * tint + oren_nayar(N,gamma);"
                                        "}";

    ErrorHandler errhandler;
    SORTRenderServices rs;
    auto shadingsys = std::make_unique<ShadingSystem>(&rs, nullptr, &errhandler);

    // Making sure shading system is created already.
    EXPECT_TRUE( shadingsys != nullptr );

    // Register a closure
    constexpr int MaxParams = 32;
    struct BuiltinClosures {
        const char* name;
        int id;
        ClosureParam params[MaxParams];
    };
    struct DiffuseParams    { Vec3 N; };
    struct OrenNayarParams  { Vec3 N; float sigma; };
    constexpr int DIFFUSE_ID = 1;
    constexpr int OREN_NAYAR_ID = 2;

    constexpr int CC = 2; // Closure count
    BuiltinClosures closures[CC] = {
        { "diffuse"    , DIFFUSE_ID,            { CLOSURE_VECTOR_PARAM(DiffuseParams, N),
                                                  CLOSURE_FINISH_PARAM(DiffuseParams) }},
        { "oren_nayar" , OREN_NAYAR_ID,         { CLOSURE_VECTOR_PARAM(OrenNayarParams, N),
                                                  CLOSURE_FINISH_PARAM(OrenNayarParams) }}
    };
    for( int i = 0 ; i < CC ; ++i )
        shadingsys->register_closure( closures[i].name , closures[i].id , closures[i].params , nullptr , nullptr );
    
    const auto group_name = "Closure_Basic_Test_ShaderGroup";
    const auto shader_name = "closure_basic_osl";
    const auto shader_layer = "closure_basic_layer";
    const auto shadergroup = shadingsys->ShaderGroupBegin ("Closure_Test_ShaderGroup");
    const auto ret = compile_buffer( shadingsys.get() , shader_source , shader_name );
    shadingsys->Shader ("surface", shader_name, shader_layer);
    shadingsys->ShaderGroupEnd ();

    OSL::PerThreadInfo *thread_info = shadingsys->create_thread_info();
    ShadingContext *ctx = shadingsys->get_context (thread_info);

    ShaderGlobals shaderglobals;
    shadingsys->execute (ctx, *shadergroup, shaderglobals);

    const auto closure = shaderglobals.Ci;

    EXPECT_TRUE( closure != nullptr );
    EXPECT_TRUE( closure->id == ClosureColor::ADD );

    const auto closureA = closure->as_add()->closureA;
    const auto closureB = closure->as_add()->closureB;

    EXPECT_TRUE( closureA != nullptr );
    EXPECT_TRUE( closureB != nullptr );
    EXPECT_EQ( closureA->id , DIFFUSE_ID );
    EXPECT_EQ( closureB->id , OREN_NAYAR_ID );

    const auto* compA = closureA->as_comp();
    const auto cwA = compA->w;

    EXPECT_EQ( cwA.x , 0.25f );
    EXPECT_EQ( cwA.y , 1.25f );
    EXPECT_EQ( cwA.z , 0.125f );

    // It doesn't work somehow, need investigation
    //const auto* compB = closureB->as_comp();
    //const auto& params = *compB->as<OrenNayarParams>();
    //const float expected_sigam = 0.1f;
    //EXPECT_EQ( params.sigma , expected_sigam );

    shadingsys->release_context (ctx);
    shadingsys->destroy_thread_info(thread_info);
}